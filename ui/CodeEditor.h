#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QCompleter>
#include <QList>
#include <QPlainTextEdit>
#include <QRegularExpression>
#include <QString>
#include <QStringListModel>

#include "src/ConnectionManager.h"
#include "src/DatabaseObjectMetadata.h"
#include "ui/Highlighter.h"

class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit CodeEditor(QWidget *parent = nullptr);
    ~CodeEditor();

    QString selectedText();
    QTextCursor queryAtCursor();
    void setConnectionManager(ConnectionManager *connectionManager);
    void setConnectionId(const QString &connectionId);

protected:
    void keyPressEvent(QKeyEvent *e) override;

private slots:
    void insertCompletion(const QString &completion);

private:
    void highlightCurrentLine();
    QString textUnderCursor() const;
    void updateCompleterModel(const QString &textUnderCursor, const QString &blockText, const QString &fullText);

    Highlighter m_highlighter;
    QCompleter m_completer;
    QList<QRegularExpression> m_nonQueryExpressions;
    QStringListModel *m_completerModel;
    QStringList m_sqlKeywords;
    ConnectionManager *m_connectionManager;
    QString m_connectionId;
};

#endif // CODEEDITOR_H
