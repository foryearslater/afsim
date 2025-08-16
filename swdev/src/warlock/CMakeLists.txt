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

include(swdev_project)

add_subdirectory(warlock_exec/source)

# *****************************************************************************
# Add project to Sphinx for documentation
add_wsf_doc_input(${CMAKE_CURRENT_SOURCE_DIR}/warlock_exec)
add_wsf_doc_input(${CMAKE_CURRENT_SOURCE_DIR}/warlock_core)
add_wsf_doc_sidebar_ext(${CMAKE_CURRENT_SOURCE_DIR}/warlock_exec/wsftheme/wsf_sidebar_warlock.html)

install_sources_all_files(data warlock)
install_source_files(CMakeLists.txt README.md wsf_cmake_extension.cmake wsf_module warlock)

if(WSF_INSTALL_TRAINING)
   # Training directories ALWAYS included with warlock build
   set(WSF_USER_TRAIN_DIRS
       7_Warlock_Features
      )
   install_wsf_data("${WSF_USER_TRAIN_DIRS}" ${WSF_TRAINING_ROOT}/user training/user Training)

   set(WSF_DEV_TRAIN_WARLOCK_DIRS
       wkf
      )
   install_wsf_data("${WSF_DEV_TRAIN_WARLOCK_DIRS}" ${WSF_TRAINING_ROOT}/developer training/developer Training)
endif()

if(WSF_INSTALL_DEMOS)
   set(WSF_WARLOCK_DEMO_DIRS
       wargame
   )
   install_wsf_data("${WSF_WARLOCK_DEMO_DIRS}" ${WSF_DEMOS_ROOT} demos Demo)
endif()
