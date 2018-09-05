#ifndef PAGEDTABLEMODEL_H
#define PAGEDTABLEMODEL_H

#include <QAbstractItemModel>
#include <QCache>
#include <QMutex>
#include <QObject>
#include <QStandardItemModel>
#include <QTemporaryFile>
#include <QVariant>
#include <QVector>

/*!
 * \brief A table that is mostly held in a temporary file.
 *
 *  Supports appending. Modifying existing rows is not supported. Only supports the Edit/Display role. Thread safe.
 */
class PagedTableModel : public QAbstractItemModel
{

  public:
    PagedTableModel(int maxCacheSize = 500);
    ~PagedTableModel();
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole);
    bool insertColumns(int column, int count, const QModelIndex &parent = QModelIndex());
    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex());

  private:
    QVector<qint64> m_rowOffsets;
    QStandardItemModel m_headerData;
    QModelIndex m_lastInsert;
    mutable QTemporaryFile m_tmpFile;
    mutable QCache<int, QVector<QVariant>> m_cache;
    mutable QMutex m_fileCursorMutex;
};

#endif // PAGEDTABLEMODEL_H
