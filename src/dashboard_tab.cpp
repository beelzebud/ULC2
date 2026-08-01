#include "dashboard_tab.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QObject>
#include <QGroupBox>
#include <QHeaderView>
#include <QFont>

DashboardTab::DashboardTab(const QList<EmulatorTab*>& tabs,
    RetroArchTab* raTab,
    QWidget* parent)
    : QWidget(parent), m_tabs(tabs), m_raTab(raTab)
{
    buildUi();

    for (auto* tab : m_tabs) {
        connect(tab, &EmulatorTab::checkComplete, this, [this, tab](bool hasUpdate) {
            if (tab->config().id != m_currentId) return;
            setStatus(tab->config().id, hasUpdate ? "Update available" : "Up to date");
            setVersion(tab->config().id, tab->currentVersion());
            ++m_queueDone;
            m_overallBar->setValue(m_queueDone);
            advanceQueue();
            });
        connect(tab, &EmulatorTab::updateComplete, this, [this, tab]() {
            if (tab->config().id != m_currentId) return;
            setStatus(tab->config().id, "Updated");
            setVersion(tab->config().id, tab->currentVersion());
            ++m_queueDone;
            m_overallBar->setValue(m_queueDone);
            advanceQueue();
            });
    }

    if (m_raTab) {
        connect(m_raTab, &RetroArchTab::binaryCheckFinished, this, [this](bool hasUpdate) {
            if (m_currentId != "retroarch") return;
            setStatus("retroarch", hasUpdate ? "Update available" : "Up to date");
            setVersion("retroarch", m_raTab->currentVersion());
            ++m_queueDone;
            m_overallBar->setValue(m_queueDone);
            advanceQueue();
            });
        connect(m_raTab, &RetroArchTab::binaryUpdateFinished, this, [this]() {
            if (m_currentId != "retroarch") return;
            setStatus("retroarch", "Updated");
            setVersion("retroarch", m_raTab->currentVersion());
            ++m_queueDone;
            m_overallBar->setValue(m_queueDone);
            advanceQueue();
            });

        connect(m_raTab, &RetroArchTab::coresCheckFinished, this, [this](int needCount, int total) {
            m_coresRunning = false;
            setButtonsEnabled(true);
            m_summaryLabel->setText(total == 0
                ? "No cores found."
                : QString("Cores: %1 of %2 have updates available.").arg(needCount).arg(total));
            });
        connect(m_raTab, &RetroArchTab::coresUpdateFinished, this, [this]() {
            m_coresRunning = false;
            setButtonsEnabled(true);
            m_summaryLabel->setText("Core updates complete.");
            });
    }
}

void DashboardTab::buildUi()
{
    auto* root = new QVBoxLayout(this);
    root->setSpacing(8);
    root->setContentsMargins(10, 10, 10, 8);

    auto* title = new QLabel("Emulator Updater  —  Dashboard");
    QFont tf = title->font();
    tf.setFamily("Aldrich");
    tf.setPointSize(13);
    tf.setBold(true);
    title->setFont(tf);
    root->addWidget(title);

    // Bulk actions — RetroArch + emulators
    {
        auto* grp = new QGroupBox("RetroArch + Emulators");
        auto* hl = new QHBoxLayout(grp);

        m_btnCheckAll = new QPushButton("Check All for Updates");
        m_btnUpdateAll = new QPushButton("Update All");

        connect(m_btnCheckAll, &QPushButton::clicked, this, &DashboardTab::onCheckAll);
        connect(m_btnUpdateAll, &QPushButton::clicked, this, &DashboardTab::onUpdateAll);

        hl->addWidget(m_btnCheckAll);
        hl->addWidget(m_btnUpdateAll);
        hl->addStretch();

        root->addWidget(grp);
    }

    // RetroArch cores — separate
    {
        auto* grp = new QGroupBox("RetroArch Cores");
        auto* hl = new QHBoxLayout(grp);

        m_btnCheckCores = new QPushButton("Check Core Updates");
        m_btnUpdateCores = new QPushButton("Update All Cores");

        connect(m_btnCheckCores, &QPushButton::clicked, this, &DashboardTab::onCheckCores);
        connect(m_btnUpdateCores, &QPushButton::clicked, this, &DashboardTab::onUpdateCores);

        hl->addWidget(m_btnCheckCores);
        hl->addWidget(m_btnUpdateCores);
        hl->addStretch();

        root->addWidget(grp);
    }

    // Stop + progress
    {
        auto* hl = new QHBoxLayout;

        m_btnStopAll = new QPushButton("Stop");
        m_btnStopAll->setObjectName("stopBtn");
        m_btnStopAll->setFixedWidth(80);
        m_btnStopAll->setEnabled(false);
        connect(m_btnStopAll, &QPushButton::clicked, this, &DashboardTab::onStopAll);

        m_overallBar = new QProgressBar;
        m_overallBar->setValue(0);

        hl->addWidget(m_btnStopAll);
        hl->addWidget(m_overallBar, 1);
        root->addLayout(hl);
    }

    m_summaryLabel = new QLabel(
        QString("%1 emulator(s) + RetroArch tracked.").arg(m_tabs.size()));
    root->addWidget(m_summaryLabel);

    // Status list
    {
        auto* grp = new QGroupBox("Status");
        auto* lay = new QVBoxLayout(grp);

        m_tree = new QTreeWidget;
        m_tree->setColumnCount(3);
        m_tree->setHeaderLabels({ "Emulator", "Current Version", "Status" });
        m_tree->setRootIsDecorated(false);
        m_tree->setUniformRowHeights(true);
        m_tree->header()->setStretchLastSection(false);
        m_tree->header()->setSectionResizeMode(0, QHeaderView::Stretch);
        m_tree->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
        m_tree->header()->setSectionResizeMode(2, QHeaderView::Fixed);
        m_tree->header()->resizeSection(2, 160);
        m_tree->setStyleSheet(
            "QTreeWidget { background:#000; color:#00FF00; border:1px solid #005500; }"
            "QHeaderView::section { background:#001a00; color:#00FF00; "
            "border:1px solid #003300; padding:4px; }"
            "QTreeWidget::item { padding:3px; }"
            "QTreeWidget::item:selected { background:#003300; color:#00FF00; }"
            "QTreeWidget::item:hover { background:#001a00; }");

        if (m_raTab) {
            auto* raItem = new QTreeWidgetItem(m_tree);
            raItem->setText(0, "RetroArch");
            raItem->setText(1, m_raTab->currentVersion());
            raItem->setText(2, "Not checked");
            raItem->setData(0, Qt::UserRole, "retroarch");
        }

        for (auto* tab : m_tabs) {
            auto* item = new QTreeWidgetItem(m_tree);
            item->setText(0, tab->config().displayName);
            item->setText(1, tab->currentVersion());
            item->setText(2, "Not checked");
            item->setData(0, Qt::UserRole, tab->config().id);
        }

        lay->addWidget(m_tree);
        root->addWidget(grp, 1);
    }
}

void DashboardTab::setStatus(const QString& id, const QString& text)
{
    for (int i = 0; i < m_tree->topLevelItemCount(); ++i) {
        auto* item = m_tree->topLevelItem(i);
        if (item->data(0, Qt::UserRole).toString() == id) {
            item->setText(2, text);
            return;
        }
    }
}

void DashboardTab::setVersion(const QString& id, const QString& version)
{
    for (int i = 0; i < m_tree->topLevelItemCount(); ++i) {
        auto* item = m_tree->topLevelItem(i);
        if (item->data(0, Qt::UserRole).toString() == id) {
            item->setText(1, version);
            return;
        }
    }
}

QString DashboardTab::getStatus(const QString& id) const
{
    for (int i = 0; i < m_tree->topLevelItemCount(); ++i) {
        auto* item = m_tree->topLevelItem(i);
        if (item->data(0, Qt::UserRole).toString() == id)
            return item->text(2);
    }
    return {};
}

void DashboardTab::setButtonsEnabled(bool on)
{
    m_btnCheckAll->setEnabled(on);
    m_btnUpdateAll->setEnabled(on);
    m_btnCheckCores->setEnabled(on);
    m_btnUpdateCores->setEnabled(on);
    m_btnStopAll->setEnabled(!on);
}

void DashboardTab::onCheckAll() { startQueue(false); }
void DashboardTab::onUpdateAll() { startQueue(true); }

void DashboardTab::startQueue(bool isUpdate)
{
    if (!m_queue.isEmpty() || !m_currentId.isEmpty() || m_coresRunning) return;

    m_isUpdateRun = isUpdate;
    m_queue.clear();

    if (m_raTab) {
        DashboardJob job;
        job.id = "retroarch";
        job.displayName = "RetroArch";
        job.start = isUpdate
            ? std::function<void()>([this]() { m_raTab->onDownloadRA(); })
            : std::function<void()>([this]() { m_raTab->onCheckRA(); });
        job.stop = [this]() { m_raTab->stopOperation(); };
        m_queue.append(job);
    }

    for (auto* tab : m_tabs) {
        DashboardJob job;
        job.id = tab->config().id;
        job.displayName = tab->config().displayName;
        job.start = isUpdate
            ? std::function<void()>([tab]() { tab->onUpdate(); })
            : std::function<void()>([tab]() { tab->onCheckForUpdate(); });
        job.stop = [tab]() { tab->stopOperation(); };
        m_queue.append(job);
    }

    m_queueTotal = m_queue.size();
    m_queueDone = 0;
    m_currentId.clear();
    m_currentStop = nullptr;

    m_overallBar->setMaximum(m_queueTotal);
    m_overallBar->setValue(0);
    m_summaryLabel->setText(isUpdate
        ? QString("Updating %1 item(s)...").arg(m_queueTotal)
        : QString("Checking %1 item(s)...").arg(m_queueTotal));

    if (m_raTab) setStatus("retroarch", "Queued");
    for (auto* tab : m_tabs) {
        const QString id = tab->config().id;
        // Only mark as Queued if not already confirmed up to date
        const QString current = getStatus(id);
        if (current != "Up to date")
            setStatus(id, "Queued");
    }

    setButtonsEnabled(false);
    advanceQueue();
}

void DashboardTab::advanceQueue()
{
    m_currentId.clear();
    m_currentStop = nullptr;

    // Skip any queued items already known to be up to date
    while (!m_queue.isEmpty()) {
        const QString status = getStatus(m_queue.first().id);
        if (status == "Up to date") {
            ++m_queueDone;
            m_overallBar->setValue(m_queueDone);
            m_queue.takeFirst();
        }
        else {
            break;
        }
    }

    if (m_queue.isEmpty()) {
        setButtonsEnabled(true);
        m_summaryLabel->setText(
            QString("Done — %1 of %2 processed.").arg(m_queueDone).arg(m_queueTotal));
        return;
    }

    DashboardJob job = m_queue.takeFirst();
    m_currentId = job.id;
    m_currentStop = job.stop;

    setStatus(job.id, m_isUpdateRun ? "Updating..." : "Checking...");
    job.start();
}

void DashboardTab::onStopAll()
{
    if (m_currentStop) m_currentStop();
    if (m_coresRunning && m_raTab) m_raTab->stopOperation();
    m_queue.clear();
    m_summaryLabel->setText("Stopping...");
}

void DashboardTab::onCheckCores()
{
    if (!m_raTab || m_coresRunning || !m_currentId.isEmpty()) return;

    // Only set m_coresRunning if the operation actually starts.
    // onCheckCores returns false if the RA tab is already busy.
    if (!m_raTab->startCheckCores()) return;

    m_coresRunning = true;
    setButtonsEnabled(false);
    m_summaryLabel->setText("Checking RetroArch cores...");
}

void DashboardTab::onUpdateCores()
{
    if (!m_raTab || m_coresRunning || !m_currentId.isEmpty()) return;

    if (!m_raTab->startDownloadCores()) return;

    m_coresRunning = true;
    setButtonsEnabled(false);
    m_summaryLabel->setText("Updating RetroArch cores...");
}