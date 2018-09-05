#ifndef TESTPAGEDTABLEMODEL_H
#define TESTPAGEDTABLEMODEL_H

#include <QtTest>

class TestPagedTableModel : public QObject
{
    Q_OBJECT

  private slots:

    void testCanReadDataBack();
    void testOnlyInsertColumnsOnce();
    void testOnlySetDataOnce();
    void testReadWriteRead();
};

#endif // TESTPAGEDTABLEMODEL_H
