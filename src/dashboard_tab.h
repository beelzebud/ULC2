#pragma once

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QTreeWidget>
#include <QProgressBar>
#include <QList>
#include <functional>

#include "emulator_tab.h"
#include "retroarch_tab.h"

struct DashboardJob {
    QString                id;
    QString                displayName;
    std::function<void()>  start;
    std::function<void()>  stop;
};

class DashboardTab : public QWidget
{
    Q_OBJECT
public:
    explicit DashboardTab(const QList<EmulatorTab*>& tabs,
        RetroArchTab* raTab,
        QWidget* parent = nullptr);

private slots:
    void onCheckAll();
    void onUpdateAll();
    void onStopAll();
    void onCheckCores();
    void onUpdateCores();
    void advanceQueue();

private:
    void buildUi();
    void setStatus(const QString& id, const QString& status);
    void setVersion(const QString& id, const QString& version);
    void setButtonsEnabled(bool on);
    void startQueue(bool isUpdate);

    QList<EmulatorTab*> m_tabs;
    RetroArchTab* m_raTab = nullptr;

    QTreeWidget* m_tree = nullptr;
    QPushButton* m_btnCheckAll = nullptr;
    QPushButton* m_btnUpdateAll = nullptr;
    QPushButton* m_btnCheckCores = nullptr;
    QPushButton* m_btnUpdateCores = nullptr;
    QPushButton* m_btnStopAll = nullptr;
    QProgressBar* m_overallBar = nullptr;
    QLabel* m_summaryLabel = nullptr;

    QList<DashboardJob>   m_queue;
    QString               m_currentId;
    std::function<void()> m_currentStop;
    int                   m_queueTotal = 0;
    int                   m_queueDone = 0;
    bool                  m_isUpdateRun = false;
    bool                  m_coresRunning = false;
};