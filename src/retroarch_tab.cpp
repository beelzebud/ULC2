#include "retroarch_tab.h"
#include "constants.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QScrollBar>
#include <QFileDialog>
#include <QDateTime>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QVariant>
#include <QEventLoop>
#include <QTimer>
#include <QFile>
#include <QRegularExpression>
#include <QStyleOption>
#include <QTextDocument>
#include <QTextCursor>
#include <QTextBlock>

#ifdef Q_OS_WIN
#  include <windows.h>
#  include <winver.h>
#  include <vector>
#endif

static QLabel* lbl(const QString& t)
{
    auto* l = new QLabel(t);
    l->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    return l;
}

static QPushButton* browseBtn()
{
    auto* b = new QPushButton("Browse");
    b->setFixedWidth(72);
    return b;
}

RetroArchTab::RetroArchTab(EtagCache* cache, QWidget* parent)
    : QWidget(parent)
{
    m_worker = new QThread(this);
    m_updater = new Updater(cache);
    m_updater->moveToThread(m_worker);
    m_worker->start();

    connect(m_updater, &Updater::log, this, &RetroArchTab::appendLog);
    connect(m_updater, &Updater::coreProgressMax, this, &RetroArchTab::setCoreProgMax);
    connect(m_updater, &Updater::coreProgressInc, this, &RetroArchTab::incCoreProgress);
    connect(m_updater, &Updater::stepProgressMax, this, &RetroArchTab::setStepProgMax);
    connect(m_updater, &Updater::stepProgressInc, this, &RetroArchTab::incStepProgress);
    connect(m_updater, &Updater::operationDone, this, &RetroArchTab::onOperationDone);

    buildUi();
}

RetroArchTab::~RetroArchTab()
{
    m_cancel = true;
    m_worker->quit();
    m_worker->wait(6000);
    delete m_updater;
}

void RetroArchTab::applySettings(const AppSettings& s)
{
    m_corePath->setText(s.corePath);
    m_assetsPath->setText(s.assetsPath);
    m_infoPath->setText(s.infoPath);
    m_dbPath->setText(s.databasePath);
    m_raPath->setText(s.retroarchPath);
}

void RetroArchTab::collectSettings(AppSettings& s) const
{
    s.corePath = m_corePath->text();
    s.assetsPath = m_assetsPath->text();
    s.infoPath = m_infoPath->text();
    s.databasePath = m_dbPath->text();
    s.retroarchPath = m_raPath->text();
}

void RetroArchTab::stopOperation()
{
    if (!m_running.load()) return;
    m_cancel = true;
    m_btnStop->setEnabled(false);
    appendLog("Cancellation requested.");
}

void RetroArchTab::buildUi()
{
    auto* root = new QVBoxLayout(this);
    root->setSpacing(6);
    root->setContentsMargins(8, 8, 8, 6);

    // Paths
    {
        auto* grp = new QGroupBox("Paths");
        auto* grid = new QGridLayout(grp);
        grid->setColumnStretch(1, 1);
        grid->setSpacing(4);

        m_corePath = new QLineEdit;
        m_assetsPath = new QLineEdit;
        m_infoPath = new QLineEdit;
        m_dbPath = new QLineEdit;
        m_raPath = new QLineEdit;

        struct R { const char* label; QLineEdit* field; void (RetroArchTab::* slot)(); };
        R rows[] = {
            { "Core Location:",      m_corePath,   &RetroArchTab::onBrowseCore      },
            { "Assets Location:",    m_assetsPath, &RetroArchTab::onBrowseAssets    },
            { "Info Location:",      m_infoPath,   &RetroArchTab::onBrowseInfo      },
            { "Database Location:",  m_dbPath,     &RetroArchTab::onBrowseDatabase  },
            { "RetroArch Location:", m_raPath,     &RetroArchTab::onBrowseRetroarch },
        };
        for (int i = 0; i < 5; ++i) {
            auto* btn = browseBtn();
            connect(btn, &QPushButton::clicked, this, rows[i].slot);
            grid->addWidget(lbl(rows[i].label), i, 0);
            grid->addWidget(rows[i].field, i, 1);
            grid->addWidget(btn, i, 2);
        }
        root->addWidget(grp);
    }

    // Actions
    {
        auto* grp = new QGroupBox("Update Actions");
        auto* grid = new QGridLayout(grp);
        grid->setSpacing(4);
        for (int c = 0; c < 3; ++c) grid->setColumnStretch(c, 1);

        m_btnCores = new QPushButton("Update Cores");
        m_btnAssets = new QPushButton("Update Assets");
        m_btnInfo = new QPushButton("Update Core Info");
        m_btnDb = new QPushButton("Update Database");
        m_btnRa = new QPushButton("Update RetroArch");
        m_btnAll = new QPushButton("Update Everything");
        m_btnStop = new QPushButton("Stop");
        m_btnStop->setObjectName("stopBtn");

        grid->addWidget(m_btnCores, 0, 0);
        grid->addWidget(m_btnAssets, 0, 1);
        grid->addWidget(m_btnInfo, 0, 2);
        grid->addWidget(m_btnDb, 1, 0);
        grid->addWidget(m_btnRa, 1, 1);
        grid->addWidget(m_btnAll, 1, 2);
        grid->addWidget(m_btnStop, 2, 2);

        connect(m_btnCores, &QPushButton::clicked, this, &RetroArchTab::onUpdateCores);
        connect(m_btnAssets, &QPushButton::clicked, this, &RetroArchTab::onUpdateAssets);
        connect(m_btnInfo, &QPushButton::clicked, this, &RetroArchTab::onUpdateCoreInfo);
        connect(m_btnDb, &QPushButton::clicked, this, &RetroArchTab::onUpdateDatabase);
        connect(m_btnRa, &QPushButton::clicked, this, &RetroArchTab::onUpdateRetroarch);
        connect(m_btnAll, &QPushButton::clicked, this, &RetroArchTab::onUpdateAll);
        connect(m_btnStop, &QPushButton::clicked, this, &RetroArchTab::stopOperation);

        root->addWidget(grp);
    }

    // Progress
    {
        auto* grp = new QGroupBox("Progress");
        auto* grid = new QGridLayout(grp);
        grid->setColumnStretch(1, 1);
        grid->setSpacing(4);

        m_coreBar = new QProgressBar;
        m_stepBar = new QProgressBar;
        m_overallBar = new QProgressBar;

        grid->addWidget(lbl("Core progress:"), 0, 0); grid->addWidget(m_coreBar, 0, 1);
        grid->addWidget(lbl("Step progress:"), 1, 0); grid->addWidget(m_stepBar, 1, 1);
        grid->addWidget(lbl("Update All steps:"), 2, 0); grid->addWidget(m_overallBar, 2, 1);
        root->addWidget(grp);
    }

    // Log
    {
        auto* grp = new QGroupBox("Log");
        auto* lay = new QVBoxLayout(grp);
        m_log = new QTextEdit;
        m_log->setReadOnly(true);
        lay->addWidget(m_log);
        root->addWidget(grp, 1);
    }

    setButtonsEnabled(true);
}

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

void RetroArchTab::setCoreProgMax(int max) { m_coreBar->setMaximum(max); m_coreBar->setValue(0); }
void RetroArchTab::incCoreProgress()
{
    if (m_coreBar->value() < m_coreBar->maximum()) m_coreBar->setValue(m_coreBar->value() + 1);
}
void RetroArchTab::setStepProgMax(int max) { m_stepBar->setMaximum(max); m_stepBar->setValue(0); }
void RetroArchTab::incStepProgress()
{
    if (m_stepBar->maximum() > 0 && m_stepBar->value() < m_stepBar->maximum())
        m_stepBar->setValue(m_stepBar->value() + 1);
}

void RetroArchTab::onOperationDone()
{
    ++m_overallDone;
    if (m_overallSteps > 0)
        m_overallBar->setValue(qMin(m_overallDone, m_overallSteps));
    if (m_overallDone >= m_overallSteps) {
        m_running = false;
        setButtonsEnabled(true);
    }
}

void RetroArchTab::runOperation(std::function<void(std::atomic<bool>&)> fn, int steps)
{
    if (m_running.exchange(true)) { appendLog("Already running."); return; }
    m_cancel = false; m_overallSteps = steps; m_overallDone = 0;
    m_overallBar->setMaximum(steps); m_overallBar->setValue(0);
    setButtonsEnabled(false);
    QMetaObject::invokeMethod(m_updater,
        [this, fn = std::move(fn)]() mutable { fn(m_cancel); },
        Qt::QueuedConnection);
}

void RetroArchTab::onUpdateCores()
{
    const QString dir = m_corePath->text();
    runOperation([this, dir](std::atomic<bool>& c) { m_updater->updateCores(dir, c); }, 1);
}
void RetroArchTab::onUpdateAssets()
{
    const QString dir = m_assetsPath->text();
    runOperation([this, dir](std::atomic<bool>& c) {
        m_updater->updateZip(Constants::AssetsUrl, dir, "Assets", c); }, 1);
}
void RetroArchTab::onUpdateCoreInfo()
{
    const QString dir = m_infoPath->text();
    runOperation([this, dir](std::atomic<bool>& c) {
        m_updater->updateZip(Constants::InfoUrl, dir, "Core info", c); }, 1);
}
void RetroArchTab::onUpdateDatabase()
{
    const QString dir = m_dbPath->text();
    runOperation([this, dir](std::atomic<bool>& c) {
        m_updater->updateZip(Constants::DatabaseUrl, dir, "Database", c); }, 1);
}
void RetroArchTab::onUpdateRetroarch()
{
    const QString dir = m_raPath->text();
    runOperation([this, dir](std::atomic<bool>& c) {
        m_updater->update7z(Constants::RetroarchUrl, dir, c); }, 1);
}
void RetroArchTab::onUpdateAll()
{
    const QString cd = m_corePath->text(), ad = m_assetsPath->text(),
        id = m_infoPath->text(), dd = m_dbPath->text(), rd = m_raPath->text();
    runOperation([this, cd, ad, id, dd, rd](std::atomic<bool>& c) {
        emit m_updater->log("Starting full update...");
        m_updater->updateCores(cd, c); if (c.load()) return;
        m_updater->updateZip(Constants::AssetsUrl, ad, "Assets", c); if (c.load()) return;
        m_updater->updateZip(Constants::InfoUrl, id, "Core info", c); if (c.load()) return;
        m_updater->updateZip(Constants::DatabaseUrl, dd, "Database", c); if (c.load()) return;
        m_updater->update7z(Constants::RetroarchUrl, rd, c);
        QMetaObject::invokeMethod(this, [this]() {
            appendLog("Full update completed."); }, Qt::QueuedConnection);
        }, 5);
}

void RetroArchTab::setButtonsEnabled(bool on)
{
    for (auto* b : { m_btnCores,m_btnAssets,m_btnInfo,m_btnDb,m_btnRa,m_btnAll })
        b->setEnabled(on);
    m_btnStop->setEnabled(!on);
}

QString RetroArchTab::browseFolder(const QString& current)
{
    return QFileDialog::getExistingDirectory(this, "Select folder", current);
}

void RetroArchTab::onBrowseCore() { auto p = browseFolder(m_corePath->text());   if (!p.isEmpty()) m_corePath->setText(p + "/"); }
void RetroArchTab::onBrowseAssets() { auto p = browseFolder(m_assetsPath->text()); if (!p.isEmpty()) m_assetsPath->setText(p + "/"); }
void RetroArchTab::onBrowseInfo() { auto p = browseFolder(m_infoPath->text());   if (!p.isEmpty()) m_infoPath->setText(p + "/"); }
void RetroArchTab::onBrowseDatabase() { auto p = browseFolder(m_dbPath->text());     if (!p.isEmpty()) m_dbPath->setText(p + "/"); }
void RetroArchTab::onBrowseRetroarch() { auto p = browseFolder(m_raPath->text());     if (!p.isEmpty()) m_raPath->setText(p + "/"); }