#include "ItemModelStyleDecorator.h"

#include <QColor>

ItemModelStyleDecorator::ItemModelStyleDecorator() : QAbstractItemModel(nullptr)
{
    m_nullBackground = QColor("#ffffe0"); //light yellow
    m_nullValue = QVariant();
}

ItemModelStyleDecorator::~ItemModelStyleDecorator() {}

QModelIndex ItemModelStyleDecorator::index(int row, int column, const QModelIndex &) const
{
    if (!m_model->index(row, column).isValid())
        return QModelIndex();
    return QAbstractItemModel::createIndex(row, column); //needed to prevent QModelIndex.data() calls
}

QModelIndex ItemModelStyleDecorator::parent(const QModelIndex &index) const { return m_model->parent(m_model->index(index.row(), index.column())); }

int ItemModelStyleDecorator::rowCount(const QModelIndex &parent) const { return m_model.data() ? m_model->rowCount(parent) : 0; }

int ItemModelStyleDecorator::columnCount(const QModelIndex &parent) const { return m_model.data() ? m_model->columnCount(parent) : 0; }

QVariant ItemModelStyleDecorator::data(const QModelIndex &index, int role) const
{
    QModelIndex index2 = m_model->index(index.row(), index.column());

    switch (role)
    {
    case Qt::BackgroundRole:
    {
        QVariant value = m_model->data(index2);
        return (value.isNull() ? m_nullBackground : QVariant());
        break;
    }
    case Qt::ForegroundRole:
    {
        QVariant value = m_model->data(index2);
        switch (value.type())
        {
        case QVariant::Type::Int:
        case QVariant::Type::UInt:
        case QVariant::Type::Double:
        case QVariant::Type::LongLong:
        case QVariant::Type::ULongLong:
            return m_numberStyle;
        case QVariant::Type::String:
        case QVariant::Type::Char:
            return m_textStyle;
        case QVariant::Type::Date:
        case QVariant::Type::Time:
        case QVariant::Type::DateTime:
            return m_timeStyle;
        default:
            return QVariant();
        }
    }
    default:
    {
        QVariant value = m_model->data(index2, role);
        return value.isNull() ? m_nullValue : value.toString(); // use toString() to avoid locale differences between grid, csv, and clipboard output
    }
    }
}

QVariant ItemModelStyleDecorator::headerData(int section, Qt::Orientation orientation, int role) const { return m_model.data() ? m_model->headerData(section, orientation, role) : QVariant(); }

Qt::ItemFlags ItemModelStyleDecorator::flags(const QModelIndex &) const { return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren; }

void ItemModelStyleDecorator::setModel(const QSharedPointer<QAbstractItemModel> &model)
{
    if (model == m_model)
        return;

    beginResetModel();

    if (m_model)
        m_model->disconnect(this);

    m_model = model;

    if (!m_model)
        return;

    connect(model.data(), SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&, const QVector<int>&)),
            this, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&, const QVector<int>&)));
    connect(model.data(), SIGNAL(rowsInserted(const QModelIndex&, int, int)),
            this, SIGNAL(rowsInserted(const QModelIndex&, int, int)));
    connect(model.data(), SIGNAL(modelReset()),
            this, SIGNAL(modelReset()));

    endResetModel();
}

QVariant ItemModelStyleDecorator::nullBackground() const { return m_nullBackground; }

void ItemModelStyleDecorator::setNullBackground(const QVariant &nullBackground) { m_nullBackground = nullBackground; }

QVariant ItemModelStyleDecorator::nullValue() const { return m_nullValue; }

void ItemModelStyleDecorator::setNullValue(const QVariant &nullValue) { m_nullValue = nullValue; }

QVariant ItemModelStyleDecorator::numberStyle() const { return m_numberStyle; }

void ItemModelStyleDecorator::setNumberStyle(const QVariant &numberStyle) { m_numberStyle = numberStyle; }

QVariant ItemModelStyleDecorator::textStyle() const { return m_textStyle; }

void ItemModelStyleDecorator::setTextStyle(const QVariant &textStyle) { m_textStyle = textStyle; }

QVariant ItemModelStyleDecorator::timeStyle() const { return m_timeStyle; }

void ItemModelStyleDecorator::setTimeStyle(const QVariant &timeStyle) { m_timeStyle = timeStyle; }
