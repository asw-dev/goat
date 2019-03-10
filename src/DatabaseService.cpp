#include "DatabaseService.h"

#include <QMap>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QUuid>
#include <QVariant>
#include <QDebug>

#include "StringUtils.h"

void initQSqlDatabase(QSqlDatabase &db, const Connection &connection, Credentials *credentials)
{
    QString user = "";
    QString pass = "";

    if (connection.driver() != "QSQLITE")
        credentials->get(connection.connectionId(), &user, &pass);

    if (connection.driver() == "QODBC")
    {
        QMap<QString, QString> details = connection.details();
        details["user"] = user;
        details["password"] = pass;

        QMap<QString, QString> values;

        foreach (QString key, details.keys())
        {
            values[key] = details[key];
            values["escaped-" + key] = details[key].replace("}", "}}");
        }

        QString connection = StringUtils::interpolate(values["connection"], values);

        db.setDatabaseName(connection);
        db.setConnectOptions(details["options"]);
    }
    else
    {
        db.setHostName(connection.details()["server"]);
        db.setPort(connection.details()["port"].toInt());
        db.setDatabaseName(connection.details()["database"]);
        db.setConnectOptions(connection.details()["options"]);
        db.setUserName(user);
        db.setPassword(pass);
    }
}

QHash<QString /*id*/, DatabaseObjectMetadata> loadDatabaseMetadata(const Connection &connection, Credentials *credentials)
{
    QHash<QString /*id*/, DatabaseObjectMetadata> ret;

    QString dbId = connection.connectionId() + ":" + QUuid::createUuid().toString().mid(1, 36);
    {
        QSqlDatabase db = QSqlDatabase::addDatabase(connection.driver(), dbId);
        initQSqlDatabase(db, connection, credentials);

        if (db.open())
        {
            if (connection.driver() == "QSQLITE")
            {
                QSqlQuery query(db);
                QString sql = "select type, name, tbl_name, sql from sqlite_master;";
                bool success = query.exec(sql);
                if (success)
                {
                    DatabaseObjectMetadata item;
                    while(query.next())
                    {
                        QSqlRecord record = query.record();
                        QString type = record.value("type").toString();
                        QString name = record.value("name").toString();
                        QString tableName = record.value("tbl_name").toString();
                        QString definition = record.value("sql").toString();
                        QString id = name;
                        
                        item.setParentId(type == "table" ? "" : tableName);
                        item.setType(type);
                        item.setId(id);
                        item.setName(name);
                        if (!ret.contains(item.id()))
                            ret[id] = item;

                        if (type == "table")
                        {
                            DatabaseObjectMetadata item;

                            foreach(QString line, definition.split("\n"))
                            {
                                if (line.startsWith("    ["))
                                {
                                    int start = 5;
                                    int end = line.indexOf(']');
                                    QString column = line.mid(start, end - start);
                                    QString id = tableName + "." + column;

                                    item.setParentId(tableName);
                                    item.setType("column");
                                    item.setId(id);
                                    item.setName(column);
                                    if (!ret.contains(item.id()))
                                        ret[id] = item;
                                }
                            }
                        }
                    }
                }
                else
                {
                    QString msg = query.lastError().databaseText();
                    qDebug() << "loadDatabaseMetadata() query failed: " << msg;
                }
                query.finish();
            }
            else
            {
                QSqlQuery query(db);
                QString sql = "SELECT TABLE_CATALOG, TABLE_SCHEMA, TABLE_NAME, COLUMN_NAME FROM INFORMATION_SCHEMA.COLUMNS;";
                bool success = query.exec(sql);
                if (success)
                {
                    DatabaseObjectMetadata item;
                    while(query.next())
                    {
                        QSqlRecord record = query.record();
                        QString database = record.value("TABLE_CATALOG").toString();
                        QString schema = record.value("TABLE_SCHEMA").toString();
                        QString table = record.value("TABLE_NAME").toString();
                        QString column = record.value("COLUMN_NAME").toString();
                        QString id = "";

                        item.setParentId(id);
                        id += "[" + database + "]";
                        item.setType("database");
                        item.setId(id);
                        item.setName(database);
                        if (!ret.contains(item.id()))
                            ret[id] = item;

                        item.setParentId(id);
                        id += ".[" + schema + "]";
                        item.setType("schema");
                        item.setId(id);
                        item.setName(schema);
                        if (!ret.contains(item.id()))
                            ret[id] = item;

                        item.setParentId(id);
                        id += ".[" + table + "]";
                        item.setType("table"); //FIXME this could be a view
                        item.setId(id);
                        item.setName(table);
                        if (!ret.contains(item.id()))
                            ret[id] = item;

                        item.setParentId(id);
                        id += ".[" + column + "]";
                        item.setType("column");
                        item.setId(id);
                        item.setName(column);
                        if (!ret.contains(item.id()))
                            ret[id] = item;
                    }
                }
                else
                {
                    QString msg = query.lastError().databaseText();
                    qDebug() << "loadDatabaseMetadata() query failed: " << msg;
                }
                query.finish();
            }
            

        }

        /*TODO
         * function
         * sequence
         * view
         * constraint
         * index
         * trigger
         * foreign key
         */

        if (db.isOpen())
            db.close();
    }
    QSqlDatabase::removeDatabase(dbId);

    return ret;
}
