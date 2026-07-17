#pragma once
#include <QStyledItemDelegate>

class SidebarDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit SidebarDelegate(QObject* parent = nullptr);

    void  paint(QPainter* painter,
        const QStyleOptionViewItem& option,
        const QModelIndex& index) const override;

    QSize sizeHint(const QStyleOptionViewItem& option,
        const QModelIndex& index) const override;

    static constexpr int ThumbSize = 72;
    static constexpr int ItemWidth = 110;
    static constexpr int ItemHeight = 104;
    static constexpr int TextPadding = 4;
};