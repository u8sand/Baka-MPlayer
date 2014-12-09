#-------------------------------------------------
#
# Project created by QtCreator 2014-07-06T12:08:44
#
#-------------------------------------------------

QT       += core gui network

CODECFORSRC = UTF-8

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = baka-mplayer
DEFINES += 'BAKA_MPLAYER_VERSION=\\"2.0.0\\"' \
           'BAKA_MPLAYER_LANG=\\"baka-mplayer-en\\"' \
           'SETTINGS_FILE=\\"bakamplayer\\"'

TEMPLATE = app
CONFIG += c++11 link_pkgconfig
PKGCONFIG += mpv

DESTDIR = build/
OBJECTS_DIR = $${DESTDIR}/obj
MOC_DIR = $${DESTDIR}/moc
RCC_DIR = $${DESTDIR}/rcc
UI_DIR = $${DESTDIR}/ui

unix {
    QT += x11extras
    PKGCONFIG += x11
}

win32 {
    CONFIG += static
}

SOURCES += main.cpp\
    mpvhandler.cpp \
    updatemanager.cpp \
    util.cpp \
    ui/aboutdialog.cpp \
    ui/infodialog.cpp \
    ui/inputdialog.cpp \
    ui/jumpdialog.cpp \
    ui/locationdialog.cpp \
    ui/mainwindow.cpp \
    ui/preferencesdialog.cpp \
    ui/updatedialog.cpp \
    ui/screenshotdialog.cpp \
    widgets/customlabel.cpp \
    widgets/customslider.cpp \
    widgets/customsplitter.cpp \
    widgets/dimdialog.cpp \
    widgets/indexbutton.cpp \
    widgets/openbutton.cpp \
    widgets/seekbar.cpp \
    widgets/playlistwidget.cpp

HEADERS  += \
    mpvtypes.h \
    mpvhandler.h \
    updatemanager.h \
    util.h \
    widgets/customslider.h \
    widgets/openbutton.h \
    widgets/indexbutton.h \
    widgets/seekbar.h \
    widgets/customlabel.h \
    widgets/customsplitter.h \
    widgets/dimdialog.h \
    widgets/playlistwidget.h \
    ui/mainwindow.h \
    ui/locationdialog.h \
    ui/infodialog.h \
    ui/jumpdialog.h \
    ui/aboutdialog.h \
    ui/updatedialog.h \
    ui/inputdialog.h \
    ui/preferencesdialog.h \
    ui/screenshotdialog.h

FORMS    += \
    ui/mainwindow.ui \
    ui/locationdialog.ui \
    ui/infodialog.ui \
    ui/jumpdialog.ui \
    ui/aboutdialog.ui \
    ui/updatedialog.ui \
    ui/inputdialog.ui \
    ui/preferencesdialog.ui \
    ui/screenshotdialog.ui

RESOURCES += \
    rsclist.qrc

TRANSLATIONS += \
    baka-mplayer-en.ts
