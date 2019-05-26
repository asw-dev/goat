#ifndef HTMLSTYLEDELEGATE_H
#define HTMLSTYLEDELEGATE_H

#include <QStyledItemDelegate>

class HtmlStyleDelegate : public QStyledItemDelegate
{
protected:
    void paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
    QSize sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const;
};

#endif // HTMLSTYLEDELEGATE_H
