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

# Outside includes. swdev_project.cmake contains many useful macros.
include(GenerateExportHeader)
include(swdev_project)

# Add all files into the project to be compiled.
FILE(GLOB SRCS *.cpp *.hpp)

wsf_grammar_file(SRCS "${CMAKE_CURRENT_SOURCE_DIR}/../grammar/sensor_exercise.ag")

# Add the library with sources
add_library(${PROJECT_NAME} ${SRCS})

# Generate header with the __declspec export macro defined.
generate_export_header(${PROJECT_NAME})

# The includes for our export.
target_include_directories(${PROJECT_NAME} PUBLIC
                          "${CMAKE_CURRENT_SOURCE_DIR}"
                          "${PROJECT_BINARY_DIR}/source")

# Link with required libs.
target_link_libraries(${PROJECT_NAME} ${WSF_LIBS})

# Set the project warning level.
swdev_warning_level(${PROJECT_NAME})

if(WSF_PLUGIN_BUILD)
   swdev_plugin_install(${PROJECT_NAME} wsf_plugins)
else()
   swdev_lib_install(${PROJECT_NAME})
endif()
