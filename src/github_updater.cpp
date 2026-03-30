#include "github_updater.h"
#include "downloader.h"
#include "archive_zip.h"
#include "archive_7z.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QVariant>
#include <QEventLoop>
#include <QTimer>
#include <QTemporaryDir>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>

GitHubUpdater::GitHubUpdater(EtagCache* cache, QObject* parent)
    : QObject(parent)
    , m_cache(cache)
    , m_nam(new QNetworkAccessManager(this))
{
}

// ─────────────────────────────────────────────────────────────────────────────
// Public: fetch release — routes to the correct backend
// ─────────────────────────────────────────────────────────────────────────────

GitHubRelease GitHubUpdater::fetchLatestRelease(const EmulatorConfig& config,
    ReleaseChannel        channel)
{
    switch (config.source) {
    case UpdateSource::GitHub:
        return fetchFromGitHub(config, channel);
    case UpdateSource::DolphinBuildbot:
        return fetchFromBuildbot(config);
    case UpdateSource::Rpcs3Net:
        return fetchFromRpcs3Net(config);
    case UpdateSource::Gitea:
        return fetchFromGitea(config);
    }
    return {};
}

// ─────────────────────────────────────────────────────────────────────────────
// GitHub Releases backend
// ─────────────────────────────────────────────────────────────────────────────

GitHubRelease GitHubUpdater::fetchFromGitHub(const EmulatorConfig& config,
    ReleaseChannel        channel)
{
    GitHubRelease result;

    const QString endpoint =
        (channel == ReleaseChannel::Stable)
        ? "https://api.github.com/repos/" + config.githubRepo + "/releases/latest"
        : "https://api.github.com/repos/" + config.githubRepo + "/releases?per_page=10";

    QNetworkRequest req;
    req.setUrl(QUrl(endpoint));
    req.setRawHeader("User-Agent", "ulc-emulator-updater/1.0");
    req.setRawHeader("Accept", "application/vnd.github+json");
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
        QVariant::fromValue(QNetworkRequest::NoLessSafeRedirectPolicy));

    QEventLoop loop;
    QNetworkReply* reply = m_nam->get(req);
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QTimer::singleShot(10000, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        emit log("GitHub API error for " + config.githubRepo +
            ": " + reply->errorString());
        reply->deleteLater();
        return result;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    reply->deleteLater();

    auto parseRelease = [&](const QJsonObject& obj) {
        result.tagName = obj.value("tag_name").toString();
        result.isPreRelease = obj.value("prerelease").toBool();
        for (const auto& val : obj.value("assets").toArray()) {
            const auto a = val.toObject();
            GitHubAsset asset;
            asset.name = a.value("name").toString();
            asset.downloadUrl = a.value("browser_download_url").toString();
            asset.size = a.value("size").toInteger();
            result.assets.append(asset);
        }
        result.valid = !result.tagName.isEmpty();
        };

    if (channel == ReleaseChannel::Stable) {
        if (doc.isObject()) parseRelease(doc.object());
    }
    else {
        if (doc.isArray()) {
            const QJsonArray arr = doc.array();
            QJsonObject best;
            for (const auto& val : arr) {
                const auto obj = val.toObject();
                if (obj.value("prerelease").toBool()) { best = obj; break; }
            }
            if (best.isEmpty() && !arr.isEmpty())
                best = arr.first().toObject();
            if (!best.isEmpty()) parseRelease(best);
        }
    }

    return result;
}

// ─────────────────────────────────────────────────────────────────────────────
// Dolphin-style buildbot JSON backend
//
// The Dolphin API returns an array like:
// [
//   {
//     "url":     "https://dl.dolphin-emu.org/builds/win/x64/dolphin-master-5.0-21264-x64.7z",
//     "version": "5.0-21264",
//     "shortrev": "abc1234",
//     ...
//   },
//   ...
// ]
// First entry is always the most recent build.
// We use version as the tag and url as the single asset download.
// ─────────────────────────────────────────────────────────────────────────────

GitHubRelease GitHubUpdater::fetchFromBuildbot(const EmulatorConfig& config)
{
    GitHubRelease result;

    QNetworkRequest req;
    req.setUrl(QUrl(config.buildbotApiUrl));
    req.setRawHeader("User-Agent", "ulc-emulator-updater/1.0");
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
        QVariant::fromValue(QNetworkRequest::NoLessSafeRedirectPolicy));

    QEventLoop loop;
    QNetworkReply* reply = m_nam->get(req);
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QTimer::singleShot(10000, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        emit log("Buildbot fetch error for " + config.displayName +
            ": " + reply->errorString());
        reply->deleteLater();
        return result;
    }

    const QString html = QString::fromUtf8(reply->readAll());
    reply->deleteLater();

    // Find the first Windows x64 .7z build link on the page.
    // Links look like:
    // https://dl.dolphin-emu.org/builds/a5/59/dolphin-master-2603-78-x64.7z
    const QRegularExpression urlRx(
        R"((https://dl\.dolphin-emu\.org/builds/\w+/\w+/(dolphin-master-[\d]+-[\d]+-x64\.7z)))",
        QRegularExpression::CaseInsensitiveOption);

    const auto match = urlRx.match(html);
    if (!match.hasMatch()) {
        emit log("Could not find a download link in Dolphin buildbot page.");
        return result;
    }

    const QString downloadUrl = match.captured(1);
    const QString filename = match.captured(2);

    // Extract version from filename: dolphin-master-2603-78-x64.7z -> 2603-78
    const QRegularExpression verRx(R"(dolphin-master-([\d]+-[\d]+)-x64\.7z)");
    const auto verMatch = verRx.match(filename);

    result.tagName = verMatch.hasMatch() ? verMatch.captured(1) : filename;
    result.isPreRelease = true;

    GitHubAsset asset;
    asset.name = filename;
    asset.downloadUrl = downloadUrl;
    result.assets.append(asset);
    result.valid = true;

    return result;
}
GitHubRelease GitHubUpdater::fetchFromRpcs3Net(const EmulatorConfig& config)
{
    GitHubRelease result;

    QNetworkRequest req;
    req.setUrl(QUrl(config.buildbotApiUrl));
    req.setRawHeader("User-Agent", "ulc-emulator-updater/1.0");
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
        QVariant::fromValue(QNetworkRequest::NoLessSafeRedirectPolicy));

    QEventLoop loop;
    QNetworkReply* reply = m_nam->get(req);
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QTimer::singleShot(10000, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        emit log("RPCS3 update API error: " + reply->errorString());
        reply->deleteLater();
        return result;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    reply->deleteLater();

    if (!doc.isObject()) {
        emit log("RPCS3 update API: unexpected response format.");
        return result;
    }

    const QJsonObject root = doc.object();
    const int returnCode = root.value("return_code").toInt(-99);

    // return_code:
    //  0  = update available
    //  1  = already up to date
    // -1  = unknown commit (our dummy hash) — still contains valid latest_build
    // -2+ = real server error
    if (returnCode < -1) {
        emit log(QString("RPCS3 update API returned server error: %1").arg(returnCode));
        return result;
    }

    const QJsonObject latest = root.value("latest_build").toObject();
    const QString version = latest.value("version").toString();
    const QString datetime = latest.value("datetime").toString();
    const QString downloadUrl = latest.value("windows").toObject()
        .value("download").toString();

    if (version.isEmpty() || downloadUrl.isEmpty()) {
        emit log("RPCS3 update API: missing version or download URL.");
        return result;
    }

    result.tagName = QString("%1 (%2)").arg(version, datetime);
    result.isPreRelease = true;

    GitHubAsset asset;
    asset.name = QFileInfo(QUrl(downloadUrl).path()).fileName();
    asset.downloadUrl = downloadUrl;
    result.assets.append(asset);
    result.valid = true;

    return result;
}
GitHubRelease GitHubUpdater::fetchFromGitea(const EmulatorConfig& config)
{
    GitHubRelease result;

    // config.buildbotApiUrl holds the full Gitea API endpoint
    QNetworkRequest req;
    req.setUrl(QUrl(config.buildbotApiUrl));
    req.setRawHeader("User-Agent", "ulc-emulator-updater/1.0");
    req.setRawHeader("Accept", "application/json");
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
        QVariant::fromValue(QNetworkRequest::NoLessSafeRedirectPolicy));

    QEventLoop loop;
    QNetworkReply* reply = m_nam->get(req);
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QTimer::singleShot(10000, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        emit log("Gitea API error for " + config.displayName +
            ": " + reply->errorString());
        reply->deleteLater();
        return result;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    reply->deleteLater();

    if (!doc.isObject()) {
        emit log("Gitea API: unexpected response for " + config.displayName);
        return result;
    }

    // Gitea /releases/latest returns the same shape as GitHub:
    // { "tag_name": "v0.2.0-rc2", "prerelease": false,
    //   "assets": [ { "name": "...", "browser_download_url": "..." } ] }
    const QJsonObject root = doc.object();
    result.tagName = root.value("tag_name").toString();
    result.isPreRelease = root.value("prerelease").toBool();

    for (const auto& val : root.value("assets").toArray()) {
        const auto a = val.toObject();
        GitHubAsset asset;
        asset.name = a.value("name").toString();
        asset.downloadUrl = a.value("browser_download_url").toString();
        asset.size = a.value("size").toInteger();
        result.assets.append(asset);
    }

    result.valid = !result.tagName.isEmpty();
    return result;
}
// ─────────────────────────────────────────────────────────────────────────────
// Main update entry point
// ─────────────────────────────────────────────────────────────────────────────

void GitHubUpdater::update(const EmulatorConfig& config,
    const QString& installPath,
    const QString& knownTag,
    ReleaseChannel        channel,
    std::atomic<bool>& cancel)
{
    const QString channelLabel =
        (channel == ReleaseChannel::Nightly) ? "nightly" : "stable";

    emit log(QString("[%1] Checking for updates (%2)...")
        .arg(config.displayName, channelLabel));

    const GitHubRelease release = fetchLatestRelease(config, channel);

    if (!release.valid) {
        emit log(QString("[%1] Could not fetch release info.").arg(config.displayName));
        emit done(false, knownTag);
        return;
    }

    emit log(QString("[%1] Latest %2 release: %3")
        .arg(config.displayName, channelLabel, release.tagName));

    // For repos that use a floating tag (e.g. "latest-nightly"), the tag
    // never changes so we compare the asset filename instead.
    const bool floatingTag = !release.assets.isEmpty() &&
        (release.tagName == "latest-nightly" ||
            release.tagName == "latest" ||
            release.tagName == "nightly");

    const QString compareKey = floatingTag
        ? release.assets.first().name
        : release.tagName;

    if (!knownTag.isEmpty() && knownTag == compareKey) {
        emit log(QString("[%1] Already up to date (%2).")
            .arg(config.displayName, compareKey));
        emit done(false, knownTag);
        return;
    }

    // Pick asset pattern for channel
    const QString& pattern =
        (channel == ReleaseChannel::Nightly && !config.nightlyAssetPattern.isEmpty())
        ? config.nightlyAssetPattern
        : config.stableAssetPattern;

    const QRegularExpression rx(pattern, QRegularExpression::CaseInsensitiveOption);
    GitHubAsset chosen;
    for (const auto& a : release.assets) {
        if (rx.match(a.name).hasMatch()) { chosen = a; break; }
    }

    if (chosen.downloadUrl.isEmpty()) {
        emit log(QString("[%1] No matching asset for pattern: %2")
            .arg(config.displayName, pattern));
        emit log(QString("[%1] Available assets:").arg(config.displayName));
        for (const auto& a : release.assets)
            emit log(QString("[%1]   - %2").arg(config.displayName, a.name));
        emit done(false, knownTag);
        return;
    }

    emit log(QString("[%1] Downloading %2 ...")
        .arg(config.displayName, chosen.name));

    QTemporaryDir tmp;
    if (!tmp.isValid()) {
        emit log(QString("[%1] Cannot create temp dir.").arg(config.displayName));
        emit done(false, knownTag);
        return;
    }

    const QString archivePath = tmp.filePath(chosen.name);

    try {
        if (!downloadSync(chosen.downloadUrl, archivePath, cancel)) {
            emit done(false, knownTag);
            return;
        }

        QDir().mkpath(installPath);
        extractAndInstall(config, archivePath, installPath, cancel);

    const QString storedTag = (floatingTag && !chosen.name.isEmpty())
            ? chosen.name
            : release.tagName;

        emit log(QString("[%1] Updated to %2.").arg(config.displayName, storedTag));
        emit done(true, storedTag);

    }
    catch (const std::exception& ex) {
        emit log(QString("[%1] Update error: %2")
            .arg(config.displayName, ex.what()));
        emit done(false, knownTag);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Synchronous download
// ─────────────────────────────────────────────────────────────────────────────

bool GitHubUpdater::downloadSync(const QString& url,
    const QString& dest,
    std::atomic<bool>& cancel)
{
    bool    ok = false;
    QString err;
    bool    done_ = false;

    Downloader dl(m_cache);
    connect(&dl, &Downloader::log, this, &GitHubUpdater::log, Qt::DirectConnection);

    QEventLoop loop;
    connect(&dl, &Downloader::finished, &loop,
        [&](const QString&, const QString& e) {
            if (e.isEmpty() || e == "NOT_MODIFIED") ok = true;
            else err = e;
            done_ = true;
            loop.quit();
        }, Qt::DirectConnection);

    dl.download(url, dest);

    QTimer ct;
    ct.setInterval(200);
    connect(&ct, &QTimer::timeout, [&]() {
        if (cancel.load() && !done_) dl.cancel();
        });
    ct.start();
    loop.exec();

    if (cancel.load()) throw std::runtime_error("Cancelled");
    if (!err.isEmpty()) throw std::runtime_error(err.toStdString());
    return ok;
}

// ─────────────────────────────────────────────────────────────────────────────
// Extract and install
// ─────────────────────────────────────────────────────────────────────────────

void GitHubUpdater::extractAndInstall(const EmulatorConfig& config,
    const QString& archivePath,
    const QString& installPath,
    std::atomic<bool>& cancel)
{
    if (config.archiveType == ArchiveType::SingleFile) {
        const QString dest = installPath + "/" + QFileInfo(archivePath).fileName();
        QDir().mkpath(installPath);
        atomicReplace(archivePath, dest);
        emit log(QString("[%1] Installed %2")
            .arg(config.displayName, QFileInfo(dest).fileName()));
        return;
    }

    QTemporaryDir tmp;
    if (!tmp.isValid()) throw std::runtime_error("Cannot create temp dir");

    struct Entry { QString relPath; QString tempFile; };
    QList<Entry> entries;

    if (config.archiveType == ArchiveType::Zip) {
        ZipExtractor::extract(archivePath, tmp.path(),
            [&](const ZipEntry& ze, const QString& tf) -> bool {
                if (!ze.isDirectory && !tf.isEmpty())
                    entries.append({ ze.name, tf });
                return true;
            });
    }
    else {
        SevenZExtractor::extract(archivePath, tmp.path(),
            [&](const SevenZEntry& se, const QString& tf) -> bool {
                if (!se.isDirectory && !tf.isEmpty())
                    entries.append({ se.name, tf });
                return !cancel.load();
            });
    }

    // Strip common top-level prefix if configured
    QString stripPrefix;
    if (config.stripTopLevelDir && !entries.isEmpty()) {
        const int slash = entries.first().relPath.indexOf('/');
        if (slash > 0) {
            const QString candidate = entries.first().relPath.left(slash + 1);
            bool allMatch = true;
            for (const auto& e : entries)
                if (!e.relPath.startsWith(candidate)) { allMatch = false; break; }
            if (allMatch) stripPrefix = candidate;
        }
    }

    emit progressMax(entries.size());

    for (const auto& e : entries) {
        if (cancel.load()) throw std::runtime_error("Cancelled");

        QString rel = e.relPath;
        if (!stripPrefix.isEmpty() && rel.startsWith(stripPrefix))
            rel = rel.mid(stripPrefix.length());

        const QString dest = installPath + "/" + rel;
        QDir().mkpath(QFileInfo(dest).absolutePath());
        atomicReplace(e.tempFile, dest);
        emit log(QString("[%1] Installed %2").arg(config.displayName, rel));
        emit progressInc();
    }
}

bool GitHubUpdater::atomicReplace(const QString& src, const QString& dest)
{
    QDir().mkpath(QFileInfo(dest).absolutePath());
    if (QFile::exists(dest)) QFile::remove(dest);
    return QFile::rename(src, dest);
}