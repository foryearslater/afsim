# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************
include_directories(${CMAKE_CURRENT_BINARY_DIR})

file(GLOB TEST_SRCS Test*.cpp)

foreach (TEST_SRC ${TEST_SRCS})
   moc_files(TEST_MOC_SRC ${TEST_SRC})
   get_filename_component(TEST_NAME ${TEST_SRC} NAME_WE)
   add_executable(${TEST_NAME} ${TEST_SRC} ${TEST_MOC_SRC})
   swdev_package_link_libraries(${TEST_NAME} qt proj)
#if (WIN32)
#      link_gtiff(${TEST_NAME})
#      link_png(${TEST_NAME})
#   endif (WIN32)
   target_link_libraries(${TEST_NAME} wizard_core)
   add_test(NAME ${TEST_NAME}
            WORKING_DIRECTORY .
            COMMAND ${TEST_NAME})
   set_property(TARGET ${TEST_NAME} PROPERTY FOLDER "Test")
endforeach(TEST_SRC)

