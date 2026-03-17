#pragma once
#include <QString>
#include <functional>
#include <cstdint>

struct SevenZEntry {
    QString name;
    quint64 unpackSize        = 0;
    bool    isDirectory       = false;
    bool    hasLastWriteTime  = false;
    quint64 lastWriteTime     = 0;  // Windows FILETIME (100-ns ticks since 1601-01-01)
};

// visitor(entry, tempFile) -> true  = caller took/moved the file (or it's a dir)
//                             false = discard / already handled
using SevenZVisitor = std::function<bool(const SevenZEntry &, const QString &tempFile)>;

class SevenZExtractor {
public:
    // Extract archivePath, writing each file to a unique path in tempDir,
    // then calling visitor. progressCb(done, total) is optional.
    // Throws std::runtime_error on fatal errors.
    static void extract(const QString       &archivePath,
                        const QString       &tempDir,
                        const SevenZVisitor &visitor,
                        const std::function<void(int,int)> &progressCb = {});
};
