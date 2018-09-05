#ifndef WINDOWEDITEMMODELDECORATOR_H
#define WINDOWEDITEMMODELDECORATOR_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QSharedPointer>

/*!
 * \brief A read only decorator that views a subsecton of the model being decorated.
 *
 * Signals from the decorated are discarded.
 */
class WindowedItemModelDecorator : public QAbstractItemModel
{
  public:
    WindowedItemModelDecorator();
    ~WindowedItemModelDecorator();
    QSharedPointer<QAbstractItemModel> model() const;
    void setModel(const QSharedPointer<QAbstractItemModel> &model);

    /*!
     * \brief setRange changes the subsection of the model being viewed
     *
     * Will raise rowsInserted() or modelReset() as needed.
     *
     * \param minRow -1 for no rows
     * \param minCol -1 for no columns
     * \param maxRow 0+
     * \param maxCol 0+
     */
    void setRange(int minRow, int minCol, int maxRow, int maxCol);

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;

private:
    QModelIndex toInternalIndex(const QModelIndex & index) const;
    QSharedPointer<QAbstractItemModel> m_model;
    int m_minCol;
    int m_minRow;
    int m_maxCol;
    int m_maxRow;
};

#endif // WINDOWEDITEMMODELDECORATOR_H
