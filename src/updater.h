#pragma once
#include <QObject>
#include <QString>
#include <QSemaphore>
#include <atomic>
#include "etag_cache.h"

// All public methods are synchronous/blocking — run them on a worker thread.
// Emits signals that are safe to connect across threads (Qt::QueuedConnection).
class Updater : public QObject
{
    Q_OBJECT
public:
    explicit Updater(EtagCache *cache, QObject *parent = nullptr);

    void updateCores(const QString &coreDir,  std::atomic<bool> &cancel);
    void updateZip  (const QString &url,
                     const QString &destDir,
                     const QString &label,
                     std::atomic<bool> &cancel);
    void update7z   (const QString &url,
                     const QString &destDir,
                     std::atomic<bool> &cancel);

signals:
    void log            (const QString &msg);
    void coreProgressMax(int max);
    void coreProgressInc();
    void stepProgressMax(int max);
    void stepProgressInc();
    void operationDone  ();

private:
    // Synchronous download helper.
    // Returns true  = new file written to dest.
    //         false = NOT_MODIFIED (ETag matched).
    // Throws std::runtime_error on error or cancellation.
    bool downloadSync(const QString &url,
                      const QString &dest,
                      std::atomic<bool> &cancel);

    void processCore(const QString     &dllPath,
                     QSemaphore        &sem,
                     std::atomic<bool> &cancel);

    bool updateCoreFromZip(const QString     &zipUrl,
                           const QString     &dllDir,
                           const QString     &coreName,
                           std::atomic<bool> &cancel);

    static bool    atomicReplace(const QString &src, const QString &dest);
    static QString fileVersion  (const QString &path);

    EtagCache *m_cache; // not owned
};
