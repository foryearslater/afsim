# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************
plugin_cmakelists_template(WizCRDImporter wizard
                           LIBRARIES wizard_core crd_importer_lib)
set_property(TARGET WizCRDImporter PROPERTY FOLDER "wizard/plugins/WizCRDImporter")
