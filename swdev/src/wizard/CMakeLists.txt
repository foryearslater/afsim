# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************
# *****************************************************************************
# CMAKE file: wizard
# *****************************************************************************
IF (NOT WSF_PLUGIN_BUILD)
   message(FATAL_ERROR "WSF_PLUGIN_BUILD must be set to true to build the wizard project.")
ENDIF()

project(wizard)

# *****************************************************************************
# Sets a CMake, cache or environment variables

# Set the IDE root directory
get_filename_component(WIZARD_ROOT ${CMAKE_CURRENT_SOURCE_DIR} ABSOLUTE)

set(BUILD_SHARED_LIBS TRUE) # Must set before including swdev_project.cmake
set(SWDEV_USE_NULLPTR TRUE) # define nullptr if not available

# Set the install path to the resource directory
set(WIZARD_RSC_INSTALL_DIR "${INSTALL_EXE_PATH}/../resources")

# Set the path where binary resources will be built and create the directory
set(WIZARD_RCC_DIR ${CMAKE_BINARY_DIR}/rcc)
file(MAKE_DIRECTORY ${WIZARD_RCC_DIR})

# *****************************************************************************
# Configure the 3rd_party directory

set(SWDEV_GDAL_PACKAGE      "gdal-3.3.2"      CACHE STRING "" FORCE)
set(SWDEV_GEOS_PACKAGE      "geos-3.5.1"      CACHE STRING "" FORCE)
set(SWDEV_OSG_EARTH_PACKAGE "osgEarth-2.10.1" CACHE STRING "" FORCE)
set(SWDEV_OSG_PACKAGE       "osg-3.6.3"       CACHE STRING "" FORCE)
set(SWDEV_QT_PACKAGE        "qt-5.12.11"       CACHE STRING "" FORCE)
set(SWDEV_SQLITE_PACKAGE    "sqlite-3.32.3"   CACHE STRING "" FORCE)
set(SWDEV_TIFF_PACKAGE      "tiff-4.3.0"      CACHE STRING "" FORCE)
set(SWDEV_PROJ_PACKAGE      "proj-8.1.1"      CACHE STRING "" FORCE)

mark_as_advanced(FORCE SWDEV_GDAL_PACKAGE)
mark_as_advanced(FORCE SWDEV_GEOS_PACKAGE)
mark_as_advanced(FORCE SWDEV_OSG_EARTH_PACKAGE)
mark_as_advanced(FORCE SWDEV_OSG_PACKAGE)
mark_as_advanced(FORCE SWDEV_QT_PACKAGE)
mark_as_advanced(FORCE SWDEV_SQLITE_PACKAGE)
mark_as_advanced(FORCE SWDEV_TIFF_PACKAGE)
mark_as_advanced(FORCE SWDEV_PROJ_PACKAGE)

# Download the packages
# Ref: 3rd_party-cmake/config.cmake
swdev_acquire_packages(${WIZARD_ROOT}
                       ${SWDEV_GDAL_PACKAGE}
                       ${SWDEV_GEOS_PACKAGE}
                       ${SWDEV_OSG_EARTH_PACKAGE}
                       ${SWDEV_OSG_PACKAGE}
                       ${SWDEV_QT_PACKAGE}
                       ${SWDEV_SQLITE_PACKAGE}
                       ${SWDEV_PROJ_PACKAGE})

if (WIN32)
   swdev_acquire_packages(${WIZARD_ROOT} ${SWDEV_TIFF_PACKAGE})
endif()

# *****************************************************************************
# User-defined macros - Order is important in this section
include(swdev_project)
include(${TOOLS_DIRECTORY}/vespatk/cmake/config.cmake)       # For WIN32 Debug

# *****************************************************************************
# Includes

include_directories(${CMAKE_BINARY_DIR}/include ../wizard/lib/source)

# *****************************************************************************
# Organize the project - adding subdirectories to the build
try_add_subdirectory(${TOOLS_DIRECTORY}/utilosg/source utilosg)
try_add_subdirectory(${TOOLS_DIRECTORY}/utilqt/source utilqt)
try_add_subdirectory(${TOOLS_DIRECTORY}/vespatk/source vespatk)
try_add_subdirectory(${TOOLS_DIRECTORY}/vespatk/vespatk_qt/source vespatk_qt)
try_add_subdirectory(${TOOLS_DIRECTORY}/wkf/core/source wkf)
try_add_subdirectory(${TOOLS_DIRECTORY}/wkf/common/source wkf_common)
add_subdirectory(${WIZARD_ROOT}/lib/source wizard_core)
add_subdirectory(${WIZARD_ROOT}/usmtf usmtf) 
#add_subdirectory(test)

# *****************************************************************************
# WIZARD plugins

# plugins
set(PLUGIN_ROOT_DIRS "${WIZARD_ROOT}/plugins"
                     "${CMAKE_SOURCE_DIR}/../afsim_*shared"
                     "${WSF_ADD_EXTENSION_PATH}")
set(PLUGIN_INSTALL_DIR "${INSTALL_PLUGIN_PATH}/wizard_plugins")
include(vis_plugins)
use_vis_plugins(WIZARD)

add_subdirectory(${WIZARD_ROOT}/main/source wizard) # Build executable

# *****************************************************************************
# Installation

# install 3rd_party build files that are required to build the IDE not covered elsewhere
#install_sources_all_files(${CMAKE_THIRD_PARTY_ROOT} wizard)

# Source, data grammar and test install
install_sources_all_files(data wizard)
install_sources_all_files(grammar wizard)
install_sources_all_files(setup wizard)
install_sources_all_files(test wizard)
install_sources_all_files(tests wizard)

# Install a few files that are required to build WSF but not covered elsewhere
install_source_files(README.md wsf_cmake_extension.cmake CMakeLists.txt wsf_module wizard)

# install 3rd_party-cmake files that are required to build the IDE not covered elsewhere
install_source_files(${TOOLS_DIRECTORY}/3rd_party-cmake/config.cmake
                     ${TOOLS_DIRECTORY}/3rd_party-cmake/shared.cmake
                     ${TOOLS_DIRECTORY}/3rd_party-cmake/${SWDEV_GDAL_PACKAGE}.cmake
                     ${TOOLS_DIRECTORY}/3rd_party-cmake/${SWDEV_GEOS_PACKAGE}.cmake
                     ${TOOLS_DIRECTORY}/3rd_party-cmake/${SWDEV_OSG_EARTH_PACKAGE}.cmake
                     ${TOOLS_DIRECTORY}/3rd_party-cmake/${SWDEV_OSG_PACKAGE}.cmake
                     ${TOOLS_DIRECTORY}/3rd_party-cmake/${SWDEV_QT_PACKAGE}.cmake
                     ${TOOLS_DIRECTORY}/3rd_party-cmake/${SWDEV_SQLITE_PACKAGE}.cmake
                     ${TOOLS_DIRECTORY}/3rd_party-cmake/${SWDEV_TIFF_PACKAGE}.cmake
                     ${TOOLS_DIRECTORY}/3rd_party-cmake/${SWDEV_PROJ_PACKAGE}.cmake
                     tools/3rd_party-cmake)
