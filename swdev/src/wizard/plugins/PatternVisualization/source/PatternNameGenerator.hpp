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

#ifndef PATTERNNAMEGENERATOR_HPP
#define PATTERNNAMEGENERATOR_HPP

#include <QString>

class WsfObject;

namespace PatternVisualizer
{
// This class generates the display name for the given pattern.
class PatternNameGenerator
{
public:
   explicit PatternNameGenerator(const QString& aFileName);

   QString GetPatternName(const WsfObject& aWsfPattern);

private:
   QString mBaseFileName;
   int     mNextId{1};
};

} // namespace PatternVisualizer

#endif
