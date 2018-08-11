#ifndef QUERYTAB_H
#define QUERYTAB_H

#include "src/QueryState.h"

#include <QPlainTextEdit>
#include <QString>
#include <QWidget>
#include <QWidget>
#include <QSqlDatabase>
#include <QTableView>
#include <QSqlQueryModel>
#include <QSplitter>
#include <QComboBox>

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
    void executeQueryAtCursor(QSqlDatabase sqlDatabase);
    void executeQuery(QSqlDatabase sqlDatabase, QString query);
    QString filename() const;
    void setFilename(const QString &filename);
    bool modified() const;
    void setModified(const bool &modified);
    void readFile();
    void writeFile();
    void clearResults();
    QueryState queryState() const;
    bool hasResults();

public slots:
    void on_button_exportQueryResults_clicked();

private:
    Ui::QueryTab *ui;
    QSqlQueryModel* m_queryResultsModel;
    QString m_filename;
    QueryState m_queryState;

    void onQuerySucess(QSqlQuery query, QDateTime start);
    void onQueryFailure(QSqlQuery query, QDateTime start);
};

#endif // QUERYTAB_H
