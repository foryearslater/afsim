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

#ifndef SATELLITETETHERSIMCOMMANDS_HPP
#define SATELLITETETHERSIMCOMMANDS_HPP

#include "SatelliteTetherSimInterface.hpp"
#include "UtOrbitalElements.hpp"
#include "WkSimInterface.hpp"
#include "satellite_tether/WkfSatelliteTetherDockWidget.hpp"

namespace SatelliteTether
{

class AddTrackCommand : public SimInterface::Command
{
public:
   AddTrackCommand(SimInterface*      aSimInterfacePtr,
                   const std::string& aPlatformName,
                   size_t             aTetherId,
                   const QColor&      aColor,
                   bool               aUseWallClock = false)
      : Command(aSimInterfacePtr, aUseWallClock)
      , mPlatformName(aPlatformName)
      , mTetherId(aTetherId)
      , mColor(aColor)
   {
   }

   void Process(WsfSimulation& aSimulation) override;

private:
   std::string mPlatformName;
   size_t      mTetherId;
   QColor      mColor;
};

class AddOwnshipCommand : public SimInterface::Command
{
public:
   AddOwnshipCommand(SimInterface* aSimInterfacePtr, const std::string& aPlatformName, bool aUseWallclock = false)
      : Command(aSimInterfacePtr, aUseWallclock)
      , mPlatformName{aPlatformName}
   {
   }

   void Process(WsfSimulation& aSimulation) override;

private:
   std::string mPlatformName;
};
} // namespace SatelliteTether

#endif // ORBITSIMEVENTS_HPP
