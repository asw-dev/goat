#ifndef QUERYTAB_H
#define QUERYTAB_H

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

private:
    Ui::QueryTab *ui;
    QSqlQueryModel* m_queryResultsModel;
    QString m_filename;
};

#endif // QUERYTAB_H
