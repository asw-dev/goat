#include "ui/MainWindow.h"
#include "ui_MainWindow.h"

#include "../src/DatabaseService.h"
#include "AboutDialog.h"
#include "ConnectionDialog.h"
#include "LoginDialog.h"
#include "QueryTab.h"

#include <QAbstractItemModel>
#include <QCloseEvent>
#include <QCoreApplication>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QSettings>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),	ui(new Ui::MainWindow) {
	ui->setupUi(this);

    //form editor does not support adding widgets to toolbar so do it with code
    ui->toolBar->insertWidget(ui->actionNewConnection, ui->connectionComboBox);

	readSettings();
    on_actionNewFile_triggered();

    foreach(Connection connection, m_connectionManager.getConnections().values())
    {
        ui->connectionComboBox->addItem(connection.name(), connection.connectionId());
    }
    ui->connectionComboBox->model()->sort(0);
    if (ui->connectionComboBox->count())
        ui->connectionComboBox->setCurrentIndex(0); //TODO remember setting from last session
    connect(ui->tabBarConnections, SIGNAL(currentChanged(int)), this, SLOT(invalidateEnabledStates()));
    resizeDocks({ui->databaseObjectDockWidget}, {300}, Qt::Horizontal); //HACK QTBUG-65592 avoid dock resize bug (fixed in Qt 5.12)
}

MainWindow::~MainWindow() {
	delete ui;
}

void MainWindow::on_actionExit_triggered() {
	this->close();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    bool unsavedChanges = false;
    for (int i = 0; i < ui->tabBarConnections->count(); ++i)
    {
        QueryTab *queryTab = ((QueryTab*) ui->tabBarConnections->widget(i));
        unsavedChanges |= queryTab->modified();
    }

    QMessageBox exitConfirmationDialog;
    exitConfirmationDialog.setWindowTitle(tr("Exit?"));
    exitConfirmationDialog.setText(tr("Are you sure you want to exit?"));
    exitConfirmationDialog.setInformativeText(tr("All unsaved changes will be lost."));
    exitConfirmationDialog.setIcon(QMessageBox::Warning);
    exitConfirmationDialog.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    exitConfirmationDialog.setMinimumSize(QSize(600, 120));
    QSpacerItem* horizontalSpacer = new QSpacerItem(500, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
    QGridLayout* layout = (QGridLayout*)exitConfirmationDialog.layout();
    layout->addItem(horizontalSpacer, layout->rowCount(), 0, 1, layout->columnCount());
    //TODO option to ignore this warning in the future

    if(!unsavedChanges || exitConfirmationDialog.exec() == QMessageBox::Yes)
	{
		writeSettings();
		event->accept();
    }
    else
    {
        event->ignore();
    }
}

void MainWindow::readSettings()
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::applicationName(), "settings");
	settings.beginGroup("MainWindow");

	resize(settings.value("size",  QSize(640, 480)).toSize());
	move(settings.value("position", QPoint(200, 200)).toPoint());

	settings.endGroup();
}


void MainWindow::writeSettings()
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::applicationName(), "settings");
	settings.beginGroup("MainWindow");

	settings.setValue("size", this->size());
	settings.setValue("position", this->pos());

	settings.endGroup();
}

void MainWindow::on_tabBarConnections_tabCloseRequested(int index)
{
    QueryTab *tab = (QueryTab*) ui->tabBarConnections->widget(index);

    QMessageBox closeConfirmationDialog;
    closeConfirmationDialog.setWindowTitle(tr("Close?"));
    closeConfirmationDialog.setText(tr("Are you sure you want to close?"));
    closeConfirmationDialog.setInformativeText(tr("All unsaved changes will be lost."));
    closeConfirmationDialog.setIcon(QMessageBox::Warning);
    closeConfirmationDialog.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    closeConfirmationDialog.setMinimumSize(QSize(600, 120));
    QSpacerItem* horizontalSpacer = new QSpacerItem(500, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
    QGridLayout* layout = (QGridLayout*)closeConfirmationDialog.layout();
    layout->addItem(horizontalSpacer, layout->rowCount(), 0, 1, layout->columnCount());
    //TODO option to ignore this warning in the future

    if(!tab->modified() || closeConfirmationDialog.exec() == QMessageBox::Yes)
    {
        ui->tabBarConnections->removeTab(index);
        tab->close();
        tab->deleteLater();

        invalidateEnabledStates();
        QueryTab *queryTab = (QueryTab*) ui->tabBarConnections->currentWidget();
        if (queryTab)
            queryTab->codeEditor()->setFocus();
    }
}

void MainWindow::on_actionAbout_triggered()
{
    AboutDialog dialog;
    dialog.setModal(true);
    dialog.exec();
}

void MainWindow::on_actionNewFile_triggered()
{
    QueryTab *queryTab = new QueryTab("", &m_connectionManager, &m_credentials, ui->tabBarConnections);
    queryTab->setConnectionId(selectedConnectionId());
    ui->tabBarConnections->insertTab(ui->tabBarConnections->count(), queryTab, tr("Untitled"));
    ui->tabBarConnections->setCurrentIndex(ui->tabBarConnections->count()-1);

    connect(queryTab, SIGNAL(textChanged()), this, SLOT(on_currentTabTextChanged()));
    connect(queryTab, SIGNAL(queryStateChanged()), this, SLOT(invalidateEnabledStates()));
    invalidateEnabledStates();
    queryTab->codeEditor()->setFocus();
}

void MainWindow::on_actionNewConnection_triggered()
{
    Connection connection = Connection::defaultConnection();
    ConnectionDialog dialog(connection);
    if(dialog.exec() == QDialog::Accepted)
    {
        connection = dialog.getConnection();
        m_connectionManager.saveConnection(connection);
        ui->connectionComboBox->addItem(connection.name(), connection.connectionId());
        ui->connectionComboBox->model()->sort(0);
        setSelectedConnectionId(connection.connectionId());
    }
}

void MainWindow::setSelectedConnectionId(const QString &connectionId) {
    QModelIndexList match = ui->connectionComboBox->model()->match(
                    ui->connectionComboBox->model()->index(0, 0),
                    Qt::UserRole,
                    connectionId);

    if (match.isEmpty())
        return;

    int index = match.at(0).row();
    ui->connectionComboBox->setCurrentIndex(index);
}

void MainWindow::on_connectionComboBox_currentIndexChanged(int)
{
    int index = ui->connectionComboBox->currentIndex();
    bool connectionAtIndex = index != -1;
    QString connectionId = connectionAtIndex ? ui->connectionComboBox->itemData(index).toString() : "";
    updateTabConnectionId(currentTab(), connectionId);

    invalidateEnabledStates();
}

QueryTab* MainWindow::currentTab() {
    return (QueryTab*) ui->tabBarConnections->currentWidget();
}

QString MainWindow::selectedConnectionId() {
    int index = ui->connectionComboBox->currentIndex();
    bool connectionAtIndex = index != -1;
    if (!connectionAtIndex)
        return "";
    QString connectionId = ui->connectionComboBox->itemData(index).toString();
    return connectionId;
}

void MainWindow::updateTabConnectionId(QueryTab *tab, const QString &connectionId) {
    if (tab)
        tab->setConnectionId(connectionId);
}

void MainWindow::invalidateEnabledStates()
{
    QueryTab *tab = currentTab();

    ui->actionCloseFile->setDisabled(tab == nullptr);
    ui->actionSaveFile->setDisabled(tab == nullptr);
    ui->actionSaveFileAs->setDisabled(tab == nullptr);

    int index = ui->connectionComboBox->currentIndex();
    bool connectionAtIndex = index != -1;

    ui->actionEditConnection->setDisabled(!connectionAtIndex);
    ui->actionDeleteConnection->setDisabled(!connectionAtIndex);
    ui->actionRefreshMetadata->setDisabled(!connectionAtIndex);

    bool queryExists = ui->tabBarConnections->currentIndex() != -1;
    QueryTab *queryTab = (QueryTab*) ui->tabBarConnections->currentWidget();
    bool queryReady = queryTab && queryTab->queryState() != ACTIVE;
    if (queryTab)
        setSelectedConnectionId(queryTab->connectionId());

    ui->actionQueryBlockAtCursor->setDisabled(!connectionAtIndex || !queryExists || !queryReady);
    ui->actionExportResultsToFile->setDisabled(!queryTab || !queryTab->hasResults());
    ui->actionExportResultsToClipboard->setDisabled(!queryTab || !queryTab->hasResults());

    bool hasCredentials = connectionAtIndex && m_credentials.contains(ui->connectionComboBox->itemData(index).toString());
    ui->actionClearCredentials->setDisabled(!hasCredentials);

    bool canCancel = queryTab && queryTab->canCancel();
    ui->actionCancelQuery->setDisabled(!canCancel);
}

void MainWindow::on_actionEditConnection_triggered()
{
    int index = ui->connectionComboBox->currentIndex();
    QString connectionId = ui->connectionComboBox->itemData(index).toString();
    Connection connection = m_connectionManager.getConnections()[connectionId];
    ConnectionDialog dialog(connection);
    dialog.setWindowTitle(tr("Edit Connection"));
    if(dialog.exec() == QDialog::Accepted)
    {
        ui->databaseObjectTreeView->remove(connectionId);
        m_credentials.remove(connectionId);
        connection = dialog.getConnection();
        m_connectionManager.saveConnection(connection);
        ui->connectionComboBox->setItemText(index, connection.name());
        ui->connectionComboBox->model()->sort(0);
        index = ui->connectionComboBox->model()->match(
                    ui->connectionComboBox->model()->index(0, 0),
                    Qt::UserRole,
                    connection.connectionId()).at(0).row();
        ui->connectionComboBox->setCurrentIndex(index);
        invalidateEnabledStates();
    }
}

void MainWindow::on_actionDeleteConnection_triggered()
{
    int index = ui->connectionComboBox->currentIndex();
    QString connectionId = ui->connectionComboBox->itemData(index).toString();
    m_connectionManager.deleteConnection(connectionId);
    ui->connectionComboBox->removeItem(index);
    ui->databaseObjectTreeView->remove(connectionId);
    invalidateEnabledStates();
}

QDialog::DialogCode MainWindow::promptLogin(const Connection &connection)
{
    if (connection.driver() != "QSQLITE")
    {
        QString user;
        QString pass;
        m_credentials.get(connection.connectionId(), &user, &pass);
        if (user.isEmpty())
        {
            LoginDialog loginDialog(this); //create it here so we use the gui thread
            if (loginDialog.exec() == QDialog::Accepted)
            {
                m_credentials.set(connection.connectionId(), loginDialog.user(), loginDialog.pass());
                invalidateEnabledStates();
                return QDialog::Accepted;
            }
            return QDialog::Rejected;
        }
    }
    return QDialog::Accepted;
}

void MainWindow::on_actionQueryBlockAtCursor_triggered()
{
    int index = ui->connectionComboBox->currentIndex();
    QString connectionId = ui->connectionComboBox->itemData(index).toString();
    Connection connection = m_connectionManager.getConnections()[connectionId];

    QueryTab *tab = currentTab();

    if (promptLogin(connection) == QDialog::DialogCode::Accepted)
    {
        tab->executeQuery(connectionId);
        on_actionRefreshMetadata_triggered();
    }
}

void MainWindow::on_actionCloseFile_triggered()
{
    on_tabBarConnections_tabCloseRequested(ui->tabBarConnections->currentIndex());
}

void MainWindow::on_actionOpenFile_triggered()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Open File"), QDir::homePath(), tr("Sql files (*.sql) ;; All files (*.*)"));

    if (!filename.isEmpty())
    {
        int index = ui->tabBarConnections->currentIndex();
        if (index >= 0)
        {
            //can we "replace" the current tab with the file being opened?
            QueryTab *queryTab = (QueryTab*) ui->tabBarConnections->widget(index);
            if (!queryTab->modified() && queryTab->filename().isEmpty())
                on_tabBarConnections_tabCloseRequested(index);
        }

        QFileInfo fileInfo(filename);
        QueryTab *queryTab = new QueryTab(filename, &m_connectionManager, &m_credentials, ui->tabBarConnections);
        ui->tabBarConnections->insertTab(ui->tabBarConnections->count(), queryTab, fileInfo.fileName());
        ui->tabBarConnections->setCurrentIndex(ui->tabBarConnections->count()-1);
        updateTabConnectionId(queryTab, selectedConnectionId());

        connect(queryTab, SIGNAL(textChanged()), this, SLOT(on_currentTabTextChanged()));
        connect(queryTab, SIGNAL(queryStateChanged()), this, SLOT(invalidateEnabledStates()));
        invalidateEnabledStates();
        queryTab->codeEditor()->setFocus();
    }
}

void MainWindow::changeTabFilename(QueryTab *queryTab)
{
    QString filename = QFileDialog::getSaveFileName(this, tr("Save As"), QDir::homePath(), tr("Sql files (*.sql) ;; All files (*.*)"));
    queryTab->setFilename(filename);
}

void MainWindow::saveTab(QueryTab *queryTab)
{
    queryTab->writeFile();
    QString text = queryTab->filename();
    if (text.isEmpty())
        text = tr("Untitled");
    else
        text = QFileInfo(queryTab->filename()).fileName();
    ui->tabBarConnections->setTabText(ui->tabBarConnections->currentIndex(), text);
}

void MainWindow::on_actionSaveFile_triggered()
{
    QueryTab *queryTab = currentTab();
    if (queryTab->filename().isEmpty())
    {
        changeTabFilename(queryTab);
    }
    saveTab(queryTab);
}

void MainWindow::on_actionSaveFileAs_triggered()
{
    QueryTab *queryTab = currentTab();
    changeTabFilename(queryTab);
    saveTab(queryTab);
}

void MainWindow::on_currentTabTextChanged()
{
    invalidateEnabledStates(); //save button

    QueryTab *queryTab = currentTab();
    if (queryTab->modified())
    {
        //have tab text show file has been changed
        int index = ui->tabBarConnections->currentIndex();
        QString currentTabText = ui->tabBarConnections->tabText(index);
        if (!currentTabText.startsWith("*"))
            ui->tabBarConnections->setTabText(index, "*" + currentTabText);
    }
}

void MainWindow::on_actionExportResultsToFile_triggered()
{
    QueryTab *queryTab = currentTab();
    if (!queryTab)
        return;
    queryTab->on_exportResultsToFileButton_clicked();
}

void MainWindow::on_actionExportResultsToClipboard_triggered()
{
    QueryTab *queryTab = currentTab();
    if (!queryTab)
        return;
    queryTab->on_exportResultsToClipboard_clicked();
}

void MainWindow::on_actionClearCredentials_triggered()
{
    int index = ui->connectionComboBox->currentIndex();
    QString connectionId = ui->connectionComboBox->itemData(index).toString();
    m_credentials.remove(connectionId);
    invalidateEnabledStates();
}

void MainWindow::on_actionCancelQuery_triggered()
{
    QueryTab *queryTab = currentTab();
    if (!queryTab)
        return;
    queryTab->cancel();
}

void MainWindow::on_actionRefreshMetadata_triggered()
{
    int index = ui->connectionComboBox->currentIndex();
    QString connectionId = ui->connectionComboBox->itemData(index).toString();
    Connection connection = m_connectionManager.getConnections()[connectionId];

    if (promptLogin(connection) == QDialog::DialogCode::Accepted)
    {
        QHash<QString /*id*/, DatabaseObjectMetadata> data = loadDatabaseMetadata(connection, &m_credentials);
        m_connectionManager.setMetadata(connectionId, data);
        ui->databaseObjectTreeView->set(connection, data);
    }
}
