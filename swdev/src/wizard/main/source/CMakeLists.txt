# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************
# *****************************************************************************
# AFSIM Wizard CMake Configuration
#
# This file creates the wizard executable. It is expect to be invoked by the
# wsf_package cmake.
# *****************************************************************************

if (MSVC)
   add_compile_options(-wd4251 -wd4286 -wd4275)
endif()

# *****************************************************************************
# User-defined macros
include(swdev_project)

# *****************************************************************************
# Sets a CMake, cache or environment variables

# If the CMAKE_INSTALL_PREFIX has not been specified by the user, set default.
# If any sub-projects use this check they will need to verify that CMAKE_INSTALL_PREFIX
# has not been set to the 'wsf_install' path
# Note: CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT is a valid variable
# but the CMake developers consider it 'internal.' It is not offically
# documented and cannot be directly set.
IF (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT AND NOT (CMAKE_INSTALL_PREFIX MATCHES "wsf_install?"))
  SET(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/wizard_install CACHE PATH "Installation path" FORCE)
ENDIF()

SET(GENIO_WITHOUT_UMP TRUE)            # We don't want UMP

# *****************************************************************************
# Configure the 3rd_party directory
# Set the shared library "paths" before calling "add_executable"
# The actual lib names can just be added, in the modules cmake file,
# using 'target_link_libraries'

if (COMMAND set_3rd_party_shared_paths)
   set_3rd_party_shared_paths()
endif()

# *****************************************************************************

# Get the main
file(GLOB SRCS *.?pp)

# The executable we are trying to build and Set the WIN32_EXECUTABLE property
add_executable(${PROJECT_NAME} WIN32 ${SRCS})
configure_qt_conf(${PROJECT_NAME})
configure_bash_launcher(${PROJECT_NAME})

swdev_warning_level(${PROJECT_NAME})

# *****************************************************************************
# Libraries

target_link_libraries(${PROJECT_NAME} wizard_core)

# *****************************************************************************
# Configure version header and resource files
include(GenerateVersionInfo)
generate_version_header(${PROJECT_NAME})
if(MSVC)
   generate_resource(${PROJECT_NAME} "../../ui/resources/icons/wizard_icon.ico")
endif()

# Required in order to run in debug from Visual Studio
if (MSVC)
   include(user_file_config)
   create_vs_debug_env("${WIZARD_ROOT}"
                       "${TEMPLATE_DIR}/Template.VisualStudio.Settings.user"
                      )
endif()

# *****************************************************************************
# Installation

set_target_properties(${PROJECT_NAME} PROPERTIES INSTALL_RPATH "$ORIGIN/lib")
set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER "applications")

# Add to start menu for installers
if(COMMAND add_to_start_menu)
   add_to_start_menu(${PROJECT_NAME} Wizard)
endif()

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${INSTALL_EXE_PATH} COMPONENT Runtime)

# Install required qt core libs
install_qt_libs_all()
install_qt_plugins(${INSTALL_EXE_PATH}/qt_plugins)
install_qt_licenses()

# Install third party libs
install_third_party(${GDALROOT})
install_third_party(${GEOSROOT})
install_third_party(${PROJROOT})
install_osg(${OSGROOT})
install_osgearth(${OSGEARTHROOT})
if (WIN32)
   install_third_party(${TIFFROOT})
endif()

# Install other resources
install(DIRECTORY ../../data DESTINATION resources COMPONENT Resource)
install(FILES ${WIZARD_RCC} DESTINATION resources/rcc COMPONENT Resource)

include(${TOOLS_DIRECTORY}/vespatk/cmake/install_resources.cmake)
swdev_install_resources(.)

# Source install
install_sources_all_files(../ wizard/main)

# Add source directories to doxygen input
#add_wsf_doxygen_input(${SRCS}) TODO Use this instead? Adds all source files
add_wsf_doxygen_input(${CMAKE_CURRENT_SOURCE_DIR})

add_wsf_doc_input(${CMAKE_CURRENT_SOURCE_DIR}/..)
add_wsf_doc_sidebar_ext(${CMAKE_CURRENT_SOURCE_DIR}/../wsftheme/wsf_sidebar_wizard.html)
