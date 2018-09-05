#ifndef TESTWINDOWEDITEMMODELDECORATOR_H
#define TESTWINDOWEDITEMMODELDECORATOR_H

#include <QtTest>

class TestWindowedItemModelDecorator : public QObject
{
    Q_OBJECT

  private slots:

    void testDataIsWindowed();
};

#endif // TESTWINDOWEDITEMMODELDECORATOR_H
