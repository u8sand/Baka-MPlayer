#-------------------------------------------------
#
# Project created by QtCreator 2014-07-06T12:08:44
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Baka-MPlayer
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    mpvhandler.cpp

HEADERS  += mainwindow.h \
    mpvhandler.h \
    customslider.h

FORMS    += mainwindow.ui

unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += mpv

RESOURCES += \
    rsclist.qrc
