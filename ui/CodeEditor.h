#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QCompleter>
#include <QList>
#include <QPlainTextEdit>
#include <QRegularExpression>
#include <QString>
#include <QStringListModel>

#include "../src/ConnectionManager.h"
#include "../src/DatabaseObjectMetadata.h"
#include "Completer.h"
#include "Highlighter.h"
#include "HtmlStyleDelegate.h"
#include "SqlCompleterEngine.h"

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
    void updateCompleterModel(const QString &textUnderCursor, const QString &blockText, const QString &fullText);

    Highlighter m_highlighter;
    Completer m_completer;
    SqlCompleterEngine m_sqlCompleterEngine;
    HtmlStyleDelegate m_htmlStyleDelegate;
    QList<QRegularExpression> m_nonQueryExpressions;
    ConnectionManager *m_connectionManager;
    QString m_connectionId;
};

#endif // CODEEDITOR_H
