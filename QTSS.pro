#-------------------------------------------------
#
# Project created by QtCreator 2018-04-21T14:53:18
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QTSS
TEMPLATE = app

QT += widgets
requires(qtConfig(tableview))

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += main.cpp \
           spreadsheet.cpp \
           macro.cpp \
           delegate.cpp \
           item.cpp \
           spreadsheetapp.cpp \
    scanner.cpp \
    parser.cpp

HEADERS += spreadsheetapp.h \
           spreadsheet.h \
           macro.h \
           delegate.h \
           item.h \
    scanner.h \
    parser.h

FORMS += \
    mainwindow.ui \
    macro.ui



