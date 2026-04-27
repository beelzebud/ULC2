#include "emulator_tab.h"
#include "constants.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QDateTime>
#include <QScrollBar>
#include <QTextDocument>
#include <QTextCursor>
#include <QTextBlock>
#include <QStyleOption>

static bool isFloatingTag(const QString& tag)
{
    const QStringList floating = {
        "latest-nightly", "latest", "nightly", "preview",
        "canary", "dev", "master", "main", "edge"
    };
    return floating.contains(tag.toLower());
}

EmulatorTab::EmulatorTab(const EmulatorConfig& config,
    EtagCache* cache,
    QWidget* parent)
    : QWidget(parent), m_config(config)
{
    m_worker = new QThread(this);
    m_updater = new GitHubUpdater(cache);
    m_updater->moveToThread(m_worker);
    m_worker->start();

    connect(m_updater, &GitHubUpdater::log, this, &EmulatorTab::appendLog);
    connect(m_updater, &GitHubUpdater::progressMax, this, &EmulatorTab::setProgMax);
    connect(m_updater, &GitHubUpdater::progressInc, this, &EmulatorTab::incProgress);
    connect(m_updater, &GitHubUpdater::done, this, &EmulatorTab::onDone);

    buildUi();
}

EmulatorTab::~EmulatorTab()
{
    m_cancel = true;
    m_worker->quit();
    m_worker->wait(6000);
    delete m_updater;
}

void EmulatorTab::applySettings(const EmulatorSettings& s)
{
    if (!s.installPath.isEmpty())
        m_pathEdit->setText(s.installPath);

    m_lastKnownTag = s.lastKnownTag;

    m_channelBox->setCurrentIndex(
        s.channel == ReleaseChannel::Nightly ? 1 : 0);

    updateVersionLabel();
}

void EmulatorTab::collectSettings(EmulatorSettings& s) const
{
    s.installPath = m_pathEdit->text();
    s.lastKnownTag = m_lastKnownTag;
    s.channel = selectedChannel();
}

ReleaseChannel EmulatorTab::selectedChannel() const
{
    return m_channelBox->currentIndex() == 1
        ? ReleaseChannel::Nightly
        : ReleaseChannel::Stable;
}

void EmulatorTab::stopOperation()
{
    if (!m_running.load()) return;
    m_cancel = true;
    m_btnStop->setEnabled(false);
    appendLog("Cancellation requested.");
}

void EmulatorTab::buildUi()
{
    auto* root = new QVBoxLayout(this);
    root->setSpacing(6);
    root->setContentsMargins(8, 8, 8, 6);

    // Install path + channel selection
    {
        auto* grp = new QGroupBox("Install Location");
        auto* grid = new QGridLayout(grp);
        grid->setColumnStretch(1, 1);
        grid->setSpacing(4);

        m_pathEdit = new QLineEdit(m_config.defaultInstallPath);

        m_channelBox = new QComboBox;
        m_channelBox->addItem("Stable");
        m_channelBox->addItem("Nightly");
        m_channelBox->setFixedWidth(90);
        m_channelBox->setCurrentIndex(
            m_config.defaultChannel == ReleaseChannel::Nightly ? 1 : 0);
        m_channelBox->setStyleSheet(
            "QComboBox { background:#000; color:#00FF00; border:1px solid #005500; padding:2px 6px; }"
            "QComboBox::drop-down { border: none; }"
            "QComboBox QAbstractItemView { background:#000; color:#00FF00; "
            "selection-background-color:#003300; }");

        m_btnBrowse = new QPushButton("Browse");
        m_btnBrowse->setFixedWidth(72);
        connect(m_btnBrowse, &QPushButton::clicked, this, &EmulatorTab::onBrowse);

        grid->addWidget(new QLabel("Path:"), 0, 0);
        grid->addWidget(m_pathEdit, 0, 1);
        grid->addWidget(m_btnBrowse, 0, 2);
        grid->addWidget(new QLabel("Channel:"), 1, 0);
        grid->addWidget(m_channelBox, 1, 1, 1, 2);

        root->addWidget(grp);
    }

    // Version + check
    {
        auto* grp = new QGroupBox("Version");
        auto* hlay = new QHBoxLayout(grp);

        m_verLabel = new QLabel("Installed: unknown");
        m_verLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

        m_btnCheck = new QPushButton("Check for Update");
        m_btnCheck->setFixedWidth(160);
        connect(m_btnCheck, &QPushButton::clicked, this, &EmulatorTab::onCheckForUpdate);

        hlay->addWidget(m_verLabel, 1);
        hlay->addWidget(m_btnCheck);
        root->addWidget(grp);
    }

    // Update + progress
    {
        auto* grp = new QGroupBox("Update");
        auto* vlay = new QVBoxLayout(grp);

        auto* hlay = new QHBoxLayout;
        m_btnUpdate = new QPushButton("Download && Install Latest");
        m_btnStop = new QPushButton("Stop");
        m_btnStop->setObjectName("stopBtn");
        m_btnStop->setFixedWidth(72);

        connect(m_btnUpdate, &QPushButton::clicked, this, &EmulatorTab::onUpdate);
        connect(m_btnStop, &QPushButton::clicked, this, &EmulatorTab::stopOperation);

        hlay->addWidget(m_btnUpdate, 1);
        hlay->addWidget(m_btnStop);

        m_bar = new QProgressBar;
        m_bar->setValue(0);

        vlay->addLayout(hlay);
        vlay->addWidget(m_bar);
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

void EmulatorTab::onUpdate()
{
    if (m_running.exchange(true)) { appendLog("Already running."); return; }

    m_cancel = false;
    m_bar->setValue(0);
    setButtonsEnabled(false);

    const QString        path = m_pathEdit->text();
    const QString        tag = m_lastKnownTag;
    const EmulatorConfig cfg = m_config;
    const ReleaseChannel channel = selectedChannel();

    QMetaObject::invokeMethod(m_updater,
        [this, cfg, path, tag, channel]() mutable {
            m_updater->update(cfg, path, tag, channel, m_cancel);
        }, Qt::QueuedConnection);
}

void EmulatorTab::onCheckForUpdate()
{
    const QString channelLabel =
        selectedChannel() == ReleaseChannel::Nightly ? "nightly" : "stable";

    appendLog(QString("Checking latest %1 release for %2...")
        .arg(channelLabel, m_config.displayName));
    m_btnCheck->setEnabled(false);

    const EmulatorConfig cfg = m_config;
    const ReleaseChannel channel = selectedChannel();

    QMetaObject::invokeMethod(m_updater,
        [this, cfg, channel]() {
            const GitHubRelease r = m_updater->fetchLatestRelease(cfg, channel);
            QMetaObject::invokeMethod(this, [this, r, cfg]() {
                m_btnCheck->setEnabled(true);

                if (!r.valid) {
                    appendLog("Could not fetch release info.");
                    return;
                }

                // Use the same floating tag logic as update() so the
                // comparison key matches what was stored after install.
                const bool floating = isFloatingTag(r.tagName);
                const QString storedTag = floating
                    ? (!r.publishedAt.isEmpty() ? r.publishedAt : r.tagName)
                    : r.tagName;

                const QString preTag = r.isPreRelease ? " [pre-release]" : "";

                // Display label — for floating tags show the tag name,
                // not the publishedAt timestamp, so it's human-readable.
                const QString displayTag = r.tagName + preTag;

                if (m_lastKnownTag.isEmpty()) {
                    appendLog(QString("Latest available: %1").arg(displayTag));
                }
                else if (m_lastKnownTag == storedTag) {
                    appendLog(QString("Up to date (%1).").arg(displayTag));
                }
                else {
                    appendLog(QString("Update available: %1 -> %2")
                        .arg(m_lastKnownTag, displayTag));
                }

                m_verLabel->setText(
                    QString("Installed: %1   |   Latest: %2")
                    .arg(m_lastKnownTag.isEmpty() ? "unknown" : m_lastKnownTag,
                        displayTag));

                }, Qt::QueuedConnection);
        }, Qt::QueuedConnection);
}

void EmulatorTab::onBrowse()
{
    const QString p = QFileDialog::getExistingDirectory(
        this, "Select install folder", m_pathEdit->text());
    if (!p.isEmpty()) m_pathEdit->setText(p + "/");
}

void EmulatorTab::appendLog(const QString& msg)
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

void EmulatorTab::setProgMax(int max)
{
    m_bar->setMaximum(max);
    m_bar->setValue(0);
}

void EmulatorTab::incProgress()
{
    if (m_bar->value() < m_bar->maximum())
        m_bar->setValue(m_bar->value() + 1);
}

void EmulatorTab::onDone(bool updated, const QString& newTag)
{
    m_running = false;
    if (updated) {
        m_lastKnownTag = newTag;
        emit versionChanged();
    }
    updateVersionLabel();
    setButtonsEnabled(true);
}

void EmulatorTab::updateVersionLabel()
{
    m_verLabel->setText(m_lastKnownTag.isEmpty()
        ? "Installed: unknown"
        : QString("Installed: %1").arg(m_lastKnownTag));
}

void EmulatorTab::setButtonsEnabled(bool on)
{
    m_btnUpdate->setEnabled(on);
    m_btnCheck->setEnabled(on);
    m_btnBrowse->setEnabled(on);
    m_channelBox->setEnabled(on);
    m_btnStop->setEnabled(!on);
}