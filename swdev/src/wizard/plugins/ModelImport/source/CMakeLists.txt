# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# Copyright 2019 Infoscitex, a DCS Company. All rights reserved.
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************
plugin_cmakelists_template(WizModelImport wizard
                           LIBRARIES wizard_core wkf_common)

file(GLOB DETAIL_SRCS detail/*)
target_sources(WizModelImport PRIVATE ${DETAIL_SRCS})
