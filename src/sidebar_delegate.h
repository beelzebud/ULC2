#pragma once
#include <QStyledItemDelegate>

// Renders each sidebar item as a centred thumbnail with name text below.
// Expects each item to carry:
//   Qt::DecorationRole  -> QPixmap  (the thumbnail)
//   Qt::DisplayRole     -> QString  (primary name)
//   Qt::UserRole        -> QString  (subtitle / platform hint, optional)
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
