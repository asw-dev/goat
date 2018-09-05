#include "WindowedItemModelDecorator.h"

WindowedItemModelDecorator::WindowedItemModelDecorator() : QAbstractItemModel(nullptr)
{
    m_minCol = -1;
    m_minRow = -1;
    m_maxCol = 0;
    m_maxRow = 0;
}

WindowedItemModelDecorator::~WindowedItemModelDecorator()
{
    setModel(QSharedPointer<QAbstractItemModel>());
}

void WindowedItemModelDecorator::setModel(const QSharedPointer<QAbstractItemModel> &model)
{
    if (model == m_model)
        return;

    beginResetModel();

    if (m_model)
        m_model->disconnect(this);

    m_model = model;
    m_minCol = -1;
    m_minRow = -1;
    m_maxCol = 0;
    m_maxRow = 0;

    if (!m_model)
        return;

    endResetModel();
}

void WindowedItemModelDecorator::setRange(int minRow, int minCol, int maxRow, int maxCol)
{
    if (minCol == m_minCol && minRow == m_minRow && maxCol == m_maxCol && maxRow > m_maxRow)
    {
        beginInsertRows(QModelIndex(), m_maxRow + 1, maxRow);
        m_maxRow = maxRow;
        endInsertRows();
    }
    else
    {
        beginResetModel();
        m_minCol = minCol;
        m_minRow = minRow;
        m_maxCol = maxCol;
        m_maxRow = maxRow;
        endResetModel();
        emit dataChanged(index(minRow, minCol), index(maxRow, maxCol));
        emit headerDataChanged(Qt::Horizontal, minCol, maxCol);
    }
}

QModelIndex WindowedItemModelDecorator::index(int row, int column, const QModelIndex &) const
{
    if (row < 0 || column < 0 || row > rowCount() - 1 || column > columnCount() - 1)
        return QModelIndex();
    return QAbstractItemModel::createIndex(row, column);
}

QModelIndex WindowedItemModelDecorator::parent(const QModelIndex &index) const
{
    return m_model->parent(toInternalIndex(index));
}

int WindowedItemModelDecorator::rowCount(const QModelIndex &) const
{
    if (m_minRow == -1)
        return 0;
    return 1 + m_maxRow - m_minRow;
}

int WindowedItemModelDecorator::columnCount(const QModelIndex &) const
{
    if (m_minCol == -1)
        return 0;
    return 1 + m_maxCol - m_minCol;
}

QVariant WindowedItemModelDecorator::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    return m_model->data(toInternalIndex(index), role);
}

QVariant WindowedItemModelDecorator::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (m_minCol == -1)
        return QVariant();
    return m_model->headerData(section + m_minCol, orientation, role);
}

Qt::ItemFlags WindowedItemModelDecorator::flags(const QModelIndex &) const
{
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren;
}

QModelIndex WindowedItemModelDecorator::toInternalIndex(const QModelIndex & index) const
{
    if (!index.isValid())
        return index;

    return m_model->index(index.row() + m_minRow, index.column() + m_minCol);
}

QSharedPointer<QAbstractItemModel> WindowedItemModelDecorator::model() const
{
    return m_model;
}
