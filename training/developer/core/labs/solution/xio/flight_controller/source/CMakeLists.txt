# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# Copyright 2019 Infoscitex, a DCS Company. All rights reserved.
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************

# Outside includes.  "swdev_project" contains many useful macros for WSF.
include(GenerateExportHeader)
include(swdev_project)

# Specify the name of this project.
project(flight_controller)

set(SWDEV_QT_PACKAGE "qt-5.12.11" CACHE STRING "" FORCE)
mark_as_advanced(FORCE SWDEV_QT_PACKAGE)
swdev_acquire_packages(${CMAKE_CURRENT_SOURCE_DIR} ${SWDEV_QT_PACKAGE})

# Add all files into the project to be compiled.
FILE(GLOB SRCS *.cpp *.hpp)
file(GLOB UIS ${CMAKE_CURRENT_SOURCE_DIR}/../ui/*.ui)

include(qt_project)
configure_qt_project(UI UI_HEADERS ${UIS})

# Our target is an executable
add_executable(${PROJECT_NAME} ${SRCS} ${UI_HEADERS})

# Places the project inside the "training" folder of the WSF solution.
set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER "training")

# Set the project warning level to the WSF project's level.
swdev_warning_level(${PROJECT_NAME})

# The includes for our export.
target_include_directories(${PROJECT_NAME} PUBLIC
   "${CMAKE_CURRENT_SOURCE_DIR}"
   "${PROJECT_BINARY_DIR}/source")

# Link with core WSF libs.
target_link_libraries(${PROJECT_NAME} ${WSF_LIBS} Qt5::Core Qt5::Gui Qt5::Widgets)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${INSTALL_EXE_PATH} COMPONENT RUNTIME)
