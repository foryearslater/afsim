# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************
plugin_cmakelists_template(SensorVolumes warlock
                           REQUIRES wsf_mil
                           LIBRARIES warlock_core wkf_common
                           THIRD_PARTY_INCLUDE_DIRS osg)
