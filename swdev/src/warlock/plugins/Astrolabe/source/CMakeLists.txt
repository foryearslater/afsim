# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************
try_add_subdirectory(${TOOLS_DIRECTORY}/wkf/wsfg/wsf_spaceg/source wsf_spaceg)
plugin_cmakelists_template(Astrolabe warlock
                           THIRD_PARTY_INCLUDE_DIRS osg
                           REQUIRES wsf_space
                           LIBRARIES warlock_core wsf_spaceg)
