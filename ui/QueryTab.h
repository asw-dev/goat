#ifndef QUERYTAB_H
#define QUERYTAB_H

#include "../src/ConnectionManager.h"
#include "../src/Credentials.h"
#include "../src/ItemModelStyleDecorator.h"
#include "../src/Query.h"
#include "../src/QueryResult.h"
#include "../src/QueryState.h"
#include "../src/WindowedItemModelDecorator.h"
#include "CodeEditor.h"

#include <QString>
#include <QWidget>
#include <QSharedPointer>
#include <QAbstractItemModel>

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
    QueryTab(QString filename, ConnectionManager *connectionManager, Credentials *credentials, QWidget *parent = 0);
   ~QueryTab();
    void executeQuery(const QString &connectionId);
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
    bool canCancel();
    void cancel();
    QString connectionId() const;
    void setConnectionId(const QString &connectionId);

public slots:
    void on_exportResultsToClipboard_clicked();
    void on_exportResultsToFileButton_clicked();

private:
    Ui::QueryTab *ui;
    ConnectionManager *m_connectionManager;
    Credentials *m_credentials;
    QString m_filename;
    QueryState m_queryState;
    QString m_connectionId;
    QString m_queryId;
    ItemModelStyleDecorator m_styleDecorator;
    QSharedPointer<WindowedItemModelDecorator> m_windowedItemModelDecorator;
    Query *m_cancelQuery;

private slots:
    void onConnectionOpened(const QString &queryId, int pid);
    void onColumnsLoaded(const QString &queryId, int batchIdx, int rowSetIdx, const QSharedPointer<QAbstractItemModel> &rowSet);
    void onRowsLoaded(const QString &queryId, int batchIdx, int rowSetIdx, const QSharedPointer<QAbstractItemModel> &rowSet, const QModelIndex &first, const QModelIndex &last);
    void onQueryFinished(const QString &queryId, int batchIdx, const QueryResult &result);
    void onBatchFinished(const QString &queryId, bool batchSuccess, const QString &errorTxt);
    void onQueryStateChanged(const QString &queryId, const QueryState &newState);
};

#endif // QUERYTAB_H
