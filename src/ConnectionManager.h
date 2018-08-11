#ifndef CONNECTIONMANAGER_H
#define CONNECTIONMANAGER_H

#include <QList>
#include <QMap>
#include <QString>

#include "src/Connection.h"

class ConnectionManager
{
public:
    ConnectionManager();
    void saveConnection(const Connection &connection);
    void deleteConnection(const QString &connectionId);
    QMap<QString, Connection> getConnections() const;

private:
    QList<Connection> loadConnections();
    QMap<QString, Connection> m_connections;
};

#endif // CONNECTIONMANAGER_H
