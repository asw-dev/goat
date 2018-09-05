#include "TestPagedTableModel.h"
#include "src/PagedTableModel.h"

void TestPagedTableModel::testCanReadDataBack()
{
    PagedTableModel model;
    QVERIFY(model.insertColumns(0, 3));
    QVERIFY(model.insertRows(model.rowCount(), 1));
    QVERIFY(model.setData(model.index(0, 0), "a"));
    QVERIFY(model.setData(model.index(0, 1), "b"));
    QVERIFY(model.setData(model.index(0, 2), "c"));

    QVERIFY(model.insertRows(model.rowCount(), 1));
    QVERIFY(model.setData(model.index(1, 0), "d"));
    QVERIFY(model.setData(model.index(1, 1), "e"));
    QVERIFY(model.setData(model.index(1, 2), "f"));

    QCOMPARE(model.rowCount(), 2);
    QCOMPARE(model.columnCount(), 3);
    QCOMPARE(model.data(model.index(0, 0)), QVariant("a"));
    QCOMPARE(model.data(model.index(0, 1)), QVariant("b"));
    QCOMPARE(model.data(model.index(0, 2)), QVariant("c"));
    QCOMPARE(model.data(model.index(1, 0)), QVariant("d"));
    QCOMPARE(model.data(model.index(1, 1)), QVariant("e"));
    QCOMPARE(model.data(model.index(1, 2)), QVariant("f"));

    QVERIFY(!model.index(2, 0).isValid());
}

void TestPagedTableModel::testOnlyInsertColumnsOnce()
{
    PagedTableModel model;
    QVERIFY(model.insertColumns(0, 3));
    QVERIFY(!model.insertColumns(0, 3));
}

void TestPagedTableModel::testOnlySetDataOnce()
{
    PagedTableModel model;
    QVERIFY(model.insertColumns(0, 3));
    QVERIFY(model.insertRows(model.rowCount(), 1));
    QVERIFY(model.setData(model.index(0, 0), "a"));
    QVERIFY(model.setData(model.index(0, 1), "b"));
    QVERIFY(model.setData(model.index(0, 2), "c"));

    QVERIFY(model.insertRows(model.rowCount(), 1));
    QVERIFY(model.setData(model.index(1, 0), "d"));
    QVERIFY(model.setData(model.index(1, 1), "e"));
    QVERIFY(model.setData(model.index(1, 2), "f"));

    QVERIFY(!model.setData(model.index(1, 2), "g"));
    QVERIFY(!model.setData(model.index(0, 0), "h"));
}

void TestPagedTableModel::testReadWriteRead()
{
    //test case that will cause the temp file to seek to a previous row while writing

    PagedTableModel model;
    QVERIFY(model.insertColumns(0, 3));

    for (int row = 0; row < 7; ++row)
    {
        QVERIFY(model.insertRows(model.rowCount(), 1));
        for (int col = 0; col < 3; ++col)
        {
            QVERIFY(model.setData(model.index(row, col), 3 * row + col));
        }

        if (row)
        {
            int r = row - 1;
            int col = 1;
            QCOMPARE(model.data(model.index(r, col)), QVariant(3 * r + col));
        }
    }
}
