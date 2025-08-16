# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************
cmake_minimum_required (VERSION 3.2.3)

file(GLOB SRCS *.cpp *.hpp)
if(GTest_FOUND)
   add_executable(event_output_plugin_test ${SRCS})
   target_link_libraries(event_output_plugin_test
      wizard_core
      ${SWDEV_THREAD_LIB}
      ${SWDEV_DL_LIB}
      GTest::Main
      GTest::GTest
   )
   add_test(NAME "EventOutput" COMMAND event_output_plugin_test)
   set_property(TARGET event_output_plugin_test PROPERTY FOLDER UnitTests)
endif()
