# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************
add_subdirectory(wsf_prompt)
plugin_cmakelists_template(WsfPrompt warlock
                           LIBRARIES warlock_core wsf_prompt)
# Group Warlock plugin and WSF plugin together
set_property(TARGET WsfPrompt  PROPERTY FOLDER "warlock/plugins/WsfPrompt")
set_property(TARGET wsf_prompt PROPERTY FOLDER "warlock/plugins/WsfPrompt")
