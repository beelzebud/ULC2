#pragma once

#include <QMainWindow>
#include <QListWidget>
#include <QStackedWidget>
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
    void addPage(const QString& label,
        const QString& subtitle,
        const QString& iconResource,
        QWidget* widget);

    QListWidget* m_sidebar = nullptr;
    QStackedWidget* m_stack = nullptr;
    RetroArchTab* m_raTab = nullptr;
    QList<EmulatorTab*> m_emuTabs;
    int                 m_aboutRow = -1;

    SettingsManager* m_settings = nullptr;
    EtagCache* m_cache = nullptr;
};