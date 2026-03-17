#include "etag_cache.h"
#include <QDir>
#include <QFile>
#include <QCryptographicHash>

EtagCache::EtagCache(const QString &cacheDir) : m_dir(cacheDir)
{
    QDir().mkpath(cacheDir);
}

QString EtagCache::filePath(const QString &url) const
{
    const QByteArray hash =
        QCryptographicHash::hash(url.toUtf8(), QCryptographicHash::Sha256).toHex();
    return m_dir + "/" + QString::fromLatin1(hash) + ".etag";
}

QString EtagCache::load(const QString &url) const
{
    QFile f(filePath(url));
    if (!f.open(QIODevice::ReadOnly)) return {};
    return QString::fromUtf8(f.readAll()).trimmed();
}

void EtagCache::save(const QString &url, const QString &etag) const
{
    QFile f(filePath(url));
    if (f.open(QIODevice::WriteOnly | QIODevice::Truncate))
        f.write(etag.toUtf8());
}
