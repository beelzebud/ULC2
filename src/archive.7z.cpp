#include "archive_7z.h"

// 7-zip LZMA SDK headers
#include "7z.h"
#include "7zAlloc.h"
#include "7zBuf.h"
#include "7zCrc.h"
#include "7zFile.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <stdexcept>
#include <vector>
#include <string>
#include <cstring>

// ── Helpers ───────────────────────────────────────────────────────────────────

// Convert UTF-16LE (as stored by 7-zip SDK) to QString
static QString fromUtf16LE(const Byte *p, size_t len16)
{
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
    return QString::fromUtf16(reinterpret_cast<const char16_t*>(p), static_cast<int>(len16));
#else
    // Byte-swap on big-endian
    std::vector<char16_t> buf(len16);
    for (size_t i = 0; i < len16; ++i)
        buf[i] = static_cast<char16_t>(p[i*2] | (p[i*2+1] << 8));
    return QString::fromUtf16(buf.data(), static_cast<int>(len16));
#endif
}

// The LZMA SDK uses C-style allocators
static ISzAlloc g_alloc = { SzAlloc, SzFree };

// ── SevenZExtractor::extract ──────────────────────────────────────────────────

void SevenZExtractor::extract(const QString         &archivePath,
                               const QString         &tempDir,
                               const SevenZVisitor   &visitor,
                               const std::function<void(int,int)> &progressCb)
{
    // One-time CRC table init (idempotent)
    CrcGenerateTable();

    // Open archive file
    CFileInStream archiveStream;
    CSzArEx db;
    CLookToRead2 lookStream;
    Byte      lookBuf[1 << 16];

#ifdef Q_OS_WIN
    // The SDK's InFile_OpenW accepts a wchar_t path
    if (InFile_OpenW(&archiveStream.file, archivePath.toStdWString().c_str()) != 0)
        throw std::runtime_error("Cannot open 7z archive: " + archivePath.toStdString());
#else
    if (InFile_Open(&archiveStream.file, archivePath.toLocal8Bit().constData()) != 0)
        throw std::runtime_error("Cannot open 7z archive: " + archivePath.toStdString());
#endif

    FileInStream_CreateVTable(&archiveStream);
    LookToRead2_CreateVTable(&lookStream, False);
    lookStream.buf     = lookBuf;
    lookStream.bufSize = sizeof(lookBuf);
    lookStream.realStream = &archiveStream.vt;
    LookToRead2_Init(&lookStream);

    SzArEx_Init(&db);

    SRes res = SzArEx_Open(&db, &lookStream.vt, &g_alloc, &g_alloc);
    if (res != SZ_OK) {
        File_Close(&archiveStream.file);
        throw std::runtime_error("SzArEx_Open failed: " + std::to_string(res));
    }

    QDir().mkpath(tempDir);

    const int total = static_cast<int>(db.NumFiles);
    int       done  = 0;

    // Buffer reused across folder extractions
    UInt32    blockIndex  = 0xFFFFFFFFu;
    Byte     *outBuffer   = nullptr;
    size_t    outBufferSize = 0;

    for (UInt32 i = 0; i < db.NumFiles; ++i) {
        // ── Get entry metadata ────────────────────────────────────────────────
        SevenZEntry entry{};
        entry.isDirectory = SzArEx_IsDir(&db, i);

        // File name (UTF-16LE in SDK, returned as sequence of UInt16)
        const size_t nameLen = SzArEx_GetFileNameUtf16(&db, i, nullptr);
        std::vector<UInt16> nameBuf(nameLen);
        SzArEx_GetFileNameUtf16(&db, i, nameBuf.data());
        // nameLen includes the null terminator; convert without it
        entry.name = QString::fromUtf16(
            reinterpret_cast<const char16_t*>(nameBuf.data()),
            static_cast<int>(nameLen) - 1);
        // Normalise path separators
        entry.name.replace('\\', '/');

        entry.unpackSize = SzArEx_GetFileSize(&db, i);

        if (SzBitWithVals_Check(&db.MTime, i)) {
            entry.hasLastWriteTime = true;
            entry.lastWriteTime    = db.MTime.Vals[i].Low |
                                     (static_cast<quint64>(db.MTime.Vals[i].High) << 32);
        }

        // ── Extract file data ─────────────────────────────────────────────────
        QString tempFile;

        if (!entry.isDirectory && entry.unpackSize > 0) {
            size_t   offset    = 0;
            size_t   outSizeProcessed = 0;

            res = SzArEx_Extract(&db, &lookStream.vt, i,
                                 &blockIndex, &outBuffer, &outBufferSize,
                                 &offset, &outSizeProcessed,
                                 &g_alloc, &g_alloc);

            if (res != SZ_OK) {
                // Log but continue with remaining files
                ++done;
                if (progressCb) progressCb(done, total);
                continue;
            }

            // Write to unique temp file
            const QString safeName = QString(entry.name).replace('/', '_').replace('\\', '_');
            tempFile = tempDir + "/" + QString::number(i) + "_" + safeName;
            QDir().mkpath(QFileInfo(tempFile).absolutePath());

            QFile out(tempFile);
            if (!out.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
                ++done;
                if (progressCb) progressCb(done, total);
                continue;
            }
            out.write(reinterpret_cast<const char*>(outBuffer + offset),
                      static_cast<qint64>(outSizeProcessed));
            out.close();
        }

        // ── Call visitor ──────────────────────────────────────────────────────
        visitor(entry, tempFile);

        ++done;
        if (progressCb) progressCb(done, total);
    }

    IAlloc_Free(&g_alloc, outBuffer);
    SzArEx_Free(&db, &g_alloc);
    File_Close(&archiveStream.file);
}