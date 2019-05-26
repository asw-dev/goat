#-------------------------------------------------
#
# Project created by QtCreator 2016-07-08T23:58:37
#
#-------------------------------------------------

QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

QMAKE_CXXFLAGS += -std=c++0x

SOURCES += \
	src/Connection.cpp \
	src/ConnectionManager.cpp \
	src/Credentials.cpp \
	src/Csv.cpp \
	src/DatabaseObjectMetadata.cpp \
	src/DatabaseService.cpp \
	src/ItemModelStyleDecorator.cpp \
	src/PagedTableModel.cpp \
	src/Query.cpp \
	src/QueryResult.cpp \
	src/StringUtils.cpp \
	src/WindowedItemModelDecorator.cpp \
	ui/AboutDialog.cpp \
	ui/CodeEditor.cpp \
	ui/Completer.cpp \
	ui/CompleterEngine.cpp \
	ui/ConnectionDialog.cpp \
	ui/DatabaseObjectTreeView.cpp \
	ui/Highlighter.cpp \
	ui/HtmlStyleDelegate.cpp \
	ui/LoginDialog.cpp \
	ui/MainWindow.cpp \
	ui/QueryTab.cpp \
	ui/SqlCompleterEngine.cpp \
	ui/TableView.cpp


HEADERS  += \
	src/Connection.h \
	src/ConnectionManager.h \
	src/Credentials.h \
	src/Csv.h \
	src/DatabaseObjectMetadata.h \
	src/DatabaseService.h \
	src/ItemModelStyleDecorator.h \
	src/PagedTableModel.h \
	src/Query.h \
	src/QueryResult.h \
	src/QueryState.h \
	src/StringUtils.h \
	src/WindowedItemModelDecorator.h \
	ui/AboutDialog.h \
	ui/CodeEditor.h \
	ui/Completer.h \
	ui/CompleterEngine.h \
	ui/ConnectionDialog.h \
	ui/DatabaseObjectTreeView.h \
	ui/Highlighter.h \
	ui/HtmlStyleDelegate.h \
	ui/LoginDialog.h \
	ui/MainWindow.h \
	ui/QueryTab.h \
	ui/SqlCompleterEngine.h \
	ui/TableView.h


FORMS    += \
	ui/MainWindow.ui \
	ui/AboutDialog.ui \
	ui/ConnectionDialog.ui \
	ui/LoginDialog.ui \
	ui/QueryTab.ui \
	ui/TableView.ui

DISTFILES += \
	LICENSE \
	README.md \
	CMakeLists.txt

RESOURCES += \
	resources/data.qrc \
	resources/icons.qrc

test {

	TEMPLATE = app
	TARGET = tests

	QT += testlib

	HEADERS += \
		test/TestStringUtils.h \
		test/TestPagedTableModel.h \
		test/TestQuery.h \
		test/TestWindowedItemModelDecorator.h

	SOURCES += \
		test/TestStringUtils.cpp \
		test/TestPagedTableModel.cpp \
		test/test_main.cpp \
		test/TestQuery.cpp \
		test/TestWindowedItemModelDecorator.cpp

} else {

	TEMPLATE = app
	TARGET = sqlgull

	SOURCES += \
		src/main.cpp

}

