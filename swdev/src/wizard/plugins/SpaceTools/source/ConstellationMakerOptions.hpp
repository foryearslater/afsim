// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2020 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef CONSTELLATIONMAKEROPTIONS_HPP
#define CONSTELLATIONMAKEROPTIONS_HPP

#include "UtUnitTypes.hpp"

class QTextDocument;

#include "WsfConstellationOptions.hpp"

namespace SpaceTools
{

namespace ConstellationMakerOptions
{

WsfConstellationOptions DefaultConstellationMakerOptions();
WsfConstellationOptions InterpretOptions(QTextDocument* aDocument);

} // namespace ConstellationMakerOptions

} // namespace SpaceTools

#endif // CONSTELLATIONMAKEROPTIONS_HPP
