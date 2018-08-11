#include "QueryTab.h"
#include "ui_QueryTab.h"
#include "src/Csv.h"

#include <QAction>
#include <QComboBox>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QShortcut>
#include <QSplitter>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QSqlResult>
#include <QTabBar>
#include <QTabWidget>
#include <QTextBlock>
#include <QTextCursor>
#include <QVBoxLayout>

QueryTab::QueryTab(QString filename, QWidget *parent) : QWidget(parent)
{
    ui = new Ui::QueryTab;
	ui->setupUi(this);

    m_queryResultsModel = new QSqlQueryModel(this);
    m_filename = filename;
    m_queryState = READY;

    ui->resultsGrid->setModel(m_queryResultsModel);

    ui->resultsText->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

    readFile();
    setModified(false);

    connect(ui->codeEditor, SIGNAL(textChanged()), this, SIGNAL(textChanged()));
}

QueryTab::~QueryTab()
{
	delete ui;
}

void QueryTab::executeQueryAtCursor(QSqlDatabase sqlDatabase)
{
    executeQuery(sqlDatabase, ui->codeEditor->getQueryAtCursor());
}

void QueryTab::clearResults()
{
    ui->resultsText->clear();
    m_queryResultsModel->clear();
    ui->button_exportQueryResults->setDisabled(true);
}

void QueryTab::onQuerySucess(QSqlQuery query, QDateTime start)
{
    QDateTime end = QDateTime::currentDateTime();

    if (query.isSelect())
    {
        m_queryResultsModel->setQuery(query);
        ui->resultsGrid->resizeColumnsToContents();
        ui->resultsTabBar->setCurrentIndex(0);
        ui->button_exportQueryResults->setDisabled(false);
    }
    else
    {
        m_queryResultsModel->clear();
        ui->resultsTabBar->setCurrentIndex(1);
    }

    ui->resultsText->appendPlainText("Timestamp: " + end.toString("yyyy-MM-dd hh:mm:ss"));
    ui->resultsText->appendPlainText("Elapsed: " + QString::number(start.msecsTo(end)) + " ms");
    if (!query.isSelect())
        ui->resultsText->appendPlainText("Number of rows affected: " + QString::number(query.numRowsAffected()));
    ui->resultsText->appendPlainText("");
    ui->resultsText->appendPlainText("Query:");
    ui->resultsText->appendPlainText("-------------------------------");
    ui->resultsText->appendPlainText(query.lastQuery());

    m_queryState = FINISHED;
    emit queryStateChanged();
}

void QueryTab::onQueryFailure(QSqlQuery query, QDateTime start)
{
    QDateTime end = QDateTime::currentDateTime();

    m_queryResultsModel->clear();
    ui->resultsTabBar->setCurrentIndex(1);

    ui->resultsText->appendPlainText("Timestamp: " + end.toString("yyyy-mm-dd hh:mm:ss"));
    ui->resultsText->appendPlainText("Elapsed: " + QString::number(start.msecsTo(end)) + " ms");
    ui->resultsText->appendPlainText(query.lastError().text());
    ui->resultsText->appendPlainText("");
    ui->resultsText->appendPlainText("Query:");
    ui->resultsText->appendPlainText("-------------------------------");
    ui->resultsText->appendPlainText(query.lastQuery());

    m_queryState = FAILED;
    emit queryStateChanged();
}

void QueryTab::executeQuery(QSqlDatabase sqlDatabase, QString query)
{
    if (query.trimmed().isEmpty())
        return;

    clearResults();
    QSqlQuery sqlQuery(sqlDatabase);

    QDateTime start = QDateTime::currentDateTime();
    m_queryState = ACTIVE;
    emit queryStateChanged();

    bool success = sqlQuery.exec(query);
    if (success)
        onQuerySucess(sqlQuery, start);
    else
        onQueryFailure(sqlQuery, start);
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
      return m_queryResultsModel->rowCount() || m_queryResultsModel->columnCount();
}
