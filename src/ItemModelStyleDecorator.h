#ifndef ITEMMODELSTYLEDECORATOR_H
#define ITEMMODELSTYLEDECORATOR_H

#include <QAbstractItemModel>
#include <QSharedPointer>
#include <QVariant>

/*!
 * \brief A read only decorator that overrides the return values of data() for visual roles.
 *
 * Signals from the decorated are raised in most cases.
 */
class ItemModelStyleDecorator : public QAbstractItemModel
{
public:
    ItemModelStyleDecorator();
    ~ItemModelStyleDecorator();

    QVariant nullBackground() const;
    void setNullBackground(const QVariant &nullBackground);
    QVariant nullValue() const;
    void setNullValue(const QVariant &nullValue);
    QVariant numberStyle() const;
    void setNumberStyle(const QVariant &numberStyle);
    QVariant textStyle() const;
    void setTextStyle(const QVariant &textStyle);
    QVariant timeStyle() const;
    void setTimeStyle(const QVariant &timeStyle);

    void setModel(const QSharedPointer<QAbstractItemModel> &model);

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;

private:
    QSharedPointer<QAbstractItemModel> m_model;
    QVariant m_nullBackground;
    QVariant m_nullValue;
    QVariant m_numberStyle;
    QVariant m_textStyle;
    QVariant m_timeStyle;
};

#endif // ITEMMODELSTYLEDECORATOR_H
