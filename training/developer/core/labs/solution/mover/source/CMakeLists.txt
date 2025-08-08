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

# Outside includes. swdev_project.cmake contains many useful macros.
include(GenerateExportHeader)
include(swdev_project)

# Add all files into the project to be compiled.
FILE(GLOB SRCS *.cpp *.hpp)

wsf_grammar_file(SRCS "${CMAKE_CURRENT_SOURCE_DIR}/../grammar/mover_exercise.ag")

# Set option to find the mcr header file location
set(MCR_INCLUDE_LOCATION "C:/Program Files/MATLAB/MATLAB Runtime/v91/extern/include" CACHE STRING "Location of mcr header files")

# Set option to find the mcr header file location
set(MCR_LIBRARY_LOCATION "C:/Program Files/MATLAB/MATLAB Runtime/v91/extern/lib/win64/microsoft" CACHE STRING "Location of mcr header files")

# Add the non-cpp matlab header in its own filter/folder
## ../MATLAB_mover/lib/libAFSIM_Mover.h
set(SUB ../MATLAB_mover/lib)
FILE(GLOB SUB_SRCS ${SUB}/*.h)
set(SRCS ${SRCS} ${SUB_SRCS})
source_group(${SUB} FILES ${SUB_SRCS})

# Add search paths for linking in MATLAB libs.
link_directories(${WSF_PLUGIN_BUILD_LOCATION}/libs/${CMAKE_BUILD_TYPE}
                 ${CMAKE_CURRENT_SOURCE_DIR}/../MATLAB_mover/lib
                 ${MCR_LIBRARY_LOCATION})

# Add the library with sources
add_library(${PROJECT_NAME} ${SRCS})

# Generate header with the __declspec export macro defined.
generate_export_header(${PROJECT_NAME})

# The includes for our export.
target_include_directories(${PROJECT_NAME} PUBLIC
                           "${CMAKE_CURRENT_SOURCE_DIR}"
                           "${PROJECT_BINARY_DIR}/source"
                           "${CMAKE_CURRENT_SOURCE_DIR}/../MATLAB_mover/lib"
                           "${MCR_INCLUDE_LOCATION}"
                           "${PROJECT_BINARY_DIR}"
                          )

# Link with required libs.
target_link_libraries(${PROJECT_NAME} ${WSF_LIBS} libAFSIM_Mover mclmcrrt)

# Set the project warning level.
swdev_warning_level(${PROJECT_NAME})

if(WSF_PLUGIN_BUILD)
   swdev_plugin_install(${PROJECT_NAME} wsf_plugins)
   if(WIN32)
      set(MOVER_RUNTIME ${CMAKE_CURRENT_SOURCE_DIR}/../data/libAFSIM_Mover.dll)
      add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                         COMMAND ${CMAKE_COMMAND} -E copy ${MOVER_RUNTIME} $<TARGET_FILE_DIR:${PROJECT_NAME}> VERBATIM)
      install(FILES ${MOVER_RUNTIME} DESTINATION ${INSTALL_DLL_PATH})
   endif()
else()
   swdev_lib_install(${PROJECT_NAME})
endif()
