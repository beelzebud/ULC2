#pragma once
#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QProgressBar>
#include <QTextEdit>
#include <QThread>
#include <atomic>
#include "settings.h"
#include "etag_cache.h"
#include "updater.h"

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
    void onUpdateCores();
    void onUpdateAssets();
    void onUpdateCoreInfo();
    void onUpdateDatabase();
    void onUpdateRetroarch();
    void onUpdateAll();

    void onBrowseCore();
    void onBrowseAssets();
    void onBrowseInfo();
    void onBrowseDatabase();
    void onBrowseRetroarch();

    void appendLog(const QString& msg);
    void setCoreProgMax(int max);
    void incCoreProgress();
    void setStepProgMax(int max);
    void incStepProgress();
    void onOperationDone();

private:
    void buildUi();
    void setButtonsEnabled(bool on);
    void updateTabTitle();
    void runOperation(std::function<void(std::atomic<bool>&)> fn, int steps);
    QString browseFolder(const QString& current);

    QLineEdit* m_corePath, * m_assetsPath, * m_infoPath, * m_dbPath, * m_raPath;
    QPushButton* m_btnCores, * m_btnAssets, * m_btnInfo;
    QPushButton* m_btnDb, * m_btnRa, * m_btnAll, * m_btnStop;
    QProgressBar* m_coreBar, * m_stepBar, * m_overallBar;
    QTextEdit* m_log;

    Updater* m_updater;
    QThread* m_worker;
    std::atomic<bool>  m_cancel{ false };
    std::atomic<bool>  m_running{ false };
    int  m_overallSteps = 0;
    int  m_overallDone = 0;
};