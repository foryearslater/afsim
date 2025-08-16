// ****************************************************************************
// CUI//REL TO USA ONLY
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2020 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef AIRCOMBATVISUALIZATIONSIMINTERFACE_HPP
#define AIRCOMBATVISUALIZATIONSIMINTERFACE_HPP

#include "AirCombatVisualizationSimEvents.hpp"
#include "WkSimInterface.hpp"

namespace WkAirCombat
{
class SimInterface : public warlock::SimInterfaceT<AirCombatSimEvent>
{
public:
   explicit SimInterface(const QString& aPluginName);

   // The source platform/target platforms are the only platforms data is gathered for.
   void SetSourcePlatform(const std::string& aPlatformName)
   {
      QMutexLocker locker(&mMutex);
      mSourcePlatformName = aPlatformName;
   }
   std::string GetSourcePlatformName() const
   {
      QMutexLocker locker(&mMutex);
      return mSourcePlatformName;
   }

   std::vector<std::string> GetTargetPlatformNames() const
   {
      QMutexLocker locker(&mMutex);
      return mTargetPlatformNames;
   }
   void AddTargetPlatform(const std::string& aPlatformName)
   {
      QMutexLocker locker(&mMutex);
      mTargetPlatformNames.emplace_back(aPlatformName);
   }
   void RemoveTargetPlatform(const std::string& aPlatformName);

private:
   void WallClockRead(const WsfSimulation& aSimulation) override;
   void PlatformAdded(double aSimTime, WsfPlatform& aPlatform) override;
   void PlatformDeleted(double aSimTime, const WsfPlatform& aPlatform) override;
   void SimulationComplete(const WsfSimulation& aSimulation) override;

   void PopulateData(const WsfSimulation& aSimulation, WsfPlatform& aPlatform, wkf::AirCombatDataEngagementSummary& aData);

   std::string              mSourcePlatformName;
   std::vector<std::string> mTargetPlatformNames;
};
} // namespace WkAirCombat

#endif // !AIRCOMBATVISUALIZATIONSIMINTERFACE_HPP
