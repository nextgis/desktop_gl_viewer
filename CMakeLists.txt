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

if(OSX_FRAMEWORK)
  set(INSTALL_BIN_DIR "${CMAKE_INSTALL_PREFIX}/Applications" CACHE INTERNAL "Installation directory for executables" FORCE)
  set(INSTALL_LIB_DIR "${CMAKE_INSTALL_PREFIX}/Library/Frameworks" CACHE INTERNAL "Installation directory for libraries" FORCE)
  set(SKIP_INSTALL_HEADERS ON)
  set(SKIP_INSTALL_EXECUTABLES ON)
  set(SKIP_INSTALL_FILES ON)
  set(SKIP_INSTALL_EXPORT ON)
  set(CMAKE_MACOSX_RPATH ON)
else()
    include(GNUInstallDirs)
    set(INSTALL_BIN_DIR ${CMAKE_INSTALL_BINDIR} CACHE INTERNAL "Installation directory for executables" FORCE)
    set(INSTALL_LIB_DIR ${CMAKE_INSTALL_LIBDIR} CACHE INTERNAL "Installation directory for libraries" FORCE)
    set(INSTALL_INC_DIR ${CMAKE_INSTALL_INCLUDEDIR} CACHE INTERNAL "Installation directory for headers" FORCE)

    option (BUILD_SHARED_LIBS "Build as dynamic library" ON)
endif()

# Tell CMake to run moc when necessary:
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include(util)
check_version(NGGLV_MAJOR_VERSION NGGLV_MINOR_VERSION NGGLV_PATCH_NUMBER)
set(VERSION ${NGGLV_MAJOR_VERSION}.${NGGLV_MINOR_VERSION}.${NGGLV_PATCH_NUMBER})

report_version(${PROJECT_NAME} ${VERSION})

enable_testing()

add_subdirectory (lib)
add_subdirectory (test)

include_directories (${CMAKE_SOURCE_DIR}/lib/src
    ${CMAKE_SOURCE_DIR}/lib/include)

# Widgets finds its own dependencies.
find_package(Qt5 REQUIRED COMPONENTS Widgets Svg)

qt5_add_resources(RESOURCES "${CMAKE_CURRENT_SOURCE_DIR}/res/glviewer.qrc")

set(PROJECT_HEADERS
    src/version.h
    src/mainwindow.h
#    src/mapview.h
    src/glmapview.h
    src/eventsstatus.h
    src/progressstatus.h
    src/locationstatus.h
    src/catalogdialog.h
    src/catalogmodel.h
)

set(PROJECT_SOURCES
    src/main.cpp
    src/mainwindow.cpp
#    src/mapview.cpp
    src/glmapview.cpp
    src/eventsstatus.cpp
    src/progressstatus.cpp
    src/locationstatus.cpp
    src/catalogdialog.cpp
    src/catalogmodel.cpp
)

set(UIS_HDRS
    src/catalogdialog.ui
)

if(MSVC)
    set(PROJECT_SOURCES ${PROJECT_SOURCES} "${CMAKE_CURRENT_SOURCE_DIR}/res/glviewer.rc")
endif()


if(WIN32)
    add_executable(${APP_NAME} WIN32 ${PROJECT_HEADERS} ${PROJECT_SOURCES} ${RESOURCES} ${UIS_HDRS})
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    add_executable(${APP_NAME} MACOSX_BUNDLE ${PROJECT_HEADERS} ${PROJECT_SOURCES} ${RESOURCES} ${UIS_HDRS})

    set_target_properties(${APP_NAME} PROPERTIES
        MACOSX_BUNDLE_INFO_STRING "${PROJECT_NAME} Version ${VERSION}, Copyright 2017 NextGIS."
        # MACOSX_BUNDLE_ICON_FILE ${ICON_FILE_NAME}
       MACOSX_BUNDLE_GUI_IDENTIFIER "${PROJECT_NAME}"
       MACOSX_BUNDLE_LONG_VERSION_STRING "${PROJECT_NAME} Version ${VERSION}"
       MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME}
       MACOSX_BUNDLE_SHORT_VERSION_STRING ${VERSION}
       MACOSX_BUNDLE_BUNDLE_VERSION ${NGGLV_MAJOR_VERSION}
       MACOSX_BUNDLE_COPYRIGHT "Copyright 2017, NextGIS. All Rights Reserved."
       MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/cmake/MacOSXBundleInfo.plist.in"
     )

     add_custom_command(TARGET ${APP_NAME}
         POST_BUILD COMMAND
         ${CMAKE_INSTALL_NAME_TOOL} -add_rpath "~/Applications/NextGIS/Library/Frameworks/"
         $<TARGET_FILE:${APP_NAME}>)
else()
    add_executable(${APP_NAME} ${PROJECT_HEADERS} ${PROJECT_SOURCES} ${RESOURCES} ${UIS_HDRS})
endif()

set_property(TARGET ${APP_NAME} PROPERTY CXX_STANDARD 11)
target_link_libraries(${APP_NAME} Qt5::Widgets Qt5::Svg ngstore)

# install
if(NOT SKIP_INSTALL_LIBRARIES AND NOT SKIP_INSTALL_ALL )
    install(TARGETS ${APP_NAME}
        RUNTIME DESTINATION ${INSTALL_BIN_DIR}
        ARCHIVE DESTINATION ${INSTALL_LIB_DIR}
        LIBRARY DESTINATION ${INSTALL_LIB_DIR}
        FRAMEWORK DESTINATION ${INSTALL_LIB_DIR}
        BUNDLE DESTINATION ${INSTALL_BIN_DIR}
    )
endif()
