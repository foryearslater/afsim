# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************
plugin_cmakelists_template(Joystick warlock
                           THIRD_PARTY_INCLUDE_DIRS sdl2
                           THIRD_PARTY_LINK_LIBS sdl2
                           REQUIRES wsf_p6dof wsf_six_dof
                           LIBRARIES warlock_core wkf_common)
