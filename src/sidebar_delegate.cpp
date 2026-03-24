#include "sidebar_delegate.h"

#include <QPainter>
#include <QStyleOptionViewItem>
#include <QModelIndex>
#include <QPixmap>
#include <QFont>
#include <QFontMetrics>
#include <QColor>
#include <QRect>

SidebarDelegate::SidebarDelegate(QObject* parent)
    : QStyledItemDelegate(parent) {
}

QSize SidebarDelegate::sizeHint(const QStyleOptionViewItem&,
    const QModelIndex&) const
{
    return QSize(ItemWidth, ItemHeight);
}

void SidebarDelegate::paint(QPainter* painter,
    const QStyleOptionViewItem& option,
    const QModelIndex& index) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setRenderHint(QPainter::SmoothPixmapTransform);

    const QRect  rect = option.rect;
    const bool   selected = option.state & QStyle::State_Selected;
    const bool   hovered = option.state & QStyle::State_MouseOver;

    // ?? Background ????????????????????????????????????????????????????????????
    if (selected) {
        painter->fillRect(rect, QColor("#001a00"));
        // Left accent bar
        painter->fillRect(QRect(rect.left(), rect.top(), 3, rect.height()),
            QColor("#00FF00"));
    }
    else if (hovered) {
        painter->fillRect(rect, QColor("#000d00"));
    }
    else {
        painter->fillRect(rect, QColor("#000000"));
    }

    // ?? Thumbnail ?????????????????????????????????????????????????????????????
    const QPixmap pixmap = qvariant_cast<QPixmap>(index.data(Qt::DecorationRole));
    const int thumbY = rect.top() + 10;
    const int thumbX = rect.left() + (rect.width() - ThumbSize) / 2;
    const QRect thumbRect(thumbX, thumbY, ThumbSize, ThumbSize);

    if (!pixmap.isNull()) {
        painter->drawPixmap(thumbRect,
            pixmap.scaled(ThumbSize, ThumbSize,
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation));
    }
    else {
        // Placeholder — draw a simple outlined square with a question mark
        painter->setPen(QPen(QColor("#005500"), 1));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(thumbRect.adjusted(4, 4, -4, -4));
        painter->setPen(QColor("#005500"));
        painter->drawText(thumbRect, Qt::AlignCenter, "?");
    }

    // ?? Primary label ?????????????????????????????????????????????????????????
    const QString name = index.data(Qt::DisplayRole).toString();
    QFont nameFont = option.font;
    nameFont.setPointSize(8);
    nameFont.setBold(selected);
    painter->setFont(nameFont);
    painter->setPen(selected ? QColor("#00FF00") : QColor("#00CC00"));

    const int textTop = thumbY + ThumbSize + TextPadding;
    const QRect nameRect(rect.left() + 4, textTop, rect.width() - 8, 16);
    painter->drawText(nameRect, Qt::AlignHCenter | Qt::AlignTop,
        QFontMetrics(nameFont).elidedText(
            name, Qt::ElideRight, nameRect.width()));

    // ?? Subtitle ??????????????????????????????????????????????????????????????
    const QString sub = index.data(Qt::UserRole).toString();
    if (!sub.isEmpty()) {
        QFont subFont = option.font;
        subFont.setPointSize(7);
        painter->setFont(subFont);
        painter->setPen(selected ? QColor("#00AA00") : QColor("#007700"));

        const QRect subRect(rect.left() + 4, textTop + 17, rect.width() - 8, 14);
        painter->drawText(subRect, Qt::AlignHCenter | Qt::AlignTop,
            QFontMetrics(subFont).elidedText(
                sub, Qt::ElideRight, subRect.width()));
    }

    // ?? Bottom separator ??????????????????????????????????????????????????????
    painter->setPen(QPen(QColor("#001a00"), 1));
    painter->drawLine(rect.bottomLeft(), rect.bottomRight());

    painter->restore();
}