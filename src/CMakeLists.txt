# Note: ui -> widgets -> sources

set(SOURCES
	mpvhandler
	updatemanager
)
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
	locationdialog
	infodialog
	jumpdialog
	aboutdialog
	updatedialog
	inputdialog
	preferencesdialog
	mainwindow
)

# build and link libraries

foreach(src ${SOURCES})
	add_library(${src} ${src}.cpp)
	target_link_libraries(${src} ${LIBRARIES})
	set(LIBRARIES "${LIBRARIES};${src}")
endforeach()

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

# add qt5 resources
qt5_add_resources(RESOURCES rsclist.qrc)

# build and link baka-mplayer
add_executable(baka-mplayer main.cpp ${RESOURCES})
target_link_libraries(baka-mplayer ${LIBRARIES})

qt5_use_modules(baka-mplayer Widgets Core Gui Network Svg)
if(UNIX)
	qt5_use_modules(baka-mplayer X11Extras)
endif()
