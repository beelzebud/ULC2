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
// Fetch release info
// ─────────────────────────────────────────────────────────────────────────────

GitHubRelease GitHubUpdater::fetchLatestRelease(const QString& repo,
    ReleaseChannel  channel)
{
    GitHubRelease result;

    // Stable  → /releases/latest  (single object, no pre-releases)
    // Nightly → /releases?per_page=10 (list, pick first pre-release or
    //           first entry if no explicit pre-release exists)
    QString endpoint;
    if (channel == ReleaseChannel::Stable) {
        endpoint = "https://api.github.com/repos/" + repo + "/releases/latest";
    }
    else {
        endpoint = "https://api.github.com/repos/" + repo + "/releases?per_page=10";
    }

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
        emit log("GitHub API error for " + repo + ": " + reply->errorString());
        reply->deleteLater();
        return result;
    }

    const QByteArray body = reply->readAll();
    reply->deleteLater();

    const QJsonDocument doc = QJsonDocument::fromJson(body);

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
        // Response is a single JSON object
        if (doc.isObject())
            parseRelease(doc.object());
    }
    else {
        // Response is an array — pick the first pre-release entry.
        // If none are flagged as pre-release (some repos use releases for
        // nightlies without the flag), fall back to the very first entry.
        if (doc.isArray()) {
            const QJsonArray arr = doc.array();
            QJsonObject best;
            for (const auto& val : arr) {
                const auto obj = val.toObject();
                if (obj.value("prerelease").toBool()) { best = obj; break; }
            }
            if (best.isEmpty() && !arr.isEmpty())
                best = arr.first().toObject();
            if (!best.isEmpty())
                parseRelease(best);
        }
    }

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
    const QString channelLabel = (channel == ReleaseChannel::Nightly)
        ? "nightly" : "stable";

    emit log(QString("[%1] Checking for updates (%2)...")
        .arg(config.displayName, channelLabel));

    const GitHubRelease release = fetchLatestRelease(config.githubRepo, channel);

    if (!release.valid) {
        emit log(QString("[%1] Could not fetch release info.").arg(config.displayName));
        emit done(false, knownTag);
        return;
    }

    emit log(QString("[%1] Latest %2 release: %3")
        .arg(config.displayName, channelLabel, release.tagName));

    if (!knownTag.isEmpty() && knownTag == release.tagName) {
        emit log(QString("[%1] Already up to date (%2).")
            .arg(config.displayName, knownTag));
        emit done(false, knownTag);
        return;
    }

    // Pick the right asset pattern for the channel
    const QString& pattern = (channel == ReleaseChannel::Nightly &&
        !config.nightlyAssetPattern.isEmpty())
        ? config.nightlyAssetPattern
        : config.stableAssetPattern;

    const QRegularExpression rx(pattern, QRegularExpression::CaseInsensitiveOption);
    GitHubAsset chosen;
    for (const auto& a : release.assets) {
        if (rx.match(a.name).hasMatch()) { chosen = a; break; }
    }

    if (chosen.downloadUrl.isEmpty()) {
        emit log(QString("[%1] No matching asset found for pattern: %2")
            .arg(config.displayName, pattern));
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

        emit log(QString("[%1] Updated to %2.")
            .arg(config.displayName, release.tagName));
        emit done(true, release.tagName);

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
            if (e.isEmpty()) ok = true;
            else if (e == "NOT_MODIFIED") ok = true; // treat as success
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

    // Detect common top-level prefix to strip
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