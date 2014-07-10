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
    mpvhandler.cpp \
    settingsmanager.cpp \
    windows/mainwindow.cpp \
    playlistmanager.cpp

HEADERS  += \
    mpvhandler.h \
    settingsmanager.h \
    windows/mainwindow.h \
    widgets/customslider.h \
    playlistmanager.h

FORMS    += \
    windows/mainwindow.ui

unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += mpv

RESOURCES += \
    rsclist.qrc
