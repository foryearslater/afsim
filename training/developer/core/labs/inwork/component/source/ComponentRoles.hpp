// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2019 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef COMPONENTROLES_HPP
#define COMPONENTROLES_HPP

//! @name Definitions to support templated component methods.
//@{
// We must define the component roles here as it is not a core value
// Define a called "cWSF_COMPONENT_SHIELDS" and assign it a unique number
// Define a called "cWSF_COMPONENT_LATINUM" and assign it a unique number
// Define a called "cWSF_COMPONENT_CYBER_SENSOR_EFFECT" and assign it a unique number
enum
{
   cWSF_COMPONENT_SHIELDS = 1234567,
   cWSF_COMPONENT_LATINUM = 1234568,
   cWSF_COMPONENT_CYBER_SENSOR_EFFECT = 654321
};

#endif
