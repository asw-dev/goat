#ifndef DATABASEOBJECTTREEVIEW_H
#define DATABASEOBJECTTREEVIEW_H

#include "src/Connection.h"
#include "src/DatabaseObjectMetadata.h"

#include <QHash>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QTreeView>

class DatabaseObjectTreeView : public QTreeView
{
public:
    DatabaseObjectTreeView(QWidget *parent);

    QHash<QString /*id*/, DatabaseObjectMetadata> get(const Connection &connection);
    void set(const Connection &connection, const QHash<QString /*id*/, DatabaseObjectMetadata> &data);
    void remove(const QString &connectionId);

private:
    QStandardItem* findChild(QStandardItem *node, QString value, int role = Qt::DisplayRole);
    QStandardItem* insertSubNode(QStandardItem *node, QString displayValue, QString id);
    void updateSubTree(QStandardItem *root, const QHash<QString /*id*/, DatabaseObjectMetadata> &data);
    void retainIds(QStandardItem *node, const QHash<QString /*id*/, DatabaseObjectMetadata> &data);

    QHash<QString /*connectionId*/, QHash<QString /*objectId*/, DatabaseObjectMetadata>> m_data;
    QStandardItemModel* m_model;
    QSortFilterProxyModel* m_proxyModel;
};

#endif // DATABASEOBJECTTREEVIEW_H
