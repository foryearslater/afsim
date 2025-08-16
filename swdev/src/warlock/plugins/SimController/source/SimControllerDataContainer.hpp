// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2017 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef SIMCONTROLLERDATACONTAINER_HPP
#define SIMCONTROLLERDATACONTAINER_HPP

#include <string>
#include <unordered_set>

#include <QObject>

#include "UtCalendar.hpp"
#include "WsfSimulation.hpp"

namespace WkSimController
{
class SimControllerDataContainer : public QObject
{
   Q_OBJECT

public:
   struct SimState
   {
      double               mSimTime{0.0};
      UtCalendar           mWallTime{};
      double               mEndTime{0.0};
      WsfSimulation::State mSimState{WsfSimulation::cCOMPLETE};
      bool                 mPaused{false};
      double               mClockRate{1.0};
      double               mTimeBehind{0.0};
      bool                 mIsRealTime{true};
   };

   SimControllerDataContainer() = default;

   void SetSimState(const SimState& aSimState);
   void SetStarting(bool aPaused);

   double               GetSimTime() const { return mSimStateData.mSimTime; }
   const UtCalendar&    GetWallTime() const { return mSimStateData.mWallTime; }
   double               GetEndTime() const { return mSimStateData.mEndTime; }
   WsfSimulation::State GetSimState() const { return mSimStateData.mSimState; }
   const std::string&   GetStateString() const { return WsfSimulation::GetStateString(mSimStateData.mSimState); }
   bool                 GetPaused() const { return mSimStateData.mPaused; }
   double               GetClockRate() const { return mSimStateData.mClockRate; }
   double               GetTimeBehind() const { return mSimStateData.mTimeBehind; }
   bool                 GetRealTime() const { return mSimStateData.mIsRealTime; }

   bool GetApplicationsBehind() const { return !mOthersBehindSet.empty(); }
   void SetApplicationBehind(const std::string& aApplicationId, bool aRemoteBehind);

signals:
   void Initialized();
   void RealTimeChanged(bool aIsRealTime);
   void RemoteSimulationBehind();

   void SimPausing();
   void SimResuming();
   void SimComplete(double aSimTime);
   void SimClockRateChanged(double aClockRate);

private:
   SimState mSimStateData;

   // a set is necessary since we need to track all the behind simulations that are connected, not just 1
   std::unordered_set<std::string> mOthersBehindSet;
};
} // namespace WkSimController

#endif
