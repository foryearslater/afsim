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

#include "SatelliteTetherSimEvents.hpp"

#include "SatelliteTetherDockWidget.hpp"
#include "SatelliteTetherPropagationManager.hpp"
#include "WkfPlatform.hpp"
#include "WkfScenario.hpp"
#include "WkfVtkEnvironment.hpp"

void SatelliteTether::OrbitColorUpdateEvent::Process(const std::map<unsigned int, DockWidget*>& aWidgetList,
                                                     PropagationManager&                        aPropagationManager)
{
   auto scenarioPtr = vaEnv.GetStandardScenario();
   if (scenarioPtr)
   {
      auto platformPtr = scenarioPtr->FindPlatformByIndex(mPlatformId);
      if (platformPtr)
      {
         for (auto& w : aWidgetList)
         {
            w.second->UpdateColor(platformPtr, mDefinedColor);
         }
      }
   }
}

void SatelliteTether::AddTrackEvent::Process(const std::map<unsigned int, DockWidget*>& aWidgetList,
                                             PropagationManager&                        aPropagationManager)
{
   auto scenarioPtr = vaEnv.GetStandardScenario();
   if (scenarioPtr)
   {
      auto platformPtr = scenarioPtr->FindPlatformByIndex(mPlatformId);
      if (platformPtr)
      {
         auto it = aWidgetList.find(mTetherId);
         if (it != aWidgetList.end())
         {
            it->second->UpdateColor(platformPtr, mColor);
            it->second->AddTrackToDock(platformPtr);
            aPropagationManager.SetPropagator(platformPtr->GetName(), std::move(mPropagatorPtr));
         }
      }
   }
}

void SatelliteTether::ReportPropagatorEvent::Process(const std::map<unsigned int, DockWidget*>& aWidgetList,
                                                     PropagationManager&                        aPropagationManager)
{
   auto scenarioPtr = vaEnv.GetStandardScenario();
   if (scenarioPtr)
   {
      auto platformPtr = scenarioPtr->FindPlatformByIndex(mPlatformId);
      if (platformPtr)
      {
         if (aPropagationManager.IsOfInterest(platformPtr->GetName()))
         {
            for (auto iter : aWidgetList)
            {
               if (iter.second->PropagatorIsOfInterest(platformPtr->GetName()))
               {
                  iter.second->UpdateTraceDataForPlatform(platformPtr->GetName(), mSimTime);
               }
            }

            aPropagationManager.SetPropagator(platformPtr->GetName(), std::move(mPropagatorPtr));
         }
      }
   }
}

void SatelliteTether::ReportInitialEpochEvent::Process(const std::map<unsigned int, DockWidget*>& aWidgetList,
                                                       PropagationManager&                        aPropagationManager)
{
   aPropagationManager.SetSimulationStartEpoch(mInitialEpoch);
   aPropagationManager.Clear();

   for (auto&& iter : aWidgetList)
   {
      iter.second->SetupOwnship();
   }
}
