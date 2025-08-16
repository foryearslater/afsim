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
#ifndef AdvancedWeaponBrowserSimCommands_HPP
#define AdvancedWeaponBrowserSimCommands_HPP

#include "WkSimInterface.hpp"

namespace WkWeaponBrowser
{
class FireCommand : public warlock::SimCommand
{
public:
   FireCommand(const std::string& aPlatformName, const std::string& aWeaponName, int aTrackIndex, int aQuantity = 1)
      : warlock::SimCommand()
      , mPlatformName(aPlatformName)
      , mWeaponName(aWeaponName)
      , mTrackIndex(aTrackIndex)
      , mQuantity(aQuantity)
   {
   }

   void Process(WsfSimulation& aSimulation) override;

protected:
   std::string mPlatformName;
   std::string mWeaponName;
   int         mTrackIndex;
   int         mQuantity;
};
} // namespace WkWeaponBrowser
#endif