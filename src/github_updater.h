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
    // Commit SHA the release tag points at (floating tags like "preview" are
    // retargeted on every build, so the SHA identifies the actual build).
    QString            tagSha;
    QList<GitHubAsset> assets;
    bool               valid = false;
    bool               isPreRelease = false;
};

// Friendly display form of a stored version marker. Floating nightlies stored
// as a commit SHA are shown as a short SHA plus build date, e.g.
// "cbe7951 (2026-09-08)"; every other marker passes through unchanged.
inline QString displayTagFor(const GitHubRelease& release, bool floating,
    const QString& storedTag)
{
    if (!floating || storedTag.isEmpty())
        return storedTag;

    QString date;
    if (!release.publishedAt.isEmpty())
        date = release.publishedAt.left(10);
    else if (!release.assets.isEmpty() && !release.assets.first().updatedAt.isEmpty())
        date = release.assets.first().updatedAt.left(10);

    bool isHexSha = (storedTag.size() == 40);
    for (const QChar c : storedTag) {
        const bool hex = c.isDigit() ||
            (c >= QLatin1Char('a') && c <= QLatin1Char('f')) ||
            (c >= QLatin1Char('A') && c <= QLatin1Char('F'));
        if (!hex) { isHexSha = false; break; }
    }

    if (isHexSha)
        return date.isEmpty() ? storedTag.left(7)
                              : storedTag.left(7) + " (" + date + ")";
    // Timestamp fallbacks: show just the date instead of full ISO 8601.
    return date.isEmpty() ? storedTag : date;
}

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

    // Changelog / Readme text for the tab's docs pane, as plain text ready to
    // display. Synchronous — call from the worker thread. Empty on failure.
    QString fetchChangelogText(const EmulatorConfig& config, ReleaseChannel channel);
    QString fetchReadmeText(const EmulatorConfig& config);

signals:
    void log(const QString& msg);
    void progressMax(int max);
    void progressInc();
    void done(bool updated, const QString& newTag,
        const QString& displayTag = QString());

private:
    GitHubRelease fetchFromGitHub(const EmulatorConfig& config, ReleaseChannel channel);
    GitHubRelease fetchFromBuildbot(const EmulatorConfig& config);
    GitHubRelease fetchFromRpcs3Net(const EmulatorConfig& config);
    GitHubRelease fetchFromGitea(const EmulatorConfig& config, ReleaseChannel channel);
    GitHubRelease fetchFromDirectUrl(const EmulatorConfig& config, const QString& url);
    GitHubRelease fetchFromNightlyManifest(const EmulatorConfig& config);

    bool downloadSync(const QString& url, const QString& dest, std::atomic<bool>& cancel);

    // Plain GET returning the body (empty on error). Used by the docs pane.
    QByteArray httpGetText(const QString& url, int* httpCode = nullptr,
        QString* error = nullptr, const QString& accept = QString());

    void extractAndInstall(const EmulatorConfig& config,
        const QString& archivePath,
        const QString& installPath,
        std::atomic<bool>& cancel);

    static bool atomicReplace(const QString& src, const QString& dest);

    EtagCache* m_cache;
    QNetworkAccessManager* m_nam;
};