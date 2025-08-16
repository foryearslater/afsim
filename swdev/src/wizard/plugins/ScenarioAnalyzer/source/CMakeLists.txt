# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************
# *****************************************************************************
# CMAKE file: wizard plugin
# *****************************************************************************
include(vis_plugins)
plugin_cmakelists_template(WizScenarioAnalyzer wizard
                           LIBRARIES wizard_core wsf_scenario_analyzer_lib
                           SOURCE_INSTALL_DIR wizard/plugins/ScenarioAnalyzer)
