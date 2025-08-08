# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************
# configuration for automatic inclusion as a WSF extension
set(WSF_EXT_NAME mover_exercise)
# mover_exercise not supported on Linux at this time
if(WIN32)
   if(WSF_PLUGIN_BUILD)
      set(WSF_EXT_TYPE plugin)
   endif()
   set(WSF_EXT_SOURCE_PATH .)
else()
# This is only needed to support the grammar check performed during regression
# testing, and may be safely ignored for the purposes of training. Once the
# mover exercise is supported on Linux, this may be removed.
   configure_file("${CMAKE_CURRENT_LIST_DIR}/grammar/${WSF_EXT_NAME}.ag" "${CMAKE_BINARY_DIR}/grammar/${WSF_EXT_NAME}.ag" COPYONLY)
   set(WSF_EXT_NAME) # Clear variable so the exercise is not added to the buildsystem
endif()
