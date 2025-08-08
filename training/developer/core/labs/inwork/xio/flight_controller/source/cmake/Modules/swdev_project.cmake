# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************

# This file defines the commands and macros that are used
# to support a software build. It is intended to be included
# by the top-level CMakeLists.txt file.

# GCC Compiler version:
#---Obtain the major and minor version of the GNU compiler---
MACRO(get_gcc_version MAJOR_VAR MINOR_VAR PATCH_VAR)
   # cache the results so that we don't have execute gcc for each lookup
  get_property(ALREADY_SET GLOBAL PROPERTY SWDEV_GCC_MAJOR SET)
  IF (NOT ALREADY_SET)
     exec_program(${CMAKE_C_COMPILER} ARGS "-dumpversion" OUTPUT_VARIABLE _gcc_version_info)
     string(REGEX REPLACE "^([0-9]+).*$"                   "\\1" GCC_MAJOR ${_gcc_version_info})
     string(REGEX REPLACE "^[0-9]+\\.([0-9]+).*$"          "\\1" GCC_MINOR ${_gcc_version_info})
     string(REGEX REPLACE "^[0-9]+\\.[0-9]+\\.([0-9]+).*$" "\\1" GCC_PATCH ${_gcc_version_info})

     if(GCC_PATCH MATCHES "\\.+")
       set(GCC_PATCH "")
     endif()
     if(GCC_MINOR MATCHES "\\.+")
       set(GCC_MINOR "")
     endif()
     if(GCC_MAJOR MATCHES "\\.+")
       set(GCC_MAJOR "")
     endif()
     set_property(GLOBAL PROPERTY SWDEV_GCC_MAJOR "${GCC_MAJOR}")
     set_property(GLOBAL PROPERTY SWDEV_GCC_MINOR "${GCC_MINOR}")
     set_property(GLOBAL PROPERTY SWDEV_GCC_PATCH "${GCC_PATCH}")
     message(STATUS "Found GCC. Major version ${GCC_MAJOR}, minor version ${GCC_MINOR}")
  ENDIF()
  get_property(${MAJOR_VAR} GLOBAL PROPERTY "SWDEV_GCC_MAJOR")
  get_property(${MINOR_VAR} GLOBAL PROPERTY "SWDEV_GCC_MINOR")
  get_property(${PATCH_VAR} GLOBAL PROPERTY "SWDEV_GCC_PATCH")
ENDMACRO()

# Set up C++ standard definitions and swdev_use_cpp11() macro based on compiler
if (CMAKE_COMPILER_IS_GNUCXX)
  get_gcc_version(GCC_MAJOR GCC_MINOR GCC_PATCH)
  set(COMPILER_VERSION gcc${GCC_MAJOR}${GCC_MINOR}${GCC_PATCH})
  if(GCC_MAJOR STRGREATER 4)
    if(CMAKE_VERSION VERSION_LESS "3.1")
      ADD_DEFINITIONS(-std=c++11)
    else()
      set(CMAKE_CXX_STANDARD 11)
    endif()
  elseif(GCC_MAJOR STREQUAL 4)
    if(GCC_MINOR STRGREATER 6)
      if (CMAKE_VERSION VERSION_LESS "3.1")
         ADD_DEFINITIONS(-std=c++11)
      else()
         set(CMAKE_CXX_STANDARD 11)
      endif()
    elseif(GCC_MINOR STRGREATER 3)
      ADD_DEFINITIONS(-std=c++0x)
    else()
      message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
    endif()
  else()
    message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
  endif()
elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
  if (CMAKE_VERSION VERSION_LESS "3.1")
    ADD_DEFINITIONS(-std=c++11)
  else()
    set(CMAKE_CXX_STANDARD 11)
  endif()
else()
  set(GCC_MAJOR 0)
  set(GCC_MINOR 0)
endif()

# Force the inclusion of the '_input' file. Used by IDE build
MACRO(ADD_FORCED_INCLUDE _input)
   GET_FILENAME_COMPONENT(_name ${_input} NAME)
   SET(_name ${_input})
   IF(MSVC)
      set_property(GLOBAL PROPERTY COMPILE_FLAGS "/FI${_name}")
   ENDIF(MSVC)
   IF(CMAKE_COMPILER_IS_GNUCXX)
      set_property(GLOBAL PROPERTY COMPILE_FLAGS "-include ${_name}")
   ENDIF(CMAKE_COMPILER_IS_GNUCXX)
ENDMACRO()

# Define compiler definitions and shared library install locations that we want for every project
if(WIN32)
   add_definitions("-D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_WARNINGS")
   add_definitions("/MP /EHsc")
   if(BUILD_SHARED_LIBS)
      add_definitions("-DSWDEV_ALL_USE_DLL")
   endif(BUILD_SHARED_LIBS)
   # On windows, the dll's must reside in the same path as the .exe
   if("${INSTALL_DLL_PATH}" STREQUAL "")
      set(INSTALL_DLL_PATH .)
   endif("${INSTALL_DLL_PATH}" STREQUAL "")
else(WIN32)
   # On linux, we can use rpath to locate so's in a subdirectory
   if("${INSTALL_DLL_PATH}" STREQUAL "")
      set(INSTALL_DLL_PATH ./libs)
   endif("${INSTALL_DLL_PATH}" STREQUAL "")
   # disable strict aliasing optimization (for UtFunction)
   add_definitions("-fno-strict-aliasing")
endif(WIN32)

if("${INSTALL_EXE_PATH}" STREQUAL "")
   set(INSTALL_EXE_PATH .)
endif("${INSTALL_EXE_PATH}" STREQUAL "")

# Setup install of project libraries. Used by extension and sub-project
# CMakeLists.txt files.
macro(swdev_lib_install TARGET)
   get_target_property(library_type ${TARGET} TYPE)

   if ("${library_type}" STREQUAL "SHARED_LIBRARY")
      if (DEFINED INSTALL_LIB_PATH AND NOT "${INSTALL_LIB_PATH}" STREQUAL "")
         install(TARGETS ${TARGET}
            RUNTIME DESTINATION ${INSTALL_DLL_PATH} COMPONENT RUNTIME
               PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
                                 GROUP_EXECUTE GROUP_WRITE GROUP_READ
                                 WORLD_READ WORLD_EXECUTE
            ARCHIVE DESTINATION ${INSTALL_LIB_PATH} COMPONENT ARCHIVE
               PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
                                 GROUP_EXECUTE GROUP_WRITE GROUP_READ
                                 WORLD_READ WORLD_EXECUTE
            LIBRARY DESTINATION ${INSTALL_DLL_PATH} COMPONENT LIBRARY
               PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
                                 GROUP_EXECUTE GROUP_WRITE GROUP_READ
                                 WORLD_READ WORLD_EXECUTE
                                 )
      else()
         install(TARGETS ${TARGET}
            RUNTIME DESTINATION ${INSTALL_DLL_PATH} COMPONENT RUNTIME
               PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
                                 GROUP_EXECUTE GROUP_WRITE GROUP_READ
                                 WORLD_READ WORLD_EXECUTE
            LIBRARY DESTINATION ${INSTALL_DLL_PATH} COMPONENT LIBRARY
               PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
                                 GROUP_EXECUTE GROUP_WRITE GROUP_READ
                                 WORLD_READ WORLD_EXECUTE
                                 )
      endif()
   endif()
endmacro()

# Add libraries for Windows sockets
macro(link_sockets TARGET)
   if(WIN32)
      target_link_libraries(${TARGET} ws2_32)
   endif(WIN32)
endmacro(link_sockets TARGET)

if(WIN32)
   set(SWDEV_THREAD_LIB)
   set(SWDEV_DL_LIB)
else(WIN32)
   set(SWDEV_THREAD_LIB pthread)
   set(SWDEV_DL_LIB dl)
endif(WIN32)

# Force 32 bit exe build.
macro(force_32bit)
   if(NOT WIN32)
      add_definitions("-m32")
      set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -m32")
      set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -m32")
      set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
   endif(NOT WIN32)
endmacro()

# The add_subdirectory() command can only be used once for a directory
# this wraps add_subdirectory() so that it only executes the first time
# We assume TARGET_NAME will be the same as the bin subdirectory name
macro(try_add_subdirectory DIR TARGET_NAME)
   if(NOT TARGET ${TARGET_NAME})
      add_subdirectory(${DIR} ${TARGET_NAME})
   endif(NOT TARGET ${TARGET_NAME})
endmacro(try_add_subdirectory DIR TARGET_NAME)

# Recurse directory (SRC_DIR) installing that directory with the source files
# matching the pattern under INSTALL_SOURCE_ROOT/TARGET_DIR
macro(install_sources SRC_DIR TARGET_DIR)
   if(NOT "${INSTALL_SOURCE_ROOT}" STREQUAL "")
      install(DIRECTORY ${SRC_DIR} DESTINATION ${INSTALL_SOURCE_ROOT}/${TARGET_DIR}
         FILES_MATCHING PATTERN "*.cpp"
                        PATTERN "*.hpp"
                        PATTERN "*.hxx"
                        PATTERN "*.cxx"
                        PATTERN "*.c"
                        PATTERN "*.h"
                        PATTERN "*.txt"
                        PATTERN "Doxyfile"
                        PATTERN "CMakeLists.txt"
                        PATTERN "wsf_module"
                        PATTERN "*.cmake"
                        PATTERN "*.in"
                        PATTERN "*.rc"
                        PATTERN "*.ico"
                        PATTERN "*.png"
                        PATTERN "*.timestamp"      EXCLUDE
                        PATTERN ".git"             EXCLUDE
                        ${ARGN})
   endif()
endmacro(install_sources SRC_DIR TARGET_DIR)

# Recurse directory (SRC_DIR) installing that directory with all files,
# except those excluded, under INSTALL_SOURCE_ROOT/TARGET_DIR
macro(install_sources_all_files SRC_DIR TARGET_DIR)
   if(NOT "${INSTALL_SOURCE_ROOT}" STREQUAL "")
      install(DIRECTORY ${SRC_DIR} DESTINATION ${INSTALL_SOURCE_ROOT}/${TARGET_DIR}
         FILES_MATCHING PATTERN "*.*"
                        PATTERN "wsf_module"
                        PATTERN ".*.swp"         EXCLUDE
                        PATTERN ".*.swo"         EXCLUDE
                        PATTERN "*.timestamp"    EXCLUDE
                        PATTERN ".git"           EXCLUDE
                        # doesn't work:  PATTERN ".*"             EXCLUDE
                        PATTERN "AAA_PACKAGE"    EXCLUDE
                        ${ARGN})
   endif()
endmacro()

# Take a list of relative or absolute paths and assign to VARNAME the absolute paths
macro(swdev_absolute_paths VARNAME)
   set(${VARNAME})
   foreach(ARG ${ARGN})
      if (IS_ABSOLUTE "${ARG}")
         GET_FILENAME_COMPONENT(ARG_ABS_PATH "${ARG}" ABSOLUTE)
         set(${VARNAME} ${${VARNAME}} "${ARG_ABS_PATH}")
      elseif(EXISTS "${ARG}")
         GET_FILENAME_COMPONENT(ARG_ABS_PATH "${ARG}" ABSOLUTE)
         set(${VARNAME} ${${VARNAME}} "${ARG_ABS_PATH}")
      elseif (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${ARG}")
         GET_FILENAME_COMPONENT(ARG_ABS_PATH "${CMAKE_CURRENT_SOURCE_DIR}/${ARG}" ABSOLUTE)
         set(${VARNAME} ${${VARNAME}} "${ARG_ABS_PATH}")
      endif()
   endforeach()
endmacro()

# Microsoft's compilers need to use the /bigobj flag for source files containing many symbols
# This macro marks a list of sources as large (symbols over the 2^16 limit)
# These options tell the compiler to use 32-bit addressing and enable inline
# expansion, which cuts down on the symbol count about 30%
MACRO(large_source_files)
   IF(WIN32)
      FOREACH(SRC ${ARGV})
         set_source_files_properties(${SRC} PROPERTIES COMPILE_FLAGS "/bigobj /Ob2")
      ENDFOREACH()
   ENDIF()
ENDMACRO()

# Suffix libraries with a semi-unique ending
MACRO(use_swdev_lib_suffixes)
   IF(WIN32)
      SET(CMAKE_DEBUG_POSTFIX "_d")
   ELSE()
      get_gcc_version(GCC_MAJOR GCC_MINOR GCC_PATCH)
      # 64-bit detection
      SET(pARCH_SUFFIX)
      if(CMAKE_SIZEOF_VOID_P EQUAL 8)
         string(FIND "${CMAKE_EXE_LINKER_FLAGS}" "-m32" IS_32_BIT)
         if ("${IS_32_BIT}" STREQUAL "-1")
            set(pARCH_SUFFIX "m64")
         endif()
      endif()
      SET(CMAKE_DEBUG_POSTFIX "_d_ln${GCC_MAJOR}${pARCH_SUFFIX}")
      SET(CMAKE_RELEASE_POSTFIX "_ln${GCC_MAJOR}${pARCH_SUFFIX}")
   ENDIF()
ENDMACRO()

# Can be used to suffix executable names with _win and _ln4 (for example) on
# Windows and Linux respectively.
MACRO(set_swdev_output_suffixes)
   IF(WIN32)
      SET(SWDEV_OUTPUT_POSTFIX "_win")
   ELSE()
      get_gcc_version(GCC_MAJOR GCC_MINOR GCC_PATCH)
      SET(SWDEV_OUTPUT_POSTFIX "_ln${GCC_MAJOR}")
   ENDIF()
ENDMACRO()

# Sets the correct executable name for mission, sensor_plot, etc.
# * Suffixes the executable name with "_x" when building shared libraries/plugin version.
# * Suffixes Linux executable names with ".exe".
macro(set_wsf_project_output_name WSF_PROJECT_NAME)
   set_swdev_output_suffixes()
   set(WSF_OUTPUT_NAME "${WSF_PROJECT_NAME}${SWDEV_OUTPUT_POSTFIX}")
   if(BUILD_SHARED_LIBS)
      set(WSF_OUTPUT_NAME "${WSF_OUTPUT_NAME}_x")
   endif()
   if(NOT WIN32)
      set(WSF_OUTPUT_NAME "${WSF_OUTPUT_NAME}.exe")
   endif()
   set_target_properties(${WSF_PROJECT_NAME} PROPERTIES OUTPUT_NAME "${WSF_OUTPUT_NAME}")
endmacro()

# Add Microsoft redistributable runtime libs for Windows
MACRO(swdev_install_c_runtime)
   if(WIN32)
      set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION ${INSTALL_DLL_PATH})
      include(InstallRequiredSystemLibraries)
   endif(WIN32)
ENDMACRO()

# Sets the compiler warning level for a project
MACRO(swdev_warning_level)
   IF(WIN32)
      #  C4266 (level 4) 'function': no override available for virtual member function from base 'type'; function is hidden
      # ADD_DEFINITIONS(/W3 /w34706 /w34264 /w34266)
      ADD_DEFINITIONS(/W3)
   ELSE()
      ADD_DEFINITIONS(-Wall)
   ENDIF()
ENDMACRO()

# Are we compiling 64 or 32 bit
# usage:   swdev_is_64bit(is64)
#          if (is64) ...
MACRO(swdev_is_64bit OUTVAR)
   SET(${OUTVAR} FALSE)
   IF (CMAKE_CL_64)
      SET(${OUTVAR} TRUE)
   ELSE()
      IF(CMAKE_SIZEOF_VOID_P EQUAL 8)
         STRING(FIND "${CMAKE_EXE_LINKER_FLAGS}" "-m32" IS_32_BIT)
         IF ("${IS_32_BIT}" STREQUAL "-1")
            SET(${OUTVAR} TRUE)
         ENDIF()
      ENDIF()
   ENDIF()
ENDMACRO()

# This must be in the top-level CMake file in order for CMake's unit
# testing facilities to be used; e.g. add_test(...).
# See: http://cmake.org/Wiki/CMake/Testing_With_CTest
enable_testing()

