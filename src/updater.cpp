#include "updater.h"
#include "constants.h"
#include "downloader.h"
#include "archive_zip.h"
#include "archive_7z.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFileInfoList>
#include <QTemporaryDir>
#include <QEventLoop>
#include <QTimer>
#include <QThread>

#ifdef Q_OS_WIN
#  include <windows.h>
#  include <winver.h>
#  include <vector>
#endif

Updater::Updater(EtagCache *cache, QObject *parent)
    : QObject(parent), m_cache(cache) {}

// ─────────────────────────────────────────────────────────────────────────────
// Synchronous download (blocks caller via QEventLoop)
// ─────────────────────────────────────────────────────────────────────────────

bool Updater::downloadSync(const QString     &url,
                            const QString     &dest,
                            std::atomic<bool> &cancel)
{
    bool   resultFresh = false;
    QString errorMsg;
    bool   done = false;

    // Downloader must live on the calling (worker) thread
    Downloader dl(m_cache);
    connect(&dl, &Downloader::log, this, &Updater::log, Qt::DirectConnection);

    QEventLoop loop;
    connect(&dl, &Downloader::finished, &loop,
            [&](const QString &, const QString &err) {
                if      (err == "NOT_MODIFIED") resultFresh = false;
                else if (!err.isEmpty())        errorMsg    = err;
                else                            resultFresh = true;
                done = true;
                loop.quit();
            }, Qt::DirectConnection);

    dl.download(url, dest);

    // Poll cancel flag while waiting
    QTimer ct;
    ct.setInterval(200);
    connect(&ct, &QTimer::timeout, [&]() {
        if (cancel.load() && !done) dl.cancel();
    });
    ct.start();

    loop.exec();

    if (cancel.load())
        throw std::runtime_error("Cancelled");
    if (!errorMsg.isEmpty())
        throw std::runtime_error(errorMsg.toStdString());

    return resultFresh;
}

// ─────────────────────────────────────────────────────────────────────────────
// Core update — parallel with QSemaphore-limited concurrency
// ─────────────────────────────────────────────────────────────────────────────

void Updater::updateCores(const QString &coreDir, std::atomic<bool> &cancel)
{
    QDir dir(coreDir);
    if (!dir.exists()) {
        emit log("Core directory not found.");
        emit operationDone();
        return;
    }

#ifdef Q_OS_WIN
    const QFileInfoList cores = dir.entryInfoList({"*.dll"}, QDir::Files);
#else
    const QFileInfoList cores = dir.entryInfoList({"*.so"},  QDir::Files);
#endif

    emit log(QString("Found %1 cores.").arg(cores.size()));
    emit coreProgressMax(cores.size());

    if (cores.isEmpty()) { emit operationDone(); return; }

    QSemaphore sem(Constants::CoreParallel);
    QList<QThread*> threads;

    for (const QFileInfo &fi : cores) {
        if (cancel.load()) break;
        sem.acquire(1);
        if (cancel.load()) { sem.release(1); break; }

        const QString path = fi.absoluteFilePath();
        auto *t = QThread::create([this, path, &sem, &cancel]() {
            processCore(path, sem, cancel);
        });
        connect(t, &QThread::finished, t, &QThread::deleteLater);
        threads << t;
        t->start();
    }

    for (auto *t : threads) t->wait();

    emit log("Core update completed.");
    emit operationDone();
}

void Updater::processCore(const QString     &dllPath,
                           QSemaphore        &sem,
                           std::atomic<bool> &cancel)
{
    const QString coreName = QFileInfo(dllPath).completeBaseName();
    const QString dllDir   = QFileInfo(dllPath).absolutePath();

#ifdef Q_OS_WIN
    const QString zipUrl = Constants::RemoteCoreBase + coreName + ".dll.zip";
#else
    const QString zipUrl = Constants::RemoteCoreBase + coreName + ".so.zip";
#endif

    const QString oldVer = fileVersion(dllPath);
    emit log(QString("[%1] Current version: %2")
                 .arg(coreName, oldVer.isEmpty() ? "unknown" : oldVer));

    try {
        const bool updated = updateCoreFromZip(zipUrl, dllDir, coreName, cancel);
        const QString newVer = fileVersion(dllPath);

        if (updated) {
            if (!oldVer.isEmpty() && !newVer.isEmpty() && oldVer != newVer)
                emit log(QString("[%1] Updated %2 -> %3").arg(coreName, oldVer, newVer));
            else
                emit log(QString("[%1] Updated (version %2)")
                             .arg(coreName, newVer.isEmpty() ? "unknown" : newVer));
        } else {
            emit log(QString("[%1] Already up to date (%2)")
                         .arg(coreName, newVer.isEmpty() ? oldVer : newVer));
        }
    } catch (const std::exception &ex) {
        emit log(QString("[%1] Error: %2").arg(coreName, ex.what()));
    }

    emit coreProgressInc();
    sem.release(1);
}

bool Updater::updateCoreFromZip(const QString     &zipUrl,
                                 const QString     &dllDir,
                                 const QString     &coreName,
                                 std::atomic<bool> &cancel)
{
    QTemporaryDir tmp;
    if (!tmp.isValid())
        throw std::runtime_error("Cannot create temp dir");

    const bool fresh = downloadSync(zipUrl, tmp.filePath("core.zip"), cancel);
    if (!fresh) return false; // NOT_MODIFIED

    bool updated = false;

    ZipExtractor::extract(tmp.filePath("core.zip"), tmp.path(),
        [&](const ZipEntry &ze, const QString &tempFile) -> bool
    {
        if (ze.isDirectory || tempFile.isEmpty()) return false;
        if (cancel.load()) return false;

        const QString destPath  = dllDir + "/" + QFileInfo(ze.name).fileName();
        const bool    destExist = QFile::exists(destPath);
        const qint64  entryTs   = ze.lastModified.toSecsSinceEpoch();

        const bool shouldReplace =
            !destExist ||
            entryTs > QFileInfo(destPath).lastModified().toSecsSinceEpoch();

        if (shouldReplace) {
            if (atomicReplace(tempFile, destPath)) {
                emit log(QString("[%1] Updated %2").arg(coreName, ze.name));
                updated = true;
                return true;
            }
        } else {
            emit log(QString("[%1] Skipped %2 (up to date)").arg(coreName, ze.name));
        }
        return false;
    });

    return updated;
}

// ─────────────────────────────────────────────────────────────────────────────
// ZIP update (assets / info / database)
// ─────────────────────────────────────────────────────────────────────────────

void Updater::updateZip(const QString     &url,
                         const QString     &destDir,
                         const QString     &label,
                         std::atomic<bool> &cancel)
{
    if (!QDir(destDir).exists()) {
        emit log(label + " directory missing.");
        emit operationDone();
        return;
    }

    QTemporaryDir tmp;
    if (!tmp.isValid()) {
        emit log("Cannot create temp dir.");
        emit operationDone();
        return;
    }

    const QString zipPath = tmp.filePath("download.zip");

    try {
        const bool fresh = downloadSync(url, zipPath, cancel);
        if (!fresh) {
            emit log(label + " already up to date (ETag matched).");
            emit operationDone();
            return;
        }

        const int total = ZipExtractor::countEntries(zipPath);
        emit stepProgressMax(total > 0 ? total : 0);

        ZipExtractor::extract(zipPath, tmp.path(),
            [&](const ZipEntry &ze, const QString &tempFile) -> bool
        {
            emit stepProgressInc();
            if (cancel.load()) return false;

            if (ze.isDirectory) {
                QDir().mkpath(destDir + "/" + ze.name);
                return false;
            }
            if (tempFile.isEmpty()) return false;

            const QString destPath  = destDir + "/" + ze.name;
            const bool    destExist = QFile::exists(destPath);
            const qint64  entryTs   = ze.lastModified.toSecsSinceEpoch();

            QDir().mkpath(QFileInfo(destPath).absolutePath());

            if (!destExist || entryTs > QFileInfo(destPath).lastModified().toSecsSinceEpoch()) {
                if (atomicReplace(tempFile, destPath)) {
                    emit log(QString("[%1] Updated %2").arg(label, ze.name));
                    return true;
                }
            } else {
                QFile::remove(tempFile);
                emit log(QString("[%1] Skipped %2").arg(label, ze.name));
            }
            return false;
        });

        emit log(label + " updated.");
    } catch (const std::exception &ex) {
        emit log(QString("%1 error: %2").arg(label, ex.what()));
    }

    emit operationDone();
}

// ─────────────────────────────────────────────────────────────────────────────
// 7z update (RetroArch)
// ─────────────────────────────────────────────────────────────────────────────

void Updater::update7z(const QString     &url,
                        const QString     &destDir,
                        std::atomic<bool> &cancel)
{
    if (!QDir(destDir).exists()) {
        emit log("RetroArch directory missing.");
        emit operationDone();
        return;
    }

    QTemporaryDir tmp;
    if (!tmp.isValid()) {
        emit log("Cannot create temp dir.");
        emit operationDone();
        return;
    }

    const QString archPath = tmp.filePath("retroarch.7z");

    try {
        const bool fresh = downloadSync(url, archPath, cancel);
        if (!fresh) {
            emit log("RetroArch already up to date (ETag matched).");
            emit operationDone();
            return;
        }

        SevenZExtractor::extract(archPath, tmp.path(),
            [&](const SevenZEntry &ze, const QString &tempFile) -> bool
        {
            emit stepProgressInc();
            if (cancel.load()) return false;

            if (ze.isDirectory) {
                QDir().mkpath(destDir + "/" + ze.name);
                return true;
            }
            if (tempFile.isEmpty()) return false;

            const QString destPath = destDir + "/" + ze.name;
            QDir().mkpath(QFileInfo(destPath).absolutePath());

            if (atomicReplace(tempFile, destPath)) {
                emit log("Extracted " + ze.name);
                return true;
            }
            return false;
        },
        [&](int done, int total) {
            Q_UNUSED(done) Q_UNUSED(total)
        });

        emit log("RetroArch updated.");
    } catch (const std::exception &ex) {
        emit log(QString("7z update error: %1").arg(ex.what()));
    }

    emit operationDone();
}

// ─────────────────────────────────────────────────────────────────────────────
// Utilities
// ─────────────────────────────────────────────────────────────────────────────

bool Updater::atomicReplace(const QString &src, const QString &dest)
{
    QDir().mkpath(QFileInfo(dest).absolutePath());
    if (QFile::exists(dest) && !QFile::remove(dest)) return false;
    return QFile::rename(src, dest);
}

QString Updater::fileVersion(const QString &path)
{
#ifdef Q_OS_WIN
    DWORD  dummy = 0;
    const DWORD sz = GetFileVersionInfoSizeW(path.toStdWString().c_str(), &dummy);
    if (!sz) return {};
    std::vector<BYTE> buf(sz);
    if (!GetFileVersionInfoW(path.toStdWString().c_str(), 0, sz, buf.data())) return {};
    VS_FIXEDFILEINFO *fi  = nullptr;
    UINT              len = 0;
    if (!VerQueryValueW(buf.data(), L"\\",
                        reinterpret_cast<LPVOID*>(&fi), &len) || !fi) return {};
    return QString("%1.%2.%3.%4")
        .arg(HIWORD(fi->dwFileVersionMS)).arg(LOWORD(fi->dwFileVersionMS))
        .arg(HIWORD(fi->dwFileVersionLS)).arg(LOWORD(fi->dwFileVersionLS));
#else
    Q_UNUSED(path)
    return {};
#endif
}
