#ifndef QUERYTHREAD_H
#define QUERYTHREAD_H

#include "src/Connection.h"
#include "src/Credentials.h"
#include "src/QueryState.h"
#include "src/QueryResult.h"

#include <QAbstractItemModel>
#include <QDateTime>
#include <QList>
#include <QObject>
#include <QSharedPointer>
#include <QSqlQuery>
#include <QString>
#include <QStringList>
#include <QThread>

class Query : public QObject
{
    Q_OBJECT

public:

    /*!
     * \brief
     * \param connection
     * \param credentials
     * \param batchTxt ordered statements to run in a single QSqlDatabase connection
     * \param pidQuery query used to find servers sessionId. Using "" will skip lookup and return -1 for the connectionOpened() signal.
     * \param rowBatchSize controls how often to send rowsLoaded() signal
     */
    Query(const Connection &connection, Credentials *credentials, const QStringList &batchTxt, const QString &pidQuery = "", const int rowBatchSize = 1000);
    ~Query();
    QString queryId() const;
    QueryState queryState() const;
    QDateTime start() const;
    QDateTime end() const;
    QVector<QueryResult> results() const;

    void setDestThread(QThread *destThread);

signals:

    /*!
     * \brief This signal is emitted after preforming the sessionId lookup.
     * \param queryId
     * \param pid
     */
    void connectionOpened(const QString &queryId, int pid);

    /*!
     * \brief This signal is emitted when QSqlQuery.exec() returns from a select statement.
     * \param queryId
     * \param batchIdx
     * \param rowSetIdx for queries returning multiple rowSets
     * \param rowSet contains headerData for columns
     */
    void columnsLoaded(const QString &queryId, int batchIdx, int rowSetIdx, const QSharedPointer<QAbstractItemModel> &rowSet);

    /*!
     * \brief
     * \param queryId
     * \param batchIdx
     * \param rowSetIdx for queries returning multiple rowSets
     * \param rowSet
     * \param first index of the first column and row for the loaded data (not guarenteed to be the same as the first row in the model).
     * \param last index of the last column and row for the loaded data (not guarenteed to be the same as the last row in the model).
     */
    void rowsLoaded(const QString &queryId, int batchIdx, int rowSetIdx, const QSharedPointer<QAbstractItemModel> &rowSet, const QModelIndex &first, const QModelIndex &last);

    /*!
     * \brief This signal is emitted after all rows for QSqlQuery have been extracted, or an the statement has returned an error.
     * \param queryId
     * \param batchIdx
     * \param result
     */
    void queryFinished(const QString &queryId, int batchIdx, const QueryResult &result);

    /*!
     * \brief This signal is emitted after all statements in the batch have been executed or an error has occurred. No other signals will be emitted after this signal.
     * \param queryId
     * \param batchSuccess
     * \param errorTxt
     */
    void batchFinished(const QString &queryId, bool batchSuccess, const QString &errorTxt);

    void queryStateChanged(const QString &queryId, const QueryState &newState);

public slots:
    void run();

private:
    QString m_queryId;
    Connection m_connection;
    Credentials *m_credentials;
    QStringList m_queryTxt;
    QString m_pidQuery;
    int m_rowBatchSize;
    QThread *m_destThread;

    QVector<QueryResult> m_results;
    QueryState m_queryState;
    QDateTime m_start;
    QDateTime m_end;

    bool runQuery(const QSqlDatabase &db);
    void initQueryProcessId(const QSqlDatabase &db);
};

#endif // QUERYTHREAD_H
