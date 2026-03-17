#include "mainwindow.h"
#include "aboutdialog.h"
#include "constants.h"

#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QScrollBar>
#include <QFileDialog>
#include <QCloseEvent>
#include <QDateTime>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QProcess>
#include <QRegularExpression>   // Qt6: QRegExp is removed, use QRegularExpression
#include <QTextDocument>
#include <QTextCursor>
#include <QTextBlock>
#include <QStyle>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include <QEventLoop>
#include <QVariant>

#ifdef Q_OS_WIN
#  include <windows.h>
#  include <winver.h>
#  include <vector>
#endif

// ─────────────────────────────────────────────────────────────────────────────
// Stylesheet
// ─────────────────────────────────────────────────────────────────────────────
static const char *kStyle = R"(
* {
    font-family: Consolas, "Courier New", monospace;
    font-size: 9pt;
}
QMainWindow, QDialog, QWidget {
    background-color: #000000;
    color: #00FF00;
}
QGroupBox {
    border: 1px solid #00FF00;
    margin-top: 10px;
    padding: 6px 4px 4px 4px;
    color: #00FF00;
}
QGroupBox::title {
    subcontrol-origin: margin;
    left: 8px;
    padding: 0 4px;
    color: #00FF00;
}
QLineEdit {
    background: #000000;
    color: #00FF00;
    border: 1px solid #005500;
    padding: 1px 4px;
    selection-background-color: #003300;
}
QTextEdit {
    background: #000000;
    color: #00FF00;
    border: 1px solid #005500;
}
QPushButton {
    background: #000000;
    color: #00FF00;
    border: 1px solid #00FF00;
    padding: 4px 8px;
    min-height: 24px;
}
QPushButton:hover    { background: #001a00; }
QPushButton:pressed  { background: #003300; }
QPushButton:disabled { color: #004400; border-color: #004400; }
QPushButton[danger="true"]          { border-color: #FF4444; color: #FF4444; }
QPushButton[danger="true"]:hover    { background: #1a0000; }
QPushButton[danger="true"]:disabled { color: #440000; border-color: #440000; }
QProgressBar {
    border: 1px solid #005500;
    background: #000000;
    color: #00FF00;
    text-align: center;
}
QProgressBar::chunk { background: #007700; }
QScrollBar:vertical            { background: #0a0a0a; width: 12px; }
QScrollBar::handle:vertical    { background: #005500; min-height: 24px; border-radius: 3px; }
QScrollBar::add-line:vertical,
QScrollBar::sub-line:vertical  { height: 0; }
)";

// ─────────────────────────────────────────────────────────────────────────────
// Construction / Destruction
// ─────────────────────────────────────────────────────────────────────────────

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    const QString base = QCoreApplication::applicationDirPath();
    m_settings = new SettingsManager(base + "/settings.json");
    m_cache    = new EtagCache(base + "/etag-cache");

    m_worker  = new QThread(this);
    m_updater = new Updater(m_cache);
    m_updater->moveToThread(m_worker);
    m_worker->start();

    // Cross-thread signal connections (auto -> QueuedConnection across threads)
    connect(m_updater, &Updater::log,             this, &MainWindow::appendLog);
    connect(m_updater, &Updater::coreProgressMax, this, &MainWindow::setCoreProgMax);
    connect(m_updater, &Updater::coreProgressInc, this, &MainWindow::incCoreProgress);
    connect(m_updater, &Updater::stepProgressMax, this, &MainWindow::setStepProgMax);
    connect(m_updater, &Updater::stepProgressInc, this, &MainWindow::incStepProgress);
    connect(m_updater, &Updater::operationDone,   this, &MainWindow::onOperationDone);

    buildUi();
    setStyleSheet(kStyle);
    loadSettings();
    updateWindowTitle();

    resize(760, 740);
    setWindowIcon(QIcon(":/icons/ulc.png"));
}

MainWindow::~MainWindow()
{
    m_cancel = true;
    m_worker->quit();
    m_worker->wait(6000);
    delete m_updater;
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    saveSettings();
    m_cancel = true;
    e->accept();
}

// ─────────────────────────────────────────────────────────────────────────────
// UI Construction helpers
// ─────────────────────────────────────────────────────────────────────────────

static QLabel *lbl(const QString &t)
{
    auto *l = new QLabel(t);
    l->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    return l;
}

static QPushButton *browseBtn()
{
    auto *b = new QPushButton("Browse");
    b->setFixedWidth(72);
    return b;
}

void MainWindow::buildUi()
{
    auto *central = new QWidget;
    setCentralWidget(central);
    auto *root = new QVBoxLayout(central);
    root->setSpacing(6);
    root->setContentsMargins(8, 8, 8, 6);

    // ── Paths group ───────────────────────────────────────────────────────────
    {
        auto *grp  = new QGroupBox("Paths");
        auto *grid = new QGridLayout(grp);
        grid->setColumnStretch(1, 1);
        grid->setSpacing(4);

        m_corePath   = new QLineEdit;
        m_assetsPath = new QLineEdit;
        m_infoPath   = new QLineEdit;
        m_dbPath     = new QLineEdit;
        m_raPath     = new QLineEdit;

        struct R { const char *label; QLineEdit *field; void (MainWindow::*slot)(); };
        R rows[] = {
            { "Core Location:",      m_corePath,   &MainWindow::onBrowseCore      },
            { "Assets Location:",    m_assetsPath, &MainWindow::onBrowseAssets    },
            { "Info Location:",      m_infoPath,   &MainWindow::onBrowseInfo      },
            { "Database Location:",  m_dbPath,     &MainWindow::onBrowseDatabase  },
            { "RetroArch Location:", m_raPath,     &MainWindow::onBrowseRetroarch },
        };

        for (int i = 0; i < 5; ++i) {
            auto *btn = browseBtn();
            connect(btn, &QPushButton::clicked, this, rows[i].slot);
            grid->addWidget(lbl(rows[i].label), i, 0);
            grid->addWidget(rows[i].field,       i, 1);
            grid->addWidget(btn,                 i, 2);
        }
        root->addWidget(grp);
    }

    // ── Update actions group ──────────────────────────────────────────────────
    {
        auto *grp  = new QGroupBox("Update Actions");
        auto *grid = new QGridLayout(grp);
        grid->setSpacing(4);

        m_btnCores  = new QPushButton("Update libretro Cores");
        m_btnAssets = new QPushButton("Update libretro Assets");
        m_btnInfo   = new QPushButton("Update Core Info");
        m_btnDb     = new QPushButton("Update libretro Database");
        m_btnRa     = new QPushButton("Update RetroArch");
        m_btnAll    = new QPushButton("Update Everything");

        for (int c = 0; c < 3; ++c) grid->setColumnStretch(c, 1);

        grid->addWidget(m_btnCores,  0, 0);
        grid->addWidget(m_btnAssets, 0, 1);
        grid->addWidget(m_btnInfo,   0, 2);
        grid->addWidget(m_btnDb,     1, 0);
        grid->addWidget(m_btnRa,     1, 1);
        grid->addWidget(m_btnAll,    1, 2);

        connect(m_btnCores,  &QPushButton::clicked, this, &MainWindow::onUpdateCores);
        connect(m_btnAssets, &QPushButton::clicked, this, &MainWindow::onUpdateAssets);
        connect(m_btnInfo,   &QPushButton::clicked, this, &MainWindow::onUpdateCoreInfo);
        connect(m_btnDb,     &QPushButton::clicked, this, &MainWindow::onUpdateDatabase);
        connect(m_btnRa,     &QPushButton::clicked, this, &MainWindow::onUpdateRetroarch);
        connect(m_btnAll,    &QPushButton::clicked, this, &MainWindow::onUpdateAll);

        root->addWidget(grp);
    }

    // ── Progress group ────────────────────────────────────────────────────────
    {
        auto *grp  = new QGroupBox("Progress");
        auto *grid = new QGridLayout(grp);
        grid->setColumnStretch(1, 1);
        grid->setSpacing(4);

        m_coreBar    = new QProgressBar;
        m_stepBar    = new QProgressBar;
        m_overallBar = new QProgressBar;

        grid->addWidget(lbl("Core progress:"),    0, 0); grid->addWidget(m_coreBar,    0, 1);
        grid->addWidget(lbl("Step progress:"),    1, 0); grid->addWidget(m_stepBar,    1, 1);
        grid->addWidget(lbl("Update All steps:"), 2, 0); grid->addWidget(m_overallBar, 2, 1);

        root->addWidget(grp);
    }

    // ── Log group ─────────────────────────────────────────────────────────────
    {
        auto *grp = new QGroupBox("Log Output");
        auto *lay = new QVBoxLayout(grp);
        m_log = new QTextEdit;
        m_log->setReadOnly(true);
        lay->addWidget(m_log);
        root->addWidget(grp, 1);
    }

    // ── Bottom bar ────────────────────────────────────────────────────────────
    {
        m_btnSave  = new QPushButton("Save Paths");
        m_btnStop  = new QPushButton("Stop");
        m_btnAbout = new QPushButton("About");

        m_btnStop->setProperty("danger", true);
        m_btnAbout->setProperty("danger", true);

        // Qt6: dynamic properties set before show() need a style refresh
        for (auto *b : { m_btnStop, m_btnAbout }) {
            b->style()->unpolish(b);
            b->style()->polish(b);
        }

        auto *bar = new QHBoxLayout;
        bar->addStretch();
        bar->addWidget(m_btnSave);
        bar->addWidget(m_btnStop);
        bar->addWidget(m_btnAbout);
        root->addLayout(bar);

        connect(m_btnSave,  &QPushButton::clicked, this, &MainWindow::onSavePaths);
        connect(m_btnStop,  &QPushButton::clicked, this, &MainWindow::onStop);
        connect(m_btnAbout, &QPushButton::clicked, this, &MainWindow::onAbout);
    }

    setButtonsEnabled(true);
}

// ─────────────────────────────────────────────────────────────────────────────
// Logging
// ─────────────────────────────────────────────────────────────────────────────

void MainWindow::appendLog(const QString &msg)
{
    // Qt6-compatible log trimming: explicitly move to EndOfBlock before extending
    QTextDocument *doc = m_log->document();
    while (doc->blockCount() > Constants::MaxLogLines) {
        QTextCursor cur(doc->begin());
        cur.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        cur.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
        cur.removeSelectedText();
    }

    m_log->append(QDateTime::currentDateTime().toString("HH:mm:ss") + " - " + msg);
    m_log->verticalScrollBar()->setValue(m_log->verticalScrollBar()->maximum());
}

// ─────────────────────────────────────────────────────────────────────────────
// Progress slots
// ─────────────────────────────────────────────────────────────────────────────

void MainWindow::setCoreProgMax(int max)
{
    m_coreBar->setMaximum(max);
    m_coreBar->setValue(0);
}

void MainWindow::incCoreProgress()
{
    if (m_coreBar->value() < m_coreBar->maximum())
        m_coreBar->setValue(m_coreBar->value() + 1);
}

void MainWindow::setStepProgMax(int max)
{
    m_stepBar->setMaximum(max); // 0 = indeterminate animation in Qt
    m_stepBar->setValue(0);
}

void MainWindow::incStepProgress()
{
    if (m_stepBar->maximum() > 0 && m_stepBar->value() < m_stepBar->maximum())
        m_stepBar->setValue(m_stepBar->value() + 1);
}

void MainWindow::onOperationDone()
{
    ++m_overallDone;
    if (m_overallSteps > 0)
        m_overallBar->setValue(qMin(m_overallDone, m_overallSteps));

    if (m_overallDone >= m_overallSteps) {
        m_running = false;
        setButtonsEnabled(true);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Operation runner
// ─────────────────────────────────────────────────────────────────────────────

void MainWindow::runOperation(std::function<void(std::atomic<bool>&)> fn, int steps)
{
    if (m_running.exchange(true)) {
        appendLog("Another operation is already running.");
        return;
    }

    m_cancel       = false;
    m_overallSteps = steps;
    m_overallDone  = 0;
    m_overallBar->setMaximum(steps);
    m_overallBar->setValue(0);

    setButtonsEnabled(false);

    QMetaObject::invokeMethod(m_updater,
        [this, fn = std::move(fn)]() mutable { fn(m_cancel); },
        Qt::QueuedConnection);
}

// ─────────────────────────────────────────────────────────────────────────────
// Button handlers
// ─────────────────────────────────────────────────────────────────────────────

void MainWindow::onUpdateCores()
{
    const QString dir = m_corePath->text();
    runOperation([this, dir](std::atomic<bool> &cancel) {
        m_updater->updateCores(dir, cancel);
    }, 1);
}

void MainWindow::onUpdateAssets()
{
    const QString dir = m_assetsPath->text();
    runOperation([this, dir](std::atomic<bool> &cancel) {
        m_updater->updateZip(Constants::AssetsUrl, dir, "Assets", cancel);
    }, 1);
}

void MainWindow::onUpdateCoreInfo()
{
    const QString dir = m_infoPath->text();
    runOperation([this, dir](std::atomic<bool> &cancel) {
        m_updater->updateZip(Constants::InfoUrl, dir, "Core info", cancel);
    }, 1);
}

void MainWindow::onUpdateDatabase()
{
    const QString dir = m_dbPath->text();
    runOperation([this, dir](std::atomic<bool> &cancel) {
        m_updater->updateZip(Constants::DatabaseUrl, dir, "Database", cancel);
    }, 1);
}

void MainWindow::onUpdateRetroarch()
{
    const QString dir = m_raPath->text();
    runOperation([this, dir](std::atomic<bool> &cancel) {
        m_updater->update7z(Constants::RetroarchUrl, dir, cancel);
        QMetaObject::invokeMethod(this, &MainWindow::updateWindowTitle,
                                  Qt::QueuedConnection);
    }, 1);
}

void MainWindow::onUpdateAll()
{
    const QString coreDir = m_corePath->text();
    const QString assDir  = m_assetsPath->text();
    const QString infoDir = m_infoPath->text();
    const QString dbDir   = m_dbPath->text();
    const QString raDir   = m_raPath->text();

    runOperation([this, coreDir, assDir, infoDir, dbDir, raDir]
                 (std::atomic<bool> &cancel)
    {
        emit m_updater->log("Starting full update...");
        m_updater->updateCores(coreDir, cancel);
        if (cancel.load()) return;
        m_updater->updateZip(Constants::AssetsUrl,   assDir,  "Assets",    cancel);
        if (cancel.load()) return;
        m_updater->updateZip(Constants::InfoUrl,     infoDir, "Core info", cancel);
        if (cancel.load()) return;
        m_updater->updateZip(Constants::DatabaseUrl, dbDir,   "Database",  cancel);
        if (cancel.load()) return;
        m_updater->update7z(Constants::RetroarchUrl, raDir, cancel);
        QMetaObject::invokeMethod(this, [this]() {
            appendLog("Full update completed.");
            updateWindowTitle();
        }, Qt::QueuedConnection);
    }, 5);
}

void MainWindow::onStop()
{
    if (!m_running.load()) { appendLog("No running operation to cancel."); return; }
    m_cancel = true;
    m_btnStop->setEnabled(false);
    appendLog("Cancellation requested. Current operation will stop soon.");
}

void MainWindow::onSavePaths() { saveSettings(); }

void MainWindow::onAbout() { AboutDialog(this).exec(); }

// ─────────────────────────────────────────────────────────────────────────────
// Browse
// ─────────────────────────────────────────────────────────────────────────────

QString MainWindow::browseFolder(const QString &current)
{
    return QFileDialog::getExistingDirectory(this, "Select folder", current);
}

void MainWindow::onBrowseCore()
{
    const auto p = browseFolder(m_corePath->text());
    if (!p.isEmpty()) m_corePath->setText(p + "/");
}
void MainWindow::onBrowseAssets()
{
    const auto p = browseFolder(m_assetsPath->text());
    if (!p.isEmpty()) m_assetsPath->setText(p + "/");
}
void MainWindow::onBrowseInfo()
{
    const auto p = browseFolder(m_infoPath->text());
    if (!p.isEmpty()) m_infoPath->setText(p + "/");
}
void MainWindow::onBrowseDatabase()
{
    const auto p = browseFolder(m_dbPath->text());
    if (!p.isEmpty()) m_dbPath->setText(p + "/");
}
void MainWindow::onBrowseRetroarch()
{
    const auto p = browseFolder(m_raPath->text());
    if (!p.isEmpty()) { m_raPath->setText(p + "/"); updateWindowTitle(); }
}

// ─────────────────────────────────────────────────────────────────────────────
// Settings
// ─────────────────────────────────────────────────────────────────────────────

void MainWindow::loadSettings()
{
    const AppSettings s = m_settings->load();
    m_corePath->setText(s.corePath);
    m_assetsPath->setText(s.assetsPath);
    m_infoPath->setText(s.infoPath);
    m_dbPath->setText(s.databasePath);
    m_raPath->setText(s.retroarchPath);
    appendLog("Settings loaded.");
}

void MainWindow::saveSettings()
{
    AppSettings s;
    s.corePath      = m_corePath->text();
    s.assetsPath    = m_assetsPath->text();
    s.infoPath      = m_infoPath->text();
    s.databasePath  = m_dbPath->text();
    s.retroarchPath = m_raPath->text();
    m_settings->save(s);
    appendLog("Settings saved.");
}

// ─────────────────────────────────────────────────────────────────────────────
// Window title (reads version from retroarch.exe / retroarch binary)
// ─────────────────────────────────────────────────────────────────────────────

//void MainWindow::updateWindowTitle()
//{
//    const QString dir = m_raPath->text();
//    const QString exe = dir + "retroarch"
//#ifdef Q_OS_WIN
//        ".exe"
//#endif
//        ;
//
//    QString ver;
//
//#ifdef Q_OS_WIN
//    DWORD dummy = 0;
//    const DWORD sz = GetFileVersionInfoSizeW(exe.toStdWString().c_str(), &dummy);
//    if (sz) {
//        std::vector<BYTE> buf(sz);
//        if (GetFileVersionInfoW(exe.toStdWString().c_str(), 0, sz, buf.data())) {
//            VS_FIXEDFILEINFO *fi = nullptr;
//            UINT len = 0;
//            if (VerQueryValueW(buf.data(), L"\\",
//                               reinterpret_cast<LPVOID*>(&fi), &len) && fi) {
//                ver = QString("v%1.%2.%3.%4")
//                    .arg(HIWORD(fi->dwFileVersionMS))
//                    .arg(LOWORD(fi->dwFileVersionMS))
//                    .arg(HIWORD(fi->dwFileVersionLS))
//                    .arg(LOWORD(fi->dwFileVersionLS));
//            }
//        }
//    }
//#else
//    {
//        QProcess p;
//        p.start(exe, {"--version"});
//        if (p.waitForFinished(2000)) {
//            const QString out = p.readAllStandardOutput() + p.readAllStandardError();
//            // Qt6: use QRegularExpression (QRegExp was removed)
//            const QRegularExpression rx(R"((\d+\.\d+[\d.]*))");
//            const auto m = rx.match(out);
//            if (m.hasMatch()) ver = "v" + m.captured(1);
//        }
//    }
//#endif
//
//    // Parse optional buildbot date from retroarch.VERSION
//    QString date;
//    QFile vf(dir + "retroarch.VERSION");
//    if (vf.open(QIODevice::ReadOnly)) {
//        const QString content = QString::fromUtf8(vf.readAll());
//        for (const QString &line : content.split('\n')) {
//            if (!line.contains("RetroArch", Qt::CaseInsensitive)) continue;
//            const QRegularExpression dateRx(R"(\b(\d{4}-\d{2}-\d{2})\b)");
//            const auto dm = dateRx.match(line);
//            if (dm.hasMatch()) { date = dm.captured(1); break; }
//        }
//    }
//
//    if (!ver.isEmpty()) {
//        setWindowTitle(date.isEmpty()
//            ? QString("libretro Updater - RetroArch %1 (buildbot)").arg(ver)
//            : QString("libretro Updater - RetroArch %1 (%2 buildbot)").arg(ver, date));
//    } else {
//        setWindowTitle("libretro Updater - RetroArch (version unavailable)");
//    }
//}

void MainWindow::updateWindowTitle()
{
    const QString dir = m_raPath->text();
    const QString exe = dir + "retroarch"
#ifdef Q_OS_WIN
        ".exe"
#endif
        ;

    QString ver;

    // ── Try reading version from the exe binary ───────────────────────────────
#ifdef Q_OS_WIN
    DWORD dummy = 0;
    const DWORD sz = GetFileVersionInfoSizeW(exe.toStdWString().c_str(), &dummy);
    if (sz) {
        std::vector<BYTE> buf(sz);
        if (GetFileVersionInfoW(exe.toStdWString().c_str(), 0, sz, buf.data())) {
            VS_FIXEDFILEINFO* fi = nullptr;
            UINT len = 0;
            if (VerQueryValueW(buf.data(), L"\\",
                reinterpret_cast<LPVOID*>(&fi), &len) && fi) {
                ver = QString("v%1.%2.%3.%4")
                    .arg(HIWORD(fi->dwFileVersionMS))
                    .arg(LOWORD(fi->dwFileVersionMS))
                    .arg(HIWORD(fi->dwFileVersionLS))
                    .arg(LOWORD(fi->dwFileVersionLS));
            }
        }
    }
#else
    {
        QProcess p;
        p.start(exe, { "--version" });
        if (p.waitForFinished(2000)) {
            const QString out = p.readAllStandardOutput() + p.readAllStandardError();
            const QRegularExpression rx(R"((\d+\.\d+[\d.]*))");
            const auto m = rx.match(out);
            if (m.hasMatch()) ver = "v" + m.captured(1);
        }
    }
#endif

    // ── Try reading a build date from retroarch.VERSION on disk ───────────────
    QString date;
    QFile vf(dir + "retroarch.VERSION");
    if (vf.open(QIODevice::ReadOnly)) {
        const QString content = QString::fromUtf8(vf.readAll());
        for (const QString& line : content.split('\n')) {
            if (!line.contains("RetroArch", Qt::CaseInsensitive)) continue;
            const QRegularExpression dateRx(R"(\b(\d{4}-\d{2}-\d{2})\b)");
            const auto dm = dateRx.match(line);
            if (dm.hasMatch()) { date = dm.captured(1); break; }
        }
    }

    // ── Try pulling build date from the buildbot index file ───────────────────
    // The nightly index lists filenames like RetroArch_20240315.7z — we parse
    // the date out of that. This is a quick synchronous fetch; it's only called
    // on startup and after a RetroArch update so the brief block is acceptable.
    if (date.isEmpty()) {
        QNetworkAccessManager nam;
        QNetworkRequest req;
        req.setUrl(QUrl(Constants::BuildbotBase + Constants::PlatformSegment +
            "/latest/.index-extended"));
        req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
            QVariant::fromValue(QNetworkRequest::NoLessSafeRedirectPolicy));

        QEventLoop loop;
        QString indexBody;

        QNetworkReply* reply = nam.get(req);
        connect(reply, &QNetworkReply::finished, &loop, [&]() {
            if (reply->error() == QNetworkReply::NoError)
                indexBody = QString::fromUtf8(reply->readAll());
            reply->deleteLater();
            loop.quit();
            });

        // 5-second timeout so a dead network doesn't hang the title update
        QTimer::singleShot(5000, &loop, &QEventLoop::quit);
        loop.exec();

        if (!indexBody.isEmpty()) {
            // Lines look like: "RetroArch_20240315.7z  <size>  <hash>"
            const QRegularExpression idxRx(R"(RetroArch_(\d{8}))");
            const auto im = idxRx.match(indexBody);
            if (im.hasMatch()) {
                const QString raw = im.captured(1); // "20240315"
                const QDate d = QDate::fromString(raw, "yyyyMMdd");
                if (d.isValid())
                    date = d.toString("yyyy-MM-dd");
            }
        }
    }

    // ── Compose title — omit version/date entirely if nothing was found ────────
    if (!ver.isEmpty() && !date.isEmpty())
        setWindowTitle(QString("libretro Updater - RetroArch %1 (%2)").arg(ver, date));
    else if (!ver.isEmpty())
        setWindowTitle(QString("libretro Updater - RetroArch %1").arg(ver));
    else if (!date.isEmpty())
        setWindowTitle(QString("libretro Updater - RetroArch (%1)").arg(date));
    else
        setWindowTitle("libretro Updater");
}

// ─────────────────────────────────────────────────────────────────────────────
// Enable / disable buttons
// ─────────────────────────────────────────────────────────────────────────────

void MainWindow::setButtonsEnabled(bool on)
{
    for (auto *b : { m_btnCores, m_btnAssets, m_btnInfo,
                     m_btnDb, m_btnRa, m_btnAll, m_btnSave })
        b->setEnabled(on);

    m_btnStop->setEnabled(!on);
}
