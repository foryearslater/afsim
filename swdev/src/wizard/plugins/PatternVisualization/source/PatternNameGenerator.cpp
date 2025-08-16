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

#include "PatternNameGenerator.hpp"

#include <QDir>
#include <QString>

#include "UtException.hpp"
#include "WsfObject.hpp"

namespace PatternVisualizer
{

// This class generates the display name for the given pattern.
PatternNameGenerator::PatternNameGenerator(const QString& aFileName)
   : mBaseFileName{aFileName}
{
   // Remove the directory information leaving just the base file name.
   mBaseFileName.remove(0, mBaseFileName.lastIndexOf(QDir::separator()) + 1);
}

QString PatternNameGenerator::GetPatternName(const WsfObject& aWsfPattern)
{
   // First choice is the pattern's name.
   const auto& name = aWsfPattern.GetName();
   if (!name.empty())
   {
      return QString::fromStdString(name);
   }

   // Next choice is the pattern's type.
   const auto& type = aWsfPattern.GetType();
   if (!type.empty())
   {
      return QString::fromStdString(type);
   }

   // No good name found, so make up a unique one.
   auto fallbackName = QString{"%1 #%2"}.arg(mBaseFileName).arg(mNextId);
   ++mNextId;
   return fallbackName;
}

} // namespace PatternVisualizer
