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

    // Installed RetroArch binary version, derived from the cached ETag.
    QString currentVersion() const;

    static const QString RaDownloadUrl;

public slots:
    void stopOperation();
    void onCheckRA();
    void onDownloadRA();
    void onCheckCores();
    void onDownloadCores();

signals:
    void binaryCheckFinished(bool hasUpdate);
    void binaryUpdateFinished();
    void coresCheckFinished(int needCount, int total);
    void coresUpdateFinished();

private slots:
    void onBrowseRA();
    void onBrowseCores();
    void appendLog(const QString& msg);
    void setProgMax(int max);
    void incProgress();
    void onWorkerDone();
    void onRACheckResult(bool hasUpdate, const QString& latestTag);
    void onCoresCheckResult(const QStringList& needsUpdate, int total);

private:
    void buildUi();
    void setButtonsEnabled(bool on);

    enum class RAOp { None, CheckBinary, DownloadBinary, CheckCores, DownloadCores };

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
    int               m_lastCoreTotal = 0;
    bool              m_raHasUpdate = false;
    RAOp              m_currentOp = RAOp::None;
};