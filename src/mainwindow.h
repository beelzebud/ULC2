#pragma once
#include <QMainWindow>
#include <QTabWidget>
#include "settings.h"
#include "etag_cache.h"
#include "retroarch_tab.h"
#include "emulator_tab.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

protected:
    void closeEvent(QCloseEvent*) override;

private slots:
    void onSavePaths();
    void onAbout();

private:
    void buildUi();
    void loadSettings();
    void saveSettings();

    QTabWidget* m_tabs = nullptr;
    RetroArchTab* m_raTab = nullptr;
    QList<EmulatorTab*> m_emuTabs;

    SettingsManager* m_settings = nullptr;
    EtagCache* m_cache = nullptr;
};