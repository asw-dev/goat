#ifndef DATABASEMETADATASERVICE_H
#define DATABASEMETADATASERVICE_H

#include <QHash>
#include <QSqlDatabase>
#include <QString>

#include "Connection.h"
#include "Credentials.h"
#include "DatabaseObjectMetadata.h"

void initQSqlDatabase(QSqlDatabase &db, const Connection &connection, Credentials *credentials);

QHash<QString /*id*/, DatabaseObjectMetadata> loadDatabaseMetadata(const Connection &connection, Credentials *credentials);


#endif // DATABASEMETADATASERVICE_H
