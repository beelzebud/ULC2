#include "aboutdialog.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFont>
#include <QPixmap>

AboutDialog::AboutDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("About");
    setFixedSize(300, 230);
    setStyleSheet("background-color:#000000; color:#00FF00;");

    auto *layout = new QVBoxLayout(this);
    layout->setSpacing(6);
    layout->setContentsMargins(16, 16, 16, 16);

    auto *icon = new QLabel;
    icon->setPixmap(
        QPixmap(":/icons/ulc.png").scaled(64, 64, Qt::KeepAspectRatio,
                                          Qt::SmoothTransformation));
    icon->setAlignment(Qt::AlignCenter);

    auto makeLabel = [](const QString &text, int ptSize = 9, bool bold = false) {
        auto *l = new QLabel(text);
        l->setAlignment(Qt::AlignCenter);
        QFont f = l->font();
        f.setPointSize(ptSize);
        f.setBold(bold);
        f.setFamily("Consolas");
        l->setFont(f);
        return l;
    };

    auto *ok = new QPushButton("OK");
    ok->setFixedWidth(80);
    ok->setStyleSheet(
        "QPushButton { background:#000; color:#00FF00; border:1px solid #00FF00; padding:4px 12px; }"
        "QPushButton:hover { background:#002200; }");
    connect(ok, &QPushButton::clicked, this, &QDialog::accept);

    layout->addWidget(icon);
    layout->addWidget(makeLabel("libretro Updater 2", 14, true));
    layout->addWidget(makeLabel("Qt6 Port - CMake + LZMA SDK"));
    layout->addWidget(makeLabel("(c) 2026 John N. Bilbrey + Claude"));
    layout->addStretch();
    layout->addWidget(ok, 0, Qt::AlignCenter);
}
