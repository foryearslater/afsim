# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************
if (WIN32)
   set(qtModules WinExtras)
endif()
plugin_cmakelists_template(SimController warlock
                           LIBRARIES warlock_core
                           QT_MODULES ${qtModules})
