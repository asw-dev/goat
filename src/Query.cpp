#include "Query.h"

#include "DatabaseService.h"
#include "PagedTableModel.h"
#include "StringUtils.h"

#include <QApplication>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlRecord>
#include <QUuid>

Query::Query(const Connection &connection, Credentials *credentials, const QStringList &queryTxt, const QString &pidQuery, const int rowBatchSize) : QObject()
{
    m_queryId = connection.connectionId() + ":" + QUuid::createUuid().toString().mid(1, 36);
    m_connection = connection;
    m_credentials = credentials;
    m_queryTxt = queryTxt;
    m_queryState = READY;
    m_pidQuery = pidQuery;
    m_rowBatchSize = rowBatchSize;
    m_destThread = 0;
}

Query::~Query() {}

void Query::run()
{
    m_queryState = ACTIVE;
    emit queryStateChanged(m_queryId, m_queryState);
    bool success = true;
    QString errorTxt = "";

    QString dbId = m_connection.connectionId() + ":" + m_queryId;
    {
        QSqlDatabase db = QSqlDatabase::addDatabase(m_connection.driver(), dbId);
        initQSqlDatabase(db, m_connection, m_credentials);

        success = db.open();

        if (success)
        {
            initQueryProcessId(db);
            success = runQuery(db);
        }
        else
            errorTxt = db.lastError().text();

        if (db.isOpen())
            db.close();
    }
    QSqlDatabase::removeDatabase(dbId);

    m_queryState = success ? FINISHED : FAILED;
    emit queryStateChanged(m_queryId, m_queryState);

    if (m_destThread)
    {
        foreach (QueryResult result, m_results)
        {
            foreach (QSharedPointer<QAbstractItemModel> rowSet, result.rowSets())
            {
                rowSet->moveToThread(m_destThread);
            }
        }
    }

    emit batchFinished(m_queryId, success, errorTxt);
}

void Query::setDestThread(QThread *destThread)
{
    m_destThread = destThread;
}

QString Query::queryId() const
{
    return m_queryId;
}

void Query::initQueryProcessId(const QSqlDatabase &db)
{
    int pid = -1;

    if (!m_pidQuery.isEmpty())
    {
        QSqlQuery query(db);

        bool success = query.exec(m_pidQuery);

        if (success && query.next())
        {
            pid = query.value(0).toInt();
        }
        query.finish();
    }

    emit connectionOpened(m_queryId, pid);
}

static void doDeleteLater(QAbstractItemModel *obj)
{
    obj->deleteLater();
}

bool Query::runQuery(const QSqlDatabase &db)
{
    m_start = QDateTime::currentDateTime();

    bool success = true;
    for (int batchIdx = 0; batchIdx < m_queryTxt.count(); ++batchIdx)
    {
        QString queryTxt = m_queryTxt[batchIdx];
        QSqlQuery query(db);
        query.setForwardOnly(true);

        QDateTime queryStart = QDateTime::currentDateTime();
        success = query.exec(queryTxt);
        QDateTime queryExecuted = QDateTime::currentDateTime();

        QVector<QSharedPointer<QAbstractItemModel>> rowSets;

        if (query.isSelect())
        {
            int rowSetIdx = -1;
            int lastRowSent = -1;
            do
            {
                QSharedPointer<QAbstractItemModel> model = QSharedPointer<QAbstractItemModel>(new PagedTableModel(), doDeleteLater);
                ++rowSetIdx;

                QSqlRecord record = query.record();
                model->insertColumns(0, record.count());
                for (int col = 0; col < record.count(); ++col)
                {
                    model->setHeaderData(col, Qt::Horizontal, record.fieldName(col));
                }

                emit columnsLoaded(m_queryId, batchIdx, rowSetIdx, model);

                for (int row = 0; query.next(); ++row)
                {
                    record = query.record();
                    model->insertRows(model->rowCount(), 1);
                    for (int col = 0; col < record.count(); ++col)
                    {
                        QVariant value = record.value(col);
                        model->setData(model->index(row, col), value);
                    }

                    if ((model->rowCount() - 1) - lastRowSent >= m_rowBatchSize)
                    {
                        emit rowsLoaded(m_queryId, batchIdx, rowSetIdx, model, model->index(lastRowSent + 1, 0), model->index(model->rowCount() - 1, model->columnCount() - 1));
                        lastRowSent = model->rowCount() - 1;
                    }
                }

                if (lastRowSent != model->rowCount() - 1)
                {
                    emit rowsLoaded(m_queryId, batchIdx, rowSetIdx, model, model->index(lastRowSent + 1, 0), model->index(model->rowCount() - 1, model->columnCount() - 1));
                }

                rowSets.append(model);
            } while (query.nextResult());
        }
        query.finish();

        QDateTime queryEnd = QDateTime::currentDateTime();

        QueryResult result(rowSets, query.lastError().text(), query.numRowsAffected(), success ? FINISHED : FAILED, queryStart, queryExecuted, queryEnd);
        m_results.append(result);
        emit queryFinished(m_queryId, batchIdx, result);

        if (!success)
            break;
    }

    return success;
}

QVector<QueryResult> Query::results() const { return m_results; }

QueryState Query::queryState() const { return m_queryState; }

QDateTime Query::start() const { return m_start; }

QDateTime Query::end() const { return m_end; }
