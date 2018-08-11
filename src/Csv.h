#ifndef CSV_H
#define CSV_H

#include <QAbstractItemModel>
#include <QString>
#include <QTextStream>

class Csv
{
public:
    Csv(QString delimiter = ",", QString quote = "\"");
    void write(QTextStream *stream, QAbstractItemModel *model);
private:
    QString m_delimiter;
    QString m_quote;
    QString escape(QString value);
};

#endif // CSV_H
