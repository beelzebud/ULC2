#include "archive_zip.h"

#include <QObject>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QTimeZone>
#include <QByteArray>

#include <stdexcept>
#include <cstring>
#include <zlib.h>

// ─────────────────────────────────────────────────────────────────────────────
// ZIP format structures and constants
// ─────────────────────────────────────────────────────────────────────────────
namespace {

constexpr quint32 SIG_LOCAL   = 0x04034b50u;
constexpr quint32 SIG_CENTRAL = 0x02014b50u;
constexpr quint32 SIG_EOCD    = 0x06054b50u;

constexpr quint16 METHOD_STORE   = 0;
constexpr quint16 METHOD_DEFLATE = 8;

#pragma pack(push, 1)
struct LocalFileHeader {
    quint32 sig;
    quint16 versionNeeded;
    quint16 flags;
    quint16 compression;
    quint16 modTime;
    quint16 modDate;
    quint32 crc32;
    quint32 compressedSize;
    quint32 uncompressedSize;
    quint16 fileNameLen;
    quint16 extraLen;
};

struct CentralDirEntry {
    quint32 sig;
    quint16 versionMade;
    quint16 versionNeeded;
    quint16 flags;
    quint16 compression;
    quint16 modTime;
    quint16 modDate;
    quint32 crc32;
    quint32 compressedSize;
    quint32 uncompressedSize;
    quint16 fileNameLen;
    quint16 extraLen;
    quint16 commentLen;
    quint16 diskStart;
    quint16 intAttr;
    quint32 extAttr;
    quint32 localOffset;
};

struct EndOfCentralDir {
    quint32 sig;
    quint16 diskNumber;
    quint16 startDisk;
    quint16 diskEntries;
    quint16 totalEntries;
    quint32 centralDirSize;
    quint32 centralDirOffset;
    quint16 commentLen;
};
#pragma pack(pop)

// DOS date/time -> QDateTime (UTC)
QDateTime dosToQt(quint16 date, quint16 time)
{
    const int year  = ((date >> 9) & 0x7F) + 1980;
    const int month = (date >> 5) & 0x0F;
    const int day   =  date       & 0x1F;
    const int hour  = (time >> 11) & 0x1F;
    const int min   = (time >>  5) & 0x3F;
    const int sec   = (time        & 0x1F) * 2;
    return QDateTime(QDate(year, qMax(1,month), qMax(1,day)),
                     QTime(hour, min, sec), Qt::UTC);
}

quint32 read32le(const char *p)
{
    quint32 v; memcpy(&v, p, 4); return v;
}

bool readSafe(QFile &f, void *buf, qint64 len)
{
    return f.read(static_cast<char*>(buf), len) == len;
}

// Raw deflate decompression (no zlib wrapper header)
bool inflateDeflate(QFile &src, QFile &dst, quint32 compLen)
{
    z_stream zs{};
    if (inflateInit2(&zs, -MAX_WBITS) != Z_OK) return false;

    const int CHUNK = 65536;
    QByteArray inBuf(CHUNK, Qt::Uninitialized);
    QByteArray outBuf(CHUNK, Qt::Uninitialized);

    quint32 remaining = compLen;
    int ret = Z_OK;

    while (remaining > 0 && ret != Z_STREAM_END) {
        const quint32 toRead = qMin<quint32>(CHUNK, remaining);
        const qint64  got    = src.read(inBuf.data(), toRead);
        if (got <= 0) { inflateEnd(&zs); return false; }
        remaining -= static_cast<quint32>(got);

        zs.next_in  = reinterpret_cast<Bytef*>(inBuf.data());
        zs.avail_in = static_cast<uInt>(got);

        while (zs.avail_in > 0) {
            zs.next_out  = reinterpret_cast<Bytef*>(outBuf.data());
            zs.avail_out = CHUNK;
            ret = inflate(&zs, Z_NO_FLUSH);
            if (ret == Z_NEED_DICT || ret == Z_DATA_ERROR || ret == Z_MEM_ERROR) {
                inflateEnd(&zs);
                return false;
            }
            const qint64 produced = CHUNK - zs.avail_out;
            if (dst.write(outBuf.constData(), produced) != produced) {
                inflateEnd(&zs);
                return false;
            }
        }
    }
    inflateEnd(&zs);
    return (ret == Z_STREAM_END || remaining == 0);
}

// Scan backwards for EOCD signature (handles up to 64 KiB comment)
bool findEocd(QFile &f, EndOfCentralDir &eocd)
{
    const qint64 fileSize = f.size();
    const qint64 scanFrom = qMax<qint64>(0LL,
        fileSize - 65535LL - static_cast<qint64>(sizeof(EndOfCentralDir)));

    if (!f.seek(scanFrom)) return false;
    const QByteArray tail = f.read(fileSize - scanFrom);

    for (int i = tail.size() - static_cast<int>(sizeof(EndOfCentralDir)); i >= 0; --i) {
        if (read32le(tail.constData() + i) == SIG_EOCD) {
            memcpy(&eocd, tail.constData() + i, sizeof(EndOfCentralDir));
            return true;
        }
    }
    return false;
}

} // anonymous namespace

// ─────────────────────────────────────────────────────────────────────────────
// Public API
// ─────────────────────────────────────────────────────────────────────────────

int ZipExtractor::countEntries(const QString &zipPath)
{
    QFile f(zipPath);
    if (!f.open(QIODevice::ReadOnly)) return 0;
    EndOfCentralDir eocd{};
    if (!findEocd(f, eocd)) return 0;
    return eocd.totalEntries;
}

int ZipExtractor::extract(const QString    &zipPath,
                           const QString    &tempDir,
                           const ZipVisitor &visitor)
{
    QFile f(zipPath);
    if (!f.open(QIODevice::ReadOnly))
        throw std::runtime_error("Cannot open ZIP: " + zipPath.toStdString());

    EndOfCentralDir eocd{};
    if (!findEocd(f, eocd))
        throw std::runtime_error("Not a valid ZIP file: " + zipPath.toStdString());

    if (!f.seek(eocd.centralDirOffset))
        throw std::runtime_error("Seek to central directory failed");

    int extracted = 0;

    for (int i = 0; i < eocd.totalEntries; ++i) {
        // ── Read central directory entry ──────────────────────────────────────
        CentralDirEntry cd{};
        if (!readSafe(f, &cd, sizeof(cd)) || cd.sig != SIG_CENTRAL)
            throw std::runtime_error("Central directory read error at entry " + std::to_string(i));

        const QByteArray nameBytes = f.read(cd.fileNameLen);
        f.read(cd.extraLen + cd.commentLen); // skip

        const QString entryName = QString::fromUtf8(nameBytes);
        const bool isDir = entryName.endsWith('/') || (cd.extAttr & 0x10);

        ZipEntry ze;
        ze.name             = entryName;
        ze.crc32            = cd.crc32;
        ze.compressedSize   = cd.compressedSize;
        ze.uncompressedSize = cd.uncompressedSize;
        ze.lastModified     = dosToQt(cd.modDate, cd.modTime);
        ze.isDirectory      = isDir;

        if (isDir) {
            visitor(ze, {});
            continue;
        }

        // ── Jump to local file header ─────────────────────────────────────────
        const qint64 centralPos = f.pos();

        if (!f.seek(cd.localOffset))
            throw std::runtime_error("Seek to local header failed");

        LocalFileHeader lh{};
        if (!readSafe(f, &lh, sizeof(lh)) || lh.sig != SIG_LOCAL)
            throw std::runtime_error("Local file header signature mismatch");

        f.read(lh.fileNameLen + lh.extraLen); // skip local name+extra

        // ── Extract to temp file ──────────────────────────────────────────────
        // Make a safe flat filename for the temp dir
        const QString safeName =
            QString("%1_%2").arg(i).arg(
                QString(entryName).replace('/', '_').replace('\\', '_'));
        const QString tempFile = tempDir + "/" + safeName;

        {
            QFile out(tempFile);
            if (!out.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
                f.seek(centralPos);
                continue;
            }

            if (lh.compression == METHOD_STORE) {
                qint64 remain = lh.compressedSize;
                const int CHUNK = 65536;
                QByteArray buf(CHUNK, Qt::Uninitialized);
                while (remain > 0) {
                    const qint64 rd = f.read(buf.data(), qMin<qint64>(CHUNK, remain));
                    if (rd <= 0) throw std::runtime_error("Unexpected EOF in STORE entry");
                    out.write(buf.constData(), rd);
                    remain -= rd;
                }
            } else if (lh.compression == METHOD_DEFLATE) {
                if (!inflateDeflate(f, out, lh.compressedSize))
                    throw std::runtime_error("Deflate failed: " + entryName.toStdString());
            } else {
                // Unsupported method — skip
                out.remove();
                f.seek(f.pos() + lh.compressedSize);
                f.seek(centralPos);
                continue;
            }
        }

        // ── Call visitor ──────────────────────────────────────────────────────
        if (visitor(ze, tempFile))
            ++extracted;
        else
            QFile::remove(tempFile); // visitor declined — clean up

        if (!f.seek(centralPos))
            throw std::runtime_error("Seek back to central dir failed");
    }

    return extracted;
}