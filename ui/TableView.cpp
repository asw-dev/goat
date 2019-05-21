#include "TableView.h"
#include "ui_TableView.h"

#include "../src/Csv.h"

#include <QApplication>
#include <QClipboard>
#include <QKeyEvent>
#include <QKeySequence>

TableView::TableView(QWidget *parent) : QTableView(parent), ui(new Ui::TableView) { ui->setupUi(this); }

TableView::~TableView() { delete ui; }

void TableView::keyPressEvent(QKeyEvent *event)
{
    if (!selectedIndexes().isEmpty())
    {
        if (event->matches(QKeySequence::Copy))
        {
            QString text = Csv("\t", "\"").writeSelectionToString(model(), selectionModel()->selection());
            QApplication::clipboard()->setText(text);
        }
        else
            QTableView::keyPressEvent(event);
    }
}
