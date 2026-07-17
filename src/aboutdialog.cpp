#include "aboutdialog.h"
#include <QVBoxLayout>
#include <QObject>
#include <QLabel>
#include <QPushButton>
#include <QFont>
#include <QPixmap>

AboutDialog::AboutDialog(QWidget* parent) : QDialog(parent)
{
    setWindowTitle("About");
    setFixedSize(300, 260);
    setStyleSheet("background-color:#000000; color:#00FF00;");

    auto* layout = new QVBoxLayout(this);
    layout->setSpacing(8);
    layout->setContentsMargins(16, 20, 16, 16);
    layout->setAlignment(Qt::AlignHCenter);

    auto* icon = new QLabel;
    icon->setPixmap(
        QPixmap(":/icons/ulc.png").scaled(80, 80,
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
    layout->addWidget(makeLabel("Emulator Updater", 14, true));
    layout->addWidget(makeLabel("v2.0"));
    layout->addWidget(makeLabel("Qt6  |  CMake  |  LZMA SDK"));
    layout->addWidget(makeLabel("\u00a9 2026 John N. Bilbrey + Claude"));
    layout->addStretch();
    layout->addWidget(ok, 0, Qt::AlignCenter);
}