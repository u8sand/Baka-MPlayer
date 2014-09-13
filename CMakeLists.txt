cmake_minimum_required(VERSION 2.8.3)
project(Baka-MPlayer)
set (CMAKE_VERBOSE_MAKEFILE true)

set(BAKA_MPLAYER_MAJOR_VERSION 1)
set(BAKA_MPLAYER_MINOR_VERSION 9)
set(BAKA_MPLAYER_PATCH_VERSION 8)
set(BAKA_MPLAYER_VERSION
	${BAKA_MPLAYER_MAJOR_VERSION}.${BAKA_MPLAYER_MINOR_VERSION}.${BAKA_MPLAYER_PATCH_VERSION})

# project path
set(PROJECT_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)
set(PROJECT_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
# where to put outputs
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

# C++11 support
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# qt required options
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

# get required packages
find_package(Qt5
	REQUIRED COMPONENTS
	Core
	Gui
	Network
)
find_package(PkgConfig
	REQUIRED COMPONENTS
	mpv
)

include_directories(
	${QT5_INCLUDES}
	${PkgConfigmpv_INCLUDES}
)
add_definitions(
	-DBAKA_MPLAYER_VERSION="\\"${BAKA_MPLAYER_VERSION}\\""
	-DSETTINGS_FILE="\\"bakamplayer\\""
)
set(LIBRARIES
	${Qt5Core_LIBRARIES}
	${Qt5Gui_LIBRARIES}
	${Qt5Network_LIBRARIES}
	${mpv_LIBRARIES}
)
set(SOURCES
	src/mpvhandler
	src/updatemanager
	src/widgets/customslider
	src/widgets/openbutton
	src/widgets/indexbutton
	src/widgets/customlistwidget
	src/widgets/seekbar
	src/widgets/customlabel
	src/widgets/customsplitter
	src/ui/mainwindow
	src/ui/locationdialog
	src/ui/infodialog
	src/ui/jumpdialog
	src/ui/aboutdialog
	src/ui/updatedialog
	src/ui/inputdialog
	src/ui/dimdialog
	src/ui/preferencesdialog
)

foreach(src ${SOURCES})
	get_filename_component(LIB ${src} NAME_WE)
	add_library(${LIB} ${src}.cpp)
	set(LIBS "${LIBS} ${LIB}")
endforeach()

add_executable(baka-mplayer src/main.cpp)
target_link_libraries(baka-mplayer ${LIBRARIES})