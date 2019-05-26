#ifndef COMPLETERENGINE_H
#define COMPLETERENGINE_H

#include <QStandardItemModel>
#include <QString>
#include <QTextCursor>
#include <QPlainTextEdit>

class CompleterEngine
{
public:
    enum Role {
        DisplayRole=Qt::DisplayRole,
        CompletionRole=Qt::UserRole,
        OrderRole
    };

    virtual ~CompleterEngine();
    virtual void updateModel(QStandardItemModel *model, const QPlainTextEdit *editor);
    virtual void applyCompletion(QStandardItemModel *model, const QModelIndex &index, const QPlainTextEdit *editor);
};

#endif // COMPLETERENGINE_H
