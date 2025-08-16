# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************
# *****************************************************************************
# CMAKE file: warlock
# *****************************************************************************
project(warlock)

if (MSVC)
   add_compile_options(-wd4251 -wd4286 -wd4275)
endif()

# *****************************************************************************
# Get the source files
file(GLOB SRCS *.hpp *.cpp)

# *****************************************************************************
# Configure the 3rd_party libraries

set(SWDEV_GDAL_PACKAGE      "gdal-3.3.2"       CACHE STRING "" FORCE)
set(SWDEV_GEOS_PACKAGE      "geos-3.5.1"       CACHE STRING "" FORCE)
set(SWDEV_QT_PACKAGE        "qt-5.12.11"        CACHE STRING "" FORCE)
set(SWDEV_OSG_EARTH_PACKAGE "osgEarth-2.10.1"  CACHE STRING "" FORCE)
set(SWDEV_OSG_PACKAGE       "osg-3.6.3"        CACHE STRING "" FORCE)
set(SWDEV_TIFF_PACKAGE      "tiff-4.3.0"       CACHE STRING "" FORCE)
set(SWDEV_SDL_PACKAGE       "sdl-2.0.16"       CACHE STRING "" FORCE)
set(SWDEV_PROJ_PACKAGE      "proj-8.1.1"       CACHE STRING "" FORCE)

mark_as_advanced(FORCE SWDEV_GDAL_PACKAGE)
mark_as_advanced(FORCE SWDEV_GEOS_PACKAGE)
mark_as_advanced(FORCE SWDEV_SDL_PACKAGE)
mark_as_advanced(FORCE SWDEV_QT_PACKAGE)
mark_as_advanced(FORCE SWDEV_OSG_EARTH_PACKAGE)
mark_as_advanced(FORCE SWDEV_OSG_PACKAGE)
mark_as_advanced(FORCE SWDEV_TIFF_PACKAGE)
mark_as_advanced(FORCE SWDEV_PROJ_PACKAGE)

swdev_acquire_packages(${CMAKE_CURRENT_SOURCE_DIR}
                       ${SWDEV_GDAL_PACKAGE}
                       ${SWDEV_GEOS_PACKAGE}
                       ${SWDEV_SDL_PACKAGE}
                       ${SWDEV_QT_PACKAGE}
                       ${SWDEV_OSG_EARTH_PACKAGE}
                       ${SWDEV_OSG_PACKAGE}
                       ${SWDEV_PROJ_PACKAGE})

if (WIN32)
   swdev_acquire_packages(${CMAKE_CURRENT_SOURCE_DIR} ${SWDEV_TIFF_PACKAGE})
endif()

# Include macros and configuration to support builds of Qt projects
if (WIN32)
   include(qt_project)
   configure_qt_project(MODULES WinExtras)
endif()

# *****************************************************************************
# The executable we are trying to build
try_add_subdirectory(${TOOLS_DIRECTORY}/utilosg/source utilosg)
try_add_subdirectory(${TOOLS_DIRECTORY}/utilqt/source utilqt)
try_add_subdirectory(${TOOLS_DIRECTORY}/vespatk/source vespatk)
try_add_subdirectory(${TOOLS_DIRECTORY}/vespatk/vespatk_qt/source vespatk_qt)
try_add_subdirectory(${TOOLS_DIRECTORY}/wkf/core/source wkf)
try_add_subdirectory(${TOOLS_DIRECTORY}/wkf/common/source wkf_common)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../warlock_core/source warlock_core)

add_executable(${PROJECT_NAME} WIN32 ${SRCS})
swdev_warning_level(${PROJECT_NAME})
set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER "applications")

# plugins
set(PLUGIN_ROOT_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/../../plugins"
                     "${CMAKE_SOURCE_DIR}/../afsim_*shared"
                     "${WSF_ADD_EXTENSION_PATH}")
set(PLUGIN_INSTALL_DIR "${INSTALL_PLUGIN_PATH}/warlock_plugins")

include(vis_plugins)
use_vis_plugins(WARLOCK)

# *****************************************************************************
# Configure version header and resource files
include(GenerateVersionInfo)
generate_version_header(${PROJECT_NAME})
if(MSVC)
   generate_resource(${PROJECT_NAME} "warlock_icon.ico")
endif()

# for wsf_extensions.hpp
target_include_directories(${PROJECT_NAME} PRIVATE ${WSF_EXTENSION_HEADER_DIR})

# *****************************************************************************
# Libraries
target_link_libraries(${PROJECT_NAME} wsf warlock_core)
if (WIN32)
   target_link_libraries(${PROJECT_NAME} Qt5::WinExtras)
endif()
configure_qt_conf(${PROJECT_NAME})
configure_bash_launcher(${PROJECT_NAME})
link_wsf_optional_projects(${PROJECT_NAME})
set_target_properties(${PROJECT_NAME} PROPERTIES INSTALL_RPATH "$ORIGIN/lib")

# Required in order to run in debug from Visual Studio
if (MSVC)
   include(user_file_config)
   create_vs_debug_env(${CMAKE_CURRENT_SOURCE_DIR}
                       "${TEMPLATE_DIR}/Template.VisualStudio.Settings.user"
                      )
endif()

if(COMMAND add_to_start_menu)
   add_to_start_menu(${PROJECT_NAME} Warlock)
endif()

# *****************************************************************************
# Installation
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${INSTALL_EXE_PATH} COMPONENT Runtime)

# Install all WSF grammar to get "main" grammar file and any files defining
# commands that may be used by warlock but not included in warlock.ag
get_property(WSF_GRAMMAR_FILES GLOBAL PROPERTY WSF_GRAMMAR_FILES)
install(FILES ${WSF_GRAMMAR_FILES} DESTINATION bin/${PROJECT_NAME}_grammar COMPONENT Runtime)

# Install required qt core libs
install_qt_libs_all()
install_qt_plugins(${INSTALL_EXE_PATH}/qt_plugins)
install_qt_licenses()

# Install third party libs
install_third_party(${GDALROOT})
install_third_party(${GEOSROOT})
install_third_party(${OSGEARTHROOT})
install_third_party(${PROJROOT})
install_third_party(${SDL2ROOT})
install_osg(${OSGROOT})
install_osgearth(${OSGEARTHROOT})
if (WIN32)
   install_third_party(${TIFFROOT})
endif()

include(${TOOLS_DIRECTORY}/vespatk/cmake/install_resources.cmake)
swdev_install_resources(.)

install_sources_all_files(../ warlock/warlock_exec)
# install 3rd_party-cmake files that are required to build the Warlock not covered elsewhere
install_source_files(${TOOLS_DIRECTORY}/3rd_party-cmake/config.cmake
                     ${TOOLS_DIRECTORY}/3rd_party-cmake/shared.cmake
                     ${TOOLS_DIRECTORY}/3rd_party-cmake/${SWDEV_GDAL_PACKAGE}.cmake
                     ${TOOLS_DIRECTORY}/3rd_party-cmake/${SWDEV_GEOS_PACKAGE}.cmake
                     ${TOOLS_DIRECTORY}/3rd_party-cmake/${SWDEV_SDL_PACKAGE}.cmake
                     ${TOOLS_DIRECTORY}/3rd_party-cmake/${SWDEV_QT_PACKAGE}.cmake
                     ${TOOLS_DIRECTORY}/3rd_party-cmake/${SWDEV_OSG_EARTH_PACKAGE}.cmake
                     ${TOOLS_DIRECTORY}/3rd_party-cmake/${SWDEV_OSG_PACKAGE}.cmake
                     ${TOOLS_DIRECTORY}/3rd_party-cmake/${SWDEV_TIFF_PACKAGE}.cmake
                     ${TOOLS_DIRECTORY}/3rd_party-cmake/${SWDEV_PROJ_PACKAGE}.cmake
                     tools/3rd_party-cmake
                    )
