#
# WSF CMake Configuration
#
# This file provides configuration and functionality for generating
# Doxygen documentation from the WSF code. It is intended to be included
# in the top-level CMake file and must be included before any optional
# projects/extensions are included. This file makes use of the
# WSF_INSTALL_DOXYGEN user configurable option.

# Verify that the system running this build has Doxygen installed.
# This will set the DOXYGEN_FOUND variable that will controls the
# create_doxyfile() macro.
# The QUIET option prevents CMake from writing the find results to
# the console as Doxygen is optional for WSF builds
find_package(Doxygen QUIET)

# Only enable doxygen featues if doxygen is installed on this system
if(${DOXYGEN_FOUND})
   # DEBUG
   #message("Doxygen version ${DOXYGEN_VERSION} found")

   # Setup location to build Doxygen files
   set(DOXYGEN_BUILD_DIR ${CMAKE_BINARY_DIR}/doxygen)
   if(NOT EXISTS ${DOXYGEN_BUILD_DIR})
      file(MAKE_DIRECTORY ${DOXYGEN_BUILD_DIR})
   endif()

   # Copy files needed to support Doxygen generation.
   # The files are referenced by the doxyfile
   file(COPY
        ${CMAKE_CURRENT_SOURCE_DIR}/doxygen/footer.txt
        ${CMAKE_CURRENT_SOURCE_DIR}/doxygen/mainpage.txt
        DESTINATION ${DOXYGEN_BUILD_DIR})

   # Location to output generated Doxygen
   set(DOXYGEN_OUTPUT_DIR ${DOXYGEN_BUILD_DIR})

   # Location of the doxygen file to read in/edit
   # and the file to write out to
   set(DOXYFILE_IN ${CMAKE_CURRENT_SOURCE_DIR}/doxygen/Doxyfile.in)
   set(DOXYFILE_OUT ${DOXYGEN_BUILD_DIR}/Doxyfile)

   # Based on the compiler we are using (Visual Studio or GCC)
   # Set the variable the will configure the doxyfile WARN_FORMAT
   # tag. This is for warning output format compatibilty
   if(MSVC)
      set(DOXYGEN_WARN_FORMAT "\"$file($line): $text\"")
   else()
      # GCC/Linux assumed
      set(DOXYGEN_WARN_FORMAT "\"$file:$line: $text\"")
   endif()

   # DEBUG - Messages that can be uncommented to better see what the file is doing
   #message("Doxygen root: ${CMAKE_CURRENT_SOURCE_DIR}")
   #message("Doxygen build dir: ${DOXYGEN_BUILD_DIR}")
   #message("Doxygen output dir: ${DOXYGEN_OUTPUT_DIR}")
   #message("Doxygen warn format: ${DOXYGEN_WARN_FORMAT}")
   #message("Doxygen in file : ${DOXYFILE_IN}")
   #message("Doxygen out file: ${DOXYFILE_OUT}")

   # Custom compiler target for generating Doxygen documentation
   add_custom_target(DOXYGEN COMMAND doxygen WORKING_DIRECTORY ${DOXYGEN_BUILD_DIR})
   set_property(TARGET DOXYGEN PROPERTY FOLDER CMakeCustomTargets)

else()
   if(WSF_INSTALL_DOXYGEN)
      message(WARNING "Doxygen install selected but Doxygen exe not available for generation.")
   endif()
endif()

# Global property to contain WSF sub-project and extension Doxygen inputs
# Always enabled as it is used by other CMake files
define_property(GLOBAL PROPERTY WSF_DOXYGEN_INPUT
                BRIEF_DOCS "Input for Doxygen"
                FULL_DOCS "Paths to files and directories that will passed to the Doxyfile INPUT variable"
               )
# Global property to contain WSF sub-project and extension Doxygen input exclusions
# Always enabled as it is used by other CMake files
define_property(GLOBAL PROPERTY WSF_DOXYGEN_EXCLUDE
                BRIEF_DOCS "Exclude for Doxygen"
                FULL_DOCS "Paths to files and directories that will be excluded from Doxygen INPUT"
               )

# Global property to contain WSF sub-project and extension Doxygen input exclusion patterns
# Always enabled as it is used by other CMake files
define_property(GLOBAL PROPERTY WSF_DOXYGEN_EXCLUDE_PATTERN
                BRIEF_DOCS "Exclude pattern for Doxygen"
                FULL_DOCS "Paths to files and directories that will be excluded from Doxygen INPUT"
               )

###########################################################
# Macros
###########################################################

# Add the supplied list of directories or files (DOXYGEN_INPUT) to the global
# list of files that will be used to generate Doxygen documentation
# if they have not already been added. This macro should be called by WSF
# extensions to add documentation tot he WSF build.
macro(add_wsf_doxygen_input DOXYGEN_INPUT)
   # Store global WSF_DOXYGEN_INPUT in a local variable for comparison
   get_property(CURRENT_DOXYGEN_INPUT GLOBAL PROPERTY WSF_DOXYGEN_INPUT)

   # Loop over the supplied dirs/files and add them one at a time. Because
   # we are expecting lists we use the built in ARGV variable that hold the
   # list of all variables passed in, not DOXYGEN_INPUT which would only hold
   # the first
   foreach(INPUT_SOURCE ${ARGV})
      # Only add things to INPUT list that are not already there
      if(NOT (CURRENT_DOXYGEN_INPUT MATCHES ${INPUT_SOURCE}?))
         # NOTES: APPEND_STRING is used to build a single string that is passed to
         # the doxyfile, not a ';' separated list. Backslash and new line added here are
         # important to keep the list manageable and formatted properly for the Doxygen
         # tag, the extra space is just so it lines up with the default list
         set_property(GLOBAL APPEND_STRING PROPERTY WSF_DOXYGEN_INPUT "                      ${INPUT_SOURCE} \\ \n")
      endif()
   endforeach()
endmacro()

# Add the supplied list of directories or files (DOXYGEN_EXCLUDE) to the global
# list of files that will be excluded from Doxygen documentation generation
# if they have not already been added. This macro should be called by WSF
# extensions to add excluded documentation to the WSF build.
# It functions identically to the add_wsf_doxygen_input() macro
macro(add_wsf_doxygen_exclude DOXYGEN_EXCLUDE)
   get_property(CURRENT_DOXYGEN_EXCLUDE GLOBAL PROPERTY WSF_DOXYGEN_EXCLUDE)
   foreach(EXCLUDE_SOURCE ${ARGV})
      if(NOT (CURRENT_DOXYGEN_EXCLUDE MATCHES ${EXCLUDE_SOURCE}?))
         set_property(GLOBAL APPEND_STRING PROPERTY WSF_DOXYGEN_EXCLUDE "                     ${EXCLUDE_SOURCE} \\ \n")
      endif()
   endforeach()
endmacro()

# Add the supplied list of patterns (DOXYGEN_EXCLUDE_PATTERN) to the global
# list of files that will be excluded from Doxygen documentation generation
# if they have not already been added. This macro should be called by WSF
# extensions to add excluded documentation to the WSF build.
# It functions identically to the add_wsf_doxygen_input() macro
macro(add_wsf_doxygen_exclude_pattern DOXYGEN_EXCLUDE_PATTERN)
   get_property(CURRENT_DOXYGEN_EXCLUDE_PATTERN GLOBAL PROPERTY WSF_DOXYGEN_EXCLUDE_PATTERN)
   foreach(EXCLUDE_PATTERN_SOURCE ${ARGV})
      if(NOT (CURRENT_DOXYGEN_EXCLUDE_PATTTERN MATCHES ${EXCLUDE_PATTERN_SOURCE}?))
         set_property(GLOBAL APPEND_STRING PROPERTY WSF_DOXYGEN_EXCLUDE_PATTERN "             ${EXCLUDE_PATTERN_SOURCE} \\ \n")
      endif()
   endforeach()
endmacro()

# Build the doxyfile that will be used by the custom build target added above.
# This file is configured by the other options specified above and by the source
# locations registered by optional projects
macro(create_doxyfile)
   # Only do the operations in this macro if Doxygen is available
   if(${DOXYGEN_FOUND})
      # Store global Doxygen properties in a local variables that will
      # configure doxyfile.in
      get_property(WSF_DOXYGEN_INPUT GLOBAL PROPERTY WSF_DOXYGEN_INPUT)
      get_property(WSF_DOXYGEN_EXCLUDE GLOBAL PROPERTY WSF_DOXYGEN_EXCLUDE)

      # This message kept enabled as confirmation that Doxygen is being processed
      message(STATUS "Configuring Doxygen")

      # Generate doxyfile from doxyfile.in using the following CMake variables:
      # DOXYGEN_OUTPUT_DIR      <- Location to output generated documentation
      # DOXYGEN_WARN_FORMAT     <- Warning format comptibility for Windows/Linux
      # WSF_DOXYGEN_INPUT       <- Other files to include ingeneration
      # WSF_DOXYGEN_EXCLUDE     <- Other files to exclude from generation
      # The @ONLY option replaces only varirables in the format: @VARIABLE_NAME@
      configure_file(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)

      # Upgrade output file of version is greater than 1.6.1 to ensure good output
      IF (${DOXYGEN_VERSION} VERSION_GREATER 1.6.1)
         execute_process(COMMAND doxygen -u ${DOXYFILE_OUT} WORKING_DIRECTORY ${DOXYGEN_BUILD_DIR})
      ENDIF()
   endif()
endmacro()

# Install doxygen HTML output in the 'install' location for the build. This macro
# depends on the WSF_INSTALL_DOXYGEN option set in the main CMakeLists.txt file.
macro(install_doxygen)
   # For there to be doxygen to install the build system must have DOxygen
   # installed and the optional DOXYGEN target needs to be built
   if(WSF_INSTALL_DOXYGEN AND ${DOXYGEN_FOUND})
      install(DIRECTORY ${DOXYGEN_OUTPUT_DIR}/html DESTINATION doxygen OPTIONAL)
      install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/doxygen/index-install.html DESTINATION doxygen RENAME index.html OPTIONAL)
   endif()
endmacro()
