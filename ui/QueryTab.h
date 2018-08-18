#ifndef QUERYTAB_H
#define QUERYTAB_H

#include "src/QueryState.h"
#include "src/Query.h"
#include "ui/CodeEditor.h"

#include <QString>
#include <QWidget>

namespace Ui {
class QueryTab;
}

class QueryTab : public QWidget
{
	Q_OBJECT

signals:
   void textChanged();
   void queryStateChanged();

public:
    explicit QueryTab(QString filename, QWidget *parent = 0);
   ~QueryTab();
    void executeQuery(const Connection &connection, Credentials *credentials);
    QString filename() const;
    void setFilename(const QString &filename);
    bool modified() const;
    void setModified(const bool &modified);
    void readFile();
    void writeFile();
    void clearResults();
    QueryState queryState() const;
    bool hasResults();
    CodeEditor* codeEditor();

public slots:
    void on_button_exportQueryResults_clicked();

private:
    Ui::QueryTab *ui;
    QString m_filename;
    QueryState m_queryState;
    Query *m_query;
    QThread *m_queryThread;

private slots:
    void onQuerySucess();
    void onQueryFailure();
};

#endif // QUERYTAB_H
