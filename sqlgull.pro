#-------------------------------------------------
#
# Project created by QtCreator 2016-07-08T23:58:37
#
#-------------------------------------------------

QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

SOURCES += \
    src/ConnectionManager.cpp \
    ui/MainWindow.cpp \
    ui/CodeEditor.cpp \
    ui/Highlighter.cpp \
    ui/AboutDialog.cpp \
    src/Connection.cpp \
    ui/ConnectionDialog.cpp \
    ui/LoginDialog.cpp \
    ui/QueryTab.cpp \
    src/Csv.cpp \
    src/Credentials.cpp \
    src/Query.cpp \
    ui/TableView.cpp \
    src/StringUtils.cpp \
    src/ItemModelStyleDecorator.cpp \
    src/PagedTableModel.cpp \
    src/QueryResult.cpp \
    src/WindowedItemModelDecorator.cpp


HEADERS  += \
    src/ConnectionManager.h \
    ui/MainWindow.h \
    ui/CodeEditor.h \
    ui/Highlighter.h \
    ui/AboutDialog.h \
    src/Connection.h \
    ui/ConnectionDialog.h \
    ui/LoginDialog.h \
    ui/QueryTab.h \
    src/Csv.h \
    src/QueryState.h \
    src/Credentials.h \
    src/Query.h \
    ui/TableView.h \
    src/StringUtils.h \
    src/ItemModelStyleDecorator.h \
    src/PagedTableModel.h \
    src/QueryResult.h \
    src/WindowedItemModelDecorator.h


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

