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

set(SWDEV_TINYXML2_PACKAGE "tinyxml2-7.1.0" CACHE STRING "" FORCE)
mark_as_advanced(FORCE SWDEV_TINYXML2_PACKAGE)
swdev_acquire_packages("${WIZARD_ROOT}" ${SWDEV_TINYXML2_PACKAGE})

find_package(tinyxml2 REQUIRED)

plugin_cmakelists_template(WizOSMConverter wizard
                           LIBRARIES wizard_core wkf_common tinyxml2::tinyxml2)

install_source_files(${TOOLS_DIRECTORY}/3rd_party-cmake/${SWDEV_TINYXML2_PACKAGE}.cmake tools/3rd_party-cmake)
install_tinyxml2()
