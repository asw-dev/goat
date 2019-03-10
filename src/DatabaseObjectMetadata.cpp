#include "DatabaseObjectMetadata.h"

DatabaseObjectMetadata::DatabaseObjectMetadata()
{

}

QString DatabaseObjectMetadata::id() const
{
    return m_id;
}

void DatabaseObjectMetadata::setId(const QString &id)
{
    m_id = id;
}

QString DatabaseObjectMetadata::parentId() const
{
    return m_parentId;
}

void DatabaseObjectMetadata::setParentId(const QString &parentId)
{
    m_parentId = parentId;
}

QString DatabaseObjectMetadata::name() const
{
    return m_name;
}

void DatabaseObjectMetadata::setName(const QString &name)
{
    m_name = name;
}

QString DatabaseObjectMetadata::type() const
{
    return m_type;
}

void DatabaseObjectMetadata::setType(const QString &type)
{
    m_type = type;
}
