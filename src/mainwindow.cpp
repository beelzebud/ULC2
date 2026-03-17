#include <QCoreApplication>
#include "mainwindow.h"
#include "aboutdialog.h"
#include "emulator_config.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QCloseEvent>

static const char* kStyle = R"(
* { font-family: Consolas, "Courier New", monospace; font-size: 9pt; }
QMainWindow, QDialog, QWidget { background-color: #000000; color: #00FF00; }
QGroupBox {
    border: 1px solid #00FF00; margin-top: 10px;
    padding: 6px 4px 4px 4px; color: #00FF00;
}
QGroupBox::title { subcontrol-origin: margin; left: 8px; padding: 0 4px; color: #00FF00; }
QLineEdit {
    background: #000000; color: #00FF00;
    border: 1px solid #005500; padding: 1px 4px;
    selection-background-color: #003300;
}
QTextEdit  { background: #000000; color: #00FF00; border: 1px solid #005500; }
QLabel     { color: #00FF00; }
QPushButton {
    background: #000000; color: #00FF00;
    border: 1px solid #00FF00; padding: 4px 8px; min-height: 24px;
}
QPushButton:hover    { background: #001a00; }
QPushButton:pressed  { background: #003300; }
QPushButton:disabled { color: #004400; border-color: #004400; }
QPushButton#stopBtn          { border-color: #FF4444; color: #FF4444; }
QPushButton#stopBtn:hover    { background: #1a0000; }
QPushButton#stopBtn:disabled { color: #440000; border-color: #440000; }
QPushButton#aboutBtn         { border-color: #FF4444; color: #FF4444; }
QPushButton#aboutBtn:hover   { background: #1a0000; }
QProgressBar {
    border: 1px solid #005500; background: #000000;
    color: #00FF00; text-align: center;
}
QProgressBar::chunk { background: #007700; }
QTabWidget::pane    { border: 1px solid #00FF00; }
QTabBar::tab {
    background: #000000; color: #00AA00;
    border: 1px solid #005500;
    padding: 6px 14px; margin-right: 2px;
}
QTabBar::tab:selected  { color: #00FF00; border-color: #00FF00; background: #001a00; }
QTabBar::tab:hover     { background: #001a00; }
QScrollBar:vertical           { background: #0a0a0a; width: 12px; }
QScrollBar::handle:vertical   { background: #005500; min-height: 24px; border-radius: 3px; }
QScrollBar::add-line:vertical,
QScrollBar::sub-line:vertical { height: 0; }
)";

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
    const QString base = QCoreApplication::applicationDirPath();
    m_settings = new SettingsManager(base + "/settings.json");
    m_cache = new EtagCache(base + "/etag-cache");

    buildUi();
    setStyleSheet(kStyle);
    loadSettings();

    resize(820, 780);
    setWindowTitle("Emulator Updater");
    setWindowIcon(QIcon(":/icons/ulc.png"));
}

void MainWindow::closeEvent(QCloseEvent* e)
{
    saveSettings();
    e->accept();
}

void MainWindow::buildUi()
{
    auto* central = new QWidget;
    setCentralWidget(central);
    auto* root = new QVBoxLayout(central);
    root->setSpacing(4);
    root->setContentsMargins(6, 6, 6, 4);

    // Tab widget
    m_tabs = new QTabWidget;

    // RetroArch tab
    m_raTab = new RetroArchTab(m_cache);
    m_tabs->addTab(m_raTab, "RetroArch");

    // Generic emulator tabs
    for (const auto& cfg : allEmulatorConfigs()) {
        auto* tab = new EmulatorTab(cfg, m_cache);
        m_emuTabs.append(tab);
        m_tabs->addTab(tab, cfg.displayName);
    }

    root->addWidget(m_tabs, 1);

    // Bottom bar
    auto* bar = new QHBoxLayout;
    auto* btnSave = new QPushButton("Save Paths");
    auto* btnAbout = new QPushButton("About");
    btnAbout->setObjectName("aboutBtn");

    connect(btnSave, &QPushButton::clicked, this, &MainWindow::onSavePaths);
    connect(btnAbout, &QPushButton::clicked, this, &MainWindow::onAbout);

    bar->addStretch();
    bar->addWidget(btnSave);
    bar->addWidget(btnAbout);
    root->addLayout(bar);
}

void MainWindow::loadSettings()
{
    const AppSettings s = m_settings->load();
    m_raTab->applySettings(s);

    for (auto* tab : m_emuTabs) {
        const QString id = tab->config().id;
        if (s.emulators.contains(id))
            tab->applySettings(s.emulators[id]);
    }
}

void MainWindow::saveSettings()
{
    AppSettings s;
    m_raTab->collectSettings(s);

    for (auto* tab : m_emuTabs) {
        EmulatorSettings es;
        tab->collectSettings(es);
        s.emulators[tab->config().id] = es;
    }

    m_settings->save(s);
}

void MainWindow::onSavePaths() { saveSettings(); }
void MainWindow::onAbout() { AboutDialog(this).exec(); }