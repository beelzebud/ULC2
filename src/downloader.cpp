#include "downloader.h"
#include <QNetworkRequest>
#include <QVariant>
#include <QDir>
#include <QFileInfo>

Downloader::Downloader(EtagCache *cache, QObject *parent)
    : QObject(parent)
    , m_cache(cache)
    , m_nam(new QNetworkAccessManager(this))
{}

Downloader::~Downloader()
{
    cancel();
}

void Downloader::download(const QString &url, const QString &outputPath)
{
    m_url     = url;
    m_outPath = outputPath;

    QDir().mkpath(QFileInfo(outputPath).absolutePath());

    QUrl qurl(url);
    QNetworkRequest req(qurl);

    // Qt6: RedirectPolicy is set via setTransferTimeout / attribute with QVariant
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                     QVariant::fromValue(QNetworkRequest::NoLessSafeRedirectPolicy));

    req.setRawHeader("Accept-Encoding", "identity");

    // ETag conditional request
    const QString etag = m_cache->load(url);
    if (!etag.isEmpty())
        req.setRawHeader("If-None-Match", etag.toUtf8());

    m_timer.start();
    m_reply = m_nam->get(req);

    connect(m_reply, &QNetworkReply::readyRead,
            this, &Downloader::onReadyRead);
    connect(m_reply, &QNetworkReply::finished,
            this, &Downloader::onReplyFinished);
    connect(m_reply, &QNetworkReply::downloadProgress,
            this, &Downloader::downloadProgress);
}

void Downloader::cancel()
{
    if (m_reply)
        m_reply->abort();
}

// ── Private ───────────────────────────────────────────────────────────────────

void Downloader::onReadyRead()
{
    // Qt6: use .value<int>() for QVariant -> int conversion
    const int status =
        m_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).value<int>();

    // 304 Not Modified carries no body
    if (status == 304) return;

    if (!m_file) {
        // Parent to nullptr to avoid double-delete in cleanup/cleanup: the
        // QFile object is manually deleted in cleanup(); parenting it to `this`
        // would cause QObject's child-deletion to also try to delete it.
        m_file = new QFile(m_outPath);
        if (!m_file->open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            emit log("Cannot open output file: " + m_outPath);
            m_reply->abort();
            return;
        }
    }
    m_file->write(m_reply->readAll());
}

void Downloader::onReplyFinished()
{
    if (m_file) { m_file->flush(); m_file->close(); }

    const QNetworkReply::NetworkError err = m_reply->error();

    // Qt6: use .value<int>() for QVariant -> int conversion
    const int status =
        m_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).value<int>();

    // ── Cancelled ─────────────────────────────────────────────────────────────
    if (err == QNetworkReply::OperationCanceledError) {
        cleanup();
        emit finished(m_outPath, QStringLiteral("CANCELLED"));
        return;
    }

    // ── Not Modified ──────────────────────────────────────────────────────────
    if (status == 304) {
        emit log("No change: " + m_url + " (ETag matched, existing file is current)");
        cleanup();
        emit finished(m_outPath, QStringLiteral("NOT_MODIFIED"));
        return;
    }

    // ── Network error ─────────────────────────────────────────────────────────
    if (err != QNetworkReply::NoError) {
        const QString msg = m_reply->errorString();
        cleanup();
        emit finished(m_outPath, msg);
        return;
    }

    // ── Success ───────────────────────────────────────────────────────────────
    const QString newEtag =
        QString::fromUtf8(m_reply->rawHeader("ETag")).trimmed();
    if (!newEtag.isEmpty())
        m_cache->save(m_url, newEtag);

    const qint64 elapsed = m_timer.elapsed();
    const qint64 size    = QFileInfo(m_outPath).size();
    if (size > 0 && elapsed > 0) {
        const double mb    = size  / (1024.0 * 1024.0);
        const double speed = mb    / (elapsed / 1000.0);
        emit log(QString("Downloaded %1  (%2 MiB @ %3 MiB/s)")
                     .arg(m_url)
                     .arg(mb,    0, 'f', 2)
                     .arg(speed, 0, 'f', 2));
    } else {
        emit log("Downloaded " + m_url);
    }

    cleanup();
    emit finished(m_outPath, {});
}

void Downloader::cleanup()
{
    if (m_reply) {
        m_reply->deleteLater();
        m_reply = nullptr;
    }
    if (m_file) {
        delete m_file;
        m_file = nullptr;
    }
}
