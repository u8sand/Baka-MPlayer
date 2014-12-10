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
           'BAKA_MPLAYER_LANG=\\"baka-mplayer_en\\"' \
           'SETTINGS_FILE=\\"bakamplayer\\"'

TEMPLATE = app
CONFIG += c++11 link_pkgconfig
PKGCONFIG += mpv

DESTDIR = build
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

CONFIG(release, debug|release) {
    QMAKE_EXTRA_COMPILERS += lupdate
    lupdate.input    = TRANSLATIONS
    lupdate.output   = ${QMAKE_FILE_IN}
    lupdate.commands = lupdate ${QMAKE_FILE_IN}
    lupdate.CONFIG  += no_link target_predeps

    QMAKE_EXTRA_COMPILERS += lrelease
    lrelease.input    = TRANSLATIONS
    lrelease.output   = ${DESTDIR}${QMAKE_FILE_BASE}.qm
    lrelease.commands = lrelease ${QMAKE_FILE_IN} -qm $${lrelease.output}
    lrelease.CONFIG  += no_link target_predeps
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
    ui/screenshotdialog.cpp \
    ui/updatedialog.cpp \
    widgets/customlabel.cpp \
    widgets/customlineedit.cpp \
    widgets/customslider.cpp \
    widgets/customsplitter.cpp \
    widgets/dimdialog.cpp \
    widgets/indexbutton.cpp \
    widgets/openbutton.cpp \
    widgets/playlistwidget.cpp \
    widgets/seekbar.cpp

HEADERS  += \
    mpvhandler.h \
    mpvtypes.h \
    updatemanager.h \
    util.h \
    widgets/customlabel.h \
    widgets/customlineedit.h \
    widgets/customslider.h \
    widgets/customsplitter.h \
    widgets/dimdialog.h \
    widgets/indexbutton.h \
    widgets/openbutton.h \
    widgets/playlistwidget.h \
    widgets/seekbar.h \
    ui/aboutdialog.h \
    ui/infodialog.h \
    ui/inputdialog.h \
    ui/jumpdialog.h \
    ui/locationdialog.h \
    ui/mainwindow.h \
    ui/preferencesdialog.h \
    ui/screenshotdialog.h \
    ui/updatedialog.h

FORMS    += \
    ui/aboutdialog.ui \
    ui/infodialog.ui \
    ui/inputdialog.ui \
    ui/jumpdialog.ui \
    ui/locationdialog.ui \
    ui/mainwindow.ui \
    ui/preferencesdialog.ui \
    ui/screenshotdialog.ui \
    ui/updatedialog.ui

RESOURCES += \
    rsclist.qrc

TRANSLATIONS += \
    translations/baka-mplayer_pt.ts \
    translations/baka-mplayer_ko.ts \
    translations/baka-mplayer_zh.ts
