# Note: sources -> ui -> widgets

set(WIDGETS
	customslider
	openbutton
	indexbutton
	customlistwidget
	seekbar
	customlabel
	customsplitter
	dimdialog
)
set(UI
	mainwindow
	locationdialog
	infodialog
	jumpdialog
	aboutdialog
	updatedialog
	inputdialog
	preferencesdialog
)
set(SOURCES
	mpvhandler
	updatemanager
)

# build and link libraries

foreach(widget ${WIDGETS})
	add_library(${widget} widgets/${widget}.cpp)
	target_link_libraries(${widget} ${LIBRARIES})
	set(LIBRARIES "${LIBRARIES};${widget}")
endforeach()

foreach(ui ${UI})
	qt5_wrap_ui(UI_HEADER ui/${ui}.ui)
	add_library(${ui} ui/${ui}.cpp ${UI_HEADER})
	target_link_libraries(${ui} ${LIBRARIES})
	set(LIBRARIES "${LIBRARIES};${ui}")
endforeach()

foreach(src ${SOURCES})
	add_library(${src} ${src}.cpp)
	target_link_libraries(${src} ${LIBRARIES})
	set(LIBRARIES "${LIBRARIES};${src}")
endforeach()

# add qt5 resources
qt5_add_resources(RESOURCES rsclist.qrc)

# build and link baka-mplayer
add_executable(baka-mplayer main.cpp ${RESOURCES})
target_link_libraries(baka-mplayer ${LIBRARIES})

qt5_use_modules(baka-mplayer Core Gui Network Svg X11Extras)