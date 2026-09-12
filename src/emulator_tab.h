#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QProgressBar>
#include <QTextEdit>
#include <QLabel>
#include <QStackedWidget>
#include <QComboBox>
#include <QThread>
#include <atomic>

#include "backdrop_pane.h"
#include "emulator_config.h"
#include "settings.h"
#include "etag_cache.h"
#include "github_updater.h"

class LaunchedProcess;

class EmulatorTab : public QWidget
{
    Q_OBJECT
public:
    explicit EmulatorTab(const EmulatorConfig& config,
        EtagCache* cache,
        QWidget* parent = nullptr);
    ~EmulatorTab() override;

    void applySettings(const EmulatorSettings& s);
    void collectSettings(EmulatorSettings& s) const;
    const EmulatorConfig& config() const { return m_config; }

    // True while the emulator started from this tab is still running.
    bool emulatorRunning() const;

    QString currentVersion() const
    {
        return m_lastKnownTagDisplay.isEmpty()
            ? (m_lastKnownTag.isEmpty() ? QStringLiteral("unknown") : m_lastKnownTag)
            : m_lastKnownTagDisplay;
    }

public slots:
    void stopOperation();
    void onUpdate();
    void onCheckForUpdate();
    void onLaunch();

signals:
    void versionChanged();
    void checkComplete(bool hasUpdate);
    void updateComplete();
    void launchStateChanged(bool running);

private slots:
    void onBrowse();
    void onBrowseLaunchPath();
    void onShowChangelog();
    void onShowReadme();
    void onShowLog();
    void appendLog(const QString& msg);
    void setProgMax(int max);
    void incProgress();
    void onDone(bool updated, const QString& newTag,
        const QString& displayTag = QString());

private:
    void           buildUi();
    void           setButtonsEnabled(bool on);
    void           updateVersionLabel();
    void           showDoc(bool changelog);
    ReleaseChannel selectedChannel() const;

    EmulatorConfig  m_config;
    QString         m_lastKnownTag;
    QString         m_lastKnownTagDisplay;

    QLineEdit* m_pathEdit = nullptr;
    QLineEdit* m_launchPathEdit = nullptr;
    QComboBox* m_channelBox = nullptr;
    QPushButton* m_btnUpdate = nullptr;
    QPushButton* m_btnCheck = nullptr;
    QPushButton* m_btnStop = nullptr;
    QPushButton* m_btnBrowse = nullptr;
    QPushButton* m_btnBrowseExe = nullptr;
    QPushButton* m_btnLaunch = nullptr;
    QPushButton* m_btnChangelog = nullptr;
    QPushButton* m_btnReadme = nullptr;
    QPushButton* m_btnLog = nullptr;
    QLabel* m_verLabel = nullptr;
    QProgressBar* m_bar = nullptr;
    QStackedWidget* m_paneStack = nullptr;
    LogView* m_log = nullptr;
    LogView* m_doc = nullptr;

    GitHubUpdater* m_updater = nullptr;
    LaunchedProcess* m_launcher = nullptr;
    QThread* m_worker = nullptr;
    std::atomic<bool>  m_cancel{ false };
    std::atomic<bool>  m_running{ false };
};