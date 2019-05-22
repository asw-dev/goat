#ifndef CONNECTIONMANAGER_H
#define CONNECTIONMANAGER_H

#include <QHash>
#include <QList>
#include <QMap>
#include <QString>

#include "Connection.h"
#include "DatabaseObjectMetadata.h"

class ConnectionManager
{
public:
    ConnectionManager();
    void saveConnection(const Connection &connection);
    void deleteConnection(const QString &connectionId);
    QMap<QString, Connection> getConnections() const;
    QHash<QString, DatabaseObjectMetadata> getMetadata(const QString &connectionId) const;
    void setMetadata(const QString &connectionId, QHash<QString, DatabaseObjectMetadata> &metadata);

private:
    QList<Connection> loadConnections();
    QMap<QString, Connection> m_connections;
    QMap<QString /*connectionId*/, QHash<QString /*databasObjectId*/, DatabaseObjectMetadata> > m_metadata;
};

#endif // CONNECTIONMANAGER_H
