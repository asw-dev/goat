#include <QApplication>
#include <QtTest>

#include "TestPagedTableModel.h"
#include "TestQuery.h"
#include "TestStringUtils.h"
#include "TestWindowedItemModelDecorator.h"

//following pattern from: http://xilexio.org/?p=125
int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    TestPagedTableModel testPagedTableModel;
    TestQuery testQuery;
    TestStringUtils testStringUtils;
    TestWindowedItemModelDecorator testWindowedItemModelDecorator;

    return 0
            | QTest::qExec(&testPagedTableModel, argc, argv)
            | QTest::qExec(&testQuery, argc, argv)
            | QTest::qExec(&testStringUtils, argc, argv)
            | QTest::qExec(&testWindowedItemModelDecorator, argc, argv);
}
