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
// Helper: commit SHA a tag/ref points at (GitHub tag / branch API)
// ─────────────────────────────────────────────────────────────────────────────

static QString fetchTagSha(QNetworkAccessManager* nam, const QString& repo,
    const QString& tag)
{
    auto fetchJson = [nam](const QString& url) -> QJsonObject {
        QNetworkRequest req;
        req.setUrl(QUrl(url));
        req.setRawHeader("User-Agent", "ulc-emulator-updater/1.0");
        req.setRawHeader("Accept", "application/vnd.github+json");
        req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
            QVariant::fromValue(QNetworkRequest::NoLessSafeRedirectPolicy));

        QEventLoop loop;
        QNetworkReply* reply = nam->get(req);
        QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        QTimer::singleShot(10000, &loop, &QEventLoop::quit);
        loop.exec();

        if (reply->error() != QNetworkReply::NoError) {
            reply->deleteLater();
            return {};
        }
        const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        reply->deleteLater();
        return doc.isObject() ? doc.object() : QJsonObject();
    };

    // Lightweight ref endpoint: { "ref": "refs/tags/preview", "object": {"sha": ...} }
    const QJsonObject ref = fetchJson(
        "https://api.github.com/repos/" + repo + "/git/ref/tags/" + tag);
    if (ref.isEmpty()) return {};

    QString sha = ref.value("object").toObject().value("sha").toString();

    // Annotated tags point at a tag object, not the commit — dereference once.
    if (!sha.isEmpty() &&
        ref.value("object").toObject().value("type").toString() == "tag") {
        const QJsonObject tagObj = fetchJson(
            "https://api.github.com/repos/" + repo + "/git/tags/" + sha);
        const QString commit = tagObj.value("object").toObject().value("sha").toString();
        if (!commit.isEmpty()) sha = commit;
    }
    return sha;
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
// Public: fetch release — routes to the correct backend
// ─────────────────────────────────────────────────────────────────────────────

GitHubRelease GitHubUpdater::fetchLatestRelease(const EmulatorConfig& config,
    ReleaseChannel channel)
{
    // Some emulators publish nightlies outside their main release source
    // (e.g. GitHub Actions artifacts via nightly.link, or a JSON build
    // manifest like builds.ppsspp.org) — use the dedicated nightly source.
    if (channel == ReleaseChannel::Nightly) {
        if (!config.nightlyManifestUrl.isEmpty())
            return fetchFromNightlyManifest(config);
        if (!config.nightlyDirectUrl.isEmpty())
            return fetchFromDirectUrl(config, config.nightlyDirectUrl);
    }

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
        return fetchFromDirectUrl(config, config.buildbotApiUrl);
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

    // Floating nightly tags (e.g. DuckStation's "preview") are retargeted on
    // every build, so the tag's commit SHA is the stable identifier for "which
    // build is current" — the same value the emulator's own updater compares
    // against its compiled-in SHA.
    if (result.valid && isFloatingTag(result.tagName))
        result.tagSha = fetchTagSha(m_nam, config.githubRepo, result.tagName);

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

GitHubRelease GitHubUpdater::fetchFromDirectUrl(const EmulatorConfig& config,
    const QString& url)
{
    GitHubRelease result;

    QNetworkRequest req;
    req.setUrl(QUrl(url));
    req.setRawHeader("User-Agent", "ulc-emulator-updater/1.0");
    // HEAD is rejected by some hosts (e.g. nightly.link returns 404), but a
    // 1-byte ranged GET is served — and carries ETag / Last-Modified.
    req.setRawHeader("Range", "bytes=0-0");
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
        QVariant::fromValue(QNetworkRequest::NoLessSafeRedirectPolicy));

    QEventLoop loop;
    QNetworkReply* reply = m_nam->get(req);
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
    asset.name = QFileInfo(QUrl(url).path()).fileName();
    asset.downloadUrl = url;
    result.assets.append(asset);

    return result;
}

// ─────────────────────────────────────────────────────────────────────────────
// JSON build manifest backend (tailored to PPSSPP's builds.ppsspp.org shape:
// { "latest": { "description": "...", "builds": { "Windows": ["...zip"] } } })
// ─────────────────────────────────────────────────────────────────────────────

GitHubRelease GitHubUpdater::fetchFromNightlyManifest(const EmulatorConfig& config)
{
    GitHubRelease result;

    auto fetchJson = [&](const QString& url) -> QJsonDocument {
        QNetworkRequest req;
        req.setUrl(QUrl(url));
        req.setRawHeader("User-Agent", "ulc-emulator-updater/1.0");
        req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
            QVariant::fromValue(QNetworkRequest::NoLessSafeRedirectPolicy));

        QEventLoop loop;
        QNetworkReply* reply = m_nam->get(req);
        connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        QTimer::singleShot(10000, &loop, &QEventLoop::quit);
        loop.exec();

        const QByteArray data = reply->readAll();
        const QNetworkReply::NetworkError err = reply->error();
        reply->deleteLater();
        if (err != QNetworkReply::NoError) return {};
        return QJsonDocument::fromJson(data);
    };

    auto entryWithWindowsZip = [](const QJsonObject& obj,
        QString* description, QString* fileName) {
        if (obj.isEmpty()) return false;
        *description = obj.value("description").toString();
        const QJsonArray win = obj.value("builds").toObject()
            .value("Windows").toArray();
        for (const auto& val : win) {
            const QString name = val.toString();
            if (name.endsWith(".zip", Qt::CaseInsensitive)) {
                *fileName = name;
                return !description->isEmpty();
            }
        }
        return false;
    };

    const QUrl manifestUrl(config.nightlyManifestUrl);
    const QString origin = manifestUrl.scheme() + "://" + manifestUrl.host();

    QString description;
    QString fileName;

    QJsonDocument doc = fetchJson(config.nightlyManifestUrl);
    if (doc.isObject())
        entryWithWindowsZip(doc.object().value("latest").toObject(),
            &description, &fileName);

    // The "latest" entry may lack a Windows build (still building or the
    // Windows job failed) — fall back to scanning recent history.
    if (fileName.isEmpty()) {
        const QString historyUrl = origin + "/meta/history-20.json";
        doc = fetchJson(historyUrl);
        if (doc.isArray()) {
            for (const auto& val : doc.array()) {
                if (entryWithWindowsZip(val.toObject(), &description, &fileName))
                    break;
            }
        }
    }

    if (fileName.isEmpty()) {
        emit log("Build manifest: no Windows build found for "
            + config.displayName);
        return result;
    }

    const QString downloadUrl =
        origin + "/builds/" + description + "/" + fileName;

    result.tagName = description;
    result.isPreRelease = true;
    result.valid = true;

    GitHubAsset asset;
    asset.name = fileName;
    asset.downloadUrl = downloadUrl;
    result.assets.append(asset);

    return result;
}

// ─────────────────────────────────────────────────────────────────────────────
// Changelog / readme helpers for the tab's docs pane
// ─────────────────────────────────────────────────────────────────────────────

// Repository the changelog reads from — githubRepo unless the emulator lives
// somewhere else (RPCS3, mGBA).
static QString changelogRepoFor(const EmulatorConfig& config)
{
    return config.changelogRepo.isEmpty() ? config.githubRepo
                                          : config.changelogRepo;
}

// Repository the README reads from. May differ from the changelog repo when
// that one is build-only, e.g. Eden's CI repo vs. the emulator's own repo.
static QString readmeRepoFor(const EmulatorConfig& config)
{
    if (!config.readmeRepo.isEmpty()) return config.readmeRepo;
    return changelogRepoFor(config);
}

static QString notesHeader(const QString& tag, const QString& date)
{
    if (tag.isEmpty()) return {};
    return tag + (date.isEmpty() ? QString() : "  (" + date.left(10) + ")")
        + "\n\n";
}

// Trim a trailing "/" run so we can append API paths safely.
static QString trimSlashes(QString s)
{
    s = s.trimmed();
    while (s.endsWith('/')) s.chop(1);
    return s;
}

QByteArray GitHubUpdater::httpGetText(const QString& url, int* httpCode,
    QString* error, const QString& accept)
{
    QNetworkRequest req;
    req.setUrl(QUrl(url));
    req.setRawHeader("User-Agent", "ulc-emulator-updater/1.0");
    if (!accept.isEmpty())
        req.setRawHeader("Accept", accept.toUtf8());
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
        QVariant::fromValue(QNetworkRequest::NoLessSafeRedirectPolicy));

    QEventLoop loop;
    QNetworkReply* reply = m_nam->get(req);
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QTimer::singleShot(15000, &loop, &QEventLoop::quit);
    loop.exec();

    const bool ok = (reply->error() == QNetworkReply::NoError);
    if (httpCode) *httpCode = reply->attribute(
        QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (error) *error = reply->errorString();
    const QByteArray data = reply->readAll();
    reply->deleteLater();
    return ok ? data : QByteArray();
}

QString GitHubUpdater::fetchChangelogText(const EmulatorConfig& config,
    ReleaseChannel channel)
{
    const QString repo = changelogRepoFor(config);
    if (repo.isEmpty()) {
        emit log(QString("[%1] No repository configured for changelogs.")
            .arg(config.displayName));
        return {};
    }

    // Gitea/Forgejo hosts expose their own releases API.
    if (config.source == UpdateSource::Gitea) {
        const QString base = trimSlashes(config.buildbotApiUrl);
        const QByteArray data = httpGetText(
            base + "/api/v1/repos/" + repo + "/releases?limit=1",
            nullptr, nullptr, "application/json");

        const QJsonDocument doc = QJsonDocument::fromJson(data);
        if (!doc.isArray() || doc.array().isEmpty()) {
            emit log(QString("[%1] Could not read the changelog from %2.")
                .arg(config.displayName, base));
            return {};
        }

        const QJsonObject rel = doc.array().first().toObject();
        return notesHeader(rel.value("tag_name").toString(),
            rel.value("published_at").toString())
            + rel.value("body").toString().trimmed();
    }

    // GitHub releases — stable uses /latest; nightly scans the newest few so a
    // prerelease (where most nightly builds live) is picked up too.
    const bool nightly = (channel == ReleaseChannel::Nightly);
    const QString endpoint = nightly
        ? "https://api.github.com/repos/" + repo + "/releases?per_page=5"
        : "https://api.github.com/repos/" + repo + "/releases/latest";

    const QJsonDocument doc = QJsonDocument::fromJson(
        httpGetText(endpoint, nullptr, nullptr, "application/vnd.github+json"));

    QJsonObject rel;
    // Nightly builds that live in GitHub releases are almost always marked
    // prerelease; if none is, the repo's releases are version tags and the
    // commit list is a better answer for "what changed lately".
    bool fellBackToTaggedRelease = false;
    if (nightly) {
        if (doc.isArray()) {
            const QJsonArray arr = doc.array();
            for (const auto& val : arr) {
                const QJsonObject o = val.toObject();
                if (o.value("prerelease").toBool()) { rel = o; break; }
            }
            if (rel.isEmpty() && !arr.isEmpty()) {
                rel = arr.first().toObject();
                fellBackToTaggedRelease = true;
            }
        }
    }
    else if (doc.isObject()) {
        rel = doc.object();
    }

    if (rel.isEmpty()) {
        emit log(QString("[%1] Could not fetch release notes for %2.")
            .arg(config.displayName, repo));
        return {};
    }

    const QString body = rel.value("body").toString().trimmed();
    if (!body.isEmpty() && !fellBackToTaggedRelease)
        return notesHeader(rel.value("tag_name").toString(),
            rel.value("published_at").toString()) + body;

    // No release notes of their own (common for floating nightly tags like
    // DuckStation's "preview", and for build-only repos) — fall back to the
    // newest commits on the default branch.
    emit log(QString("[%1] No release notes for the current build — "
        "listing recent commits.").arg(config.displayName));

    const QJsonDocument cdoc = QJsonDocument::fromJson(httpGetText(
        "https://api.github.com/repos/" + repo + "/commits?per_page=25",
        nullptr, nullptr, "application/vnd.github+json"));
    if (!cdoc.isArray()) return {};

    QString out = QString("Recent commits (%1):\n\n").arg(repo);
    for (const auto& val : cdoc.array()) {
        const QJsonObject c = val.toObject().value("commit").toObject();
        QString msg = c.value("message").toString();
        const int nl = msg.indexOf('\n');
        if (nl >= 0) msg = msg.left(nl);
        msg = msg.trimmed();
        if (msg.isEmpty()) continue;
        const QString date = c.value("author").toObject()
            .value("date").toString().left(10);
        out += QString("- %1   %2\n").arg(msg, date);
    }
    return out;
}

QString GitHubUpdater::fetchReadmeText(const EmulatorConfig& config)
{
    const QString repo = readmeRepoFor(config);
    if (repo.isEmpty()) {
        emit log(QString("[%1] No repository configured for a README.")
            .arg(config.displayName));
        return {};
    }

    const QStringList names = { QStringLiteral("README.md"),
                                QStringLiteral("readme.md") };

    if (config.source == UpdateSource::Gitea) {
        const QString base = trimSlashes(config.buildbotApiUrl);
        for (const QString& name : names) {
            const QJsonDocument doc = QJsonDocument::fromJson(httpGetText(
                base + "/api/v1/repos/" + repo + "/contents/" + name,
                nullptr, nullptr, "application/json"));
            if (!doc.isObject()) continue;

            QByteArray raw = doc.object().value("content").toString().toLatin1();
            raw.replace("\n", "").replace("\r", "");
            const QByteArray decoded = QByteArray::fromBase64(raw);
            if (!decoded.trimmed().isEmpty())
                return QString::fromUtf8(decoded);
        }
        emit log(QString("[%1] No README found at %2.")
            .arg(config.displayName, base));
        return {};
    }

    // GitHub serves raw file content without the API (and without rate limits).
    for (const QString& name : names) {
        const QByteArray data = httpGetText(
            "https://raw.githubusercontent.com/" + repo + "/HEAD/" + name);
        if (!data.trimmed().isEmpty())
            return QString::fromUtf8(data);
    }
    emit log(QString("[%1] No README found for %2.")
        .arg(config.displayName, repo));
    return {};
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
        ? (!release.tagSha.isEmpty() ? release.tagSha
            : !chosen.updatedAt.isEmpty() ? chosen.updatedAt
            : !release.publishedAt.isEmpty() ? release.publishedAt
            : chosen.name)
        : release.tagName;
    const QString displayTag = displayTagFor(release, floating, storedTag);

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

        emit log(QString("[%1] Updated to %2.").arg(config.displayName, displayTag));
        emit done(true, storedTag, displayTag);
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