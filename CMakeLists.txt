cmake_minimum_required(VERSION 2.8.7)
project(Baka-MPlayer)
set (CMAKE_VERBOSE_MAKEFILE true)

set(BAKA_MPLAYER_MAJOR_VERSION 1)
set(BAKA_MPLAYER_MINOR_VERSION 9)
set(BAKA_MPLAYER_PATCH_VERSION 9)
set(BAKA_MPLAYER_VERSION
	${BAKA_MPLAYER_MAJOR_VERSION}.${BAKA_MPLAYER_MINOR_VERSION}.${BAKA_MPLAYER_PATCH_VERSION})

# C++11 support
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# output in the directory cmake was executed
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

# qt
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# find qt5
find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5Svg REQUIRED)

if(UNIX)
	find_package(Qt5X11Extras REQUIRED)
	find_package(X11 REQUIRED)
endif()

# find mpv
find_package(PkgConfig)
pkg_check_modules(PC_MPV QUIET mpv)
set(MPV_DEFINITIONS ${PC_MPV_CFLAGS_OTHER})
find_path(MPV_INCLUDE_DIR mpv/client.h
          HINTS ${PC_MPV_INCLUDEDIR} ${PC_MPV_INCLUDE_DIRS}
          PATH_SUFFIXES libmpv)
find_library(MPV_LIBRARY NAMES mpv libmpv)
set(MPV_LIBRARIES ${MPV_LIBRARY} )
set(MPV_INCLUDE_DIRS ${MPV_INCLUDE_DIRS})


# add includes/definitions/libraries
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Core_EXECUTABLE_COMPILE_FLAGS}")
include_directories(
	${Qt5Core_INCLUDE_DIRS}
	${Qt5Gui_INCLUDE_DIRS}
	${Qt5Network_INCLUDE_DIRS}
	${Qt5Svg_INCLUDE_DIRS}
	${MPV_INCLUDE_DIRS}
)
add_definitions(
	-DBAKA_MPLAYER_VERSION="${BAKA_MPLAYER_VERSION}"
	-DSETTINGS_FILE="bakamplayer"
	${Qt5Core_DEFINITIONS}
	${Qt5Gui_DEFINITIONS}
	${Qt5Network_DEFINITIONS}
	${Qt5Svg_DEFINITIONS}
	${MPV_DEFINITIONS}
)
set(LIBRARIES
	${Qt5Core_LIBRARIES}
	${Qt5Gui_LIBRARIES}
	${Qt5Network_LIBRARIES}
	${Qt5Svg_LIBRARIES}
	${MPV_LIBRARIES}
)

# add os specific includes/definitions/libraries
if(UNIX)
	include_directories(
		${Qt5X11Extras_INCLUDE_DIRS}
		${X11_INCLUDE_DIRS}
	)
	add_definitions(
		${Qt5X11Extras_DEFINITIONS}
		${X11_DEFINITIONS}
	)
	list(APPEND LIBRARIES 
		${Qt5X11Extras_LIBRARIES}
		${X11_LIBRARIES}
	)
endif()

# go into source
add_subdirectory(src)
