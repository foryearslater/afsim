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
project(usmtf_test)

FILE(GLOB_RECURSE SRCS *.cpp *.hpp)

if(GTest_FOUND)
   add_executable(usmtf_test ${SRCS})
   
   # Required in order to run in debug from Visual Studio
   if (MSVC)
      include(user_file_config)
      create_vs_debug_env(${CMAKE_CURRENT_SOURCE_DIR}/..
                          "${TEMPLATE_DIR}/Template.VisualStudio.Settings.user"
                         )
    endif()
   
   swdev_warning_level(usmtf_test)
   target_include_directories(usmtf_test PUBLIC ${_INCLUDES})
   target_link_libraries(usmtf_test
      usmtf
      GTest::Main
      GTest::GTest
   )
   add_test(NAME "usmtf" COMMAND usmtf_test "${CMAKE_CURRENT_SOURCE_DIR}/message_examples")
   if(WIN32)
      # debug
      add_custom_command(TARGET usmtf_test POST_BUILD
                         COMMAND ${CMAKE_COMMAND} -E copy ${QTROOT}/bin/Qt5Cored.dll $<TARGET_FILE_DIR:usmtf_test> VERBATIM)
      # release
      add_custom_command(TARGET usmtf_test POST_BUILD
                         COMMAND ${CMAKE_COMMAND} -E copy ${QTROOT}/bin/Qt5Core.dll $<TARGET_FILE_DIR:usmtf_test> VERBATIM)
   endif()
   set_property(TARGET usmtf_test PROPERTY FOLDER UnitTests)
endif()
