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
# CMAKE file: usmtf
# *****************************************************************************
include(GenerateExportHeader)

# *****************************************************************************
# Get the source files
file(GLOB SRCS *.hpp *.cpp)

# *****************************************************************************
# The library we are trying to build
add_library(${PROJECT_NAME} ${SRCS})
generate_export_header(${PROJECT_NAME})
set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER "wizard")
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}" ${PROJECT_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR})

# *****************************************************************************
# Libraries
swdev_lib_install(${PROJECT_NAME})
swdev_acquire_packages(${CMAKE_CURRENT_SOURCE_DIR}/..
                       ${SWDEV_QT_PACKAGE})
swdev_warning_level(${PROJECT_NAME})
find_package(Qt5Core)
#dependencies on utils
target_link_libraries(${PROJECT_NAME} util Qt5::Core)

# Add project to Sphinx for documentation
add_wsf_doc_input(${CMAKE_CURRENT_SOURCE_DIR}/..)

#doxygen
add_wsf_doxygen_input(${CMAKE_CURRENT_SOURCE_DIR})

add_custom_command(TARGET usmtf POST_BUILD
                                  COMMAND ${CMAKE_COMMAND} -E echo "Path=${CMAKE_CURRENT_SOURCE_DIR}/../data" > $<TARGET_FILE_DIR:usmtf>/usmtf.conf
                                  COMMENT "Configuring usmtf.conf")


