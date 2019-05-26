#include "Completer.h"

#include <QCoreApplication>

Completer::Completer(QPlainTextEdit *parent): QObject(parent), m_editor(parent)
{
    m_popup = new QTreeView;
    m_popup->setWindowFlags(Qt::Popup);
    m_popup->setFocusPolicy(Qt::NoFocus);
    m_popup->setFocusProxy(parent);
    m_popup->setMouseTracking(true);
    m_popup->setUniformRowHeights(true);
    m_popup->setRootIsDecorated(false);
    m_popup->setEditTriggers(QTreeView::NoEditTriggers);
    m_popup->setSelectionBehavior(QTreeView::SelectRows);
    m_popup->setFrameStyle(QFrame::Box | QFrame::Plain);
    m_popup->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_popup->setHeaderHidden(true);
    m_popup->installEventFilter(this);
    m_popup->setModel(&m_model);
    connect(m_popup, SIGNAL(clicked(const QModelIndex &)), this, SLOT(clicked(const QModelIndex &)));
}

Completer::~Completer()
{
    m_popup->deleteLater();
    m_popup = nullptr;
}

bool Completer::eventFilter(QObject *obj, QEvent *ev)
{
    if (obj != m_popup)
        return false;

    if (ev->type() == QEvent::MouseButtonPress) {
        m_popup->hide();
        m_editor->setFocus();
        return true;
    }

    if (ev->type() == QEvent::KeyPress) {
        int key = static_cast<QKeyEvent*>(ev)->key();
        switch (key) {
        case Qt::Key_Tab:
            if (!m_popup->currentIndex().isValid() && m_model.index(0, 0).isValid())
            {
                m_popup->setCurrentIndex(m_model.index(0, 0));
            }
        [[clang::fallthrough]];
        case Qt::Key_Space:
        case Qt::Key_Enter:
        case Qt::Key_Return:
            if (m_popup->currentIndex().isValid())
            {
                applyCompletion();
                return true;
            }
            break;

        case Qt::Key_Escape:
            m_editor->setFocus();
            m_popup->hide();
            return  true;

        case Qt::Key_Up:
        case Qt::Key_Down:
        case Qt::Key_Home:
        case Qt::Key_End:
        case Qt::Key_PageUp:
        case Qt::Key_PageDown:
            return false;

        default:
            break;
        }

        m_editor->setFocus();
        QCoreApplication::sendEvent(m_editor, ev);

        return false;
    }

    return false;
}

void Completer::keyPressEvent(QKeyEvent *e, void (*keyPressEventCallback)(QPlainTextEdit*, QKeyEvent*))
{
    bool consumed = false;
    if (!m_popup->isVisible() && e->type() == QEvent::KeyPress)
    {
        int key = static_cast<QKeyEvent*>(e)->key();
        Qt::KeyboardModifiers modifiers = static_cast<QKeyEvent*>(e)->modifiers();

        if (modifiers.testFlag(Qt::ControlModifier) && key == Qt::Key_Space)
        {
            update();
            showPopup();
            consumed = true;
        }
    }

    if (!consumed)
    {
        QString before = m_editor->toPlainText();
        keyPressEventCallback(m_editor, e);
        QString after = m_editor->toPlainText();

        if (before != after)
        {
            update();
        }
    }
}

void Completer::showPopup()
{
    if (!m_model.rowCount())
    {
        m_popup->hide();
        return;
    }

    m_popup->resizeColumnToContents(0);
    m_popup->setUpdatesEnabled(true);

    //TODO move popup above/left of cursor if too close to screen edge
    QPoint point = m_editor->cursorRect().center();
    point.setY(point.y() + 20);
    m_popup->move(m_editor->mapToGlobal(point));

    m_popup->setFocus();
    m_popup->show();
}

void Completer::update()
{
    if (!m_completerEngine)
        return;

    m_popup->setUpdatesEnabled(false);
    int rowCountBefore = m_model.rowCount();
    //TODO keep selection between updates
    m_completerEngine->updateModel(&m_model, m_editor);

    // show on new completion, do not show if dismissed then more was typed
    bool shouldShow = m_model.rowCount() && (m_popup->isVisible() || (!rowCountBefore));
    if (shouldShow)
    {
        showPopup();
    }
    else
    {
        m_popup->hide();
    }
}

void Completer::clicked(const QModelIndex &index)
{
    if (!m_completerEngine)
        return;

    m_popup->hide();
    m_editor->setFocus();

    if (index.isValid())
    {
        m_completerEngine->applyCompletion(&m_model, index, m_editor);
    }
}

void Completer::applyCompletion()
{
    clicked(m_popup->currentIndex());
}

void Completer::setCompleterEngine(CompleterEngine *completerEngine)
{
    m_completerEngine = completerEngine;
    m_popup->hide();
}

QTreeView *Completer::popup() const
{
    return m_popup;
}
