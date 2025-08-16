# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# Copyright 2018 Infoscitex, a DCS Company. All rights reserved.
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************
try_add_subdirectory(${TOOLS_DIRECTORY}/wkf/wsfg/wsf_spaceg/source wsf_spaceg)
plugin_cmakelists_template(WizSpaceTools wizard
                           REQUIRES wsf_space
                           LIBRARIES wizard_core wkf_common wsf_spaceg)
