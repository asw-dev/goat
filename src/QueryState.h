#ifndef QUERYSTATE_H
#define QUERYSTATE_H

enum QueryState
{
    READY,
    ACTIVE,
    CANCELING,
    FINISHED,
    FAILED
};

#endif // QUERYSTATE_H
