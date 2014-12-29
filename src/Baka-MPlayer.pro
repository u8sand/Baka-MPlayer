#-------------------------------------------------
#
# Project created by QtCreator 2014-07-06T12:08:44
#
#-------------------------------------------------

VERSION   = 2.0.1
QT       += core gui network svg
CODECFORSRC = UTF-8
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
TARGET = baka-mplayer
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
    # mxe fix:
    CONFIG -= windows
    QMAKE_LFLAGS += $$QMAKE_LFLAGS_WINDOWS
    # application information
    RC_ICONS += img/logo.ico
    QMAKE_TARGET_PRODUCT += Baka MPlayer
    QMAKE_TARGET_DESCRIPTION += The libmpv based media player.
    #RC_LANG +=
}

# INSTROOT is the installation root directory, leave empty if not using a package management system
isEmpty(BINDIR):BINDIR=/usr/bin
isEmpty(MEDIADIR):MEDIADIR=/usr/share/pixmaps
isEmpty(APPDIR):APPDIR=/usr/share/applications
isEmpty(DOCDIR):DOCDIR=/usr/share/doc
isEmpty(MANDIR):MANDIR=/usr/share/man
isEmpty(LICENSEDIR):LICENSEDIR=/usr/share/licenses
isEmpty(BAKADIR):BAKADIR=/usr/share/baka-mplayer

target.path = $$INSTROOT$$BINDIR
logo.path = $$INSTROOT$$MEDIADIR
desktop.path = $$INSTROOT$$APPDIR
manual.path = $$INSTROOT$$DOCDIR/baka-mplayer
man.path = $$INSTROOT$$MANDIR/man1
license.path = $$INSTROOT$$LICENSEDIR/baka-mplayer
translations.path = $$INSTROOT$$BAKADIR/translations

logo.files = ../etc/logo/baka-mplayer.svg
desktop.files = ../etc/baka-mplayer.desktop
manual.files = ../etc/doc/baka-mplayer.md
man.files = ../etc/doc/baka-mplayer.1.gz
license.files = ../LICENSE

man.extra:system(gzip -c ../etc/doc/baka-mplayer.man > ../etc/doc/baka-mplayer.1.gz)

INSTALLS += target logo desktop manual man license

RESOURCES += rsclist.qrc

isEmpty(TRANSLATIONS) {
    include(translations.pri)
}

TRANSLATIONS_COMPILED = $$TRANSLATIONS
TRANSLATIONS_COMPILED ~= s/\.ts/.qm/g

CONFIG(embed_translations) {
    # create translations resource file
    system("echo \'<RCC><qresource prefix=\"/\">\' > translations.qrc")
    for(translation, TRANSLATIONS_COMPILED):system("echo \'<file>$$translation</file>\' >> translations.qrc")
    system("echo \'</qresource></RCC>\'" >> translations.qrc)

    # add file to build
    RESOURCES += translations.qrc

    BAKA_LANG_PATH += :/translations

    # make sure translations are updated and released
    CONFIG *= update_translations release_translations
}

CONFIG(install_translations) {
    # install translation files
    translations.files = $$TRANSLATIONS_COMPILED
    INSTALLS += translations

    BAKA_LANG_PATH += $$BAKADIR/translations

    # make sure translations are updated and released
    CONFIG *= update_translations release_translations
}

CONFIG(update_translations) {
    isEmpty(lupdate):lupdate=lupdate
    system($$lupdate $$_PRO_FILE_)
}

CONFIG(release_translations) {
    isEmpty(lrelease):lrelease=lrelease
    system($$lrelease $$_PRO_FILE_)
}


isEmpty(SETTINGS_FILE):SETTINGS_FILE=bakamplayer
DEFINES += "BAKA_MPLAYER_VERSION=\\\"$$VERSION\\\"" \
           "SETTINGS_FILE=\\\"$$SETTINGS_FILE\\\"" \
           "BAKA_MPLAYER_LANG_PATH=\\\"$$BAKA_LANG_PATH\\\""
!isEmpty(BAKA_LANG):DEFINES += "BAKA_MPLAYER_LANG=\\\"$$BAKA_LANG\\\""


SOURCES += main.cpp\
    mpvhandler.cpp \
    updatemanager.cpp \
    gesturehandler.cpp \
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
    gesturehandler.h \
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
