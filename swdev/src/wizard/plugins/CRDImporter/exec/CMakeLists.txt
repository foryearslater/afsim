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

project(crd_importer_exec)

include(swdev_project)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} crd_importer_lib)
set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER "wizard/plugins/WizCRDImporter")
set_target_properties(${PROJECT_NAME} PROPERTIES INSTALL_RPATH "$ORIGIN/lib")
swdev_warning_level(${PROJECT_NAME})

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${INSTALL_EXE_PATH} COMPONENT Runtime)
