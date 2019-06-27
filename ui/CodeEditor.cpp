#include "CodeEditor.h"

#include <QAbstractItemView>
#include <QScrollBar>
#include <QSettings>
#include <QShortcut>
#include <QStringListModel>
#include <QTextBlock>
#include <QTextCursor>

CodeEditor::CodeEditor(QWidget *parent) : QPlainTextEdit(parent), m_completer(this)
{
    setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    setStyleSheet("CodeEditor { font: 18px; }");
    setTabStopWidth(fontMetrics().width(' ') * 4);

    connect(this, &QPlainTextEdit::cursorPositionChanged, this, &CodeEditor::highlightCurrentLine);

    m_nonQueryExpressions.append(QRegularExpression("--.*")); //comment
    m_nonQueryExpressions.append(QRegularExpression("^\\s*$", QRegularExpression::MultilineOption)); //empty line
    m_nonQueryExpressions.append(QRegularExpression("/\\*+[^*]*\\*+(?:[^/*][^*]*\\*+)*/", QRegularExpression::MultilineOption)); //multi-line comment

    m_highlighter.setDocument(document());
    highlightCurrentLine();

    QSettings settings(":/syntax/syntax-highlight/sql.ini", QSettings::IniFormat, this);
    QStringList sqlKeywords;
    if (settings.value("4/TYPE").toString() == "KEYWORD")
    {
        sqlKeywords.append(settings.value("4/ITEMS").toStringList());
    }

    m_sqlCompleterEngine.setSqlKeywords(sqlKeywords);

    m_completer.popup()->setItemDelegate(&m_htmlStyleDelegate);
    m_completer.setCompleterEngine(&m_sqlCompleterEngine);
}

CodeEditor::~CodeEditor() {}

void CodeEditor::highlightCurrentLine()
{
    QTextEdit::ExtraSelection selection;
    selection.format.setBackground(QColor("#F0F0E4"));
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    selection.cursor = textCursor();
    selection.cursor.clearSelection();

    QList<QTextEdit::ExtraSelection> extraSelections;
    extraSelections.append(selection);
    setExtraSelections(extraSelections);
}

QString CodeEditor::selectedText()
{
    return textCursor().selectedText().replace("\u2029", "\n", Qt::CaseInsensitive);
}

QTextCursor CodeEditor::queryAtCursor()
{
    QTextCursor cursor = textCursor();
    bool hasSelection = cursor.position() != cursor.anchor();
    if (hasSelection)
        return cursor;

    QString txt = toPlainText();

    QList<int> lineStarts;
    lineStarts.append(0);
    int index = -1;
    while ((index = txt.indexOf("\n", index + 1)) != -1)
        lineStarts.append(index + 1);

    QList<int> lineEnds;
    for(int i = 0; i < lineStarts.count(); ++i)
    {
        if (i + 1 < lineStarts.count())
            lineEnds.append(lineStarts[i+1] - 1);
        else
            lineEnds.append(txt.count());
    }

    int startLineIdx = -1;
    int endLineIdx;

    QList<bool> isUsable;
    for (int i = 0; i < lineStarts.count(); ++i)
        isUsable.append(true);
    foreach (QRegularExpression regEx, m_nonQueryExpressions) //TODO it would be better to use a sql parser instead of regex
    {
        index = -1;
        QRegularExpressionMatch match = regEx.match(txt, index + 1);
        while (match.hasMatch())
        {
            int s = match.capturedStart();
            int e = match.capturedEnd();
            for (int i = 0; i < lineStarts.count(); ++i)
            {
                if (lineStarts[i] >= s && lineStarts[i] <= e)
                    isUsable[i] = false;
            }
            match = regEx.match(txt, e + 1);
        }
    }

    //start with the line the cursor is on
    for (int i = 0; i < lineStarts.count(); ++i)
    {
        int consider = lineStarts[i];
        if (consider <= cursor.position())
            startLineIdx = i;
    }
    endLineIdx = startLineIdx;

    if (!isUsable[startLineIdx])
        return cursor; //cursor is not on a query

    //expand the selection
    while (true)
    {
            if (startLineIdx > 0 && isUsable[startLineIdx - 1])
            {
                --startLineIdx;
                continue;
            }

            if (endLineIdx < lineStarts.count() - 1 && isUsable[endLineIdx + 1])
            {
                ++endLineIdx;
                continue;
            }

            break;
    }

    cursor.setPosition(lineStarts[startLineIdx]);
    cursor.setPosition(lineEnds[endLineIdx], QTextCursor::KeepAnchor);
    return cursor;
}

void CodeEditor::insertCompletion(const QString &completion)
{
    QTextCursor tc = textCursor();
    tc.movePosition(QTextCursor::StartOfWord);
    tc.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
    tc.removeSelectedText();
    tc.insertText(completion + " ");
}

void CodeEditor::keyPressEvent(QKeyEvent *e)
{
    auto callback = [](QPlainTextEdit* x, QKeyEvent* y){ ((CodeEditor*) x)->qPlainTextEditKeyPressEvent(y); };
    m_completer.keyPressEvent(e, callback);
}

void CodeEditor::setConnectionId(const QString &connectionId)
{
    m_connectionId = connectionId;
    m_sqlCompleterEngine.setConnectionId(connectionId);
}

void CodeEditor::setConnectionManager(ConnectionManager *connectionManager)
{
    m_connectionManager = connectionManager;
    m_sqlCompleterEngine.setConnectionManager(connectionManager);
}
