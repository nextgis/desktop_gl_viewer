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

if(UNIX AND NOT APPLE)

    find_package (EGL REQUIRED)
    if(EGL_FOUND)
        include_directories(${EGL_INCLUDE_DIRS})
    endif()

    find_package (GLES2 REQUIRED)
    if(OPENGLES2_FOUND)
        include_directories(${OPENGLES2_INCLUDE_DIRS})
    endif()

    find_package (X11 REQUIRED)
    if(X11_FOUND)
        include_directories(${X11_INCLUDE_DIR})
    endif()

    find_library(M_LIB m)

    set(HHEADERS
        esUtil.h
    )

    set(CSOURCES
        esShader.c
        esShapes.c
        esTransform.c
        esUtil.c
    )

    add_custom_target (clean_tests ALL COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_CURRENT_BINARY_DIR}/tmp" )

    add_executable(egltriangle Hello_Triangle.c ${CSOURCES} ${HHEADERS})
    target_link_libraries(egltriangle ${EGL_LIBRARIES} ${OPENGLES2_LIBRARIES}
        ${X11_LIBRARIES} ${M_LIB})

    add_executable(eglvertexshader Simple_VertexShader.c ${CSOURCES} ${HHEADERS})
    target_link_libraries(eglvertexshader ${EGL_LIBRARIES} ${OPENGLES2_LIBRARIES}
        ${X11_LIBRARIES} ${M_LIB})

endif()
