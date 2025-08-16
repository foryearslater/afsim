# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************
# *****************************************************************************
# CMAKE file: wizard_core
# *****************************************************************************

# Set the project
project(wizard_core)

if (MSVC)
   add_compile_options(-wd4251 -wd4286 -wd4275)
endif()

# *****************************************************************************
# User-defined macros
# Ordering of these is important for some reason
include(swdev_project)
include(qt_project)
include(${TOOLS_DIRECTORY}/util/source/UtStringEnum.cmake)

# Ref: util/source/UtStringEnum.cmake
UT_STRING_ENUM("${WIZARD_ROOT}/main/source/ide_strings.txt")

macro(header_directories return_list)
   file(GLOB_RECURSE new_list *.h *.hpp)
   set(dir_list "")
   foreach(file_path ${new_list})
      get_filename_component(dir_path ${file_path} PATH)
      set(dir_list ${dir_list} ${dir_path})
   endforeach()
   list(REMOVE_DUPLICATES dir_list)
   set(${return_list} ${dir_list})
endmacro()

header_directories(ALL_HDR_DIRS)
include_directories(${ALL_HDR_DIRS})

# *****************************************************************************
# Get the source files from the sub-directories
set(SRCS "")
set(WIZARD_SUB_DIRS application core editor gui map revision Us util)
foreach(SUB ${WIZARD_SUB_DIRS})
   file(GLOB SUB_SRCS ${SUB}/*.hpp ${SUB}/*.cpp ${SUB}/*.h)
   set(SRCS ${SRCS} ${SUB_SRCS})
   source_group("Source Files\\${SUB}" FILES ${SUB_SRCS})
   set(WIZARD_INCLUDES "${WIZARD_INCLUDES}" "${CMAKE_CURRENT_SOURCE_DIR}\\${SUB}")
endforeach()

# Exclude targeted compiler warnings from Coco/R generated source files.
set(_coco_source_compile_options)
set(_cmake_source_compile_property COMPILE_OPTIONS)
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
   list(APPEND _coco_source_compile_options -Wno-unused-variable)
endif()
if (CMAKE_VERSION VERSION_LESS 3.11)
   set(_cmake_source_compile_property COMPILE_FLAGS)
endif()
set_source_files_properties(Us/Scanner.cpp
   PROPERTIES
   ${_cmake_source_compile_property} "${_coco_source_compile_options}")

set(WIZARD_INCLUDES ${WIZARD_INCLUDES} PARENT_SCOPE)

file(GLOB ALL_SRCS ${SRCS} *.cpp *.hpp)
file(GLOB QRC ${CMAKE_CURRENT_SOURCE_DIR}/../ui/resources/*.qrc)
file(GLOB UIS ${CMAKE_CURRENT_SOURCE_DIR}/../ui/*.ui) # Get a list of the ui files

configure_qt_project(MODULES PrintSupport Concurrent Network UI UI_HEADERS ${UIS})

# *****************************************************************************
# The library we are trying to build
add_library(${PROJECT_NAME} ${ALL_SRCS} ${UI_HEADERS} ${QRC})
swdev_warning_level(${PROJECT_NAME})
set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER "wizard")

# *****************************************************************************
# Includes
# Ref: 3rd_party-cmake/config.cmake
swdev_package_include_directories(${PROJECT_NAME} sqlite)

set(UT_STRING_ENUM_INCLUDE_PATH ${CMAKE_BINARY_DIR}/include)
include_directories(${UT_STRING_ENUM_INCLUDE_PATH})

# *****************************************************************************
# Libraries
# Ref: 3rd_party-cmake/config.cmake
swdev_package_link_libraries(${PROJECT_NAME} sqlite)
target_link_libraries(${PROJECT_NAME} util utilqt wsf wsf_parser Qt5::PrintSupport Qt5::Concurrent Qt5::Network wkf wkf_common )

link_wsf_optional_projects(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC ${ALL_HDR_DIRS} ${PROJECT_BINARY_DIR})

# *****************************************************************************
# Write the string header after all the includes and adds
# Ref: util/source/UtStringEnum.cmake
UT_STRING_ENUM_WRITE_HEADER()

# *****************************************************************************
# Installation

# Lib install
swdev_lib_install(${PROJECT_NAME})

# Source install
install_sources_all_files(../ wizard/lib)

# Add source directories to doxygen input
add_wsf_doxygen_input(${CMAKE_CURRENT_SOURCE_DIR})
add_wsf_doxygen_input(${CMAKE_CURRENT_SOURCE_DIR}/application)
add_wsf_doxygen_input(${CMAKE_CURRENT_SOURCE_DIR}/core)
add_wsf_doxygen_input(${CMAKE_CURRENT_SOURCE_DIR}/editor)
add_wsf_doxygen_input(${CMAKE_CURRENT_SOURCE_DIR}/gui)
add_wsf_doxygen_input(${CMAKE_CURRENT_SOURCE_DIR}/map)
add_wsf_doxygen_input(${CMAKE_CURRENT_SOURCE_DIR}/revision)
add_wsf_doxygen_input(${CMAKE_CURRENT_SOURCE_DIR}/Us)
add_wsf_doxygen_input(${CMAKE_CURRENT_SOURCE_DIR}/util)

# Add project to Sphinx for documentation
add_wsf_doc_input(${CMAKE_CURRENT_SOURCE_DIR}/../)
