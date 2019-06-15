#include "DatabaseObjectTreeView.h"

DatabaseObjectTreeView::DatabaseObjectTreeView(QWidget *parent) :
    QTreeView(parent)
{
    m_proxyModel = new QSortFilterProxyModel(this);
    m_model = new QStandardItemModel();
    m_proxyModel->setSourceModel(m_model);
    m_proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    setModel(m_proxyModel);
    setSortingEnabled(true);
    sortByColumn(0, Qt::AscendingOrder);
    m_model->insertColumn(0);
}

DatabaseObjectTreeView::~DatabaseObjectTreeView()
{
    delete m_proxyModel;
    delete m_model;
}

QHash<QString /*id*/, DatabaseObjectMetadata> DatabaseObjectTreeView::get(const Connection &connection)
{
    return m_data[connection.connectionId()];
}

void DatabaseObjectTreeView::set(const Connection &connection, const QHash<QString /*id*/, DatabaseObjectMetadata> &data)
{
    m_data[connection.connectionId()] = data;

    QStandardItem *item = findChild(m_model->invisibleRootItem(), connection.connectionId(), Qt::UserRole);
    if (item == nullptr)
        item = insertSubNode(m_model->invisibleRootItem(), connection.connectionId(), connection.name());

    updateSubTree(item, data);
}

void DatabaseObjectTreeView::remove(const QString &connectionId)
{
    m_data.remove(connectionId);
    QStandardItem *item = findChild(m_model->invisibleRootItem(), connectionId, Qt::UserRole);
    if (item != nullptr)
        m_model->removeRow(item->index().row());
}

QStandardItem* DatabaseObjectTreeView::findChild(QStandardItem *node, QString value, int role)
{
    QStandardItemModel *model = node->model();
    QModelIndex index = node->index();

    for (int i = 0; i < node->rowCount(); ++i)
    {
        QStandardItem *childNode = model->itemFromIndex(model->index(i, 0, index));

        if (model->data(childNode->index(), role) == value)
            return childNode;
    }
    return nullptr;
}

QStandardItem* DatabaseObjectTreeView::insertSubNode(QStandardItem *node, QString id, QString name)
{
    QStandardItemModel *model = node->model();
    node->setRowCount(node->rowCount() + 1);
    if (!node->columnCount())
        node->setColumnCount(1);
    QModelIndex index = model->index(node->rowCount() - 1, 0, node->index());
    model->setData(index, name, Qt::DisplayRole);
    model->setData(index, id, Qt::UserRole);
    return model->itemFromIndex(index);
}

void DatabaseObjectTreeView::updateSubTree(QStandardItem *root, const QHash<QString /*id*/, DatabaseObjectMetadata> &data)
{
    QHash<QString /*id*/, QStandardItem*> modelLookup;

    QStandardItem *node;
    QStandardItem *found;

    bool skippedItem;
    do
    {
        skippedItem = false;
        foreach (DatabaseObjectMetadata metadata, data.values())
        {
            if (modelLookup.contains(metadata.id()))
                continue;

            //start at "parent" object
            if (metadata.parentId().isEmpty())
                node = root;
            else if (modelLookup.contains(metadata.parentId()))
                node = modelLookup[metadata.parentId()];
            else
            {
                //we will handle this in a future pass
                skippedItem = true;
                continue;
            }

            // move down to the type node below parent
            found = findChild(node, metadata.type());
            if (!found)
            {
                found = insertSubNode(node, "", metadata.type());
            }
            node = found;

            found = findChild(node, metadata.id(), Qt::UserRole);
            if (!found)
            {
                //insert the object
                node = insertSubNode(node, metadata.id(), metadata.name());
            }
            else
                node = found;

            modelLookup[metadata.id()] = node;
        }
    } while (skippedItem);

    //remove any nodes that are not in data
    for (int i = 0; i < root->rowCount(); ++i)
    {
        QStandardItem *childNode = root->child(i);
        retainIds(childNode, data);
    }
}

void DatabaseObjectTreeView::retainIds(QStandardItem *node, const QHash<QString /*id*/, DatabaseObjectMetadata> &data)
{
    for (int i = 0; i < node->rowCount(); ++i)
    {
        QStandardItem *childNode = node->child(i);
        retainIds(childNode, data);
    }

    QString id = node->data(Qt::UserRole).toString();
    if ((id.isEmpty() && node->rowCount() == 0) || (!id.isEmpty() && !data.contains(id)))
    {
        node->parent()->removeRow(node->row());
    }
}
