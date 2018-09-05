#ifndef QUERYRESULT_H
#define QUERYRESULT_H

#include <QAbstractItemModel>
#include <QDateTime>
#include <QMetaType>
#include <QSharedPointer>
#include <QString>
#include <QVector>

#include "QueryState.h"

class QueryResult
{
  public:
    QueryResult();
    QueryResult(const QueryResult &other);
    ~QueryResult();

    QueryResult(const QVector<QSharedPointer<QAbstractItemModel>> &rowSets, const QString &messages, const int &rowsAffected, const QueryState &queryState, const QDateTime &start, const QDateTime &executed, const QDateTime &end);

    QVector<QSharedPointer<QAbstractItemModel>> rowSets() const;
    QString messages() const;
    int rowsAffected() const;
    QueryState queryState() const;
    QDateTime start() const;
    QDateTime executed() const;
    QDateTime end() const;

  private:
    QVector<QSharedPointer<QAbstractItemModel>> m_rowSets;
    QString m_messages;
    int m_rowsAffected;
    QueryState m_queryState;
    QDateTime m_start;
    QDateTime m_executed;
    QDateTime m_end;
};

Q_DECLARE_METATYPE(QueryResult)

#endif // QUERYRESULT_H
