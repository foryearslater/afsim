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

# *****************************************************************************
# CMAKE file: crd_importer_lib
# *****************************************************************************

include(GenerateExportHeader)
project(crd_importer_lib)

# *****************************************************************************
# Get the source files
file(GLOB SRCS *.hpp *.cpp)

# *****************************************************************************
# The library we are trying to build
add_library(${PROJECT_NAME} ${SRCS})
generate_export_header(${PROJECT_NAME})
set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER "wizard/plugins/WizCRDImporter")
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}" ${PROJECT_BINARY_DIR})
swdev_warning_level(${PROJECT_NAME})

# *****************************************************************************
# Libraries
target_link_libraries(${PROJECT_NAME} wkf util)
swdev_lib_install(${PROJECT_NAME})
