#ifndef SQLCOMPLETERENGINE_H
#define SQLCOMPLETERENGINE_H

#include "CompleterEngine.h"
#include "../src/ConnectionManager.h"

class SqlCompleterEngine : public CompleterEngine
{
public:
    ~SqlCompleterEngine() override;
    void updateModel(QStandardItemModel *model, const QPlainTextEdit *editor) override;
    void applyCompletion(QStandardItemModel *model, const QModelIndex &index, const QPlainTextEdit *editor) override;

    void setSqlKeywords(const QStringList &sqlKeywords);
    void setConnectionId(const QString &connectionId);
    void setConnectionManager(ConnectionManager *connectionManager);

private:
    QStringList m_sqlKeywords;
    QString m_connectionId;
    ConnectionManager *m_connectionManager;
};

#endif // SQLCOMPLETERENGINE_H
