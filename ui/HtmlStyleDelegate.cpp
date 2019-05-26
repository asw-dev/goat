#include "HtmlStyleDelegate.h"

#include <QAbstractTextDocumentLayout>
#include <QApplication>
#include <QPainter>
#include <QTextDocument>

void HtmlStyleDelegate::paint(QPainter* painter, const QStyleOptionViewItem & option, const QModelIndex &index) const
{
    QStyleOptionViewItem options = option;
    initStyleOption(&options, index);

    QStyle *style = options.widget? options.widget->style() : QApplication::style();

    QTextDocument doc;
    doc.setHtml(options.text);
    doc.setTextWidth(options.rect.width());
    doc.setDocumentMargin(0);

    /// Painting item without text
    options.text = QString();
    style->drawControl(QStyle::CE_ItemViewItem, &options, painter);

    QAbstractTextDocumentLayout::PaintContext ctx;

    QRect textRect = style->subElementRect(QStyle::SE_ItemViewItemText, &options);
    painter->save();
    painter->translate(textRect.topLeft());
    painter->setClipRect(textRect.translated(-textRect.topLeft()));
    doc.documentLayout()->draw(painter, ctx);
    painter->restore();
}

QSize HtmlStyleDelegate::sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    QStyleOptionViewItem options = option;
    initStyleOption(&options, index);

    QTextDocument doc;
    doc.setHtml(options.text);
    doc.setTextWidth(options.rect.width());
    doc.setDocumentMargin(0);
    return QSize(doc.idealWidth(), doc.size().height());
}
