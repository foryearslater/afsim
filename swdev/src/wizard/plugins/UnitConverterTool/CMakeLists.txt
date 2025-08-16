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

if(BUILD_WKF_PLUGIN_UnitConverterTool)
   include(swdev_project)

   # Add project to Sphinx for documentation
   add_wsf_doc_input(${CMAKE_CURRENT_SOURCE_DIR})
   
   if (WSF_INSTALL_SOURCE)
      install_sources_all_files(. wizard/plugins/UnitConverterTool)
   endif()
endif()
