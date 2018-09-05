#include "../ui/MainWindow.h"
#include <QApplication>
#include <QDesktopWidget>

void registerMetaTypes()
{
    qRegisterMetaType<QueryResult>();
	qRegisterMetaType<QueryState>();
	qRegisterMetaType<QSharedPointer<QAbstractItemModel>>();
	qRegisterMetaType<QVector<int>>();
}

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	registerMetaTypes();

	MainWindow w;
	w.show();
	return a.exec();
}
