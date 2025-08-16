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

#include "JoystickDataContainer.hpp"

JoystickDataContainer::JoystickDataContainer(const JoystickDataContainer& aSrc)
   : mSimulationInitialized(aSrc.mSimulationInitialized)
   , mConnectedPlatform(aSrc.mConnectedPlatform)
   , mJoystickData(aSrc.mJoystickData)
   , mSetOfP6DofMovers(aSrc.mSetOfP6DofMovers)
{
}

bool JoystickDataContainer::IsPlatformP6DOF_Entity(const QString& aPlatformName) const
{
   return (mSetOfP6DofMovers.find(aPlatformName) != mSetOfP6DofMovers.end());
}

bool JoystickDataContainer::GetSimulationInitialized() const
{
   return mSimulationInitialized;
}

void JoystickDataContainer::SetSimulationInitialized(bool aValue)
{
   mSimulationInitialized = aValue;

   if (mSimulationInitialized)
   {
      mSetOfP6DofMovers.clear();
   }
}

void JoystickDataContainer::PlatformAdded(const std::string& aPlatformName)
{
   // This has already been verified as a P6DOF object, so add it to the list
   mSetOfP6DofMovers.insert(aPlatformName.c_str());
}

void JoystickDataContainer::PlatformDeleted(const std::string& aPlatformName)
{
   // This has already been verified as a P6DOF object,
   // so remove it if it is the connected platform
   if (aPlatformName == mConnectedPlatform)
   {
      mSetOfP6DofMovers.erase(aPlatformName.c_str());
   }
}

void JoystickDataContainer::SetConnectedPlatform(const std::string& aPlatformName)
{
   mConnectedPlatform = aPlatformName;
}

void JoystickDataContainer::ReleaseConnectedPlatform()
{
   mConnectedPlatform = "";
}

void JoystickDataContainer::SetJoystickData(const JoystickData& aJoystickData)
{
   mJoystickData = aJoystickData;
   emit JoystickDataChanged();
}
