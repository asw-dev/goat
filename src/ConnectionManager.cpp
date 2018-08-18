#include "ConnectionManager.h"

#include <QCoreApplication>
#include <QSettings>
#include <QString>

ConnectionManager::ConnectionManager()
{
    foreach(Connection connection, loadConnections())
    {
        m_connections[connection.connectionId()] = connection;
    }
}

void ConnectionManager::saveConnection(const Connection &connection)
{
    m_connections[connection.connectionId()] = connection;

    QSettings settings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::applicationName(), "connections");

    settings.beginGroup(connection.connectionId());

    settings.setValue("connectionId", connection.connectionId());
    settings.setValue("driver", connection.driver());
    settings.setValue("name", connection.name());

    foreach(QString key, connection.details().keys())
    {
        settings.setValue(key, connection.details()[key]);
    }

    settings.endGroup();
    settings.sync();
}

void ConnectionManager::deleteConnection(const QString &connectionId)
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::applicationName() , "connections");
    settings.remove(connectionId);
    settings.sync();
}

QMap<QString, Connection> ConnectionManager::getConnections() const
{
    return m_connections;
}

QList<Connection> ConnectionManager::loadConnections()
{
    QList<Connection> connections;

    QSettings settings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::applicationName() , "connections");

    foreach(QString connectionId, settings.childGroups())
    {
        settings.beginGroup(connectionId);

        QString driver = settings.value("driver").toString();
        QString name = settings.value("name").toString();

        QMap<QString, QString> details;

        foreach(QString key, settings.childKeys())
        {
            details[key] = settings.value(key).toString();
        }
        details.remove("connectionId");
        details.remove("driver");
        details.remove("name");

        Connection connection(connectionId, driver, name, details);
        connections.append(connection);

        settings.endGroup();
    }
    return connections;
}
