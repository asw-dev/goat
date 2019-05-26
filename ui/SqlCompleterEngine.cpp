#include "SqlCompleterEngine.h"

#include <QSet>

SqlCompleterEngine::~SqlCompleterEngine() {}

void addRow(QStandardItemModel *model, const QString &completion, const QList<int> &matchedChars, const QString &identifier, const QString &type, const QVariant &rank)
{
    model->insertRows(0, 1);
    QModelIndex index = model->index(0, 0);

    QString typeInfo = identifier + " (" + type + ")";

    QString suggestion;
    for (int i = 0; i < completion.size(); ++i)
    {
        if (matchedChars.contains(i))
            suggestion.append("<span style=\"font-weight: bold;\">" + QString(completion.at(i)).toHtmlEscaped() + "</span>");
        else
            suggestion.append(QString(completion.at(i)).toHtmlEscaped());
    }

    QString htmlPreview;
    htmlPreview += "<html>";
    htmlPreview += "    <body >";
    htmlPreview += "        <p style=\"font-size: large;\">" + suggestion + "</p>";
    htmlPreview += "        <p align=\"right\" style=\"font-size: small;\">" + typeInfo.toHtmlEscaped() + "</p>";
    htmlPreview += "    </body>";
    htmlPreview += "</html>";
    model->setData(index, htmlPreview, CompleterEngine::Role::DisplayRole);
    model->setData(index, completion, CompleterEngine::Role::CompletionRole);
    model->setData(index, rank, CompleterEngine::Role::OrderRole);
}

bool isMatch(QString prefix, QString completion)
{
    if (!completion.startsWith(prefix, Qt::CaseSensitivity::CaseInsensitive))
        return false;
    return completion.toLower() != prefix.toLower();
}

QList<int> matchedChars(QString prefix, QString completion)
{
    QList<int> list;
    for (int i = 0; i < prefix.size(); ++i)
    {
        if (prefix[i].toLower() == completion[i].toLower())
        {
            list.append(i);
        }
    }
    return list;
}

void SqlCompleterEngine::updateModel(QStandardItemModel *model, const QPlainTextEdit *editor)
{
    QString text = editor->toPlainText();
    QTextCursor cursor = editor->textCursor();

    QStringList list = m_sqlKeywords;

    QSet<QString> used;

    //TODO only left of cursor
    //TODO support completions beyond "word" in case of fuzzy identifier completion ex: "ms.mt -> mySchema.myTable"
    cursor.select(QTextCursor::WordUnderCursor);
    QString prefix = cursor.selectedText();

    model->clear();
    model->insertColumn(0);

    if (prefix.size() < 1)
        return;

    foreach(QString word, list) {
        if (used.contains(word.toLower()))
            continue;
        used.insert(word.toLower());

        if (isMatch(prefix, word))
        {
            addRow(model, word, matchedChars(prefix, word), "", "keyword", word);  //TODO order by best match
        }

    }

    if (!m_connectionId.isEmpty() && m_connectionManager)
    {
        QHash<QString, DatabaseObjectMetadata> databaseMetadata = m_connectionManager->getMetadata(m_connectionId);

        foreach(DatabaseObjectMetadata metadata, databaseMetadata) {
            if (!(metadata.type() == "table"
                  || (metadata.type() == "column" && text.contains(databaseMetadata[metadata.parentId()].name()))))
                continue;
            QString word = metadata.name();
            if (used.contains(word.toLower()))
                continue;
            used.insert(word.toLower());
            if (isMatch(prefix, word))
            {
                addRow(model, word, matchedChars(prefix, word), metadata.id(), metadata.type(), word); //TODO order by best match
            }
        }
    }

    list = text.split(QRegExp("[^\\w]"));
    foreach(QString word, list) {
        if (used.contains(word.toLower()))
            continue;
        used.insert(word.toLower());

        if (isMatch(prefix, word))
        {
            addRow(model, word, matchedChars(prefix, word), "", "other", word); //TODO order by best match
        }
    }

    model->setSortRole(CompleterEngine::Role::OrderRole);
    model->sort(0);
}

void SqlCompleterEngine::applyCompletion(QStandardItemModel *model, const QModelIndex &index, const QPlainTextEdit *editor)
{
    assert(index.isValid());

    QString completion = model->data(index, Qt::UserRole).toString();
    QTextCursor cursor = editor->textCursor();
    cursor.select(QTextCursor::WordUnderCursor);
    cursor.removeSelectedText();
    cursor.insertText(completion);
}

void SqlCompleterEngine::setSqlKeywords(const QStringList &sqlKeywords)
{
    m_sqlKeywords = sqlKeywords;
}

void SqlCompleterEngine::setConnectionId(const QString &connectionId)
{
    m_connectionId = connectionId;
}

void SqlCompleterEngine::setConnectionManager(ConnectionManager *connectionManager)
{
    m_connectionManager = connectionManager;
}
