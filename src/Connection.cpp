#include "Connection.h"

#include <QFileInfo>
#include <QUuid>
#include <assert.h>

Connection Connection::defaultConnection(const QString &driver)
{
    Connection connection;
    connection.setDriver(driver);

    QMap<QString, QString> details;

    if (connection.driver() == "QPSQL")
    {
        details["server"] = "localhost";
        details["port"] = "5432";
        details["database"] = "public";
    }
    else if (connection.driver() == "QMYSQL")
    {
        details["server"] = "localhost";
        details["port"] = "3306";
        details["database"] = "";
    }
    else if (connection.driver() == "QODBC")
    {
        details["server"] = "localhost";
        details["port"] = "1433";
        details["database"] = "master";
        details["connection"] = "DRIVER=%(library);Setup=%(setup);Server=%(server);Port=%(port);Database=%(database);UID=%(user);PWD={%(escaped-password)};";
        details["library"] = "/usr/lib/x86_64-linux-gnu/odbc/libtdsodbc.so"; //TODO other paths, drivers, and OSs
        details["setup"] = "/usr/lib/x86_64-linux-gnu/odbc/libtdsS.so";
    }
    else if (connection.driver() == "QSQLITE")
    {
        details["server"] = "";
        details["port"] = "";
        details["database"] = ":memory:";
    }

    connection.setDetails(details);
    connection.setName(defaultName(connection));
    return connection;
}

QString Connection::defaultName(const Connection &connection)
{
    QString type = "?";

    if (connection.driver() == "QPSQL")
    {
        type = "psql";
    }
    else if (connection.driver() == "QMYSQL")
    {
        type = "mysql";
    }
    else if (connection.driver() == "QODBC")
    {
        type = "odbc";
    }
    else if (connection.driver() == "QSQLITE")
    {
        type = "sqlite";
    }

    if (type == "?")
        return "New Connection";

    QString server = connection.details().contains("server") ? connection.details()[("server")] : "";
    QString port = connection.details().contains("port") ? connection.details()[("port")] : "";
    QString database = connection.details().contains("database") ? connection.details()[("database")] : "";

    QString name = "";

    if (connection.driver() == "QSQLITE")
    {
        if (database.contains(":"))
            name += database;
        else
            name += QFileInfo(database).fileName();
    }
    else
    {
        name += server;
        name += (port.isEmpty() ? "" : ":") + port;
        name += (database.isEmpty() || name.isEmpty() ? "" : "/") + database;
    }
    name += " [" + type + "]";

    return name;
}

QString Connection::defaultPidQuery(const QString &driver)
{
    if (driver == "QPSQL")
        return "SELECT pg_backend_pid();";
    if (driver == "QMYSQL")
        return "SELECT CONNECTION_ID();";
    if (driver == "QODBC")
        return "SELECT @@SPID;";
    return "";
}

QString Connection::defaultCancelQuery(const QString &driver)
{
    if (driver == "QPSQL")
        return "SELECT pg_cancel_backend(%(pid));";
    if (driver == "QMYSQL")
        return "KILL QUERY %(pid);";
    if (driver == "QODBC")
        return "KILL %(pid);";
    return "";
}

Connection::Connection()
{
    m_connectionId = QUuid::createUuid().toString().mid(1, 36);
    m_name = "New connection";
    m_driver = "QPSQL";

    m_details["server"] = "";
    m_details["port"] = "";
    m_details["database"] = "";
}

Connection::Connection(const QString &connectionId, const QString &driver, const QString &name, const QMap<QString, QString> &details)
{
    assert(!connectionId.isEmpty());
    assert(!driver.isEmpty());

    m_connectionId = connectionId;
    m_name = name;
    m_driver = driver;
    m_details = details;
}

QMap<QString, QString> Connection::details() const
{
    return m_details;
}

void Connection::setDetails(const QMap<QString, QString> &details)
{
    m_details = details;
}

void Connection::setDriver(const QString &driver)
{
    m_driver = driver;
}

QString Connection::driver() const
{
    return m_driver;
}

QString Connection::name() const
{
    return m_name;
}

void Connection::setName(const QString &name)
{
    m_name = name;
}

QString Connection::connectionId() const
{
    return m_connectionId;
}
