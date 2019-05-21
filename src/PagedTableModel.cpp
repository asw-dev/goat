#include "PagedTableModel.h"

PagedTableModel::PagedTableModel(int maxCacheSize) : m_cache(maxCacheSize)
{
    m_lastInsert = QModelIndex();
    m_rows = 0;
    m_columns = 0;
}

PagedTableModel::~PagedTableModel() {}

QModelIndex PagedTableModel::index(int row, int column, const QModelIndex &) const
{
    if (column < 0 || column >= m_columns || row < 0 || row >= m_rows) {
        return QModelIndex();
    }

    return QAbstractItemModel::createIndex(row, column);
}

QModelIndex PagedTableModel::parent(const QModelIndex &) const { return QModelIndex(); }

int PagedTableModel::rowCount(const QModelIndex &) const { return m_rows; }

int PagedTableModel::columnCount(const QModelIndex &) const { return m_columns; }

QVariant PagedTableModel::data(const QModelIndex &index, int role) const
{
    if (role != Qt::DisplayRole || !index.isValid())
        return QVariant();

    if (index.row() == m_rows - 1)
    {
        QVariant value;
        m_fileCursorMutex.lock();
        bool stillLastRow;
        if (index.row() == m_rows - 1)
        {
            stillLastRow = true;
            value = m_headerData.data(m_headerData.index(0, index.column()));
        }
        else {
            stillLastRow = false;
        }
        m_fileCursorMutex.unlock();
        if (stillLastRow)
            return value;
    }

    if (!m_cache.contains(index.row()))
    {
        m_cache.insert(index.row(), new QVector<QVariant>(readRow(index.row())));
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
    m_columns = m_headerData.columnCount();
    this->endInsertColumns();
    return changed;
}

bool PagedTableModel::insertRows(int row, int count, const QModelIndex &parent)
{
    if (count != 1 || row != rowCount())
        return false;

    if (m_headerData.rowCount())
    {
        QVector<QVariant> data;
        for (int column = 0; column < m_headerData.columnCount(); ++column)
        {
            QVariant value = m_headerData.data(m_headerData.index(0, column));
            data.append(value);
        }
        appendRow(data);
        m_headerData.removeRow(0);
    }

    this->beginInsertRows(parent, row, row + count - 1);
    m_headerData.insertRow(0);
    ++m_rows;
    this->endInsertRows();

    return true;
}

QVector<QVariant> PagedTableModel::readRow(int row) const
{
    QVector<QVariant> ret;

    //TODO handle file errors

    m_fileCursorMutex.lock();
    assert(m_idxFile.isOpen());
    assert(m_dataFile.isOpen());
    qint64 idxPos = m_idxFile.pos();
    qint64 dataPos = m_dataFile.pos();
    assert(idxPos != -1);
    assert(dataPos != -1);

    qint64 size = sizeof(qint64);
    qint64 dataOffset;
    qint64 nextDataOffset;

    assert(m_idxFile.seek(size * row));
    QByteArray bytes = m_idxFile.read(size);
    assert(bytes.size());
    QDataStream iStream(&bytes, QIODevice::ReadOnly);
    iStream >> dataOffset;
    if (row + 2 < m_rows) //is the next row offset on file? (2 instead of 1 because last row is in memory)
    {
        bytes = m_idxFile.read(size);
        assert(bytes.size());
        QDataStream iStream2(&bytes, QIODevice::ReadOnly);
        iStream2 >> nextDataOffset;
    }
    else {
        nextDataOffset = dataPos;
    }

    assert(m_dataFile.seek(dataOffset));
    bytes = m_dataFile.read(nextDataOffset - dataOffset);
    assert(bytes.size());
    QDataStream dStream(&bytes, QIODevice::ReadOnly);
    for (int i = 0; i < m_columns; ++i)
    {
        QVariant value;
        dStream >> value;
        ret.push_back(value);
    }

    assert(m_idxFile.seek(idxPos));
    assert(m_dataFile.seek(dataPos));
    m_fileCursorMutex.unlock();

    return ret;
}

void PagedTableModel::appendRow(const QVector<QVariant> &data)
{
    assert(data.size() == m_columns);

    //TODO handle file errors

    m_fileCursorMutex.lock();

    if (!m_idxFile.isOpen())
        assert(m_idxFile.open());
    if (!m_dataFile.isOpen())
        assert(m_dataFile.open());

    qint64 dataPos = m_dataFile.pos();
    assert(dataPos != -1);

    QByteArray dBytes;
    QDataStream dStream(&dBytes, QIODevice::WriteOnly);

    for (int i = 0; i < data.size(); ++i)
    {
        QVariant value = m_headerData.data(m_headerData.index(0, i));
        dStream << value;
    }
    assert(m_dataFile.write(dBytes) == dBytes.size());

    QByteArray iBytes;
    QDataStream iStream(&iBytes, QIODevice::WriteOnly);
    iStream << dataPos;
    assert(m_idxFile.write(iBytes) == iBytes.size());

    m_fileCursorMutex.unlock();
}
