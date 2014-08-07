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
    playlistmanager.cpp \
    widgets/customslider.cpp \
    widgets/openbutton.cpp \
    widgets/playpausebutton.cpp \
    widgets/indexbutton.cpp \
    ui/mainwindow.cpp \
    ui/locationdialog.cpp \
    ui/infodialog.cpp \
    ui/jumpdialog.cpp \
    ui/aboutdialog.cpp \
    ui/updatedialog.cpp

HEADERS  += \
    mpvhandler.h \
    playlistmanager.h \
    widgets/customslider.h \
    widgets/openbutton.h \
    widgets/playpausebutton.h \
    widgets/indexbutton.h \
    ui/mainwindow.h \
    ui/locationdialog.h \
    ui/infodialog.h \
    ui/jumpdialog.h \
    ui/aboutdialog.h \
    ui/updatedialog.h

FORMS    += \
    ui/mainwindow.ui \
    ui/locationdialog.ui \
    ui/infodialog.ui \
    ui/jumpdialog.ui \
    ui/aboutdialog.ui \
    ui/updatedialog.ui

CONFIG += c++11

unix {
        DEFINES += 'SETTINGS_FILE=\\"bakamplayer.ini\\"'
        CONFIG += link_pkgconfig
        PKGCONFIG += mpv
}

win32 {
        DEFINES += 'SETTINGS_FILE=\\"bakamplayer.ini\\"'
        CONFIG += static
        QTPLUGIN += qsvg
        INCLUDEPATH += "../etc/include"
        LIBS += -L"../etc/lib" -lmpv
}

RESOURCES += \
    rsclist.qrc
