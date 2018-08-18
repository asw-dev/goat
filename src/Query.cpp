#include "Query.h"

#include <QApplication>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlRecord>
#include <QUuid>

#define LIGHT_YELLOW QColor("#FFFFE0")

Query::Query(const Connection &connection, Credentials *credentials, const QString &queryTxt) : QObject()
{
    m_queryId = connection.connectionId() + ":" + QUuid::createUuid().toString().mid(1, 36);
    m_connection = connection;
    m_queryProcessId = -1;
    m_credentials = credentials;
    m_queryTxt = queryTxt;
    m_rowsAffected = -1;
    m_queryState = READY;
}

Query::~Query()
{
    foreach(QStandardItemModel *model, m_results)
    {
        delete model;
    }
    m_results.clear();
}

void Query::run()
{
    m_queryState = ACTIVE;

    bool success = true;
    {
        QSqlDatabase db = QSqlDatabase::addDatabase(m_connection.driver(), m_queryId);
        db.setHostName(m_connection.details()["server"]);
        db.setPort(m_connection.details()["port"].toInt());
        db.setDatabaseName(m_connection.details()["database"]);
        QString user = "";
        QString pass = "";

        if (m_connection.driver() != "QSQLITE")
            m_credentials->get(m_connection.connectionId(), &user, &pass);
        db.setUserName(user);
        db.setPassword(pass);

        if (success)
            success = db.open();

        if (success)
        {
            initQueryProcessId(db);
            success = runQuery(db);
        }
        else
            m_messages = db.lastError().text();

        db.close();
    }
    QSqlDatabase::removeDatabase(m_queryId);

    if (success)
    {
        m_queryState = FINISHED;
        emit queryFinished();
    }
    else
    {
        m_queryState = FAILED;
        emit queryFailed();
    }
}

void Query::initQueryProcessId(const QSqlDatabase &db)
{
    if (!canCancel())
        return;

    QSqlQuery query(db);

    QString sql = "";
    if (m_connection.driver() == "QPSQL")
        sql = "SELECT pg_backend_pid();";
    else if (m_connection.driver() == "QMYSQL")
        sql = "SELECT CONNECTION_ID();";

    bool success = query.exec(sql);

    if (success && query.next())
    {
        m_queryProcessId = query.value(0).toInt();
    }
    query.finish();
}

bool Query::canCancel()
{
    return m_connection.driver() == "QPSQL"
            || m_connection.driver() == "QMYSQL";
}

Query* Query::cancel()
{
    if (m_queryProcessId == -1)
        return 0;

    QString sql = "";
    if (m_connection.driver() == "QPSQL")
        sql = "SELECT pg_cancel_backend(" + QString::number(m_queryProcessId) + ");";
    else if (m_connection.driver() == "QMYSQL")
        sql = "KILL QUERY " + QString::number(m_queryProcessId);

    return new Query(m_connection, m_credentials, sql);
}

int Query::rowsAffected() const { return m_rowsAffected; }

bool Query::runQuery(const QSqlDatabase &db)
{
    QSqlQuery query(db);
    query.setForwardOnly(true);

    m_startTime = QDateTime::currentDateTime();
    bool success = query.exec(m_queryTxt);
    m_endTime = QDateTime::currentDateTime();

    if (query.isSelect())
    {
        do
        {
            QStandardItemModel *model = new QStandardItemModel();

            QSqlRecord record = query.record();
            model->setColumnCount(record.count());
            for (int col = 0; col < record.count(); ++col)
            {
                model->setHeaderData(col, Qt::Horizontal, record.fieldName(col));
            }

            for (int row = 0; query.next(); ++row)
            {
                record = query.record();
                model->setRowCount(model->rowCount() + 1);
                for (int col = 0; col < record.count(); ++col)
                {
                    QModelIndex index = model->index(row, col);
                    QVariant value = record.value(col);
                    model->setData(index, value);
                    if (value.isNull())
                        model->setData(index, LIGHT_YELLOW, Qt::BackgroundRole);
                }
            }
            m_results.append(model);

            m_rowsAffected = query.numRowsAffected();
            m_messages += "\n\n" + query.lastError().text();
            m_messages = m_messages.trimmed();
        } while (query.nextResult());
    }
    else
    {
        m_rowsAffected = query.numRowsAffected();
        m_messages += query.lastError().text();
    }

    query.finish();
    return success;
}

QString Query::messages() const { return m_messages; }

QList<QStandardItemModel *> Query::results() const { return m_results; }

QueryState Query::queryState() const { return m_queryState; }

QDateTime Query::startTime() const { return m_startTime; }

QDateTime Query::endTime() const { return m_endTime; }
