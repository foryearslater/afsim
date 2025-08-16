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

#include "P6DOF_ControllerDataContainer.hpp"

#include "WsfP6DOF_Mover.hpp"
#include "WsfPlatform.hpp"

WkP6DOF_Controller::P6DOF_ControllerDataContainer::P6DOF_ControllerDataContainer(const P6DOF_ControllerDataContainer& aSrc)
   : mSimulationInitialized(aSrc.mSimulationInitialized)
   , mConnectionDestroyed(aSrc.mConnectionDestroyed)
   , mConnectedPlatform(aSrc.mConnectedPlatform)
   , mPlatformData(aSrc.mPlatformData)
   , mSetOfP6DofMovers(aSrc.mSetOfP6DofMovers)
{
}

bool WkP6DOF_Controller::P6DOF_ControllerDataContainer::IsPlatformP6DOF_Entity(const QString& aPlatformName) const
{
   return (mSetOfP6DofMovers.find(aPlatformName) != mSetOfP6DofMovers.end());
}

bool WkP6DOF_Controller::P6DOF_ControllerDataContainer::GetSimulationInitialized() const
{
   return mSimulationInitialized;
}

void WkP6DOF_Controller::P6DOF_ControllerDataContainer::SetSimulationInitialized(bool aValue)
{
   mSimulationInitialized = aValue;

   if (mSimulationInitialized)
   {
      mConnectionDestroyed = false;
      mConnectedPlatform   = "";
      mSetOfP6DofMovers.clear();
   }
}

void WkP6DOF_Controller::P6DOF_ControllerDataContainer::PlatformAdded(const std::string& aPlatform)
{
   // This has already been verified as a P6DOF object, so add it to the list
   mSetOfP6DofMovers.insert(aPlatform.c_str());
}

void WkP6DOF_Controller::P6DOF_ControllerDataContainer::PlatformDeleted(const std::string& aPlatform)
{
   // This has already been verified as a P6DOF object,
   // so remove it if it is the connected platform
   if (aPlatform == mConnectedPlatform)
   {
      mConnectionDestroyed = true;
      mConnectedPlatform   = "";
      mSetOfP6DofMovers.erase(aPlatform.c_str());
   }
}

void WkP6DOF_Controller::P6DOF_ControllerDataContainer::SetConnectedPlatform(const std::string& aPlatformName)
{
   mConnectedPlatform   = aPlatformName;
   mConnectionDestroyed = false;
}

void WkP6DOF_Controller::P6DOF_ControllerDataContainer::ReleaseConnectedPlatform()
{
   mConnectedPlatform   = "";
   mConnectionDestroyed = true;
}

void WkP6DOF_Controller::P6DOF_ControllerDataContainer::SetPlatformData(const PlatformData& aPlatformData)
{
   mPlatformData = aPlatformData;
}
