#pragma once
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QFile>
#include <QElapsedTimer>
#include "etag_cache.h"

// Async single-file downloader with ETag conditional-GET support.
// Must be created and used on the same thread (owns its own QNAM).
// Connect signals with Qt::QueuedConnection when crossing threads.
class Downloader : public QObject
{
    Q_OBJECT
public:
    explicit Downloader(EtagCache *cache, QObject *parent = nullptr);
    ~Downloader() override;

    void download(const QString &url, const QString &outputPath);
    void cancel();

signals:
    void log(const QString &msg);
    void downloadProgress(qint64 received, qint64 total);
    // error == ""             -> success, file written
    // error == "NOT_MODIFIED" -> ETag matched, existing file is current
    // error == "CANCELLED"    -> aborted by user
    // anything else           -> network / IO error string
    void finished(const QString &outputPath, const QString &error);

private slots:
    void onReadyRead();
    void onReplyFinished();

private:
    void cleanup();

    QNetworkAccessManager *m_nam;
    EtagCache             *m_cache;
    QNetworkReply         *m_reply  = nullptr;
    QFile                 *m_file   = nullptr;
    QString                m_url;
    QString                m_outPath;
    QElapsedTimer          m_timer;
};
