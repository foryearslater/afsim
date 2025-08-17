# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************
# Generates a plugin which runs weapon_tools when given the --weapon-tools command-line argument

project(weapon_tools_plugin)
SET(GENIO_WITHOUT_UMP TRUE)

add_definitions(-DWEAPON_TOOLS_BUILD_PLUGIN)
include("${WSF_CORE_ROOT}/misc/swdev_project.cmake")
include(../source/weapon_tools.cmake)

add_library(weapon_tools_plugin SHARED ${SRCS})

target_link_libraries(${PROJECT_NAME} wsf geodata packetio util util_script dis genio ${SWDEV_THREAD_LIB} ${SWDEV_DL_LIB})

IF(WSF_INSTALL_PLUGIN_PATH)
   install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${WSF_INSTALL_PLUGIN_PATH})
ENDIF()
