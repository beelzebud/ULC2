#include "retroarch_tab.h"
#include "downloader.h"
#include "archive_zip.h"
#include "archive_7z.h"
#include "constants.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QDir>
#include <QFileInfo>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QObject>
#include <QEventLoop>
#include <QTimer>
#include <QTemporaryDir>
#include <QScrollBar>
#include <QTextDocument>
#include <QTextCursor>
#include <QTextBlock>
#include <QDateTime>
#include <QVariant>

const QString RetroArchTab::RaDownloadUrl =
"https://buildbot.libretro.com/nightly/windows/x86_64/RetroArch.7z";

static const QString CORE_BASE = "https://buildbot.libretro.com/nightly/windows/x86_64/latest/";

// ─────────────────────────────────────────────────────────────────────────────
// Worker — all network and file I/O runs in background thread
// ─────────────────────────────────────────────────────────────────────────────

class RetroArchWorker : public QObject
{
    Q_OBJECT
public:
    explicit RetroArchWorker(EtagCache* cache, QObject* parent = nullptr)
        : QObject(parent)
        , m_cache(cache)
        , m_nam(new QNetworkAccessManager(this))
    {
    }

    void checkRA(std::atomic<bool>* cancel)
    {
        emit log("Checking for RetroArch update...");

        const QString cached = m_cache->load(RetroArchTab::RaDownloadUrl);
        const QString current = fetchETag(RetroArchTab::RaDownloadUrl);

        if (cancel->load()) { emit done(); return; }

        if (current.isEmpty()) {
            emit log("Could not reach RetroArch buildbot.");
            emit raCheckResult(false, cached);
        }
        else if (!cached.isEmpty() && current == cached) {
            emit log("RetroArch is already up to date.");
            emit raCheckResult(false, current);
        }
        else {
            emit log("RetroArch update is available.");
            emit raCheckResult(true, current);
        }
        emit done();
    }

    void downloadRA(const QString& installPath, std::atomic<bool>* cancel)
    {
        emit log("Downloading RetroArch nightly...");

        QTemporaryDir tmp;
        if (!tmp.isValid()) {
            emit log("Cannot create temp dir.");
            emit done();
            return;
        }

        const QString archivePath = tmp.filePath("RetroArch.7z");
        m_cache->save(RetroArchTab::RaDownloadUrl, "");

        if (!downloadFile(RetroArchTab::RaDownloadUrl, archivePath, cancel)) {
            emit done();
            return;
        }

        if (!QFile::exists(archivePath)) {
            emit log("Download produced no output file.");
            emit done();
            return;
        }

        emit log("Extracting RetroArch...");
        QDir().mkpath(installPath);

        struct Entry { QString rel; QString src; };
        QList<Entry> entries;

        SevenZExtractor::extract(archivePath, tmp.path(),
            [&](const SevenZEntry& se, const QString& tf) -> bool {
                if (!se.isDirectory && !tf.isEmpty())
                    entries.append({ se.name, tf });
                return !cancel->load();
            });

        QString stripPrefix;
        if (!entries.isEmpty()) {
            const int slash = entries.first().rel.indexOf('/');
            if (slash > 0) {
                const QString cand = entries.first().rel.left(slash + 1);
                bool all = true;
                for (const auto& e : entries)
                    if (!e.rel.startsWith(cand)) { all = false; break; }
                if (all) stripPrefix = cand;
            }
        }

        emit progressMax(entries.size());

        for (const auto& e : entries) {
            if (cancel->load()) break;
            QString rel = e.rel;
            if (!stripPrefix.isEmpty() && rel.startsWith(stripPrefix))
                rel = rel.mid(stripPrefix.length());
            const QString dest = installPath + "/" + rel;
            QDir().mkpath(QFileInfo(dest).absolutePath());
            if (QFile::exists(dest)) QFile::remove(dest);
            QFile::rename(e.src, dest);
            emit progressInc();
        }

        const QString newETag = fetchETag(RetroArchTab::RaDownloadUrl);
        if (!newETag.isEmpty()) m_cache->save(RetroArchTab::RaDownloadUrl, newETag);

        if (!cancel->load())
            emit log("RetroArch updated successfully.");
        else
            emit log("Cancelled.");

        emit done();
    }

    void checkCores(const QString& coresPath, std::atomic<bool>* cancel)
    {
        QDir dir(coresPath);
        const QStringList dlls = dir.entryList({ "*.dll" }, QDir::Files);

        if (dlls.isEmpty()) {
            emit log("No cores found in: " + coresPath);
            emit coresCheckResult({}, 0);
            emit done();
            return;
        }

        emit log(QString("Checking %1 installed cores for updates...").arg(dlls.size()));
        emit progressMax(dlls.size());

        QStringList needsUpdate;

        for (const QString& dll : dlls) {
            if (cancel->load()) break;

            const QString url = CORE_BASE + dll + ".zip";
            const QString cached = m_cache->load(url);
            const QString current = fetchETag(url);

            if (!current.isEmpty() && current != cached)
                needsUpdate.append(dll);

            emit progressInc();
        }

        if (!cancel->load()) {
            emit log(QString("Check complete: %1 of %2 core(s) have updates available.")
                .arg(needsUpdate.size()).arg(dlls.size()));
        }

        emit coresCheckResult(needsUpdate, dlls.size());
        emit done();
    }

    void downloadCores(const QString& coresPath,
        const QStringList& cores,
        std::atomic<bool>* cancel)
    {
        if (cores.isEmpty()) {
            emit log("No core updates to download.");
            emit done();
            return;
        }

        emit log(QString("Downloading %1 core update(s)...").arg(cores.size()));
        emit progressMax(cores.size());

        int updated = 0;

        for (const QString& dll : cores) {
            if (cancel->load()) break;

            const QString url = CORE_BASE + dll + ".zip";

            QTemporaryDir tmp;
            if (!tmp.isValid()) { emit progressInc(); continue; }

            const QString archivePath = tmp.filePath(dll + ".zip");
            m_cache->save(url, "");

            if (!downloadFile(url, archivePath, cancel)) {
                emit progressInc();
                continue;
            }

            if (!QFile::exists(archivePath)) {
                emit log("No file downloaded for: " + dll);
                emit progressInc();
                continue;
            }

            bool extracted = false;
            ZipExtractor::extract(archivePath, tmp.path(),
                [&](const ZipEntry& ze, const QString& tf) -> bool {
                    if (!ze.isDirectory && ze.name.endsWith(".dll") && !tf.isEmpty()) {
                        const QString dest = coresPath + "/" +
                            QFileInfo(ze.name).fileName();
                        if (QFile::exists(dest)) QFile::remove(dest);
                        QFile::rename(tf, dest);
                        extracted = true;
                    }
                    return true;
                });

            if (extracted) {
                const QString newETag = fetchETag(url);
                if (!newETag.isEmpty()) m_cache->save(url, newETag);
                emit log("Updated: " + dll);
                ++updated;
            }
            else {
                emit log("Failed to extract: " + dll);
            }

            emit progressInc();
        }

        if (!cancel->load())
            emit log(QString("Done: %1 core(s) updated.").arg(updated));
        else
            emit log("Cancelled.");

        emit done();
    }

signals:
    void log(const QString& msg);
    void progressMax(int max);
    void progressInc();
    void done();
    void raCheckResult(bool hasUpdate, const QString& latestTag);
    void coresCheckResult(const QStringList& needsUpdate, int total);

private:
    QString fetchETag(const QString& url)
    {
        QNetworkRequest req;
        req.setUrl(QUrl(url));
        req.setRawHeader("User-Agent", "ulc-emulator-updater/1.0");
        req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
            QVariant::fromValue(QNetworkRequest::NoLessSafeRedirectPolicy));

        QEventLoop loop;
        QNetworkReply* reply = m_nam->head(req);
        QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        QTimer::singleShot(5000, &loop, &QEventLoop::quit);
        loop.exec();

        QString etag;
        if (reply->error() == QNetworkReply::NoError) {
            etag = reply->rawHeader("ETag");
            if (etag.isEmpty())
                etag = reply->rawHeader("Last-Modified");
        }
        reply->deleteLater();
        return etag;
    }

    bool downloadFile(const QString& url,
        const QString& dest,
        std::atomic<bool>* cancel)
    {
        bool    ok = false;
        QString err;
        bool    done_ = false;

        Downloader dl(m_cache);
        connect(&dl, &Downloader::log, this, &RetroArchWorker::log, Qt::DirectConnection);

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
            if (cancel->load() && !done_) dl.cancel();
            });
        ct.start();
        loop.exec();

        if (!err.isEmpty())
            emit log("Download error: " + err);

        return ok && !cancel->load();
    }

    EtagCache* m_cache;
    QNetworkAccessManager* m_nam;
};

#include "retroarch_tab.moc"

// ─────────────────────────────────────────────────────────────────────────────
// RetroArchTab
// ─────────────────────────────────────────────────────────────────────────────

RetroArchTab::RetroArchTab(EtagCache* cache, QWidget* parent)
    : QWidget(parent), m_cache(cache)
{
    m_thread = new QThread(this);
    m_worker = new RetroArchWorker(cache);
    m_worker->moveToThread(m_thread);
    m_thread->start();

    connect(m_worker, &RetroArchWorker::log, this, &RetroArchTab::appendLog);
    connect(m_worker, &RetroArchWorker::progressMax, this, &RetroArchTab::setProgMax);
    connect(m_worker, &RetroArchWorker::progressInc, this, &RetroArchTab::incProgress);
    connect(m_worker, &RetroArchWorker::done, this, &RetroArchTab::onWorkerDone);
    connect(m_worker, &RetroArchWorker::raCheckResult, this, &RetroArchTab::onRACheckResult);
    connect(m_worker, &RetroArchWorker::coresCheckResult, this, &RetroArchTab::onCoresCheckResult);

    buildUi();
}

RetroArchTab::~RetroArchTab()
{
    m_cancel = true;
    m_thread->quit();
    m_thread->wait(6000);
    delete m_worker;
}

void RetroArchTab::applySettings(const AppSettings& s)
{
    if (!s.retroarchPath.isEmpty()) m_raPathEdit->setText(s.retroarchPath);
    if (!s.corePath.isEmpty())      m_corePathEdit->setText(s.corePath);
}

void RetroArchTab::collectSettings(AppSettings& s) const
{
    s.retroarchPath = m_raPathEdit->text();
    s.corePath = m_corePathEdit->text();
}

QString RetroArchTab::currentVersion() const
{
    QString tag = m_cache->load(RaDownloadUrl);
    if (tag.isEmpty()) return "unknown";
    tag.remove('"');
    return tag.length() > 20 ? tag.left(20) + QString::fromUtf8("\xE2\x80\xA6") : tag;
}

void RetroArchTab::stopOperation()
{
    if (!m_running.load()) return;
    m_cancel = true;
    m_btnStop->setEnabled(false);
    appendLog("Cancellation requested...");
}

void RetroArchTab::buildUi()
{
    auto* root = new QVBoxLayout(this);
    root->setSpacing(6);
    root->setContentsMargins(8, 8, 8, 6);

    // ── RetroArch binary ─────────────────────────────────────────────────────
    {
        auto* grp = new QGroupBox("RetroArch");
        auto* grid = new QGridLayout(grp);
        grid->setColumnStretch(1, 1);
        grid->setSpacing(4);

        m_raPathEdit = new QLineEdit;
        auto* btnBrowseRA = new QPushButton("Browse");
        btnBrowseRA->setFixedWidth(72);
        connect(btnBrowseRA, &QPushButton::clicked, this, &RetroArchTab::onBrowseRA);

        m_raStatusLabel = new QLabel("Status: not checked");

        m_btnCheckRA = new QPushButton("Check for Update");
        m_btnDownloadRA = new QPushButton("Download Update");
        m_btnDownloadRA->setEnabled(false);

        connect(m_btnCheckRA, &QPushButton::clicked, this, &RetroArchTab::onCheckRA);
        connect(m_btnDownloadRA, &QPushButton::clicked, this, &RetroArchTab::onDownloadRA);

        auto* btnRow = new QHBoxLayout;
        btnRow->addWidget(m_btnCheckRA);
        btnRow->addWidget(m_btnDownloadRA);
        btnRow->addStretch();

        grid->addWidget(new QLabel("Path:"), 0, 0);
        grid->addWidget(m_raPathEdit, 0, 1);
        grid->addWidget(btnBrowseRA, 0, 2);
        grid->addWidget(m_raStatusLabel, 1, 0, 1, 3);
        grid->addLayout(btnRow, 2, 0, 1, 3);

        root->addWidget(grp);
    }

    // ── Cores ─────────────────────────────────────────────────────────────────
    {
        auto* grp = new QGroupBox("Cores");
        auto* grid = new QGridLayout(grp);
        grid->setColumnStretch(1, 1);
        grid->setSpacing(4);

        m_corePathEdit = new QLineEdit;
        auto* btnBrowseCores = new QPushButton("Browse");
        btnBrowseCores->setFixedWidth(72);
        connect(btnBrowseCores, &QPushButton::clicked, this, &RetroArchTab::onBrowseCores);

        m_coreStatusLabel = new QLabel("Status: not checked");

        m_btnCheckCores = new QPushButton("Check for Core Updates");
        m_btnDlCores = new QPushButton("Download Core Updates");
        m_btnDlCores->setEnabled(false);

        connect(m_btnCheckCores, &QPushButton::clicked, this, &RetroArchTab::onCheckCores);
        connect(m_btnDlCores, &QPushButton::clicked, this, &RetroArchTab::onDownloadCores);

        auto* btnRow = new QHBoxLayout;
        btnRow->addWidget(m_btnCheckCores);
        btnRow->addWidget(m_btnDlCores);
        btnRow->addStretch();

        grid->addWidget(new QLabel("Path:"), 0, 0);
        grid->addWidget(m_corePathEdit, 0, 1);
        grid->addWidget(btnBrowseCores, 0, 2);
        grid->addWidget(m_coreStatusLabel, 1, 0, 1, 3);
        grid->addLayout(btnRow, 2, 0, 1, 3);

        root->addWidget(grp);
    }

    // ── Stop + progress ───────────────────────────────────────────────────────
    {
        auto* hlay = new QHBoxLayout;
        m_btnStop = new QPushButton("Stop");
        m_btnStop->setObjectName("stopBtn");
        m_btnStop->setFixedWidth(80);
        m_btnStop->setEnabled(false);
        connect(m_btnStop, &QPushButton::clicked, this, &RetroArchTab::stopOperation);

        m_bar = new QProgressBar;
        m_bar->setValue(0);

        hlay->addWidget(m_btnStop);
        hlay->addWidget(m_bar, 1);
        root->addLayout(hlay);
    }

    // ── Log ───────────────────────────────────────────────────────────────────
    {
        auto* grp = new QGroupBox("Log");
        auto* lay = new QVBoxLayout(grp);
        m_log = new QTextEdit;
        m_log->setReadOnly(true);
        lay->addWidget(m_log);
        root->addWidget(grp, 1);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Slots — button / dashboard handlers
// ─────────────────────────────────────────────────────────────────────────────

void RetroArchTab::onCheckRA()
{
    if (m_running.exchange(true)) return;
    m_cancel = false;
    m_bar->setValue(0);
    setButtonsEnabled(false);
    m_raHasUpdate = false;
    m_currentOp = RAOp::CheckBinary;
    m_raStatusLabel->setText("Status: checking...");

    QMetaObject::invokeMethod(m_worker,
        [this]() { m_worker->checkRA(&m_cancel); },
        Qt::QueuedConnection);
}

void RetroArchTab::onDownloadRA()
{
    if (m_running.exchange(true)) return;
    m_cancel = false;
    m_bar->setValue(0);
    setButtonsEnabled(false);
    m_currentOp = RAOp::DownloadBinary;

    const QString path = m_raPathEdit->text();

    QMetaObject::invokeMethod(m_worker,
        [this, path]() { m_worker->downloadRA(path, &m_cancel); },
        Qt::QueuedConnection);
}

bool RetroArchTab::startCheckCores()
{
    if (m_running.exchange(true)) return false;
    m_cancel = false;
    m_bar->setValue(0);
    setButtonsEnabled(false);
    m_pendingCoreUpdates.clear();
    m_currentOp = RAOp::CheckCores;
    m_coreStatusLabel->setText("Status: checking...");

    const QString path = m_corePathEdit->text();

    QMetaObject::invokeMethod(m_worker,
        [this, path]() { m_worker->checkCores(path, &m_cancel); },
        Qt::QueuedConnection);
    return true;
}

bool RetroArchTab::startDownloadCores()
{
    if (m_running.exchange(true)) return false;
    m_cancel = false;
    m_bar->setValue(0);
    setButtonsEnabled(false);
    m_currentOp = RAOp::DownloadCores;

    const QString     path = m_corePathEdit->text();
    const QStringList cores = m_pendingCoreUpdates;

    QMetaObject::invokeMethod(m_worker,
        [this, path, cores]() { m_worker->downloadCores(path, cores, &m_cancel); },
        Qt::QueuedConnection);
    return true;
}

void RetroArchTab::onCheckCores()
{
    startCheckCores();
}

void RetroArchTab::onDownloadCores()
{
    if (m_pendingCoreUpdates.isEmpty()) {
        appendLog("No pending core updates — run Check for Core Updates first.");
        emit coresUpdateFinished();
        return;
    }
    startDownloadCores();
}

void RetroArchTab::onBrowseRA()
{
    const QString p = QFileDialog::getExistingDirectory(
        this, "Select RetroArch folder", m_raPathEdit->text());
    if (!p.isEmpty()) m_raPathEdit->setText(p + "/");
}

void RetroArchTab::onBrowseCores()
{
    const QString p = QFileDialog::getExistingDirectory(
        this, "Select cores folder", m_corePathEdit->text());
    if (!p.isEmpty()) m_corePathEdit->setText(p + "/");
}

// ─────────────────────────────────────────────────────────────────────────────
// Slots — worker results
// ─────────────────────────────────────────────────────────────────────────────

void RetroArchTab::onWorkerDone()
{
    m_running = false;
    setButtonsEnabled(true);

    switch (m_currentOp) {
    case RAOp::CheckBinary:
        emit binaryCheckFinished(m_raHasUpdate);
        break;
    case RAOp::DownloadBinary:
        emit binaryUpdateFinished();
        break;
    case RAOp::CheckCores:
        emit coresCheckFinished(m_pendingCoreUpdates.size(), m_lastCoreTotal);
        break;
    case RAOp::DownloadCores:
        emit coresUpdateFinished();
        break;
    case RAOp::None:
        break;
    }
    m_currentOp = RAOp::None;
}

void RetroArchTab::onRACheckResult(bool hasUpdate, const QString& /*latestTag*/)
{
    m_raHasUpdate = hasUpdate;
    m_btnDownloadRA->setEnabled(hasUpdate);
    m_raStatusLabel->setText(hasUpdate
        ? "Status: update available"
        : "Status: up to date");
}

void RetroArchTab::onCoresCheckResult(const QStringList& needsUpdate, int total)
{
    m_pendingCoreUpdates = needsUpdate;
    m_lastCoreTotal = total;
    m_btnDlCores->setEnabled(!needsUpdate.isEmpty());

    if (total == 0) {
        m_coreStatusLabel->setText("Status: no cores found");
    }
    else if (needsUpdate.isEmpty()) {
        m_coreStatusLabel->setText(
            QString("Status: all %1 cores up to date").arg(total));
    }
    else {
        m_coreStatusLabel->setText(
            QString("Status: %1 of %2 cores have updates")
            .arg(needsUpdate.size()).arg(total));
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Log / progress / buttons
// ─────────────────────────────────────────────────────────────────────────────

void RetroArchTab::appendLog(const QString& msg)
{
    QTextDocument* doc = m_log->document();
    while (doc->blockCount() > Constants::MaxLogLines) {
        QTextCursor cur(doc->begin());
        cur.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        cur.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
        cur.removeSelectedText();
    }
    m_log->append(QDateTime::currentDateTime().toString("HH:mm:ss") + " - " + msg);
    m_log->verticalScrollBar()->setValue(m_log->verticalScrollBar()->maximum());
}

void RetroArchTab::setProgMax(int max) { m_bar->setMaximum(max); m_bar->setValue(0); }
void RetroArchTab::incProgress()
{
    if (m_bar->value() < m_bar->maximum())
        m_bar->setValue(m_bar->value() + 1);
}

void RetroArchTab::setButtonsEnabled(bool on)
{
    m_btnCheckRA->setEnabled(on);
    m_btnCheckCores->setEnabled(on);
    m_btnDownloadRA->setEnabled(on && m_raHasUpdate);
    m_btnDlCores->setEnabled(on && !m_pendingCoreUpdates.isEmpty());
    m_btnStop->setEnabled(!on);
}