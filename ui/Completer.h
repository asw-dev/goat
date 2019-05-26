#ifndef COMPLETER_H
#define COMPLETER_H

#include <QKeyEvent>
#include <QPlainTextEdit>
#include <QStandardItemModel>
#include <QTimer>
#include <QTreeView>

#include "CompleterEngine.h"
#include "HtmlStyleDelegate.h"

class Completer : public QObject
{
    Q_OBJECT

public:
    Completer(QPlainTextEdit *parent = nullptr);
    ~Completer() override;

    QTreeView *popup() const;
    void setCompleterEngine(CompleterEngine *completerEngine);

    void keyPressEvent(QKeyEvent *e, void (*keyPressEventCallback)(QPlainTextEdit*, QKeyEvent*));
    bool eventFilter(QObject *obj, QEvent *ev) override;
    void showPopup();
    void update();
    void applyCompletion();

private slots:
    void clicked(const QModelIndex &index);

private:
    CompleterEngine *m_completerEngine;
    QPlainTextEdit *m_editor;
    QTreeView *m_popup;
    QStandardItemModel m_model;
};

#endif // COMPLETER_H
