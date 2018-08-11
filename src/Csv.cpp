#include "Csv.h"

Csv::Csv(QString delimiter, QString quote)
{
    m_delimiter = delimiter;
    m_quote = quote;
}

void Csv::write(QTextStream *stream, QAbstractItemModel *model)
{
    //TODO replace this with a real csv library

    for (int col = 0; col < model->columnCount(); ++col)
    {
        QString value = escape(model->headerData(col, Qt::Horizontal).toString());
        (*stream) << value;

        if (col < model->columnCount() - 1)
            (*stream) << m_delimiter;
    }

    for (int row = 0; row < model->rowCount(); ++row)
    {
        (*stream) << "\n";
        for (int col = 0; col < model->columnCount(); ++col)
        {
            QString value = escape(model->data(model->index(row, col)).toString());
            (*stream) << value;
            if (col < model->columnCount() - 1)
                (*stream) << m_delimiter;
        }
    }
}

QString Csv::escape(QString value)
{
    if (value.contains(m_delimiter) || value.contains("\n") || value.contains(m_quote))
        return m_quote + value.replace(m_quote, m_quote + m_quote) + m_quote;
    return value;
}
