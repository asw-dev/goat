#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDialog>
#include <QMainWindow>
#include <QStandardItemModel>

#include "../src/Connection.h"
#include "../src/ConnectionManager.h"
#include "../src/Credentials.h"
#include "QueryTab.h"

namespace Ui
{
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

  public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void changeTabFilename(QueryTab *connectionTab);
    void saveTab(QueryTab *connectionTab);

  private slots:
    void invalidateEnabledStates();
    void on_actionExit_triggered();
    void on_tabBarConnections_tabCloseRequested(int index);
    void on_actionAbout_triggered();
    void on_actionNewFile_triggered();
    void on_actionNewConnection_triggered();
    void on_connectionComboBox_currentIndexChanged(int index);
    void on_actionEditConnection_triggered();
    void on_actionDeleteConnection_triggered();
    void on_actionQueryBlockAtCursor_triggered();
    void on_actionCloseFile_triggered();
    void on_actionOpenFile_triggered();
    void on_actionSaveFile_triggered();
    void on_currentTabTextChanged();
    void on_actionSaveFileAs_triggered();
    void on_actionExportResultsToFile_triggered();
    void on_actionExportResultsToClipboard_triggered();
    void on_actionClearCredentials_triggered();
    void on_actionCancelQuery_triggered();
    void on_actionRefreshMetadata_triggered();

private:
    void closeEvent(QCloseEvent *event);
    void writeSettings();
    void readSettings();
    QDialog::DialogCode promptLogin(const Connection &connection);
    QueryTab* currentTab();
    QString selectedConnectionId();
    void setSelectedConnectionId(const QString &connectionId);
    void updateTabConnectionId(QueryTab *tab, const QString &connectionId);

    Ui::MainWindow *ui;
    ConnectionManager m_connectionManager;
    Credentials m_credentials;
};

#endif // MAINWINDOW_H
