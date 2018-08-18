#ifndef QUERYTHREAD_H
#define QUERYTHREAD_H

#include "src/Connection.h"
#include "src/Credentials.h"
#include "src/QueryState.h"

#include <QDateTime>
#include <QObject>
#include <QSqlQuery>
#include <QStandardItemModel>
#include <QString>
#include <QThread>

class Query : public QObject
{
    Q_OBJECT

public:
    Query(const Connection &connection, Credentials *credentials, const QString &queryTxt);
    ~Query();
    QueryState queryState() const;
    QDateTime startTime() const;
    QDateTime endTime() const;
    QList<QStandardItemModel*> results() const;
    QString messages() const;
    int rowsAffected() const;
    bool canCancel();
    Query* cancel();

signals:
    void queryFinished();
    void queryFailed();

public slots:
    void run();

private:
    QString m_queryId;
    Connection m_connection;
    int m_queryProcessId;
    Credentials *m_credentials;
    QString m_queryTxt;
    QList<QStandardItemModel*> m_results;
    QString m_messages;
    int m_rowsAffected;
    QueryState m_queryState;
    QDateTime m_startTime;
    QDateTime m_endTime;

    bool runQuery(const QSqlDatabase &db);
    void initQueryProcessId(const QSqlDatabase &db);
};

#endif // QUERYTHREAD_H
