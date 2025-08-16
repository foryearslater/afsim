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

#ifndef PLATFORMPARTBROWSERSIMCOMMANDS_HPP
#define PLATFORMPARTBROWSERSIMCOMMANDS_HPP

#include "PlatformPartBrowserTypes.hpp"
#include "WkSimInterface.hpp"

namespace WkPlatformPartBrowser
{
class ChangePartCommand : public warlock::SimCommand
{
public:
   ChangePartCommand(const std::string& aPlatformName, const std::string& aPartName, PropertyName aPropertyName, bool aState)
      : warlock::SimCommand(true)
      , mPlatformName(aPlatformName)
      , mPartName(aPartName)
      , mPropertyName(aPropertyName)
      , mState(aState)
   {
   }

   void Process(WsfSimulation& aSimulation) override;

private:
   std::string  mPlatformName;
   std::string  mPartName;
   PropertyName mPropertyName;
   bool         mState;
};
} // namespace WkPlatformPartBrowser
#endif