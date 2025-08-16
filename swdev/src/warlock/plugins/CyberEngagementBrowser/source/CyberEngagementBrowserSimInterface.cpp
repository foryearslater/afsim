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

#include "CyberEngagementBrowserSimInterface.hpp"

#include <UtMemory.hpp>

#include "WsfCyberEngagement.hpp"
#include "WsfCyberObserver.hpp"
#include "WsfPlatformTypes.hpp"
#include "WsfScenario.hpp"
#include "WsfSimulation.hpp"

WkCyberEngagementBrowser::SimInterface::SimInterface(const QString& aPluginName)
   : warlock::SimInterfaceT<CyberEvent>(aPluginName)
{
}

void WkCyberEngagementBrowser::SimInterface::SimulationComplete(const WsfSimulation& aSimulation)
{
   AddSimEvent(ut::make_unique<SimCompleteEvent>());
}

void WkCyberEngagementBrowser::SimInterface::SimulationInitializing(const WsfSimulation& aSimulation)
{
   QMutexLocker locker(&mMutex);
   mCallbacks.Add(WsfObserver::CyberAttackInitiated(&aSimulation)
                     .Connect(
                        [this](double aSimTime, const wsf::cyber::Engagement& aEngagement)
                        {
                           AddSimEvent(ut::make_unique<CyberEngagementEvent>(
                              aEngagement.GetAttackType(),
                              aEngagement.GetAttacker(),
                              aEngagement.GetSimulation().GetPlatformByName(aEngagement.GetAttacker())->GetSide(),
                              aEngagement.GetVictim(),
                              aEngagement.GetSimulation().GetPlatformByName(aEngagement.GetVictim())->GetSide(),
                              "Attacking"));
                        }));

   mCallbacks.Add(WsfObserver::CyberAttackFailed(&aSimulation)
                     .Connect(
                        [this](double aSimTime, const wsf::cyber::Engagement& aEngagement)
                        {
                           AddSimEvent(ut::make_unique<CyberEngagementEvent>(
                              aEngagement.GetAttackType(),
                              aEngagement.GetAttacker(),
                              aEngagement.GetSimulation().GetPlatformByName(aEngagement.GetAttacker())->GetSide(),
                              aEngagement.GetVictim(),
                              aEngagement.GetSimulation().GetPlatformByName(aEngagement.GetVictim())->GetSide(),
                              "Attack Failed"));
                        }));

   mCallbacks.Add(WsfObserver::CyberAttackSucceeded(&aSimulation)
                     .Connect(
                        [this](double aSimTime, const wsf::cyber::Engagement& aEngagement)
                        {
                           AddSimEvent(ut::make_unique<CyberEngagementEvent>(
                              aEngagement.GetAttackType(),
                              aEngagement.GetAttacker(),
                              aEngagement.GetSimulation().GetPlatformByName(aEngagement.GetAttacker())->GetSide(),
                              aEngagement.GetVictim(),
                              aEngagement.GetSimulation().GetPlatformByName(aEngagement.GetVictim())->GetSide(),
                              "Attack Succeeded"));
                        }));

   mCallbacks.Add(WsfObserver::CyberAttackDetected(&aSimulation)
                     .Connect(
                        [this](double aSimTime, const wsf::cyber::Engagement& aEngagement)
                        {
                           AddSimEvent(ut::make_unique<CyberEngagementEvent>(
                              aEngagement.GetAttackType(),
                              aEngagement.GetAttacker(),
                              aEngagement.GetSimulation().GetPlatformByName(aEngagement.GetAttacker())->GetSide(),
                              aEngagement.GetVictim(),
                              aEngagement.GetSimulation().GetPlatformByName(aEngagement.GetVictim())->GetSide(),
                              "Attack Detected"));
                        }));

   mCallbacks.Add(WsfObserver::CyberAttackAttributed(&aSimulation)
                     .Connect(
                        [this](double aSimTime, const wsf::cyber::Engagement& aEngagement)
                        {
                           AddSimEvent(ut::make_unique<CyberEngagementEvent>(
                              aEngagement.GetAttackType(),
                              aEngagement.GetAttacker(),
                              aEngagement.GetSimulation().GetPlatformByName(aEngagement.GetAttacker())->GetSide(),
                              aEngagement.GetVictim(),
                              aEngagement.GetSimulation().GetPlatformByName(aEngagement.GetVictim())->GetSide(),
                              "Attack Attributed"));
                        }));

   mCallbacks.Add(WsfObserver::CyberScanInitiated(&aSimulation)
                     .Connect(
                        [this](double aSimTime, const wsf::cyber::Engagement& aEngagement)
                        {
                           AddSimEvent(ut::make_unique<CyberEngagementEvent>(
                              aEngagement.GetAttackType(),
                              aEngagement.GetAttacker(),
                              aEngagement.GetSimulation().GetPlatformByName(aEngagement.GetAttacker())->GetSide(),
                              aEngagement.GetVictim(),
                              aEngagement.GetSimulation().GetPlatformByName(aEngagement.GetVictim())->GetSide(),
                              "Scanning"));
                        }));

   mCallbacks.Add(WsfObserver::CyberScanFailed(&aSimulation)
                     .Connect(
                        [this](double aSimTime, const wsf::cyber::Engagement& aEngagement)
                        {
                           AddSimEvent(ut::make_unique<CyberEngagementEvent>(
                              aEngagement.GetAttackType(),
                              aEngagement.GetAttacker(),
                              aEngagement.GetSimulation().GetPlatformByName(aEngagement.GetAttacker())->GetSide(),
                              aEngagement.GetVictim(),
                              aEngagement.GetSimulation().GetPlatformByName(aEngagement.GetVictim())->GetSide(),
                              "Scan Failed"));
                        }));

   mCallbacks.Add(WsfObserver::CyberScanSucceeded(&aSimulation)
                     .Connect(
                        [this](double aSimTime, const wsf::cyber::Engagement& aEngagement)
                        {
                           AddSimEvent(ut::make_unique<CyberEngagementEvent>(
                              aEngagement.GetAttackType(),
                              aEngagement.GetAttacker(),
                              aEngagement.GetSimulation().GetPlatformByName(aEngagement.GetAttacker())->GetSide(),
                              aEngagement.GetVictim(),
                              aEngagement.GetSimulation().GetPlatformByName(aEngagement.GetVictim())->GetSide(),
                              "Scan Succeeded"));
                        }));

   mCallbacks.Add(WsfObserver::CyberScanDetected(&aSimulation)
                     .Connect(
                        [this](double aSimTime, const wsf::cyber::Engagement& aEngagement)
                        {
                           AddSimEvent(ut::make_unique<CyberEngagementEvent>(
                              aEngagement.GetAttackType(),
                              aEngagement.GetAttacker(),
                              aEngagement.GetSimulation().GetPlatformByName(aEngagement.GetAttacker())->GetSide(),
                              aEngagement.GetVictim(),
                              aEngagement.GetSimulation().GetPlatformByName(aEngagement.GetVictim())->GetSide(),
                              "Scan Detected"));
                        }));

   mCallbacks.Add(WsfObserver::CyberScanAttributed(&aSimulation)
                     .Connect(
                        [this](double aSimTime, const wsf::cyber::Engagement& aEngagement)
                        {
                           AddSimEvent(ut::make_unique<CyberEngagementEvent>(
                              aEngagement.GetAttackType(),
                              aEngagement.GetAttacker(),
                              aEngagement.GetSimulation().GetPlatformByName(aEngagement.GetAttacker())->GetSide(),
                              aEngagement.GetVictim(),
                              aEngagement.GetSimulation().GetPlatformByName(aEngagement.GetVictim())->GetSide(),
                              "Scan Attributed"));
                        }));
}
