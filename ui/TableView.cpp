#include "TableView.h"
#include "ui_TableView.h"

#include "../src/Csv.h"

#include <QApplication>
#include <QClipboard>
#include <QKeyEvent>
#include <QKeySequence>

TableView::TableView(QWidget *parent) : QTableView(parent), ui(new Ui::TableView) { ui->setupUi(this); }

TableView::~TableView() { delete ui; }

void TableView::writeSelectionToClipboard(bool includeHeaders)
{
    QString text = Csv("\t", "\"").writeSelectionToString(model(), selectionModel()->selection(), includeHeaders);
    QApplication::clipboard()->setText(text);
}

void TableView::keyPressEvent(QKeyEvent *event)
{
    if (!selectedIndexes().isEmpty())
    {
        if (event->matches(QKeySequence::Copy))
            writeSelectionToClipboard(false);
        else if (event->modifiers() & Qt::ControlModifier && event->modifiers() & Qt::ShiftModifier && event->key() & Qt::Key::Key_C)
            writeSelectionToClipboard(true);
        else
            QTableView::keyPressEvent(event);
    }
}
