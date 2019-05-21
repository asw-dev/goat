#include "QueryTab.h"
#include "ui_QueryTab.h"

#include "../src/Csv.h"
#include "../src/StringUtils.h"
#include "../src/WindowedItemModelDecorator.h"

#include <QAbstractItemModel>
#include <QApplication>
#include <QClipboard>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QItemSelection>
#include <QMessageBox>

static void doDeleteLater(QAbstractItemModel *obj)
{
    obj->deleteLater();
}

QueryTab::QueryTab(QString filename, ConnectionManager *connectionManager, Credentials *credentials, QWidget *parent) : QWidget(parent)
{
    ui = new Ui::QueryTab;
	ui->setupUi(this);
    ui->codeEditor->setConnectionManager(connectionManager);

    m_filename = filename;
    m_connectionManager = connectionManager;
    m_credentials = credentials;
    m_queryState = READY;
    m_cancelQuery = nullptr;

    QHeaderView *verticalHeader = ui->resultsGrid->verticalHeader();
    verticalHeader->setSectionResizeMode(QHeaderView::Fixed);

    m_windowedItemModelDecorator = QSharedPointer<WindowedItemModelDecorator>(new WindowedItemModelDecorator, doDeleteLater);
    m_styleDecorator.setModel(m_windowedItemModelDecorator);
    ui->resultsGrid->setModel(&m_styleDecorator);

    readFile();
    setModified(false);

    connect(ui->codeEditor, SIGNAL(textChanged()), this, SIGNAL(textChanged()));
}

QueryTab::~QueryTab()
{
    clearResults();
	delete ui;
}

void QueryTab::clearResults()
{
    m_windowedItemModelDecorator->setModel(QSharedPointer<QAbstractItemModel>());
	if (m_cancelQuery)
		m_cancelQuery->deleteLater();
	m_cancelQuery = nullptr;
    ui->resultsText->clear();
    ui->exportResultsToClipboard->setDisabled(true);
    ui->exportResultsToFileButton->setDisabled(true);
}

void QueryTab::executeQuery(const QString &connectionId)
{
    Connection connection = m_connectionManager->getConnections()[connectionId];

    ui->codeEditor->setTextCursor(ui->codeEditor->queryAtCursor());
    QString query = ui->codeEditor->selectedText().trimmed();

    if (query.isEmpty())
        return;

    clearResults();
    ui->resultsTabBar->setCurrentIndex(1);

    QStringList queries;
    queries.append(query);

    QThread *thread = new QThread();
    Query *q = new Query(connection, m_credentials, queries, Connection::defaultPidQuery(connection.driver()));
    q->setDestThread(this->thread());

    m_queryId = q->queryId();
    setConnectionId(connection.connectionId());
    q->moveToThread(thread);

    connect(q, SIGNAL(connectionOpened(const QString&, int)), this, SLOT(onConnectionOpened(const QString&, int)));
    connect(q, SIGNAL(columnsLoaded(const QString&, int, int, const QSharedPointer<QAbstractItemModel>&)), this, SLOT(onColumnsLoaded(const QString&, int, int, const QSharedPointer<QAbstractItemModel>&)));
    connect(q, SIGNAL(rowsLoaded(const QString&, int, int, const QSharedPointer<QAbstractItemModel>, const QModelIndex &, const QModelIndex &)), this, SLOT(onRowsLoaded(const QString&, int, int, const QSharedPointer<QAbstractItemModel>&, const QModelIndex &, const QModelIndex &)));
    connect(q, SIGNAL(queryFinished(const QString&, int, const QueryResult&)), this, SLOT(onQueryFinished(const QString&, int, const QueryResult&)));
    connect(q, SIGNAL(batchFinished(const QString&, bool, const QString&)), this, SLOT(onBatchFinished(const QString&, bool, const QString&)));
    connect(q, SIGNAL(queryStateChanged(const QString&, const QueryState&)), this, SLOT(onQueryStateChanged(const QString&, const QueryState&)));

    connect(thread, SIGNAL(started()), q, SLOT(run()));
    connect(q, SIGNAL(batchFinished(const QString&, bool, const QString&)), thread, SLOT(quit()));
    connect(q, SIGNAL(batchFinished(const QString&, bool, const QString&)), q, SLOT(deleteLater()));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

    thread->start();
}

bool QueryTab::modified() const
{
    return ui->codeEditor->document()->isModified();
}

void QueryTab::setModified(const bool &modified)
{
    ui->codeEditor->document()->setModified(modified);
}

QString QueryTab::filename() const
{
    return m_filename;
}

void QueryTab::readFile()
{
    if (m_filename.isEmpty())
        return;

    ui->codeEditor->document()->clear();

    QFile file(m_filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "Error reading file: " + m_filename;

        QMessageBox messageBox;
        messageBox.setWindowTitle("Error");
        messageBox.setText("Error reading file: " + m_filename);
        messageBox.setIcon(QMessageBox::Critical);
        messageBox.setStandardButtons(QMessageBox::Ok);

        messageBox.exec();
        return;
    }

    ui->codeEditor->document()->setPlainText(QString(file.readAll()));
    file.close();
}

void QueryTab::writeFile()
{
    if (m_filename.isEmpty())
    {
        return;
    }

    QFile file(m_filename);
    bool error = !file.open(QIODevice::WriteOnly | QIODevice::Text);

    if (!error)
    {
        error = file.write(ui->codeEditor->document()->toPlainText().toUtf8()) == -1;
        file.close();
    }

    if (error)
    {
        qDebug() << "Error writing file: " + m_filename;

        QMessageBox messageBox;
        messageBox.setWindowTitle("Error");
        messageBox.setText("Error writing file: " + m_filename);
        messageBox.setIcon(QMessageBox::Critical);
        messageBox.setStandardButtons(QMessageBox::Ok);

        messageBox.exec();
    }
    else
    {
        setModified(false);
    }
}

void QueryTab::setFilename(const QString &filename)
{
    m_filename = filename;
}

QueryState QueryTab::queryState() const
{
    return m_queryState;
}

bool QueryTab::hasResults()
{
    QAbstractItemModel *model = ui->resultsGrid->model();
    return model && (model->rowCount() || model->columnCount());
}

CodeEditor* QueryTab::codeEditor()
{
    return ui->codeEditor;
}

bool QueryTab::canCancel()
{
    return m_queryState == ACTIVE && m_cancelQuery;
}

void QueryTab::cancel()
{
    if (!canCancel())
        return;

    QThread *thread = new QThread();
    Query *q = m_cancelQuery;
    m_cancelQuery = nullptr;

    q->moveToThread(thread);
    connect(thread, SIGNAL(started()), q, SLOT(run()));
    connect(q, SIGNAL(batchFinished(QString,bool,QString)), thread, SLOT(quit()));
    connect(q, SIGNAL(batchFinished(QString,bool,QString)), q, SLOT(deleteLater()));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

    m_queryState = CANCELING;
    thread->start();
    emit queryStateChanged();
}

void QueryTab::on_exportResultsToClipboard_clicked()
{
    QAbstractItemModel *model = ui->resultsGrid->model();
    QItemSelection selection;
    if (model->rowCount())
        selection = QItemSelection(model->index(0, 0), model->index(model->rowCount() - 1, model->columnCount() -1));

    QString text = Csv("\t", "\"").writeSelectionToString(ui->resultsGrid->model(), selection);
    QApplication::clipboard()->setText(text);
}

void QueryTab::on_exportResultsToFileButton_clicked()
{
    if (!hasResults())
        return;

    QString filename = QFileDialog::getSaveFileName(this, tr("Save As"), QDir::homePath(), tr("Csv files (*.csv) ;; All files (*.*)"));

    if (filename.isEmpty())
        return;

    QFile file(filename);
    if (!file.open(QFile::WriteOnly|QFile::Truncate))
    {
        QMessageBox messageBox;
        messageBox.setWindowTitle("Error");
        messageBox.setText("Error exporting file: " + filename);
        messageBox.setIcon(QMessageBox::Critical);
        messageBox.setStandardButtons(QMessageBox::Ok);
        return;
    }

    QTextStream stream(&file);

    Csv csvExport;
    csvExport.write(&stream, ui->resultsGrid->model());
    file.close();
}

QString QueryTab::connectionId() const
{
    return m_connectionId;
}

void QueryTab::setConnectionId(const QString &connectionId)
{
    m_connectionId = connectionId;
    ui->codeEditor->setConnectionId(m_connectionId);
}

void QueryTab::onConnectionOpened(const QString &queryId, int pid)
{
    if (m_queryId != queryId)
        return;

    if (pid == -1)
        return; //can't cancel for this connection

    Connection connection = m_connectionManager->getConnections()[m_connectionId];

    QMap<QString, QString> values;
    values["pid"] = QString::number(pid);
    QString cancelQueryTxt =  StringUtils::interpolate(Connection::defaultCancelQuery(connection.driver()), values);

    QStringList queries;
    queries.append(cancelQueryTxt);
    m_cancelQuery = new Query(connection, m_credentials, queries);

    emit queryStateChanged(); //FIXME create a new signal for canCancelChanged
}

void QueryTab::onColumnsLoaded(const QString &queryId, int batchIdx, int rowSetIdx, const QSharedPointer<QAbstractItemModel> &rowSet)
{
    if (m_queryId != queryId)
        return;

    if (batchIdx || rowSetIdx)
    {
        return; //TODO
    }

    m_windowedItemModelDecorator->setModel(rowSet);
    m_windowedItemModelDecorator->setRange(-1, 0, -1, rowSet->columnCount() - 1);

    ui->resultsGrid->resizeColumnsToContents();
    ui->resultsTabBar->setCurrentIndex(0);
}

void QueryTab::onRowsLoaded(const QString &queryId, int batchIdx, int rowSetIdx, const QSharedPointer<QAbstractItemModel> &, const QModelIndex &, const QModelIndex &last)
{
    if (m_queryId != queryId)
        return;

    if (batchIdx || rowSetIdx)
        return; //TODO

    bool firstRowLoad = m_windowedItemModelDecorator->rowCount() == 0;
    m_windowedItemModelDecorator->setRange(0, 0, last.row(), last.column());

	if (firstRowLoad)
		ui->resultsGrid->resizeColumnsToContents();
}

void QueryTab::onQueryFinished(const QString &queryId, int batchIdx, const QueryResult &result)
{
    if (m_queryId != queryId)
        return;

    if (batchIdx != 0)
        return; //TODO

    bool hasRowSets = !result.rowSets().isEmpty();
    ui->exportResultsToFileButton->setDisabled(!hasRowSets);
    ui->exportResultsToClipboard->setDisabled(!hasRowSets);

    ui->resultsText->appendPlainText(tr("Started At: ") + result.start().toString("yyyy-MM-dd hh:mm:ss"));
    ui->resultsText->appendPlainText(tr("Query Time: ") + QString::number(result.start().msecsTo(result.executed())) + " ms");
    ui->resultsText->appendPlainText(tr("Client Time: ") + QString::number(result.executed().msecsTo(result.end())) + " ms");
    if (result.queryState() == FINISHED)
        ui->resultsText->appendPlainText(tr("Number of rows affected: ") + QString::number(result.rowsAffected()));
    ui->resultsText->appendPlainText("");
    ui->resultsText->appendPlainText(tr("Messages:"));
    ui->resultsText->appendPlainText("-------------------------------");
    ui->resultsText->appendPlainText(result.messages());
}

void QueryTab::onBatchFinished(const QString &queryId, bool batchSuccess, const QString &errorTxt)
{
    if (m_queryId != queryId)
        return;

    if (!batchSuccess)
    {
        ui->resultsText->appendPlainText(errorTxt);
        ui->resultsTabBar->setCurrentIndex(1);
    }

	if (m_cancelQuery)
		m_cancelQuery->deleteLater();
    m_cancelQuery = nullptr;
}

void QueryTab::onQueryStateChanged(const QString &queryId, const QueryState &newState)
{
    if (m_queryId != queryId)
        return;

    m_queryState = newState;
    emit queryStateChanged();
}
