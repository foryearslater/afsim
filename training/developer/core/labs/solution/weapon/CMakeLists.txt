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

project(weapon_exercise)
include(swdev_project)

add_subdirectory(source)
#add_subdirectory(test)

# Places the project inside the "training" folder of the solution.
set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER "training")

# Add source directories to doxygen input
add_wsf_doxygen_input(${CMAKE_CURRENT_SOURCE_DIR}/source)

# Add project to Sphinx for documentation
#add_wsf_doc_input(${CMAKE_CURRENT_SOURCE_DIR})

# INSTALL
#install_sources(source wsf_plugins/${PROJECT_NAME})
#install_sources_all_files(grammar wsf_plugins/${PROJECT_NAME})
#install_sources_all_files(doc wsf_plugins/${PROJECT_NAME})
#install_sources_all_files(data wsf_plugins/${PROJECT_NAME})
#install_sources_all_files(conversion wsf_plugins/${PROJECT_NAME})
#install_tests(test_mission DESTINATION wsf_plugins/${PROJECT_NAME})
#install_source_files(CMakeLists.txt
#                     wsf_module
#                     wsf_cmake_extension.cmake
#                     wsf_plugins/${PROJECT_NAME})
#install_source_files(FILES wsf_module DESTINATION ${INSTALL_SOURCE_ROOT}/wsf_plugins)

#if(WSF_INSTALL_DEMOS)
#   # Demo directories included with the sosm build
#   set(WSF_DEMOS_WEAPON
#
#      )
#   install_wsf_data("${WSF_DEMOS_XIO}" ${WSF_DEMOS_ROOT} demos DEMO)
#endif()
