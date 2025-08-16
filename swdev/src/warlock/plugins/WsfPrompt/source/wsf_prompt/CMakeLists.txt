# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# Copyright 2016 Infoscitex, a DCS Company. All rights reserved.
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************

project(wsf_prompt)
include(swdev_project)

include_directories(source ${CMAKE_BINARY_DIR} ${CMAKE_BINARY_DIR}/include)
add_subdirectory(source)

# Add source directories to doxygen input
#add_wsf_doxygen_input(${SRCS}) TODO Use this instead? Adds all source files
add_wsf_doxygen_input(${CMAKE_CURRENT_SOURCE_DIR}/source)

# Add project to Sphinx for documentation
add_wsf_doc_input(${CMAKE_CURRENT_SOURCE_DIR})

# INSTALL
# Installed by a warlock installation command already
#install_sources_all_files(. warlock/wsf_plugins/${PROJECT_NAME})
#install_source_files(wsf_module warlock/wsf_plugins)
