#include "TestQuery.h"

#include "src/Query.h"

void TestQuery::testQuery()
{
    Connection connection = Connection::defaultConnection("QSQLITE");
    Credentials *credentials = nullptr;
    QStringList queryTxt;
    queryTxt.append("CREATE TABLE foo (a INT);");
    queryTxt.append("INSERT INTO foo (a) VALUES (2), (3)");
    queryTxt.append("SELECT 2 * a FROM foo;");

    Query query(connection, credentials, queryTxt);
    query.run();

    QCOMPARE(query.queryState(), FINISHED);

    QAbstractItemModel *model = query.results().at(2).rowSets().at(0).data();
    QCOMPARE(model->data(model->index(0, 0)).toInt(), 4);
    QCOMPARE(model->data(model->index(1, 0)).toInt(), 6);
}
