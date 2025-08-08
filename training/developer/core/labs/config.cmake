# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************

# ----------------------------------------------------------------
# turn off all build targets by default
set(ENABLE_EXTENSION_DEFAULT FALSE CACHE INTERNAL "")

# ----------------------------------------------------------------
# set the internal cmake generator build tool by uncommenting one of the following lines:
set(CMAKE_GENERATOR "Visual Studio 15 2017" CACHE INTERNAL "")
#set(CMAKE_GENERATOR "Visual Studio 16 2019" CACHE INTERNAL "")
#set(CMAKE_GENERATOR "Visual Studio 17 2022" CACHE INTERNAL "")
#set(CMAKE_GENERATOR "Unix Makefiles" CACHE INTERNAL "")

# if the CMAKE_GENERATOR is set to either Visual Studio 15 2017 or Visual Studio 16 2019, then
#make sure the following is uncommented, otherwise make sure it is commented out
set(CMAKE_GENERATOR_PLATFORM x64 CACHE INTERNAL "")

# ----------------------------------------------------------------
# set the build to be a standard build by leaving the following set commands commented out
# set the build to be a unity build by uncommenting the following lines:
#set(CMAKE_UNITY_BUILD ON CACHE BOOL "")
#set(CMAKE_UNITY_BUILD_BATCH_SIZE 32 CACHE STRING "")

# ----------------------------------------------------------------
# Bare minimum AFSIM including mission, and wsf
# Useful for testing a single plugin without dependencies on other extensions.
# set the build targets that you want to build to TRUE or ON
set(BUILD_WITH_mission TRUE CACHE BOOL "")
set(BUILD_WITH_warlock FALSE CACHE BOOL "")
set(BUILD_WITH_comm_exercise FALSE CACHE BOOL "")
set(BUILD_WITH_component_exercise FALSE CACHE BOOL "")
set(BUILD_WITH_mover_exercise FALSE CACHE BOOL "")
set(BUILD_WITH_observer_exercise FALSE CACHE BOOL "")
set(BUILD_WITH_sensor_exercise FALSE CACHE BOOL "")
set(BUILD_WITH_weapon_exercise FALSE CACHE BOOL "")
set(BUILD_WITH_xio_exercise FALSE CACHE BOOL "")
set(BUILD_WITH_wsf_mil FALSE CACHE BOOL "")
set(BUILD_WITH_wsf_p6dof FALSE CACHE BOOL "")
set(BUILD_WITH_wsf_space FALSE CACHE BOOL "")
set(BUILD_WARLOCK_PLUGIN_P6 FALSE CACHE BOOL "")
set(BUILD_WARLOCK_PLUGIN_P6dofController FALSE CACHE BOOL "")
set(BUILD_WARLOCK_PLUGIN_P6dofData FALSE CACHE BOOL "")
set(BUILD_WARLOCK_PLUGIN_PlatformBrowser FALSE CACHE BOOL "")
set(BUILD_WARLOCK_PLUGIN_PlatformData FALSE CACHE BOOL "")
set(BUILD_WARLOCK_PLUGIN_SimController FALSE CACHE BOOL "")
set(BUILD_WARLOCK_PLUGIN_VisualEffects FALSE CACHE BOOL "")
set(BUILD_WKF_PLUGIN_MapDisplay FALSE CACHE BOOL "")
set(BUILD_WKF_PLUGIN_MapHoverInfo FALSE CACHE BOOL "")
set(BUILD_WKF_PLUGIN_TetherView FALSE CACHE BOOL "")

# Do not modify below this line
# ----------------------------------------------------------------
# set the build type to be release
set(CMAKE_BUILD_TYPE "Release" CACHE STRING "")


