#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QCompleter>
#include <QList>
#include <QPlainTextEdit>
#include <QRegularExpression>
#include <QString>
#include <QStringListModel>

#include "ui/Highlighter.h"

class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit CodeEditor(QWidget *parent = nullptr);
    ~CodeEditor();

    QString selectedText();
    void selectQueryAtCursor();

protected:
    void keyPressEvent(QKeyEvent *e) override;

private slots:
    void insertCompletion(const QString &completion);

private:
    void highlightCurrentLine();
    QString textUnderCursor() const;

    Highlighter m_highlighter;
    QCompleter m_completer;
    QList<QRegularExpression> m_nonQueryExpressions;
    QStringListModel *m_completerModel;
};

#endif // CODEEDITOR_H
