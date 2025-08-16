# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************
plugin_cmakelists_template(Interactions warlock
                           THIRD_PARTY_INCLUDE_DIRS osg
                           REQUIRES wsf_cyber
                           LIBRARIES warlock_core wkf_common)
