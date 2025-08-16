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

# *****************************************************************************
# CMAKE file: USMTF
# *****************************************************************************
project(usmtf)
include(swdev_project)
add_subdirectory(source)
add_subdirectory(test)
install_sources_all_files(./ wizard/usmtf)
install(DIRECTORY data/ DESTINATION resources/data/usmtf COMPONENT Resource) 
