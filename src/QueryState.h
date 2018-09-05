#ifndef QUERYSTATE_H
#define QUERYSTATE_H

#include <QMetaType>

enum QueryState
{
    READY,
    ACTIVE,
    CANCELING,
    FINISHED,
    FAILED
};

Q_DECLARE_METATYPE(QueryState)

#endif // QUERYSTATE_H
