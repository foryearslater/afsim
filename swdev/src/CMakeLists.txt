# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************
# ****************************************************************************
# Updated by Infoscitex, a DCS Company.
# ****************************************************************************

#
# AFSIM CMake Configuration
#
# This is the top-level CMake file. It provides options to the user
# for configuring and creating a AFSIM build. This file makes use of
# swdev_project.cmake and optional_projects.cmake files to support
# the build.

# Print version of CMake
message(STATUS "CMAKE_VERSION: " ${CMAKE_VERSION})
# Set minimum version required. Also set policy_max version, for which known policies will be set to NEW.
# Later versions of CMake are usually compatible but policies added after the policy_max version will be unset.
cmake_minimum_required(VERSION 3.7...3.12)
project(afsim)

if(CMAKE_SIZEOF_VOID_P EQUAL 4)
   message(WARNING "AFSIM has officially deprecated the 32-bit compilation option starting in version 2.6. Not all AFSIM features are 32-bit compatible.")
endif()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMakeTargets")
set_property(GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS TRUE)

option(PROMOTE_HARDWARE_EXCEPTIONS "Enable promoting hardware exceptions to a ut::HardwareException." TRUE)
mark_as_advanced(PROMOTE_HARDWARE_EXCEPTIONS)

# CMake command line/GUI options for controlling what parts of AFSIM are built and installed
option(WSF_PLUGIN_BUILD          "Enable plug-ins" TRUE) # Default is shared/plug-in build
option(WSF_INSTALL_BASE_GRAMMAR  "Install AFSIM base grammar files" TRUE)  # Set to true for now since IDE requires this
option(WSF_INSTALL_SOURCE        "Install AFSIM source files" FALSE)
option(WSF_INSTALL_DEMOS         "Install AFSIM demos files for release" FALSE)
option(WSF_INSTALL_SCENARIOS     "Install AFSIM scenarios files for release" FALSE)
option(WSF_INSTALL_TOOLS         "Install AFSIM tools files for release" FALSE)
option(WSF_INSTALL_TRAINING      "Install AFSIM training files for release" FALSE)
option(WSF_INSTALL_DOXYGEN       "Install Doxygen documentation" FALSE)
option(WSF_INSTALL_DOCUMENTATION "Install documentation" FALSE)
option(WSF_INSTALL_DEPENDENCIES  "Install dependencies, e.g. MSVC Runtimes" FALSE)

# External options
option(WSF_BUILD_API             "Build AFSIM API" FALSE)

set(WSF_ADD_EXTENSION_PATH "" CACHE PATH "Semicolon separated list of additional search paths for AFSIM extensions and plugins")
set(GENIO_WITHOUT_UMP TRUE)

# Extend the CMake Module path to look for extra CMake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/Modules")

if(NOT DEFINED ENABLE_EXTENSION_DEFAULT)
   set(ENABLE_EXTENSION_DEFAULT TRUE CACHE INTERNAL "Extensions built by default (when true)")
endif()

# Setup versioning information
set(AFSIM_VERSION_MAJOR   "2")
set(AFSIM_VERSION_MINOR   "9")
set(AFSIM_VERSION_PATCH   "0")
set(AFSIM_VERSION_BUILD   "0")
set(AFSIM_VERSION         "${AFSIM_VERSION_MAJOR}.${AFSIM_VERSION_MINOR}.${AFSIM_VERSION_PATCH}")
set(AFSIM_RELEASE_DATE    "02-25-2022")
set(AFSIM_RELEASE_VERSION "${AFSIM_VERSION} ${AFSIM_RELEASE_DATE}")

set(AFSIM_COMPANYNAME     "United States Air Force (USAF)")
set(AFSIM_LEGALCOPYRIGHT  "Multiple")
set(AFSIM_LEGALTRADEMARKS "All Rights Reserved")
set(AFSIM_COMPANYDOMAIN   "https://airforce.com")

find_package(Git)
if(Git_FOUND AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git")
   # If building a release tag, there is nothing to do. Version will indicate major.minor.patch
   execute_process(COMMAND ${GIT_EXECUTABLE} describe --tags --always --match "v${AFSIM_VERSION}"
                   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                   OUTPUT_VARIABLE AFSIM_GIT_TAG
                   OUTPUT_STRIP_TRAILING_WHITESPACE)
   if (NOT AFSIM_GIT_TAG STREQUAL "v${AFSIM_VERSION}")
      # Get the current working branch
      execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
                      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                      OUTPUT_VARIABLE AFSIM_GIT_BRANCH
                      OUTPUT_STRIP_TRAILING_WHITESPACE)
      if(NOT AFSIM_GIT_BRANCH MATCHES "^release\_.*")
         execute_process(COMMAND ${GIT_EXECUTABLE} log -1 --format=%cd --date=short
                         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                         OUTPUT_VARIABLE AFSIM_GIT_COMMIT_DATE
                         OUTPUT_STRIP_TRAILING_WHITESPACE)
         # Not building a release branch. Reflect date of last commit in version.
         # Compat: could just be "git-log --date=format:%y%m%d", but it's not available until git 2.6.
         string(REPLACE "-" "" AFSIM_VERSION_BUILD "${AFSIM_GIT_COMMIT_DATE}")
         string(SUBSTRING ${AFSIM_VERSION_BUILD} 2 -1 AFSIM_VERSION_BUILD)
         set(AFSIM_VERSION "${AFSIM_VERSION}.${AFSIM_VERSION_BUILD}")
         set(AFSIM_RELEASE_VERSION "${AFSIM_VERSION} ${AFSIM_GIT_COMMIT_DATE}")
      else()
         # Building a release candidate, suffix version with -rc
         set(AFSIM_VERSION "${AFSIM_VERSION}-rc")
         set(AFSIM_RELEASE_VERSION "${AFSIM_VERSION} ${AFSIM_RELEASE_DATE}")
      endif()
   endif()
   
   # Set the git hooks directory to "afsim/.githooks/" 
   # The /.githooks directory contains custom, git hooks such as the clang-format pre-commit hook
   set(AFSIM_GIT_HOOKS_DIR ".githooks" CACHE STRING "")
   mark_as_advanced(AFSIM_GIT_HOOKS_DIR)
   execute_process(COMMAND ${GIT_EXECUTABLE} config core.hooksPath ${AFSIM_GIT_HOOKS_DIR}
                   WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})
endif()

# Add AFSIM_VERSION to internal cache, so it can be accessed outside of CMake, e.g. in a CI environment
set(AFSIM_VERSION ${AFSIM_VERSION} CACHE INTERNAL "AFSIM Version")
message(STATUS "AFSIM Version: ${AFSIM_VERSION}")
message(STATUS "AFSIM Release Version: ${AFSIM_RELEASE_VERSION}")

# Allow our projects to organize themselves in folders for declutter
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

###############################################################
# Locations Setup
# Find files/directories/extensions that are part of the build.
###############################################################

# Locate all 'wsf_module' files. These files are used to identify a directory
# as containing an optional AFSIM extension. From that optional module directory
# locate any wsf_cmake_extension files that will be used to configure the
# inclusion of the extension. The MODULE_ROOT variable accounts for the AFSIM
# code being in a flat or nested structure.
SET(MODULE_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/*/wsf_module)

# Hard setting the shared folder name(s) for now to include extra projects and plug-ins.
get_filename_component(SOURCE_PARENT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/.." REALPATH)
SET(MODULE_ROOT "${MODULE_ROOT}" "${SOURCE_PARENT_DIR}/afsim_*shared/*/wsf_module")
SET(MODULE_ROOT "${MODULE_ROOT}" "${SOURCE_PARENT_DIR}/afsim_s*t/*/wsf_module")

if(WSF_ADD_EXTENSION_PATH)
   foreach(EXTENSION_PATH ${WSF_ADD_EXTENSION_PATH})
      if(IS_DIRECTORY "${EXTENSION_PATH}")
         message(STATUS "Additional extension path: ${EXTENSION_PATH}")
         SET(MODULE_ROOT "${MODULE_ROOT}" "${EXTENSION_PATH}/wsf_module")
      endif()
   endforeach()
endif()
file(GLOB MODULE_FILES ${MODULE_ROOT})

# Clear the WSF_EXTENSION_FILES variable and use the MODULE_FILES locations
# to find all extensions
set(WSF_EXTENSION_FILES)
foreach(MODULEFILE ${MODULE_FILES})
   get_filename_component(MODULE_DIR "${MODULEFILE}" PATH)
   file(GLOB_RECURSE MODULE_EXTENSION_FILES
        ${MODULE_DIR}/wsf_cmake_extension.cmake)
   set(WSF_EXTENSION_FILES ${WSF_EXTENSION_FILES} ${MODULE_EXTENSION_FILES})
endforeach()

# Store the full path of the directory that contains the core wsf directories
# (wsf, wsf_util) in the WSF_ROOT variable that will be used in this and other CMake files
set(WSF_ROOT ${PROJECT_SOURCE_DIR}/core)

# Set variable for 'tools' directory that will be used in this and other CMake files
set(TOOLS_DIRECTORY ${PROJECT_SOURCE_DIR}/tools)

# Include support for Cppcheck output
# If found the CPPCHECK target will be generated for building an xml output file.
include(Cppcheck)

# Include support for processing Doxygen for AFSIM. This file searches for
# Doxygen on the build system. If it is found an optional DOXYGEN target
# will be available to build that will generate Doxygen documentation.
include(wsf_doxygen)

# Include support for processing documentation for AFSIM. This file searches for
# Sphinx on the build system. If it is found an optional DOCUMENTATION target
# will be available to build that will generate documentation.
include(wsf_documentation)
add_wsf_doc_input(${CMAKE_CURRENT_SOURCE_DIR}/doc)

###########################################################
# Install Configuration
# Configures where executables, source, and data (examples)
# will be placed when an 'install' command is run.
###########################################################

# put dll's in the same place as exe for windows (required for test targets in shared builds)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)

if(WSF_INSTALL_SOURCE)
   set(INSTALL_SOURCE_ROOT swdev/src)
endif()

# If the CMAKE_INSTALL_PREFIX has not been specified by the user, set default.
# If any sub-projects use this check they will need to verify that CMAKE_INSTALL_PREFIX
# has not been set to the 'wsf_install' path
# Note: CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT is a valid variable
# but the CMake developers consider it 'internal.' It is not officially
# documented and cannot be directly set.
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
   set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/wsf_install CACHE PATH "Installation path" FORCE)
endif()

if(WIN32)
   set(INSTALL_DLL_PATH bin/${INSTALL_DLL_PATH})
else()
   set(INSTALL_DLL_PATH bin/lib/${INSTALL_DLL_PATH})
endif()

set(INSTALL_EXE_PATH bin)
# Changing INSTALL_PLUGIN_PATH to not match INSTALL_EXE_PATH is not currently supported
# and will likely lead to failures at execution and/or plugin loading.
set(INSTALL_PLUGIN_PATH "${INSTALL_EXE_PATH}")

# Data install file, controlled by 'WSF_INSTALL_<DATA-TYPE>' option
include(wsf_data_install)

# Test hook, controlled by '<application-name>_AUTO_TEST' targets
include(wsf_auto_test)

# Test hook, controlled by '<application-name>_REGRESSION_TEST' targets
include(wsf_regression_test)

###########################################################
# Build Configuration
###########################################################

if(NOT DEFINED AFSIM_BUILD_CONFIG_DEFAULT)
   set(AFSIM_BUILD_CONFIG_DEFAULT Release)
endif()
get_property(AFSIM_GENERATOR_IS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
# The GENERATOR_IS_MULTI_CONFIG property was added in CMake 3.9, also directly check MSVC
# and XCODE for compatibility with prior versions.
if(NOT AFSIM_GENERATOR_IS_MULTI_CONFIG AND NOT MSVC AND NOT XCODE)
   # Single-configuration generators must specify build type, use default if unspecified
   if(NOT CMAKE_BUILD_TYPE)
      message("Defaulting CMAKE_BUILD_TYPE to ${AFSIM_BUILD_CONFIG_DEFAULT}")
      set(CMAKE_BUILD_TYPE "${AFSIM_BUILD_CONFIG_DEFAULT}" CACHE STRING "Build type" FORCE)
   endif()
else()
   # For multi-config generators with the default build configuration type included,
   # ensure that it is the first, and thereby default, configuration type.
   if("${AFSIM_BUILD_CONFIG_DEFAULT}" IN_LIST CMAKE_CONFIGURATION_TYPES)
      list(REMOVE_ITEM CMAKE_CONFIGURATION_TYPES "${AFSIM_BUILD_CONFIG_DEFAULT}")
      list(INSERT CMAKE_CONFIGURATION_TYPES 0 "${AFSIM_BUILD_CONFIG_DEFAULT}")
      set(CMAKE_CONFIGURATION_TYPES "${CMAKE_CONFIGURATION_TYPES}" CACHE STRING "Build types" FORCE)
   endif()
endif()

if(CMAKE_UNITY_BUILD)
  # By default ensure code *compiles* with all files combined. This ensures CI
  # jobs or developers can tailor the value as needed for build performance.
  if(NOT CMAKE_UNITY_BUILD_BATCH_SIZE)
    set(CMAKE_UNITY_BUILD_BATCH_SIZE 0)
  endif()
  if(MSVC)
    # Allow > 65279 symbols in a single .obj file
    add_compile_options(/bigobj)
  endif()
endif()

# If we are doing a shared/plug-in/DLL build set those options
# otherwise set options for a static build
if(WSF_PLUGIN_BUILD)
   set(BUILD_SHARED_LIBS TRUE)
   set(INSTALL_LIB_PATH "bin/lib")
   if (NOT WIN32)
      ADD_DEFINITIONS("-fPIC")
   endif()
else()
   if(MSVC)
      # For some reason, we must remove dll compile flag for the c compiler
      set(CompilerFlags
              CMAKE_C_FLAGS
              CMAKE_C_FLAGS_DEBUG
              CMAKE_C_FLAGS_MINSIZEREL
              CMAKE_C_FLAGS_RELEASE
              CMAKE_C_FLAGS_RELWITHDEBINFO
              CMAKE_CXX_FLAGS
              CMAKE_CXX_FLAGS_RELEASE
              CMAKE_CXX_FLAGS_MINSIZEREL
              CMAKE_CXX_FLAGS_RELWITHDEBINFO
              CMAKE_CXX_FLAGS_DEBUG
          )
      foreach(CompilerFlag ${CompilerFlags})
         string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
      endforeach()
   else()
      set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
      set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc")
   endif()
endif()

if(MSVC)
   # Set Stack size from default 1MB in Windows. If this value
   # is too large it can limit the number of threads we can spawn.
   # Default value used by Windows is 1MB which is not good enough either,
   # so set a value of 2MB for now until more issues arise.
   set(CMAKE_CXX_STACK_SIZE "2000000")

   # enable msvc conformance flag (for cross compilation compatibility with gcc) to
   # disable being able to pass an rvalue to a non-const reference parameter.
   add_compile_options(/Zc:referenceBinding)
else()
   # Offer usage of ccache if the program is available.
   find_program(CCACHE_EXECUTABLE ccache)
   if(CCACHE_EXECUTABLE)
      option(AFSIM_USE_CCACHE "Use ccache to accelerate rebuilds" OFF)
      if(AFSIM_USE_CCACHE)
         set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_EXECUTABLE}")
         set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_EXECUTABLE}")
      endif()
   endif()
endif()

# Use -O3 instead of -O2 for release
if(NOT WIN32)
   string(REPLACE "-O2" "-O3" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
   # Since 3rd_party sources are built with GCC 4.8, the default ABI is not the CXX11 ABI
   # that is default in GCC >= 5. Newer compilers will fail to build without this line
   # unless we rebuild 3rd_party with newer versions of GCC
   add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)
   # Add linker flags to export function names
   set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} "-rdynamic -export-dynamic")
   set(CMAKE_SHARED_LINKER_FLAGS ${CMAKE_SHARED_LINKER_FLAGS} "-rdynamic -export-dynamic")
   set(CMAKE_MODULE_LINKER_FLAGS ${CMAKE_MODULE_LINKER_FLAGS} "-rdynamic -export-dynamic")
endif()

# Include macros and configuration that will support the build.
# This include must be here as it depends on things defined above
# in the Install/Build Configuration sections and is used below
include(swdev_project)

# Include macro for configuring wsf projects. macro automatically
# sets up the build, install, grammar, sphinx, and doxygen logic for
# a project based on an expected project folder layout.
include(wsf_project_template)

# Set this here to avoid relative paths scattered everywhere for it
set(WSF_EXTENSION_HEADER_DIR ${PROJECT_BINARY_DIR}/include)

# Sets library names
use_swdev_lib_suffixes()   # use _d_ln4 etc...

###########################################################
# Project Source
# Add source files to the project for building
###########################################################

# Configure unit testing framework.
# If found the optional unit test project will be available and built.
include(ConfigureUnitTests)

# Add 'core' AFSIM directories to build
# May need find_path here in future
add_subdirectory(${TOOLS_DIRECTORY}/misc misc)  # For install only, no lib or exe
add_subdirectory(${TOOLS_DIRECTORY}/util util)
add_subdirectory(${TOOLS_DIRECTORY}/util_script util_script)
add_subdirectory(${TOOLS_DIRECTORY}/geodata geodata)
add_subdirectory(${TOOLS_DIRECTORY}/packetio/source packetio)
add_subdirectory(${TOOLS_DIRECTORY}/dis/source dis)
add_subdirectory(${TOOLS_DIRECTORY}/tracking_filters/source tracking_filters)
add_subdirectory(${TOOLS_DIRECTORY}/genio/source genio)
add_subdirectory(${TOOLS_DIRECTORY}/scene_gen scene_gen)
add_subdirectory(${TOOLS_DIRECTORY}/profiling profiling)
add_subdirectory(${TOOLS_DIRECTORY}/artificer artificer)
set(TOOLS_LIBS util util_script geodata packetio dis tracking_filters genio profiling)

# Keep wsf/source before optional projects. It defines some global and
# parent scope properties used by other projects
add_subdirectory(${WSF_ROOT}/wsf_util/source wsf_util)
add_subdirectory(${WSF_ROOT}/wsf wsf)

# Add documentation sidebar extension for demos if found. Putting this here places
# the demos after core/wsf but before any optional extensions in the sidebar.
if(IS_DIRECTORY "${WSF_DEMOS_ROOT}" AND WSF_INSTALL_DEMOS)
   add_wsf_doc_sidebar_ext(${WSF_DEMOS_ROOT}/doc/wsftheme/wsf_sidebar_demos.html)
endif()

# Incorporate optional extensions identified above in WSF_EXTENSION_FILES.
# The include file adds 'BUILD_WITH_xxxxx' options to CMake and the macro
# use_wsf_optional_projects() called from the include file adds the
# extensions to the code
include(${WSF_ROOT}/wsf/optional_projects.cmake)
use_wsf_optional_projects()

###########################################################
# Grammar and Documentation
# Rules for generating/installing AFSIM grammar files and
# developer or user documentation.
###########################################################

# Install AFSIM grammar files with executables. This must be done after
# all optional projects are added so all grammar files are located
if(WSF_INSTALL_BASE_GRAMMAR)
   get_property(WSF_GRAMMAR_FILES GLOBAL PROPERTY WSF_GRAMMAR_FILES)
   install(FILES ${WSF_GRAMMAR_FILES} DESTINATION bin/grammar COMPONENT Runtime)
endif()

# Create the doxyfile, included from wsf_doxygen.cmake.
# This is called after all subdirectories and optional projects are added
# so they can register source locations used for generation of documentation
create_doxyfile()

# Install Doxygen output, included from wsf_doxygen.cmake.
# For there to be Doxygen documentation to install the build system must
# have Doxygen installed and the optional DOXYGEN target needs to be built
install_doxygen()

# Create the docfile for Sphinx, included from wsf_documentation.cmake.
# This is called after all subdirectories and optional projects are added
# so they can register source locations used for generation of documentation
create_docfile()

# Install documentation output, included from wsf_documentation.cmake.
# For there to be documentation to install the build system must
# have Sphinx installed and the optional DOCUMENTATION target needs to be built
install_documentation()

# Install the package directory that contains this CMake file
install_sources_all_files(${CMAKE_CURRENT_SOURCE_DIR}/cmake .)
install_sources_all_files(${CMAKE_CURRENT_SOURCE_DIR}/doc .)
install_source_files(${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
                     ${CMAKE_CURRENT_SOURCE_DIR}/CMakeSettings.json
                     ${CMAKE_CURRENT_SOURCE_DIR}/CMakePresets.json
                     ${CMAKE_CURRENT_SOURCE_DIR}/CONTRIBUTING.md
                     ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.md
                     ${CMAKE_CURRENT_SOURCE_DIR}/README.md
                     ${CMAKE_CURRENT_SOURCE_DIR}/requirements.txt
                     .
                    )

# Package install files, controlled by 'WSF_PACKAGE_INSTALL' option
include(wsf_package)

###########################################################
# Custom Install
# Rules for installing the runtime components only.
# Reduces install times when other WSF_INSTALL_* options are enabled.
###########################################################
add_custom_target(INSTALL_RUNTIME_ONLY
                  ${CMAKE_COMMAND}
                  -D "CMAKE_INSTALL_CONFIG_NAME=$<CONFIG>"
                  -D "CMAKE_INSTALL_COMPONENT=Runtime"
                  -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_install.cmake")

set_property(TARGET INSTALL_RUNTIME_ONLY PROPERTY FOLDER CMakeTargets)
