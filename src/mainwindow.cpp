#include "mainwindow.h"
#include "aboutdialog.h"
#include "emulator_config.h"
#include "sidebar_delegate.h"
#include <QPixmap>

#include <QApplication>
#include <QCoreApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QCloseEvent>
#include <QSplitter>
#include <QListWidgetItem>

static const char* kStyle = R"(
* { font-family: "Aldrich", Consolas, "Courier New", monospace; font-size: 9pt; }

QMainWindow, QDialog, QWidget {
    background-color: #000000;
    color: #00FF00;
}
QSplitter::handle {
    background-color: #003300;
    width: 2px;
}
QListWidget {
    background-color: #000000;
    color: #00FF00;
    border: none;
    border-right: 1px solid #003300;
    outline: none;
}
QListWidget::item {
    padding: 10px 14px;
    border-bottom: 1px solid #001a00;
    color: #00CC00;
}
QListWidget::item:selected {
    background-color: #001a00;
    color: #00FF00;
    border-left: 3px solid #00FF00;
}
QListWidget::item:hover:!selected {
    background-color: #000d00;
    color: #00FF00;
}
QGroupBox {
    border: 1px solid #00FF00;
    margin-top: 10px;
    padding: 6px 4px 4px 4px;
    color: #00FF00;
}
QGroupBox::title {
    subcontrol-origin: margin;
    left: 8px;
    padding: 0 4px;
    color: #00FF00;
}
QLineEdit {
    background: #000000;
    color: #00FF00;
    border: 1px solid #005500;
    padding: 1px 4px;
    selection-background-color: #003300;
}
QTextEdit {
    background: #000000;
    color: #00FF00;
    border: 1px solid #005500;
}
QLabel { color: #00FF00; }
QPushButton {
    background: #000000;
    color: #00FF00;
    border: 1px solid #00FF00;
    padding: 4px 8px;
    min-height: 24px;
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
    border: 1px solid #005500;
    background: #000000;
    color: #00FF00;
    text-align: center;
}
QProgressBar::chunk { background: #007700; }
QComboBox {
    background: #000;
    color: #00FF00;
    border: 1px solid #005500;
    padding: 2px 6px;
}
QComboBox::drop-down { border: none; }
QComboBox QAbstractItemView {
    background: #000;
    color: #00FF00;
    selection-background-color: #003300;
}
QTreeWidget {
    background-color: #000000;
    color: #00FF00;
}
QScrollBar:vertical           { background: #0a0a0a; width: 12px; }
QScrollBar::handle:vertical   { background: #005500; min-height: 24px; border-radius: 3px; }
QScrollBar::add-line:vertical,
QScrollBar::sub-line:vertical { height: 0; }
QScrollBar:horizontal           { background: #0a0a0a; height: 12px; }
QScrollBar::handle:horizontal   { background: #005500; min-width: 24px; border-radius: 3px; }
QScrollBar::add-line:horizontal,
QScrollBar::sub-line:horizontal { width: 0; }
)";

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
    const QString base = QCoreApplication::applicationDirPath();
    m_settings = new SettingsManager(base + "/settings.json");
    m_cache = new EtagCache(base + "/etag-cache");

    buildUi();
    setStyleSheet(kStyle);
    loadSettings();

    resize(980, 780);
    setWindowTitle("Emulator Updater");
    setWindowIcon(QIcon(":/icons/emu_updater.png"));
}

void MainWindow::closeEvent(QCloseEvent* e)
{
    saveSettings();
    e->accept();
}

// ─────────────────────────────────────────────────────────────────────────────
// UI Construction
// ─────────────────────────────────────────────────────────────────────────────

void MainWindow::buildUi()
{
    auto* central = new QWidget;
    setCentralWidget(central);
    auto* root = new QVBoxLayout(central);
    root->setSpacing(0);
    root->setContentsMargins(0, 0, 0, 0);

    // ── Sidebar ───────────────────────────────────────────────────────────────
    m_sidebar = new QListWidget;
    m_sidebar->setFixedWidth(120);
    m_sidebar->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_sidebar->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_sidebar->setIconSize(QSize(SidebarDelegate::ThumbSize,
        SidebarDelegate::ThumbSize));
    m_sidebar->setItemDelegate(new SidebarDelegate(m_sidebar));
    m_sidebar->setMouseTracking(true);

    m_stack = new QStackedWidget;

    // ── Construct tab objects first (without adding to sidebar yet) ─────────
    m_raTab = new RetroArchTab(m_cache);

    const QList<EmulatorConfig> configs = allEmulatorConfigs();
    for (const auto& cfg : configs) {
        auto* tab = new EmulatorTab(cfg, m_cache);
        m_emuTabs.append(tab);
        connect(tab, &EmulatorTab::versionChanged, this, &MainWindow::saveSettings);
    }

    // Dashboard needs the full emulator tab list to build its status view
    m_dashboard = new DashboardTab(m_emuTabs, m_raTab);

    // ── Register pages in display order: Dashboard, RetroArch, emulators ────
    addPage("Dashboard", "Overview", ":/icons/emu_updater.png", m_dashboard);
    addPage("RetroArch", "Libretro", ":/icons/emulators/retroarch.png", m_raTab);

    for (int i = 0; i < configs.size(); ++i)
        addPage(configs[i].displayName, {}, configs[i].iconResource, m_emuTabs[i]);

    // ── About entry pinned to bottom ──────────────────────────────────────────
    {
        auto* aboutItem = new QListWidgetItem(m_sidebar);
        aboutItem->setData(Qt::DisplayRole, "About");
        aboutItem->setData(Qt::UserRole, "emulator updater");
        aboutItem->setSizeHint(QSize(120, SidebarDelegate::ItemHeight));
        QPixmap pm(":/icons/emu_updater.png");
        if (!pm.isNull())
            aboutItem->setData(Qt::DecorationRole, pm);
        m_aboutRow = m_sidebar->count() - 1;
    }

    m_sidebar->setCurrentRow(0);   // Dashboard is shown on launch

    connect(m_sidebar, &QListWidget::currentRowChanged, this, [this](int row) {
        if (row == m_aboutRow) {
            m_sidebar->blockSignals(true);
            m_sidebar->setCurrentRow(m_stack->currentIndex());
            m_sidebar->blockSignals(false);
            AboutDialog(this).exec();
        }
        else {
            m_stack->setCurrentIndex(row);
        }
        });

    auto* splitter = new QSplitter(Qt::Horizontal);
    splitter->addWidget(m_sidebar);
    splitter->addWidget(m_stack);
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);
    splitter->setHandleWidth(2);
    splitter->setChildrenCollapsible(false);
    splitter->setSizes({ 120, 860 });
    root->addWidget(splitter, 1);

    // ── Bottom bar ────────────────────────────────────────────────────────────
    auto* bottomWidget = new QWidget;
    bottomWidget->setStyleSheet("border-top: 1px solid #003300;");
    auto* bar = new QHBoxLayout(bottomWidget);
    bar->setContentsMargins(8, 4, 8, 4);

    auto* btnSave = new QPushButton("Save Paths");
    auto* btnAbout = new QPushButton("About");
    btnAbout->setObjectName("aboutBtn");

    connect(btnSave, &QPushButton::clicked, this, &MainWindow::onSavePaths);
    connect(btnAbout, &QPushButton::clicked, this, &MainWindow::onAbout);

    bar->addStretch();
    bar->addWidget(btnSave);
    bar->addWidget(btnAbout);
    root->addWidget(bottomWidget);
}

void MainWindow::addPage(const QString& label,
    const QString& subtitle,
    const QString& iconResource,
    QWidget* widget)
{
    auto* item = new QListWidgetItem(m_sidebar);
    item->setData(Qt::DisplayRole, label);
    item->setData(Qt::UserRole, subtitle);
    item->setSizeHint(QSize(120, SidebarDelegate::ItemHeight));

    if (!iconResource.isEmpty()) {
        QPixmap pm(iconResource);
        if (!pm.isNull())
            item->setData(Qt::DecorationRole, pm);
    }

    m_stack->addWidget(widget);
}

// ─────────────────────────────────────────────────────────────────────────────
// Slots
// ─────────────────────────────────────────────────────────────────────────────

void MainWindow::onSavePaths() { saveSettings(); }
void MainWindow::onAbout() { AboutDialog(this).exec(); }

// ─────────────────────────────────────────────────────────────────────────────
// Settings
// ─────────────────────────────────────────────────────────────────────────────

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