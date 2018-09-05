#include "QueryResult.h"

#include <assert.h>

QueryResult::QueryResult() : QueryResult(QVector<QSharedPointer<QAbstractItemModel>>(), QString(), 0, READY, QDateTime(), QDateTime(), QDateTime()) {}

QueryResult::QueryResult(const QueryResult &other) : QueryResult(other.m_rowSets, other.m_messages, other.m_rowsAffected, other.m_queryState, other.m_start, other.m_executed, other.m_end) {}

QueryResult::~QueryResult() {}

QueryResult::QueryResult(const QVector<QSharedPointer<QAbstractItemModel>> &rowSets, const QString &messages, const int &rowsAffected, const QueryState &queryState, const QDateTime &startTime, const QDateTime &execEndTime, const QDateTime &endTime)
{
    m_rowSets = rowSets;
    m_messages = messages;
    m_rowsAffected = rowsAffected;
    m_queryState = queryState;
    m_start = startTime;
    m_executed = execEndTime;
    m_end = endTime;
}

QVector<QSharedPointer<QAbstractItemModel>> QueryResult::rowSets() const { return m_rowSets; }

QString QueryResult::messages() const { return m_messages; }

int QueryResult::rowsAffected() const { return m_rowsAffected; }

QueryState QueryResult::queryState() const { return m_queryState; }

QDateTime QueryResult::start() const { return m_start; }

QDateTime QueryResult::executed() const { return m_executed; }

QDateTime QueryResult::end() const { return m_end; }
