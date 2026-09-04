#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QString>
#include <QList>
#include <atomic>
#include "emulator_config.h"
#include "etag_cache.h"

struct GitHubAsset {
    QString name;
    QString downloadUrl;
    QString updatedAt;
    qint64  size = 0;
};

struct GitHubRelease {
    QString            tagName;
    QString            publishedAt;
    QList<GitHubAsset> assets;
    bool               valid = false;
    bool               isPreRelease = false;
};

class GitHubUpdater : public QObject
{
    Q_OBJECT
public:
    explicit GitHubUpdater(EtagCache* cache, QObject* parent = nullptr);

    void update(const EmulatorConfig& config,
        const QString& installPath,
        const QString& knownTag,
        ReleaseChannel         channel,
        std::atomic<bool>& cancel);

    GitHubRelease fetchLatestRelease(const EmulatorConfig& config,
        ReleaseChannel        channel);

signals:
    void log(const QString& msg);
    void progressMax(int max);
    void progressInc();
    void done(bool updated, const QString& newTag);

private:
    GitHubRelease fetchFromGitHub(const EmulatorConfig& config, ReleaseChannel channel);
    GitHubRelease fetchFromBuildbot(const EmulatorConfig& config);
    GitHubRelease fetchFromRpcs3Net(const EmulatorConfig& config);
    GitHubRelease fetchFromGitea(const EmulatorConfig& config, ReleaseChannel channel);
    GitHubRelease fetchFromDirectUrl(const EmulatorConfig& config, const QString& url);
    GitHubRelease fetchFromNightlyManifest(const EmulatorConfig& config);

    bool downloadSync(const QString& url, const QString& dest, std::atomic<bool>& cancel);

    void extractAndInstall(const EmulatorConfig& config,
        const QString& archivePath,
        const QString& installPath,
        std::atomic<bool>& cancel);

    static bool atomicReplace(const QString& src, const QString& dest);

    EtagCache* m_cache;
    QNetworkAccessManager* m_nam;
};