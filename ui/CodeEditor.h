#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QList>
#include <QPlainTextEdit>
#include <QRegularExpression>
#include <QString>

#include "ui/Highlighter.h"

class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit CodeEditor(QWidget *parent = nullptr);
    ~CodeEditor();

    QString selectedText();
    void selectQueryAtCursor();

private:
    void highlightCurrentLine();
    Highlighter m_highlighter;
    QList<QRegularExpression> m_nonQueryExpressions;
};

#endif // CODEEDITOR_H
