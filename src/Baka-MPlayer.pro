#-------------------------------------------------
#
# Project created by QtCreator 2014-07-06T12:08:44
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Baka-MPlayer
DEFINES += 'BAKA_MPLAYER_VERSION=\\"1.9.8\\"' \
           'SETTINGS_FILE=\\"bakamplayer\\"'
TEMPLATE = app
CONFIG += c++11

DESTDIR = build/
OBJECTS_DIR = $${DESTDIR}/obj
MOC_DIR = $${DESTDIR}/moc
RCC_DIR = $${DESTDIR}/rcc
UI_DIR = $${DESTDIR}/ui

unix {
        CONFIG += link_pkgconfig
        PKGCONFIG += mpv
}

win32 {
        CONFIG += static
        QTPLUGIN += qsvg
        INCLUDEPATH += "../etc/include"
        LIBS += -L"../etc/lib" -lmpv
}

SOURCES += main.cpp\
    mpvhandler.cpp \
    playlistmanager.cpp \
    updatemanager.cpp \
    widgets/customslider.cpp \
    widgets/openbutton.cpp \
    widgets/playpausebutton.cpp \
    widgets/indexbutton.cpp \
    widgets/customlistwidget.cpp \
    widgets/seekbar.cpp \
    widgets/customlabel.cpp \
    widgets/customsplitter.cpp \
    ui/mainwindow.cpp \
    ui/locationdialog.cpp \
    ui/infodialog.cpp \
    ui/jumpdialog.cpp \
    ui/aboutdialog.cpp \
    ui/updatedialog.cpp \
    ui/inputdialog.cpp \
    ui/lightdialog.cpp

HEADERS  += \
    mpvhandler.h \
    playlistmanager.h \
    updatemanager.h \
    widgets/customslider.h \
    widgets/openbutton.h \
    widgets/playpausebutton.h \
    widgets/indexbutton.h \
    widgets/customlistwidget.h \
    widgets/seekbar.h \
    widgets/customlabel.h \
    widgets/customsplitter.h \
    ui/mainwindow.h \
    ui/locationdialog.h \
    ui/infodialog.h \
    ui/jumpdialog.h \
    ui/aboutdialog.h \
    ui/updatedialog.h \
    ui/inputdialog.h \
    ui/lightdialog.h

FORMS    += \
    ui/mainwindow.ui \
    ui/locationdialog.ui \
    ui/infodialog.ui \
    ui/jumpdialog.ui \
    ui/aboutdialog.ui \
    ui/updatedialog.ui \
    ui/inputdialog.ui

RESOURCES += \
    rsclist.qrc
