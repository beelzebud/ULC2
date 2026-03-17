#include "archive_7z.h"

// 7-zip LZMA SDK
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

// ── Allocator ─────────────────────────────────────────────────────────────────
static ISzAlloc g_Alloc = { SzAlloc, SzFree };

// ── SevenZExtractor::extract ──────────────────────────────────────────────────

void SevenZExtractor::extract(const QString       &archivePath,
                               const QString       &tempDir,
                               const SevenZVisitor &visitor,
                               const std::function<void(int,int)> &progressCb)
{
    // Initialise CRC table (safe to call multiple times)
    CrcGenerateTable();

    // ── Open archive ──────────────────────────────────────────────────────────
    CFileInStream archiveStream;
    CLookToRead2  lookStream;
    Byte          lookBuf[1 << 16];

#ifdef Q_OS_WIN
    if (InFile_OpenW(&archiveStream.file,
                     archivePath.toStdWString().c_str()) != 0)
        throw std::runtime_error("Cannot open 7z archive: " + archivePath.toStdString());
#else
    if (InFile_Open(&archiveStream.file,
                    archivePath.toLocal8Bit().constData()) != 0)
        throw std::runtime_error("Cannot open 7z archive: " + archivePath.toStdString());
#endif

    FileInStream_CreateVTable(&archiveStream);
    LookToRead2_CreateVTable(&lookStream, False);
    lookStream.buf      = lookBuf;
    lookStream.bufSize  = sizeof(lookBuf);
    lookStream.realStream = &archiveStream.vt;
    // Newer LZMA SDK versions use the macro; older versions have the function.
    // The macro sets pos=0 and size=0 on the stream.
    lookStream.pos  = 0;
    lookStream.size = 0;

    // ── Open DB ───────────────────────────────────────────────────────────────
    CSzArEx db;
    SzArEx_Init(&db);

    SRes res = SzArEx_Open(&db, &lookStream.vt, &g_Alloc, &g_Alloc);
    if (res != SZ_OK) {
        File_Close(&archiveStream.file);
        throw std::runtime_error("SzArEx_Open failed (SRes=" + std::to_string(res) + ")");
    }

    QDir().mkpath(tempDir);

    const int total = static_cast<int>(db.NumFiles);
    int       done  = 0;

    // Buffer reused across solid-block extractions (SDK manages it)
    UInt32 blockIndex    = 0xFFFFFFFFu;
    Byte  *outBuffer     = nullptr;
    size_t outBufferSize = 0;

    for (UInt32 i = 0; i < db.NumFiles; ++i) {
        SevenZEntry entry;
        entry.isDirectory = (SzArEx_IsDir(&db, i) != 0);

        // ── File name (UTF-16 from SDK) ───────────────────────────────────────
        const size_t nameLen16 = SzArEx_GetFileNameUtf16(&db, i, nullptr);
        std::vector<UInt16> nameBuf(nameLen16);
        SzArEx_GetFileNameUtf16(&db, i, nameBuf.data());

        // nameLen16 includes null terminator
        entry.name = QString::fromUtf16(
            reinterpret_cast<const char16_t*>(nameBuf.data()),
            static_cast<qsizetype>(nameLen16) - 1);
        entry.name.replace('\\', '/');

        entry.unpackSize = SzArEx_GetFileSize(&db, i);

        if (SzBitWithVals_Check(&db.MTime, i)) {
            entry.hasLastWriteTime = true;
            entry.lastWriteTime =
                static_cast<quint64>(db.MTime.Vals[i].Low) |
                (static_cast<quint64>(db.MTime.Vals[i].High) << 32);
        }

        // ── Extract data ──────────────────────────────────────────────────────
        QString tempFile;

        if (!entry.isDirectory && entry.unpackSize > 0) {
            size_t offset            = 0;
            size_t outSizeProcessed  = 0;

            res = SzArEx_Extract(&db, &lookStream.vt, i,
                                 &blockIndex, &outBuffer, &outBufferSize,
                                 &offset, &outSizeProcessed,
                                 &g_Alloc, &g_Alloc);

            if (res == SZ_OK && outSizeProcessed > 0) {
                // Flatten the name for a safe temp filename
                const QString safeName =
                    QString("%1_%2").arg(i).arg(
                        QString(entry.name).replace('/', '_').replace('\\', '_'));
                tempFile = tempDir + "/" + safeName;

                QFile out(tempFile);
                if (out.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
                    out.write(reinterpret_cast<const char*>(outBuffer + offset),
                              static_cast<qint64>(outSizeProcessed));
                } else {
                    tempFile.clear(); // signal failure to visitor
                }
            }
            // On extraction error we still call visitor with empty tempFile
            // so it can log/count if needed.
        }

        visitor(entry, tempFile);

        ++done;
        if (progressCb) progressCb(done, total);
    }

    IAlloc_Free(&g_Alloc, outBuffer);
    SzArEx_Free(&db, &g_Alloc);
    File_Close(&archiveStream.file);
}
