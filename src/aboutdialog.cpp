#include "aboutdialog.h"
#include <QVBoxLayout>
#include <QObject>
#include <QLabel>
#include <QPushButton>
#include <QFont>
#include <QPixmap>
#include <QPainter>
#include <QPaintEvent>
#include <QShowEvent>
#include <QHideEvent>
#include <QMovie>

AboutDialog::AboutDialog(QWidget* parent) : QDialog(parent)
{
    setWindowTitle("About");
    setFixedSize(300, 280);
    // NOTE: no background-color rule here — the global stylesheet's
    // "QDialog { background-color }" would be painted by the style on top of
    // paintEvent's custom backdrop. A per-widget transparent-background rule
    // (same trick BackdropPane uses) lets the dialog paint black + the dimmed
    // GIF itself in paintEvent().
    setStyleSheet(QStringLiteral("%1 { background: transparent; color:#00FF00; }")
        .arg(metaObject()->className()));

    m_movie = new QMovie(QStringLiteral(":/icons/background.gif"),
        QByteArray(), this);
    m_movie->setPaused(true); // no decode cost until shown
    connect(m_movie, &QMovie::frameChanged, this, [this](int) {
        m_scaled = QPixmap(); // invalidate cached scaled frame
        update();
        });

    auto* layout = new QVBoxLayout(this);
    layout->setSpacing(8);
    layout->setContentsMargins(16, 20, 16, 16);
    layout->setAlignment(Qt::AlignHCenter);

    auto* icon = new QLabel;
    icon->setPixmap(
        QPixmap(":/icons/emu_manager.png").scaled(80, 80,
            Qt::KeepAspectRatio, Qt::SmoothTransformation));
    icon->setAlignment(Qt::AlignCenter);

    auto makeLabel = [](const QString& text, int ptSize = 9, bool bold = false) {
        auto* l = new QLabel(text);
        l->setAlignment(Qt::AlignCenter);
        QFont f;
        f.setFamily("Aldrich");
        f.setPointSize(ptSize);
        f.setBold(bold);
        l->setFont(f);
        l->setStyleSheet("color:#00FF00;");
        return l;
        };

    auto* ok = new QPushButton("OK");
    ok->setFixedWidth(80);
    ok->setStyleSheet(
        "QPushButton { background:#000; color:#00FF00; border:1px solid #00FF00;"
        "              padding:4px 12px; font-family:Aldrich; }"
        "QPushButton:hover { background:#002200; }");
    connect(ok, &QPushButton::clicked, this, &QDialog::accept);

    layout->addWidget(icon);
    layout->addSpacing(4);
    layout->addWidget(makeLabel("Emu-Manager", 14, true));
    layout->addWidget(makeLabel(QStringLiteral("v" APP_VERSION)));
    layout->addWidget(makeLabel("Qt6  |  CMake  |  LZMA SDK"));
    layout->addWidget(makeLabel("\u00a9 2026 John N. Bilbrey + Claude"));
    layout->addStretch();
    layout->addWidget(ok, 0, Qt::AlignCenter);
}

// Dimmed, animated GIF behind the dialog content — same treatment as the
// tab panes, but always playing while the dialog is visible.
void AboutDialog::paintEvent(QPaintEvent* event)
{
    QPainter p(this);
    p.fillRect(rect(), Qt::black);
    const QPixmap frame = scaledFrame();
    if (!frame.isNull()) {
        p.setOpacity(0.22);
        p.drawPixmap((width() - frame.width()) / 2,
            (height() - frame.height()) / 2, frame);
        p.setOpacity(1.0);
    }
    p.end();
    QDialog::paintEvent(event);
}

void AboutDialog::showEvent(QShowEvent* event)
{
    m_movie->setPaused(false);
    QDialog::showEvent(event);
}

void AboutDialog::hideEvent(QHideEvent* event)
{
    m_movie->setPaused(true);
    QDialog::hideEvent(event);
}

QPixmap AboutDialog::scaledFrame()
{
    if (!m_scaled.isNull() && m_scaledSize == size())
        return m_scaled;
    m_scaled = m_movie->currentPixmap().scaled(
        size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    m_scaledSize = size();
    return m_scaled;
}