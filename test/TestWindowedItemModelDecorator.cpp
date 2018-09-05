#include "TestWindowedItemModelDecorator.h"
#include "src/WindowedItemModelDecorator.h"

#include <QStandardItemModel>
#include <QSharedPointer>

void TestWindowedItemModelDecorator::testDataIsWindowed()
{
    WindowedItemModelDecorator decorator;
    QStandardItemModel *model = new QStandardItemModel();

    model->insertColumns(0, 3);
    model->insertRows(model->rowCount(), 2);
    model->setData(model->index(0, 0), "a");
    model->setData(model->index(0, 1), "b");
    model->setData(model->index(0, 2), "c");
    model->setData(model->index(1, 0), "d");
    model->setData(model->index(1, 1), "e");
    model->setData(model->index(1, 2), "f");

    QSharedPointer<QAbstractItemModel> ptr(model);
    decorator.setModel(ptr);
    decorator.setRange(1, 1, 1, 2);

    QCOMPARE(decorator.rowCount(), 1);
    QCOMPARE(decorator.columnCount(), 2);
    QCOMPARE(decorator.data(decorator.index(0, 0)), QVariant("e"));
    QCOMPARE(decorator.data(decorator.index(0, 1)), QVariant("f"));
    QCOMPARE(decorator.data(decorator.index(1, 0)), QVariant());
    QCOMPARE(decorator.data(decorator.index(0, 2)), QVariant());
    ptr->setData(model->index(1, 2), "F");
    QCOMPARE(decorator.data(decorator.index(0, 1)), QVariant("F"));

    decorator.setRange(0, 0, 1, 2);
    QCOMPARE(decorator.data(decorator.index(0, 0)), QVariant("a"));
    QCOMPARE(decorator.data(decorator.index(1, 2)), QVariant("F"));
    ptr->setData(model->index(1, 2), "f");
    QCOMPARE(decorator.data(decorator.index(1, 2)), QVariant("f"));
}
