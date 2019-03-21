#include "CodeEditor.h"

#include <QAbstractItemView>
#include <QScrollBar>
#include <QSettings>
#include <QShortcut>
#include <QStringListModel>
#include <QTextBlock>
#include <QTextCursor>

CodeEditor::CodeEditor(QWidget *parent) : QPlainTextEdit(parent)
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
    if (settings.value("4/TYPE").toString() == "KEYWORD")
    {
        m_sqlKeywords = settings.value("4/ITEMS").toStringList();
    }

    QStringList list;
    m_completerModel = new QStringListModel(list);
    m_completer.setWidget(this);
    m_completer.setModel(m_completerModel);
    m_completer.setCaseSensitivity(Qt::CaseInsensitive);
    m_completer.setModelSorting(QCompleter::CaseInsensitivelySortedModel);
    m_completer.setCompletionMode(QCompleter::PopupCompletion);
    m_completer.setFilterMode(Qt::MatchContains);
    m_completer.setWrapAround(false);
    QObject::connect(&m_completer, SIGNAL(activated(QString)), this, SLOT(insertCompletion(QString)));
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

QString CodeEditor::textUnderCursor() const
{
    QTextCursor tc = textCursor();
    tc.select(QTextCursor::WordUnderCursor);
    return tc.selectedText();
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
    static int minAutoCompleteSize = 2;
    static QString endOfWord("~!@#$%^&*()_+{}|:\"<>?,./;'[]\\-= \t\n");

    QCompleter *c = &m_completer;

    if (!c)
    {
        QPlainTextEdit::keyPressEvent(e);
        return;
    }

    bool popupVisible = c->popup()->isVisible();
    bool isShortcut = ((e->modifiers() & Qt::ControlModifier) && e->key() == Qt::Key_Space);

    if (popupVisible)
    {
        // The following keys are forwarded by the completer to the widget
        switch (e->key())
        {
        case Qt::Key_Enter:
        case Qt::Key_Return:
        case Qt::Key_Escape:
        case Qt::Key_Tab:
        case Qt::Key_Backtab:
            e->ignore();
            return; // let the completer do default behavior
        default:
            break;
        }
    }

    QPlainTextEdit::keyPressEvent(e);

    QString completionPrefix = textUnderCursor();
    QString oldPrefix = c->completionPrefix();
    QString blockText = queryAtCursor().selectedText().replace("\u2029", "\n", Qt::CaseInsensitive);
    QString fullText = toPlainText();
    updateCompleterModel(completionPrefix, blockText, fullText);
    c->setCompletionPrefix(completionPrefix);
    c->popup()->setCurrentIndex(c->completionModel()->index(0, 0));
    QRect cr = cursorRect();
    cr.setWidth(200);

    bool hasCompletions = c->completionCount() != 0;
    bool isEndOfWord = endOfWord.contains(e->text().right(1));
    bool prefixChanged = oldPrefix != completionPrefix;

    bool shouldHide = popupVisible && (!hasCompletions || isEndOfWord);
    bool shouldShow = !popupVisible
            && hasCompletions
            && (
                isShortcut
                || (
                    !isEndOfWord
                    && completionPrefix.length() == minAutoCompleteSize //using == so it does not popup after esc and more typing
                    && prefixChanged //ex: pushing esc when the popup is hidden should not raise the popup
            ));

    if (shouldShow)
    {
        c->complete(cr);
    }
    else if (shouldHide)
    {
        c->popup()->hide();
    }
}

void CodeEditor::updateCompleterModel(const QString &textUnderCursor, const QString &blockText, const QString &fullText)
{
    QStringList list;

    foreach (QString keyword, m_sqlKeywords) {
        if (keyword.toLower().startsWith(textUnderCursor.toLower()))
            list += keyword;
    }

    if (m_connectionManager)
    {
        QHash<QString /*databasObjectId*/, DatabaseObjectMetadata> databaseMetadata = m_connectionManager->getMetadata(m_connectionId);

        foreach (DatabaseObjectMetadata metadata, databaseMetadata.values())
        {
            if (!metadata.name().toLower().startsWith(textUnderCursor.toLower()))
                continue;

            if (metadata.type() == "table")
                list += metadata.name();
            else if (metadata.type() == "column")
            {
                if (blockText.contains(databaseMetadata[metadata.parentId()].name()))
                    list += metadata.name();
            }
        }
    }

    //any previous text in the editor
    foreach(QString word, fullText.split(QRegExp("[^a-zA-Z\\d]", Qt::CaseInsensitive)))
    {
        if (word.length() >= 3 && word.toLower().startsWith(textUnderCursor.toLower()) && word != textUnderCursor)
            list += word;
    }

    m_completerModel->setStringList(list);
}

void CodeEditor::setConnectionId(const QString &connectionId)
{
    m_connectionId = connectionId;
}

void CodeEditor::setConnectionManager(ConnectionManager *connectionManager)
{
    m_connectionManager = connectionManager;
}
