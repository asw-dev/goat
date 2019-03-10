#ifndef DATABASEOBJECTMETADATA_H
#define DATABASEOBJECTMETADATA_H

#include <QString>

class DatabaseObjectMetadata
{
public:
    DatabaseObjectMetadata();

    QString id() const;
    void setId(const QString &id);

    QString parentId() const;
    void setParentId(const QString &parentId);

    QString name() const;
    void setName(const QString &name);

    QString type() const;
    void setType(const QString &type);

private:
    QString m_id;
    QString m_parentId;
    QString m_name;
    QString m_type;
};

#endif // DATABASEOBJECTMETADATA_H
