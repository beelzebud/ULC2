#pragma once
#include <QMainWindow>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QProgressBar>
#include <QThread>
#include <atomic>

#include "settings.h"
#include "etag_cache.h"
#include "updater.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent *) override;

private slots:
    void onUpdateCores();
    void onUpdateAssets();
    void onUpdateCoreInfo();
    void onUpdateDatabase();
    void onUpdateRetroarch();
    void onUpdateAll();
    void onStop();
    void onSavePaths();
    void onAbout();

    void onBrowseCore();
    void onBrowseAssets();
    void onBrowseInfo();
    void onBrowseDatabase();
    void onBrowseRetroarch();

    // Cross-thread slots (auto-queued from Updater)
    void appendLog       (const QString &msg);
    void setCoreProgMax  (int max);
    void incCoreProgress ();
    void setStepProgMax  (int max);
    void incStepProgress ();
    void onOperationDone ();

private:
    void buildUi();
    void loadSettings();
    void saveSettings();
    void setButtonsEnabled(bool on);
    void updateWindowTitle();
    void runOperation(std::function<void(std::atomic<bool>&)> fn, int overallSteps);
    QString browseFolder(const QString &current);

    // ── Widgets ───────────────────────────────────────────────────────────────
    QTextEdit    *m_log        = nullptr;
    QLineEdit    *m_corePath   = nullptr;
    QLineEdit    *m_assetsPath = nullptr;
    QLineEdit    *m_infoPath   = nullptr;
    QLineEdit    *m_dbPath     = nullptr;
    QLineEdit    *m_raPath     = nullptr;

    QPushButton  *m_btnCores   = nullptr;
    QPushButton  *m_btnAssets  = nullptr;
    QPushButton  *m_btnInfo    = nullptr;
    QPushButton  *m_btnDb      = nullptr;
    QPushButton  *m_btnRa      = nullptr;
    QPushButton  *m_btnAll     = nullptr;
    QPushButton  *m_btnStop    = nullptr;
    QPushButton  *m_btnSave    = nullptr;
    QPushButton  *m_btnAbout   = nullptr;

    QProgressBar *m_coreBar    = nullptr;
    QProgressBar *m_stepBar    = nullptr;
    QProgressBar *m_overallBar = nullptr;

    // ── State ─────────────────────────────────────────────────────────────────
    SettingsManager   *m_settings = nullptr;
    EtagCache         *m_cache    = nullptr;
    Updater           *m_updater  = nullptr;
    QThread           *m_worker   = nullptr;

    std::atomic<bool>  m_cancel  { false };
    std::atomic<bool>  m_running { false };
    int                m_overallSteps = 0;
    int                m_overallDone  = 0;
};
