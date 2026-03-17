#pragma once
#include <QString>
#include <QDateTime>
#include <functional>

// Minimal ZIP extractor using Qt's bundled zlib — no extra dependencies.
// Supports STORE and DEFLATE compression methods.

struct ZipEntry {
    QString   name;
    quint32   crc32            = 0;
    qint64    compressedSize   = 0;
    qint64    uncompressedSize = 0;
    QDateTime lastModified;
    bool      isDirectory      = false;
};

// visitor(entry, tempFile) -> true  = caller took/moved the file
//                             false = extractor will clean up tempFile
// For directory entries tempFile is empty.
using ZipVisitor = std::function<bool(const ZipEntry &, const QString &tempFile)>;

class ZipExtractor {
public:
    // Extract zipPath into tempDir, calling visitor per entry.
    // Returns number of files visitor accepted (returned true).
    // Throws std::runtime_error on fatal errors.
    static int extract(const QString    &zipPath,
                       const QString    &tempDir,
                       const ZipVisitor &visitor);

    // Quick scan of central directory for total entry count (for progress bars).
    static int countEntries(const QString &zipPath);
};
