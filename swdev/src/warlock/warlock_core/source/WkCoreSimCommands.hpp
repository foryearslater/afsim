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

#ifndef WKCORESIMCOMMANDS_HPP
#define WKCORESIMCOMMANDS_HPP

#include <string>

#include "WkSimInterface.hpp"

namespace warlock
{
class PlatformMovementCommand : public SimCommand
{
public:
   PlatformMovementCommand(const std::string& aPlatformName, double aLatitude, double aLongitude, double aAltitude)
      : mName(aPlatformName)
      , mLatitude(aLatitude)
      , mLongitude(aLongitude)
      , mAltitude(aAltitude)
   {
   }

   void Process(WsfSimulation& aSimulation) override;

private:
   std::string mName;
   double      mLatitude;
   double      mLongitude;
   double      mAltitude;
};
} // namespace warlock
#endif