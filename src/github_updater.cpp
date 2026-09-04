#include <QProcess>
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
#include <QStringList>

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
    ReleaseChannel channel)
{
    switch (config.source) {
    case UpdateSource::GitHub:
        return fetchFromGitHub(config, channel);
    case UpdateSource::DolphinBuildbot:
        return fetchFromBuildbot(config);
    case UpdateSource::Rpcs3Net:
        return fetchFromRpcs3Net(config);
    case UpdateSource::Gitea:
        return fetchFromGitea(config, channel);
    case UpdateSource::DirectUrl:
        return fetchFromDirectUrl(config);
    }
    return {};
}

// ─────────────────────────────────────────────────────────────────────────────
// GitHub Releases backend
// ─────────────────────────────────────────────────────────────────────────────

GitHubRelease GitHubUpdater::fetchFromGitHub(const EmulatorConfig& config,
    ReleaseChannel channel)
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
        result.publishedAt = obj.value("published_at").toString();
        result.isPreRelease = obj.value("prerelease").toBool();
        for (const auto& val : obj.value("assets").toArray()) {
            const auto a = val.toObject();
            GitHubAsset asset;
            asset.name = a.value("name").toString();
            asset.downloadUrl = a.value("browser_download_url").toString();
            asset.updatedAt = a.value("updated_at").toString();
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
// Dolphin buildbot backend
// ─────────────────────────────────────────────────────────────────────────────

GitHubRelease GitHubUpdater::fetchFromBuildbot(const EmulatorConfig& config)
{
    GitHubRelease result;

    QStringList urlsToTry;
    urlsToTry << config.buildbotApiUrl
        << "https://api.dolphin-emu.org/download/list/dev/1/"
        << "https://dolphin-emu.org/download/";

    QString html;
    for (const QString& url : urlsToTry) {
        QNetworkRequest req;
        req.setUrl(QUrl(url));
        req.setRawHeader("User-Agent",
            "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36");
        req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
            QVariant::fromValue(QNetworkRequest::NoLessSafeRedirectPolicy));

        QEventLoop loop;
        QNetworkReply* reply = m_nam->get(req);
        connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        QTimer::singleShot(10000, &loop, &QEventLoop::quit);
        loop.exec();

        const int httpCode = reply->attribute(
            QNetworkRequest::HttpStatusCodeAttribute).toInt();
        emit log(QString("[Dolphin] %1 — HTTP %2").arg(url).arg(httpCode));

        if (reply->error() == QNetworkReply::NoError) {
            html = QString::fromUtf8(reply->readAll());
            reply->deleteLater();
            emit log(QString("[Dolphin] Got %1 chars").arg(html.size()));
            break;
        }
        reply->deleteLater();
    }

    if (html.isEmpty()) {
        emit log("[Dolphin] All URLs failed — could not fetch build list.");
        return result;
    }

    const QRegularExpression urlRx(
        R"((https://dl\.dolphin-emu\.org/[^\s"'<>]+dolphin-[^\s"'<>]+-x64\.7z))",
        QRegularExpression::CaseInsensitiveOption);

    const auto match = urlRx.match(html);
    if (!match.hasMatch()) {
        emit log("[Dolphin] No .7z download link found. Page preview:");
        emit log(html.left(500));
        return result;
    }

    const QString downloadUrl = match.captured(1);
    const QString filename = QFileInfo(QUrl(downloadUrl).path()).fileName();

    const QRegularExpression verRx(
        R"(dolphin-(?:master-)?([\d]+-[\d]+)-x64\.7z)",
        QRegularExpression::CaseInsensitiveOption);
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

// ─────────────────────────────────────────────────────────────────────────────
// RPCS3 backend
// ─────────────────────────────────────────────────────────────────────────────

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
    const int         returnCode = root.value("return_code").toInt(-99);

    if (returnCode < -1) {
        emit log(QString("RPCS3 update API returned server error: %1").arg(returnCode));
        return result;
    }

    const QJsonObject latest = root.value("latest_build").toObject();
    const QString     datetime = latest.value("datetime").toString();
    const QString     downloadUrl = latest.value("windows").toObject()
        .value("download").toString();

    if (downloadUrl.isEmpty()) {
        emit log("RPCS3 update API: missing windows download URL.");
        return result;
    }

    const QString filename = QFileInfo(QUrl(downloadUrl).path()).fileName();
    const QRegularExpression verRx(R"(rpcs3-v([\d.]+-[\d]+))");
    const auto verMatch = verRx.match(filename);
    const QString version = verMatch.hasMatch()
        ? verMatch.captured(1)
        : datetime;

    result.tagName = QString("%1 (%2)").arg(version, datetime);
    result.isPreRelease = true;

    GitHubAsset asset;
    asset.name = filename;
    asset.downloadUrl = downloadUrl;
    result.assets.append(asset);
    result.valid = true;

    return result;
}

// ─────────────────────────────────────────────────────────────────────────────
// Gitea backend
// ─────────────────────────────────────────────────────────────────────────────

GitHubRelease GitHubUpdater::fetchFromGitea(const EmulatorConfig& config,
    ReleaseChannel channel)
{
    GitHubRelease result;

    QString repo = config.githubRepo;
    if (channel == ReleaseChannel::Stable && !config.giteaStableRepo.isEmpty())
        repo = config.giteaStableRepo;

    QString base = config.buildbotApiUrl.trimmed();
    while (base.endsWith('/')) base.chop(1);

    if (base.isEmpty() || repo.isEmpty()) {
        emit log("Gitea: missing base URL or repo for " + config.displayName);
        return result;
    }

    const QString endpoint =
        base + "/api/v1/repos/" + repo + "/releases?limit=10";

    QNetworkRequest req;
    req.setUrl(QUrl(endpoint));
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

    auto addBodyLinks = [&](const QString& body) {
        const QRegularExpression linkRx(R"((https?://[^\s)\]>]+))");
        auto it = linkRx.globalMatch(body);
        while (it.hasNext()) {
            const QString url = it.next().captured(1);
            const QString name = QFileInfo(QUrl(url).path()).fileName();
            if (!name.endsWith(".zip", Qt::CaseInsensitive) &&
                !name.endsWith(".7z", Qt::CaseInsensitive) &&
                !name.endsWith(".exe", Qt::CaseInsensitive))
                continue;
            bool dup = false;
            for (const auto& a : result.assets)
                if (a.downloadUrl == url) { dup = true; break; }
            if (dup) continue;
            GitHubAsset asset;
            asset.name = name;
            asset.downloadUrl = url;
            result.assets.append(asset);
        }
    };

    auto parseRelease = [&](const QJsonObject& obj) {
        result.tagName = obj.value("tag_name").toString();
        result.publishedAt = obj.value("published_at").toString();
        result.isPreRelease = obj.value("prerelease").toBool();
        for (const auto& val : obj.value("assets").toArray()) {
            const auto a = val.toObject();
            GitHubAsset asset;
            asset.name = a.value("name").toString();
            asset.downloadUrl = a.value("browser_download_url").toString();
            asset.updatedAt = a.value("updated_at").toString();
            asset.size = a.value("size").toInteger();
            result.assets.append(asset);
        }
        // Some Forgejo releases publish downloads only as links in the
        // release body (no API attachments) — surface those as assets too.
        addBodyLinks(obj.value("body").toString());
        result.valid = !result.tagName.isEmpty();
    };

    if (!doc.isArray()) {
        emit log("Gitea API: unexpected response for " + config.displayName);
        return result;
    }

    const QJsonArray arr = doc.array();
    if (arr.isEmpty()) {
        emit log("Gitea API: no releases found for " + config.displayName);
        return result;
    }

    QJsonObject best;
    const bool wantPrerelease = (channel == ReleaseChannel::Nightly);
    for (const auto& val : arr) {
        const auto obj = val.toObject();
        const bool isPre = obj.value("prerelease").toBool();
        if (isPre == wantPrerelease) { best = obj; break; }
    }
    if (best.isEmpty())
        best = arr.first().toObject();

    if (!best.isEmpty())
        parseRelease(best);

    return result;
}

// ─────────────────────────────────────────────────────────────────────────────
// DirectUrl backend — HEAD request, ETag/Last-Modified as version identifier
// ─────────────────────────────────────────────────────────────────────────────

GitHubRelease GitHubUpdater::fetchFromDirectUrl(const EmulatorConfig& config)
{
    GitHubRelease result;

    QNetworkRequest req;
    req.setUrl(QUrl(config.buildbotApiUrl));
    req.setRawHeader("User-Agent", "ulc-emulator-updater/1.0");
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
        QVariant::fromValue(QNetworkRequest::NoLessSafeRedirectPolicy));

    QEventLoop loop;
    QNetworkReply* reply = m_nam->head(req);
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QTimer::singleShot(10000, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        emit log("DirectUrl fetch error for " + config.displayName +
            ": " + reply->errorString());
        reply->deleteLater();
        return result;
    }

    QString version = reply->rawHeader("ETag");
    if (version.isEmpty())
        version = reply->rawHeader("Last-Modified");

    reply->deleteLater();

    if (version.isEmpty()) {
        emit log("Could not determine version for " + config.displayName);
        return result;
    }

    result.tagName = version;
    result.isPreRelease = true;
    result.valid = true;

    GitHubAsset asset;
    asset.name = QFileInfo(QUrl(config.buildbotApiUrl).path()).fileName();
    asset.downloadUrl = config.buildbotApiUrl;
    result.assets.append(asset);

    return result;
}

// ─────────────────────────────────────────────────────────────────────────────
// Helper: is this a floating tag that never changes between builds?
// ─────────────────────────────────────────────────────────────────────────────

static bool isFloatingTag(const QString& tag)
{
    const QStringList floating = {
        "latest-nightly", "latest", "nightly", "preview",
        "canary", "dev", "master", "main", "edge", "pre-release"
    };
    return floating.contains(tag.toLower());
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

    const QString& pattern =
        (channel == ReleaseChannel::Nightly && !config.nightlyAssetPattern.isEmpty())
        ? config.nightlyAssetPattern
        : config.stableAssetPattern;

    const QRegularExpression rx(pattern, QRegularExpression::CaseInsensitiveOption);
    GitHubAsset chosen;
    for (const auto& a : release.assets) {
        if (rx.match(a.name).hasMatch()) { chosen = a; break; }
    }

    const bool floating = isFloatingTag(release.tagName);
    const QString storedTag = floating
        ? (!chosen.updatedAt.isEmpty() ? chosen.updatedAt
            : !release.publishedAt.isEmpty() ? release.publishedAt
            : chosen.name)
        : release.tagName;

    if (!knownTag.isEmpty() && knownTag == storedTag) {
        emit log(QString("[%1] Already up to date (%2).")
            .arg(config.displayName, storedTag));
        emit done(false, knownTag);
        return;
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

    m_cache->save(chosen.downloadUrl, "");

    try {
        if (!downloadSync(chosen.downloadUrl, archivePath, cancel)) {
            emit done(false, knownTag);
            return;
        }

        if (!QFile::exists(archivePath)) {
            emit log(QString("[%1] Already up to date.").arg(config.displayName));
            emit done(false, knownTag);
            return;
        }

        QDir().mkpath(installPath);
        extractAndInstall(config, archivePath, installPath, cancel);

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
    if (config.archiveType == ArchiveType::SevenZSfx) {
        // MAME ships as a 7-Zip self-extracting archive.
        // Run it silently with the output path flag to extract in place.
        QDir().mkpath(installPath);
        emit log(QString("[%1] Extracting self-installing archive...")
            .arg(config.displayName));

        QProcess proc;
        proc.start(archivePath, {
            QString("-o%1").arg(QDir::toNativeSeparators(installPath)),
            "-y"
            });

        if (!proc.waitForFinished(300000)) {   // 5 minute timeout
            proc.kill();
            throw std::runtime_error("SFX extraction timed out.");
        }

        if (proc.exitCode() != 0) {
            throw std::runtime_error(
                "SFX extraction failed with exit code: " +
                std::to_string(proc.exitCode()));
        }

        emit log(QString("[%1] Extraction complete.").arg(config.displayName));
        return;
    }

        if (config.archiveType == ArchiveType::SingleFile) {
        QDir().mkpath(installPath);
        const QString dest = installPath + "/" + config.exeName;
        atomicReplace(archivePath, dest);
        emit log(QString("[%1] Installed %2")
            .arg(config.displayName, config.exeName));
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