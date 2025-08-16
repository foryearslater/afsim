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

#ifndef AdvancedWeaponBrowserInterface_HPP
#define AdvancedWeaponBrowserInterface_HPP

#include <QMutex>

#include "UtCallbackHolder.hpp"
#include "WeaponBrowserSimEvents.hpp"
#include "WkSimInterface.hpp"

namespace WkWeaponBrowser
{
class SimInterface : public warlock::SimInterfaceT<WeaponEvent>
{
public:
   SimInterface(const QString& aPluginName);

   void SetPlatformOfInterest(const std::string& aPlatformName);

   // mPlatformOfInterest is protected by a mutex, should not be returned by reference.
   std::string GetPlatformOfInterest() const;

   void WallClockRead(const WsfSimulation& aSimulation) override;

private:
   void SimulationComplete(const WsfSimulation& aSimulation) override;
   void SimulationInitializing(const WsfSimulation& aSimulation) override;
   void SimulationStarting(const WsfSimulation& aSimulation) override;

   std::string      mPlatformOfInterest;
   UtCallbackHolder mCallbacks;

   bool mChangedPlatform;
};
} // namespace WkWeaponBrowser

#endif // AdvancedWeaponBrowserInterface_HPP