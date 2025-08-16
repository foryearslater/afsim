# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************

file(GLOB_RECURSE SRCS *.cpp *.hpp)
if(GTest_FOUND)
   add_executable(wizard_test ${SRCS})
   swdev_package_link_libraries(wizard_test qt icu)
   target_link_libraries(wizard_test
      wizard_core
      ${SWDEV_THREAD_LIB}
      ${SWDEV_DL_LIB}
      GTest::Main
      GTest::GTest
   )
   add_test(NAME "wizard" COMMAND wizard_test)
endif()
