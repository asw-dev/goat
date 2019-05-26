#include "CompleterEngine.h"

#include <QSet>
#include <assert.h>

CompleterEngine::~CompleterEngine() {}

void CompleterEngine::updateModel(QStandardItemModel *model, const QPlainTextEdit *editor)
{
    QString text = editor->toPlainText();
    QTextCursor cursor = editor->textCursor();

    QStringList list = text.split(QRegExp("[^\\w]"));
    QSet<QString> used;

    cursor.select(QTextCursor::WordUnderCursor);
    QString prefix = cursor.selectedText();

    model->clear();
    model->insertColumn(0);

    if (prefix.size() < 1)
        return;

    for (int i = 0; i < list.count(); ++i) {
        QString word = list[i];
        if (used.contains(word))
            continue;
        used.insert(word);

        if (!word.startsWith(prefix))
            continue;
        if (word == prefix)
            continue; //no point in suggesting what you already have

        model->insertRows(0, 1);
        QModelIndex index = model->index(0, 0);
        model->setData(index, list[i], Role::DisplayRole);
        model->setData(index, list[i], Role::CompletionRole);
        model->setData(index, list[i], Role::OrderRole);
    }
}

void CompleterEngine::applyCompletion(QStandardItemModel *model, const QModelIndex &index, const QPlainTextEdit *editor)
{
    assert(index.isValid());

    QString completion = model->data(index, Qt::UserRole).toString();
    QTextCursor cursor = editor->textCursor();
    cursor.select(QTextCursor::WordUnderCursor);
    QString prefix = cursor.selectedText();
    QString remainder = completion.mid(prefix.size());
    editor->textCursor().insertText(remainder);
}
