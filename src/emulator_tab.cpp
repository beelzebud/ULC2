#include "emulator_tab.h"
#include "constants.h"
#include "launched_process.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QObject>
#include <QGroupBox>
#include <QFileDialog>
#include <QDateTime>
#include <QScrollBar>
#include <QTextDocument>
#include <QTextCursor>
#include <QTextBlock>
#include <QStyleOption>
#include <QRegularExpression>
#include <QProcess>
#include <QFileInfo>
#include <QMessageBox>

static bool isFloatingTag(const QString& tag)
{
    const QStringList floating = {
        "latest-nightly", "latest", "nightly", "preview",
        "canary", "dev", "master", "main", "edge", "pre-release"
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

    m_launcher = new LaunchedProcess(this);
    connect(m_launcher, &LaunchedProcess::runningChanged,
        this, [this](bool running) {
            if (!running) appendLog("Emulator closed.");
            emit launchStateChanged(running);
        });

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
    if (!s.launchPath.isEmpty())
        m_launchPathEdit->setText(s.launchPath);

    m_lastKnownTag = s.lastKnownTag;
    m_lastKnownTagDisplay = s.lastKnownTagDisplay;

    m_channelBox->setCurrentIndex(
        s.channel == ReleaseChannel::Nightly ? 1 : 0);

    updateVersionLabel();
}

void EmulatorTab::collectSettings(EmulatorSettings& s) const
{
    s.installPath = m_pathEdit->text();
    s.launchPath = m_launchPathEdit->text();
    s.lastKnownTag = m_lastKnownTag;
    s.lastKnownTagDisplay = m_lastKnownTagDisplay;
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

    {
        auto* grp = new QGroupBox("Launch");
        auto* grid = new QGridLayout(grp);
        grid->setColumnStretch(1, 1);
        grid->setSpacing(4);

        m_launchPathEdit = new QLineEdit(
            m_config.defaultInstallPath + m_config.exeName);
        m_launchPathEdit->setToolTip(
            "Path to the emulator executable this tab launches.");

        m_btnBrowseExe = new QPushButton("Browse");
        m_btnBrowseExe->setFixedWidth(72);
        connect(m_btnBrowseExe, &QPushButton::clicked,
            this, &EmulatorTab::onBrowseLaunchPath);

        m_btnLaunch = new QPushButton("Launch");
        m_btnLaunch->setFixedWidth(90);
        connect(m_btnLaunch, &QPushButton::clicked,
            this, &EmulatorTab::onLaunch);

        grid->addWidget(new QLabel("Emulator:"), 0, 0);
        grid->addWidget(m_launchPathEdit, 0, 1);
        grid->addWidget(m_btnBrowseExe, 0, 2);
        grid->addWidget(m_btnLaunch, 0, 3);

        root->addWidget(grp);
    }

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

    {
        auto* grp = new QGroupBox("Log / Docs");
        auto* lay = new QVBoxLayout(grp);

        // The pane below shows either the live log or fetched release/repo
        // documentation — these buttons pick which.
        auto* hlay = new QHBoxLayout;
        m_btnLog = new QPushButton("Log");
        m_btnChangelog = new QPushButton("Changelog");
        m_btnReadme = new QPushButton("Readme");
        for (QPushButton* b : { m_btnLog, m_btnChangelog, m_btnReadme }) {
            b->setFixedWidth(96);
            hlay->addWidget(b);
        }
        hlay->addStretch();

        connect(m_btnLog, &QPushButton::clicked,
            this, &EmulatorTab::onShowLog);
        connect(m_btnChangelog, &QPushButton::clicked,
            this, &EmulatorTab::onShowChangelog);
        connect(m_btnReadme, &QPushButton::clicked,
            this, &EmulatorTab::onShowReadme);

        m_btnChangelog->setToolTip(
            "Show the release notes for the newest build");
        m_btnReadme->setToolTip("Show this emulator's README");

        m_log = new LogView;
        m_log->setReadOnly(true);

        m_doc = new LogView;
        m_doc->setReadOnly(true);
        m_doc->setToolTip("Changelog / README appears here");
        m_doc->setPlainText(
            "Click Changelog or Readme above to load it here.\n");

        m_paneStack = new QStackedWidget;
        m_paneStack->addWidget(m_log);
        m_paneStack->addWidget(m_doc);

        lay->addLayout(hlay);
        lay->addWidget(m_paneStack, 1);
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
    m_log->setBusy(true);

    const EmulatorConfig cfg = m_config;
    const ReleaseChannel channel = selectedChannel();

    QMetaObject::invokeMethod(m_updater,
        [this, cfg, channel]() {
            const GitHubRelease r = m_updater->fetchLatestRelease(cfg, channel);
            QMetaObject::invokeMethod(this, [this, r, cfg, channel]() {
                m_btnCheck->setEnabled(true);
                m_log->setBusy(false);

                if (!r.valid) {
                    appendLog("Could not fetch release info.");
                    emit checkComplete(false);
                    return;
                }

                const QString& pattern =
                    (channel == ReleaseChannel::Nightly && !cfg.nightlyAssetPattern.isEmpty())
                    ? cfg.nightlyAssetPattern
                    : cfg.stableAssetPattern;
                const QRegularExpression rx(pattern, QRegularExpression::CaseInsensitiveOption);
                GitHubAsset chosen;
                for (const auto& a : r.assets) {
                    if (rx.match(a.name).hasMatch()) { chosen = a; break; }
                }

                const bool floating = isFloatingTag(r.tagName);
                // Prefer the tag's commit SHA for floating nightlies — the same
                // marker the emulator's own updater compares against.
                const QString storedTag = floating
                    ? (!r.tagSha.isEmpty() ? r.tagSha
                        : !chosen.updatedAt.isEmpty() ? chosen.updatedAt
                        : !r.publishedAt.isEmpty() ? r.publishedAt
                        : chosen.name)
                    : r.tagName;
                const QString displayTag = displayTagFor(r, floating, storedTag)
                    + (r.isPreRelease ? " [pre-release]" : "");

                bool hasUpdate;
                if (m_lastKnownTag.isEmpty()) {
                    appendLog(QString("Latest available: %1").arg(displayTag));
                    hasUpdate = true;
                }
                else if (m_lastKnownTag == storedTag) {
                    appendLog(QString("Up to date (%1).").arg(displayTag));
                    // Backfill the display string for versions recorded before
                    // it was persisted (marker itself is unchanged).
                    if (m_lastKnownTagDisplay.isEmpty())
                        m_lastKnownTagDisplay = displayTag;
                    hasUpdate = false;
                }
                else {
                    appendLog(QString("Update available: %1 -> %2")
                        .arg(m_lastKnownTagDisplay.isEmpty()
                                 ? m_lastKnownTag : m_lastKnownTagDisplay,
                             displayTag));
                    hasUpdate = true;
                }

                m_verLabel->setText(
                    QString("Installed: %1   |   Latest: %2")
                    .arg(m_lastKnownTagDisplay.isEmpty()
                             ? (m_lastKnownTag.isEmpty()
                                    ? QStringLiteral("unknown")
                                    : m_lastKnownTag)
                             : m_lastKnownTagDisplay,
                         displayTag));

                emit checkComplete(hasUpdate);

                }, Qt::QueuedConnection);
        }, Qt::QueuedConnection);
}

void EmulatorTab::onBrowse()
{
    const QString p = QFileDialog::getExistingDirectory(
        this, "Select install folder", m_pathEdit->text());
    if (!p.isEmpty()) m_pathEdit->setText(p + "/");
}

void EmulatorTab::onBrowseLaunchPath()
{
#ifdef Q_OS_WIN
    const QString filter = "Executables (*.exe);;All files (*)";
#else
    const QString filter = "All files (*)";
#endif
    const QString p = QFileDialog::getOpenFileName(
        this, "Select emulator executable", m_launchPathEdit->text(), filter);
    if (!p.isEmpty())
        m_launchPathEdit->setText(QDir::toNativeSeparators(p));
}

bool EmulatorTab::emulatorRunning() const
{
    return m_launcher && m_launcher->isRunning();
}

void EmulatorTab::onLaunch()
{
    const QString path = m_launchPathEdit->text().trimmed();
    if (path.isEmpty()) {
        appendLog("No emulator path set \u2014 click Browse to choose one.");
        return;
    }

    const QFileInfo fi(path);
    if (!fi.exists() || !fi.isFile()) {
        appendLog("Emulator not found: " + path);
        QMessageBox::warning(this, "Emu-Manager",
            QString("Emulator not found:\n%1\n\n"
                    "Set the correct path in the Launch section.")
                .arg(path));
        return;
    }

    QString error;
    if (m_launcher->launch(fi.absoluteFilePath(), &error))
        appendLog("Launched: " + fi.absoluteFilePath());
    else
        appendLog("Launch failed \u2014 " + error);
}

void EmulatorTab::onShowLog()
{
    m_paneStack->setCurrentWidget(m_log);
}

void EmulatorTab::onShowChangelog()
{
    showDoc(true);
}

void EmulatorTab::onShowReadme()
{
    showDoc(false);
}

// Fetch the changelog or README on the updater's worker thread (the network
// manager lives there) and show it in this tab's docs pane.
void EmulatorTab::showDoc(bool changelog)
{
    const QString title = changelog ? "Changelog" : "Readme";

    m_paneStack->setCurrentWidget(m_doc);
    m_doc->setPlainText(QString("Fetching %1 for %2 ...\n")
        .arg(title.toLower(), m_config.displayName));
    m_btnChangelog->setEnabled(false);
    m_btnReadme->setEnabled(false);

    const EmulatorConfig  cfg = m_config;
    const ReleaseChannel  channel = selectedChannel();

    QMetaObject::invokeMethod(m_updater,
        [this, cfg, channel, title, changelog]() {
            const QString text = changelog
                ? m_updater->fetchChangelogText(cfg, channel)
                : m_updater->fetchReadmeText(cfg);

            QMetaObject::invokeMethod(this, [this, title, text]() {
                m_btnChangelog->setEnabled(true);
                m_btnReadme->setEnabled(true);

                if (text.trimmed().isEmpty()) {
                    m_doc->setPlainText(QString(
                        "No %1 available for %2.\n\n"
                        "Some emulators don't publish release notes, or their "
                        "repository isn't readable from Emu-Manager.\n")
                        .arg(title.toLower(), m_config.displayName));
                    appendLog(QString("%1 unavailable.").arg(title));
                    return;
                }

                m_doc->setPlainText(text);
                m_doc->verticalScrollBar()->setValue(0);
                appendLog(QString("%1 loaded (%2 lines).")
                    .arg(title).arg(text.count('\n') + 1));
            }, Qt::QueuedConnection);
        }, Qt::QueuedConnection);
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

void EmulatorTab::onDone(bool updated, const QString& newTag,
    const QString& displayTag)
{
    m_running = false;
    if (updated) {
        m_lastKnownTag = newTag;
        m_lastKnownTagDisplay = displayTag;
        emit versionChanged();
    }
    updateVersionLabel();
    setButtonsEnabled(true);
    emit updateComplete();
}

void EmulatorTab::updateVersionLabel()
{
    const QString ver = m_lastKnownTagDisplay.isEmpty()
        ? (m_lastKnownTag.isEmpty() ? QStringLiteral("unknown")
                                    : m_lastKnownTag)
        : m_lastKnownTagDisplay;
    m_verLabel->setText(QString("Installed: %1").arg(ver));
}

void EmulatorTab::setButtonsEnabled(bool on)
{
    m_log->setBusy(!on); // backdrop animates only while this tab works
    m_btnUpdate->setEnabled(on);
    m_btnCheck->setEnabled(on);
    m_btnBrowse->setEnabled(on);
    m_btnBrowseExe->setEnabled(on);
    m_btnLaunch->setEnabled(on);
    m_btnChangelog->setEnabled(on);
    m_btnReadme->setEnabled(on);
    m_channelBox->setEnabled(on);
    m_btnStop->setEnabled(!on);
}