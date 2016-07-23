#*************************************************************************** 
# Project:  NextGIS GL Viewer
# Purpose:  cmake script
# Author:   Dmitry Baryshnikov, bishop.dev@gmail.com
#***************************************************************************
#   Copyright (C) 2016 NextGIS, info@nextgis.ru
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 2 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.
#***************************************************************************

cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)

set(PRJ_NAME ngglviewer)
set(APP_NAME ngglviewer)

project(${PRJ_NAME}) 

# some init settings
set(CMAKE_COLOR_MAKEFILE ON)
# set path to additional CMake modules
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

set(INSTALL_BIN_DIR "${CMAKE_INSTALL_PREFIX}/bin" CACHE INTERNAL "Installation directory for executables" FORCE)
set(INSTALL_LIB_DIR "${CMAKE_INSTALL_PREFIX}/lib" CACHE INTERNAL "Installation directory for libraries" FORCE)
set(INSTALL_INC_DIR "${CMAKE_INSTALL_PREFIX}/include" CACHE INTERNAL "Installation directory for headers" FORCE)

# Tell CMake to run moc when necessary:
set(CMAKE_AUTOMOC ON)

# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include(util)
check_version(NGGLV_MAJOR_VERSION NGGLV_MINOR_VERSION NGGLV_PATCH_NUMBER)
set(VERSION ${NGGLV_MAJOR_VERSION}.${NGGLV_MINOR_VERSION}.${NGGLV_PATCH_NUMBER})

report_version(${PROJECT_NAME} ${VERSION})

option (BUILD_SHARED_LIBS "Build as dynamic library" ON)

enable_testing()

add_subdirectory (lib)
add_subdirectory (test)

include_directories (${CMAKE_SOURCE_DIR}/lib/src)

# Widgets finds its own dependencies.
find_package(Qt5 REQUIRED COMPONENTS Widgets)

qt5_add_resources(RESOURCES "${CMAKE_CURRENT_SOURCE_DIR}/res/glviewer.qrc")

set(PROJECT_HEADERS
    src/version.h
    src/mainwindow.h
    src/mapview.h
    src/eventsstatus.h
    src/progressstatus.h
)

set(PROJECT_SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/mapview.cpp
    src/eventsstatus.cpp
    src/progressstatus.cpp
)

if(MSVC)
    set(PROJECT_SOURCES ${PROJECT_SOURCES} "${CMAKE_CURRENT_SOURCE_DIR}/res/glviewer.rc")
endif()


if(WIN32)
    add_executable(${APP_NAME} WIN32 ${PROJECT_HEADERS} ${PROJECT_SOURCES} ${RESOURCES} ${UIS_HDRS})
else()
    add_executable(${APP_NAME} ${PROJECT_HEADERS} ${PROJECT_SOURCES} ${RESOURCES} ${UIS_HDRS})
endif()

set_property(TARGET ${APP_NAME} PROPERTY CXX_STANDARD 11)
target_link_libraries(${APP_NAME} Qt5::Widgets ngstore)

# install
if(NOT SKIP_INSTALL_LIBRARIES AND NOT SKIP_INSTALL_ALL )
    install(TARGETS ${APP_NAME}
        RUNTIME DESTINATION "${INSTALL_BIN_DIR}"
        ARCHIVE DESTINATION "${INSTALL_LIB_DIR}"
        LIBRARY DESTINATION "${INSTALL_LIB_DIR}" )
endif()

