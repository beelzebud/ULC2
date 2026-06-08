#pragma once

#include <QWidget>
#include <QThread>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QTextEdit>
#include <QProgressBar>
#include <QStringList>
#include <atomic>

#include "settings.h"
#include "etag_cache.h"

class RetroArchWorker;

class RetroArchTab : public QWidget
{
    Q_OBJECT
public:
    explicit RetroArchTab(EtagCache* cache, QWidget* parent = nullptr);
    ~RetroArchTab() override;

    void applySettings(const AppSettings& s);
    void collectSettings(AppSettings& s) const;

public slots:
    void stopOperation();

private slots:
    void onCheckRA();
    void onDownloadRA();
    void onCheckCores();
    void onDownloadCores();
    void onBrowseRA();
    void onBrowseCores();
    void appendLog(const QString& msg);
    void setProgMax(int max);
    void incProgress();
    void onWorkerDone();
    void onRACheckResult(bool hasUpdate);
    void onCoresCheckResult(const QStringList& needsUpdate, int total);

private:
    void buildUi();
    void setButtonsEnabled(bool on);

    QLineEdit* m_raPathEdit = nullptr;
    QLineEdit* m_corePathEdit = nullptr;
    QPushButton* m_btnCheckRA = nullptr;
    QPushButton* m_btnDownloadRA = nullptr;
    QPushButton* m_btnCheckCores = nullptr;
    QPushButton* m_btnDlCores = nullptr;
    QPushButton* m_btnStop = nullptr;
    QLabel* m_raStatusLabel = nullptr;
    QLabel* m_coreStatusLabel = nullptr;
    QProgressBar* m_bar = nullptr;
    QTextEdit* m_log = nullptr;

    RetroArchWorker* m_worker = nullptr;
    QThread* m_thread = nullptr;
    EtagCache* m_cache = nullptr;
    std::atomic<bool> m_cancel{ false };
    std::atomic<bool> m_running{ false };
    QStringList       m_pendingCoreUpdates;
    bool              m_raHasUpdate = false;
};