# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************


find_path(TEMPLATE_DIR Template.VisualStudio.Settings.user NO_SYSTEM_ENVIRONMENT_PATH HINTS
          ${CMAKE_CURRENT_LIST_DIR}/../*
          ${CMAKE_CURRENT_LIST_DIR}/../*/*
         )

# WARNING : If changes the number of arguments in this function, 
# you MUST update the "(${ARGC} GREATER 6)" checks within the macro too.
macro(write_vcproj_user 
    EXE_TARGET_DIRECTORY
    USER_FILE_TEMPLATE
    ADDITIONAL_PATH_DEBUG
    ADDITIONAL_PATH_RELEASE
    ADDITIONAL_ENVIRONMENT_DEBUG
    ADDITIONAL_ENVIRONMENT_RELEASE )
   if (WIN32)
      set(DEBUG_PATH "${ADDITIONAL_PATH_DEBUG}\;%PATH%")
      set(RELEASE_PATH "${ADDITIONAL_PATH_RELEASE}\;%PATH%")

      if ("${EXE_TARGET_DIRECTORY}" STREQUAL "")
         get_property(PKGS GLOBAL PROPERTY SWDEV_ALL_PACKAGES_USED)
      else()
         get_property(PKGS DIRECTORY ${EXE_TARGET_DIRECTORY} PROPERTY SWDEV_ALL_PACKAGES_USED)
      endif()
      list(REMOVE_DUPLICATES PKGS)
      
      foreach(pkg ${PKGS})
         get_property(${pkg}_BINDIR_DEBUG GLOBAL PROPERTY ${pkg}_BINDIR_DEBUG)
         foreach(pkgBinDir ${${pkg}_BINDIR_DEBUG})
            set(DEBUG_PATH "${pkgBinDir}\;${DEBUG_PATH}")
         endforeach()
         
         get_property(${pkg}_BINDIR_RELEASE GLOBAL PROPERTY ${pkg}_BINDIR_RELEASE)
         foreach(pkgBinDir ${${pkg}_BINDIR_RELEASE})
            set(RELEASE_PATH "${pkgBinDir}\;${RELEASE_PATH}")
         endforeach()
      endforeach()
      set(DEBUG_ENVIRONMENT "${ADDITIONAL_ENVIRONMENT_DEBUG}\nSOURCE_ROOT=${TOOLS_DIRECTORY}/..\nPATH=${DEBUG_PATH}")
      set(RELEASE_ENVIRONMENT "${ADDITIONAL_ENVIRONMENT_RELEASE}\nSOURCE_ROOT=${TOOLS_DIRECTORY}/..\nPATH=${RELEASE_PATH}")
      string(REPLACE "/" "\\" DEBUG_ENVIRONMENT ${DEBUG_ENVIRONMENT})
      string(REPLACE "/" "\\" RELEASE_ENVIRONMENT ${RELEASE_ENVIRONMENT})

      if(${ARGC} GREATER 6)
         set(DEBUGGER_COMMAND "${ARGV6}")
         string(REPLACE "/" "\\" DEBUGGER_COMMAND ${DEBUGGER_COMMAND})
      endif()
      if(${ARGC} GREATER 7)
         set(DEBUGGER_COMMAND_ARGS "${ARGV7}")
         string(REPLACE "/" "\\" DEBUGGER_COMMAND_ARGS ${DEBUGGER_COMMAND_ARGS})
      endif()
      configure_file(${USER_FILE_TEMPLATE} ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.vcxproj.user)
   endif (WIN32)
endmacro()


