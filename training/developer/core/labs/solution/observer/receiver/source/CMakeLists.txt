# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************

# Setting this policy allows the use of relative path names
# without CMake warnings.
cmake_policy(SET CMP0015 OLD)

#Prevent warnings from use of standard c libraries.
add_definitions("-D_CRT_SECURE_NO_WARNINGS")

# Set options to find the plugin (standard AFSIM) build location, and the AFSIM source location
set(AFSIM_SOURCE_LOCATION "../../../../../" CACHE STRING "Path to 'swdev' AFSIM libraries" FORCE)

# Set the directory where we intend to install our targets
set (CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/../../bin CACHE PATH "Installation path")

# Specify the name of this project.
project(receiver)

# Add all files into the project to be compiled.
FILE(GLOB SRCS *.cpp *.hpp)

# Add other projects needed for this version.
add_subdirectory(${AFSIM_SOURCE_LOCATION}/genio/source genio)
add_subdirectory(${AFSIM_SOURCE_LOCATION}/util/source util)

# Specify include directories for the c++ code.
include_directories(${AFSIM_SOURCE_LOCATION}/genio/source
                    ${AFSIM_SOURCE_LOCATION}/util/source)

# Our target is an executable
add_executable(${PROJECT_NAME} ${SRCS})

# Link with all standard AFSIM libraries.
target_link_libraries(${PROJECT_NAME} util genio)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})
