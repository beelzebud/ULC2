#pragma once
#include <QString>

class EtagCache {
public:
    explicit EtagCache(const QString &cacheDir);
    QString load(const QString &url) const;
    void    save(const QString &url, const QString &etag) const;
private:
    QString filePath(const QString &url) const;
    QString m_dir;
};
