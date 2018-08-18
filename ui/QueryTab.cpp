#include "QueryTab.h"
#include "ui_QueryTab.h"

#include "src/Csv.h"
#include "ui/LoginDialog.h"

#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>

QueryTab::QueryTab(QString filename, QWidget *parent) : QWidget(parent)
{
    ui = new Ui::QueryTab;
	ui->setupUi(this);

    m_filename = filename;
    m_queryState = READY;
    m_query = nullptr;

    ui->resultsText->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

    readFile();
    setModified(false);

    connect(ui->codeEditor, SIGNAL(textChanged()), this, SIGNAL(textChanged()));
}

QueryTab::~QueryTab()
{
	delete ui;
}

void QueryTab::clearResults()
{
    if (ui->resultsGrid->model())
    {
        delete ui->resultsGrid->model();
        ui->resultsGrid->setModel(nullptr);
    }
    ui->resultsText->clear();
}

void QueryTab::onQuerySucess()
{
    if (!m_query || m_query->queryState() == ACTIVE)
        return;

    QDateTime start = m_query->startTime();
    QDateTime end = m_query->endTime();

    if (!m_query->results().isEmpty())
    {
        ui->resultsGrid->setModel(m_query->results()[0]);
        m_query->results().pop_front();
        ui->resultsGrid->resizeColumnsToContents();
        ui->resultsTabBar->setCurrentIndex(0);
        ui->button_exportQueryResults->setDisabled(false);
    }
    else
    {
        ui->resultsTabBar->setCurrentIndex(1);
        ui->button_exportQueryResults->setDisabled(true);
    }

    ui->resultsText->appendPlainText("Timestamp: " + end.toString("yyyy-MM-dd hh:mm:ss"));
    ui->resultsText->appendPlainText("Elapsed: " + QString::number(start.msecsTo(end)) + " ms");
    if (m_query->results().isEmpty())
        ui->resultsText->appendPlainText("Number of rows affected: " + QString::number(m_query->rowsAffected()));
    ui->resultsText->appendPlainText("");
    ui->resultsText->appendPlainText("Messages:");
    ui->resultsText->appendPlainText("-------------------------------");
    ui->resultsText->appendPlainText(m_query->messages());

    m_queryState = FINISHED;
    emit queryStateChanged();

    m_queryThread->deleteLater();
    m_queryThread = 0;

    m_query->deleteLater();
    m_query = 0;
}

void QueryTab::onQueryFailure()
{
    if (!m_query || m_query->queryState() == ACTIVE)
        return;

    QDateTime start = m_query->startTime();
    QDateTime end = m_query->endTime();

    ui->button_exportQueryResults->setDisabled(true);

    ui->resultsTabBar->setCurrentIndex(1);

    ui->resultsText->appendPlainText("Timestamp: " + end.toString("yyyy-mm-dd hh:mm:ss"));
    ui->resultsText->appendPlainText("Elapsed: " + QString::number(start.msecsTo(end)) + " ms");
    ui->resultsText->appendPlainText("");
    ui->resultsText->appendPlainText("Messages:");
    ui->resultsText->appendPlainText("-------------------------------");
    ui->resultsText->appendPlainText(m_query->messages());

    m_queryState = FAILED;
    emit queryStateChanged();

    m_queryThread->deleteLater();
    m_queryThread = 0;

    m_query->deleteLater();
    m_query = 0;
}

void QueryTab::executeQuery(const Connection &connection, Credentials *credentials)
{
    QString query = ui->codeEditor->getQueryAtCursor();

    if (query.trimmed().isEmpty())
        return;

    clearResults();

    if (connection.driver() != "QSQLITE")
    {
        QString user;
        QString pass;
        credentials->get(connection.connectionId(), &user, &pass);
        if (user.isEmpty())
        {
            LoginDialog loginDialog(this); //create it here so we use the gui thread
            if (loginDialog.exec() == QDialog::Accepted)
            {
                credentials->set(connection.connectionId(), loginDialog.user(), loginDialog.pass());
            }
            else
            {
                return;
            }
        }
    }

    m_queryThread = new QThread();
    m_query = new Query(connection, credentials, query);
    m_query->moveToThread(m_queryThread);

    connect(m_queryThread, SIGNAL(started()), m_query, SLOT(run()));
    connect(m_query, SIGNAL(queryFinished()), m_queryThread, SLOT(quit()));
    connect(m_query, SIGNAL(queryFailed()), m_queryThread, SLOT(quit()));

    connect(m_query, SIGNAL(queryFinished()), this, SLOT(onQuerySucess()));
    connect(m_query, SIGNAL(queryFailed()), this, SLOT(onQueryFailure()));

    m_queryState = ACTIVE;
    emit queryStateChanged();
    m_queryThread->start();
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

void QueryTab::on_button_exportQueryResults_clicked()
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
