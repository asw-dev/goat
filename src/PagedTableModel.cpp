#include "PagedTableModel.h"

PagedTableModel::PagedTableModel(int maxCacheSize) : m_cache(maxCacheSize)
{
    m_lastInsert = QModelIndex();
}

PagedTableModel::~PagedTableModel() {}

QModelIndex PagedTableModel::index(int row, int column, const QModelIndex &) const
{
    if (column < 0 || column >= m_headerData.columnCount() || row < 0 || row >= m_rowOffsets.size())
        return QModelIndex();

    return QAbstractItemModel::createIndex(row, column);
}

QModelIndex PagedTableModel::parent(const QModelIndex &) const { return QModelIndex(); }

int PagedTableModel::rowCount(const QModelIndex &) const { return m_rowOffsets.size(); }

int PagedTableModel::columnCount(const QModelIndex &) const { return m_headerData.columnCount(); }

QVariant PagedTableModel::data(const QModelIndex &index, int role) const
{
    if (role != Qt::DisplayRole || !index.isValid())
        return QVariant();

    if (index.row() == rowCount() - 1)
    {
        return m_headerData.data(m_headerData.index(0, index.column()));
    }

    if (!m_cache.contains(index.row()))
    {
        m_fileCursorMutex.lock();
        qint64 begin = m_rowOffsets[index.row()];
        qint64 end = m_rowOffsets.size() == index.row() + 1 ? m_tmpFile.size() : m_rowOffsets[index.row() + 1];
        m_tmpFile.seek(begin);
        QByteArray data = m_tmpFile.read(end - begin);
        QDataStream ds(&data, QIODevice::ReadOnly);

        QVector<QVariant> *row = new QVector<QVariant>();
        for (int column = 0; column < m_headerData.columnCount(); ++column)
        {
            *row << ds;
        }
        m_cache.insert(index.row(), row);
        m_fileCursorMutex.unlock();
    }

    return m_cache[index.row()]->at(index.column());
}

QVariant PagedTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Vertical)
    {
         if(role == Qt::DisplayRole)
             return QVariant(section + 1);
         return QVariant();
    }
    return m_headerData.headerData(section, orientation, role);
}

Qt::ItemFlags PagedTableModel::flags(const QModelIndex &) const { return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren | Qt::ItemIsEditable; }

bool PagedTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::EditRole)
        return false;

    if (!index.isValid())
        return false;

    QModelIndex expected;
    if (!m_lastInsert.isValid())
        expected = QAbstractItemModel::createIndex(0, 0); //first cell
    else if (m_lastInsert.column() != columnCount() - 1)
        expected = QAbstractItemModel::createIndex(m_lastInsert.row(), m_lastInsert.column() + 1); //next column
    else
        expected = QAbstractItemModel::createIndex(m_lastInsert.row() + 1, 0); //next row

    if (index != expected)
        return false;

    m_headerData.setData(m_headerData.index(0, index.column()), value);
    m_lastInsert = index;
    emit dataChanged(index, index);
    return true;
}

bool PagedTableModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
    if (orientation == Qt::Vertical)
        return false;

    bool changed = m_headerData.setHeaderData(section, orientation, value, role);
    if (changed)
        emit headerDataChanged(orientation, section, section);
    return changed;
}

bool PagedTableModel::insertColumns(int column, int count, const QModelIndex &parent)
{
    if (columnCount())
        return false;

    this->beginInsertColumns(parent, column, column + count - 1);
    bool changed = m_headerData.insertColumns(column, count, parent);
    this->endInsertColumns();
    return changed;
}

bool PagedTableModel::insertRows(int row, int count, const QModelIndex &parent)
{
    if (count != 1 || row != rowCount())
        return false;

    m_fileCursorMutex.lock();

    if (!m_tmpFile.isOpen())
    {
        if (!m_tmpFile.open())
        {
            m_fileCursorMutex.unlock();
            return false;
        }
    }

    if (m_headerData.rowCount())
    {
        if (!m_tmpFile.seek(m_tmpFile.size()))
        {
            m_fileCursorMutex.unlock();
            return false;
        }

        QByteArray data;
        QDataStream ds(&data, QIODevice::WriteOnly);

        for (int column = 0; column < m_headerData.columnCount(); ++column)
        {
            QVariant value = m_headerData.data(m_headerData.index(0, column));
            ds << value;
        }

        if (m_tmpFile.write(data) == -1)
        {
            m_fileCursorMutex.unlock();
            return false;
        }

        //pre-populate cache to avoid gui locking on the first rows
        if (m_cache.size() < m_cache.maxCost())
        {
            QVector<QVariant> *rowData = new QVector<QVariant>();
            for (int column = 0; column < m_headerData.columnCount(); ++column)
            {
                QVariant value = m_headerData.data(m_headerData.index(0, column));
                rowData->append(value);
            }
            m_cache.insert(row - 1, rowData);
        }

        m_headerData.removeRow(0);
    }

    m_fileCursorMutex.unlock();

    this->beginInsertRows(parent, row, row + count - 1);
    m_rowOffsets.append(m_tmpFile.size());
    m_headerData.insertRow(0);
    this->endInsertRows();
    return true;
}
