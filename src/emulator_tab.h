#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QProgressBar>
#include <QTextEdit>
#include <QLabel>
#include <QComboBox>
#include <QThread>
#include <atomic>

#include "emulator_config.h"
#include "settings.h"
#include "etag_cache.h"
#include "github_updater.h"

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
    QString currentVersion() const { return m_lastKnownTag.isEmpty() ? "unknown" : m_lastKnownTag; }

public slots:
    void stopOperation();
    void onUpdate();
    void onCheckForUpdate();

signals:
    void versionChanged();
    void checkComplete(bool hasUpdate);
    void updateComplete();

private slots:
    void onBrowse();
    void appendLog(const QString& msg);
    void setProgMax(int max);
    void incProgress();
    void onDone(bool updated, const QString& newTag);

private:
    void           buildUi();
    void           setButtonsEnabled(bool on);
    void           updateVersionLabel();
    ReleaseChannel selectedChannel() const;

    EmulatorConfig  m_config;
    QString         m_lastKnownTag;

    QLineEdit* m_pathEdit = nullptr;
    QComboBox* m_channelBox = nullptr;
    QPushButton* m_btnUpdate = nullptr;
    QPushButton* m_btnCheck = nullptr;
    QPushButton* m_btnStop = nullptr;
    QPushButton* m_btnBrowse = nullptr;
    QLabel* m_verLabel = nullptr;
    QProgressBar* m_bar = nullptr;
    QTextEdit* m_log = nullptr;

    GitHubUpdater* m_updater = nullptr;
    QThread* m_worker = nullptr;
    std::atomic<bool>  m_cancel{ false };
    std::atomic<bool>  m_running{ false };
};