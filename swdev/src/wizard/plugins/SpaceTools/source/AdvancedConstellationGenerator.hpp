// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2018 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef ADVANCEDCONSTELLATIONGENERATOR_HPP
#define ADVANCEDCONSTELLATIONGENERATOR_HPP

#include <ostream>
#include <string>

class WsfConstellationOptions;

namespace SpaceTools
{

//! A set of utility functions to create the output for the Constellation Maker.
namespace AdvancedConstellationGenerator
{

void CreateGenerator(const WsfConstellationOptions& aOptions,
                     const std::string&             aGeneralUserContent,
                     const std::string&             aSpecificUserContent,
                     std::ostream&                  aStream);

void GrabUserContent(const std::string& aFileContents, std::string& aGeneralContent, std::string& aSpecificContent);

} // namespace AdvancedConstellationGenerator

} // namespace SpaceTools

#endif // ADVANCEDCONSTELLATIONGENERATOR_HPP
