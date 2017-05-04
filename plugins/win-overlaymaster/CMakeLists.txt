project(win-overlaymaster)

set(BOOST_ROOT "c:/soft/boost")
set(Boost_USE_STATIC_LIBS        ON)
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)

find_package(Boost 1.55 COMPONENTS system REQUIRED)

include_directories(${Boost_INCLUDE_DIRS}) 
LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})

set(win-overlaymaster_SOURCES
	overlaymaster.cpp)

add_library(win-overlaymaster MODULE
	${win-overlaymaster_SOURCES})
target_link_libraries(win-overlaymaster
	libobs
	${Boost_LIBRARIES})

install_obs_plugin_with_data(win-overlaymaster data)


