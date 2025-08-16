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

#include "InteractionsSimInterface.hpp"

#include <sstream>

#include "UtMemory.hpp"
#include "UtTime.hpp"
#include "WsfComm.hpp"
#include "WsfCommNetworkManager.hpp"
#include "WsfCommObserver.hpp"
#include "WsfCyberEngagement.hpp"
#include "WsfCyberObserver.hpp"
#include "WsfLocalTrack.hpp"
#include "WsfMessage.hpp"
#include "WsfPlatform.hpp"
#include "WsfSensor.hpp"
#include "WsfSensorObserver.hpp"
#include "WsfSensorResult.hpp"
#include "WsfSimulation.hpp"
#include "WsfTaskObserver.hpp"
#include "WsfTrack.hpp"
#include "WsfTrackObserver.hpp"
#include "WsfWeaponEngagement.hpp"
#include "WsfWeaponObserver.hpp"
#include "interaction/WkfAttachmentInteraction.hpp"

#define FORMAT_TIME_STRING(TIME) \
   std::ostringstream oss;       \
   oss << " at T=" << UtTime(TIME, UtTime::FmtHMS)

WkInteractions::SimInterface::SimInterface(const QString& aPluginName)
   : warlock::SimInterfaceT<InteractionEvent>(aPluginName)
   , mTimeout(0.0)
{
}

void WkInteractions::SimInterface::SetTimeout(double aTimeout)
{
   QMutexLocker locker(&mMutex);
   mTimeout = aTimeout;
}

void WkInteractions::SimInterface::ProcessEnableFlagChanged(bool aEnabled)
{
   if (!aEnabled)
   {
      QMutexLocker locker(&mMutex);
      mCallbacks.Clear();
   }
}

void WkInteractions::SimInterface::SimulationInitializing(const WsfSimulation& aSimulation)
{
   if (!IsEnabled())
   {
      return;
   }

   QMutexLocker locker(&mMutex);

   mCallbacks.Clear();

   //****** Jamming

   mCallbacks.Add(
      WsfObserver::JammingRequestInitiated(&aSimulation)
         .Connect(
            [this](double aSimTime, WsfWeapon* aWeaponPtr, double aFrequency, double aBandwidth, WsfStringId aTechnique, size_t aTargetIndex)
            {
               FORMAT_TIME_STRING(aSimTime) << " with " << aWeaponPtr->GetName() << "\nFreq: " << aFrequency
                                            << " Hz, BW: " << aBandwidth << " Hz, Technique: " << aTechnique;
               AddSimEvent(ut::make_unique<InteractionEvent>(aWeaponPtr->GetPlatform()->GetIndex(),
                                                             aTargetIndex,
                                                             true,
                                                             wkf::InteractionPrefData::cJAM,
                                                             aWeaponPtr->GetUniqueId(),
                                                             oss.str()));
            }));

   mCallbacks.Add(
      WsfObserver::JammingRequestCanceled(&aSimulation)
         .Connect(
            [this](double aSimTime, WsfWeapon* aWeaponPtr, double aFrequency, double aBandwidth, size_t aTargetIndex)
            {
               AddSimEvent(ut::make_unique<InteractionEvent>(aWeaponPtr->GetPlatform()->GetIndex(),
                                                             aTargetIndex,
                                                             false,
                                                             wkf::InteractionPrefData::cJAM,
                                                             aWeaponPtr->GetUniqueId()));
            }));

   //****** Sensor Tracks

   mCallbacks.Add(WsfObserver::SensorTrackInitiated(&aSimulation)
                     .Connect(
                        [this](double aSimTime, WsfSensor* aSensorPtr, const WsfTrack* aTrackPtr)
                        {
                           FORMAT_TIME_STRING(aSimTime)
                              << " with " << aSensorPtr->GetName() << " (mode: " << aTrackPtr->GetSensorModeId() << ")";
                           AddSimEvent(ut::make_unique<InteractionEvent>(aSensorPtr->GetPlatform()->GetIndex(),
                                                                         aTrackPtr->GetTargetIndex(),
                                                                         true,
                                                                         wkf::InteractionPrefData::cTRACK,
                                                                         aTrackPtr->GetTrackId().GetLocalTrackNumber(),
                                                                         oss.str()));
                        }));

   mCallbacks.Add(WsfObserver::SensorTrackDropped(&aSimulation)
                     .Connect(
                        [this](double aSimTime, WsfSensor* aSensorPtr, const WsfTrack* aTrackPtr)
                        {
                           AddSimEvent(ut::make_unique<InteractionEvent>(aSensorPtr->GetPlatform()->GetIndex(),
                                                                         aTrackPtr->GetTargetIndex(),
                                                                         false,
                                                                         wkf::InteractionPrefData::cTRACK,
                                                                         aTrackPtr->GetTrackId().GetLocalTrackNumber()));
                        }));

   //****** Local Tracks

   mCallbacks.Add(
      WsfObserver::LocalTrackInitiated(&aSimulation)
         .Connect(
            [this](double aSimTime, WsfPlatform* aPlatformPtr, const WsfLocalTrack* aLocalTrackPtr, const WsfTrack* aTrackPtr)
            {
               FORMAT_TIME_STRING(aSimTime)
                  << " from raw track owned by " << aTrackPtr->GetTrackId().GetOwningPlatformId();
               AddSimEvent(ut::make_unique<InteractionEvent>(aPlatformPtr->GetIndex(),
                                                             aLocalTrackPtr->GetTargetIndex(),
                                                             true,
                                                             wkf::InteractionPrefData::cLOCALTRACK,
                                                             aLocalTrackPtr->GetTrackId().GetLocalTrackNumber(),
                                                             oss.str()));
            }));

   mCallbacks.Add(WsfObserver::LocalTrackDropped(&aSimulation)
                     .Connect(
                        [this](double aSimTime, WsfPlatform* aPlatformPtr, const WsfLocalTrack* aLocalTrackPtr)
                        {
                           AddSimEvent(
                              ut::make_unique<InteractionEvent>(aPlatformPtr->GetIndex(),
                                                                aLocalTrackPtr->GetTargetIndex(),
                                                                false,
                                                                wkf::InteractionPrefData::cLOCALTRACK,
                                                                aLocalTrackPtr->GetTrackId().GetLocalTrackNumber()));
                        }));

   //****** Comm Messages

   mCallbacks.Add(
      WsfObserver::MessageReceived(&aSimulation)
         .Connect(
            [this](double             aSimTime,
                   wsf::comm::Comm*   aXmtrPtr,
                   wsf::comm::Comm*   aRcvrPtr,
                   const WsfMessage&  aMessage,
                   wsf::comm::Result& aResult)
            {
               WsfPlatform* rcvrPtr = aRcvrPtr->GetPlatform();
               if (rcvrPtr)
               {
                  FORMAT_TIME_STRING(aSimTime)
                     << " using " << aXmtrPtr->GetName() << " (type: " << aMessage.GetType() << ")";
                  unsigned int sn   = aMessage.GetSerialNumber();
                  size_t       xmtr = aXmtrPtr->GetPlatform()->GetIndex();
                  size_t       rcvr = rcvrPtr->GetIndex();
                  AddSimEvent(
                     ut::make_unique<InteractionEvent>(xmtr, rcvr, true, wkf::InteractionPrefData::cMESSAGE, sn, oss.str()));

                  // Add the timeout event
                  WsfSimulation* simPtr = rcvrPtr->GetSimulation();
                  simPtr->AddEvent(ut::make_unique<WsfOneShotEvent>(
                     simPtr->GetSimTime() + mTimeout,
                     [=]() {
                        AddSimEvent(
                           ut::make_unique<InteractionEvent>(xmtr, rcvr, false, wkf::InteractionPrefData::cMESSAGE, sn));
                     }));
               }
            }));

   mCallbacks.Add(
      WsfObserver::MessageHop(&aSimulation)
         .Connect(
            [this](double aSimTime, wsf::comm::Comm* aRcvrPtr, wsf::comm::Comm* aDestPtr, const WsfMessage& aMessage)
            {
               WsfPlatform* rcvrPtr = aRcvrPtr->GetPlatform();
               if (rcvrPtr)
               {
                  MessageId mid(aMessage.GetSerialNumber(),
                                aMessage.GetSrcAddr().GetAddress(),
                                aDestPtr->GetAddress().GetAddress());

                  size_t      rcvrIdx  = rcvrPtr->GetIndex();
                  std::string rcvrName = aRcvrPtr->GetName();
                  size_t      srcIndex;
                  std::string srcName;

                  auto last = mMessageHopTracker.find(mid);

                  if (last != mMessageHopTracker.end())
                  {
                     srcName  = last->second.second;
                     srcIndex = last->second.first;
                     // line from last
                  }
                  else
                  {
                     auto* origComm = aRcvrPtr->GetSimulation()->GetCommNetworkManager()->GetComm(aMessage.GetSrcAddr());
                     if (origComm != nullptr)
                     {
                        srcName = origComm->GetName();
                     }
                     srcIndex = aMessage.GetOriginatorIndex();
                     // line from orig
                  }


                  FORMAT_TIME_STRING(aSimTime) << " using " << srcName << " (type: " << aMessage.GetType() << ")";
                  unsigned int sn   = aMessage.GetSerialNumber();
                  size_t       xmtr = srcIndex;
                  size_t       rcvr = rcvrPtr->GetIndex();
                  AddSimEvent(
                     ut::make_unique<InteractionEvent>(xmtr, rcvr, true, wkf::InteractionPrefData::cMESSAGE, sn, oss.str()));

                  // Add the timeout event
                  WsfSimulation* simPtr = rcvrPtr->GetSimulation();
                  simPtr->AddEvent(ut::make_unique<WsfOneShotEvent>(
                     simPtr->GetSimTime() + mTimeout,
                     [=]() {
                        AddSimEvent(
                           ut::make_unique<InteractionEvent>(xmtr, rcvr, false, wkf::InteractionPrefData::cMESSAGE, sn));
                     }));
                  mMessageHopTracker[mid] = std::make_pair(rcvrIdx, rcvrName);
               }
            }));

   //****** Tasks

   mCallbacks.Add(WsfObserver::TaskAccepted(&aSimulation)
                     .Connect(
                        [this](double aSimTime, const WsfTask* aTaskPtr, const WsfTrack* aTrackPtr)
                        {
                           FORMAT_TIME_STRING(aSimTime)
                              << " to " << aTaskPtr->GetTaskType() << " " << aTrackPtr->GetTargetName();
                           if (!aTaskPtr->GetResourceName().Empty())
                           {
                              oss << " with resource " << aTaskPtr->GetResourceName()
                                  << " (mode: " << aTaskPtr->GetResourceMode() << ")";
                           }
                           AddSimEvent(ut::make_unique<InteractionEvent>(aTaskPtr->GetAssignerPlatformIndex(),
                                                                         aTaskPtr->GetAssigneePlatformIndex(),
                                                                         true,
                                                                         wkf::InteractionPrefData::cTASK,
                                                                         aTaskPtr->GetTaskId(),
                                                                         oss.str()));
                        }));

   mCallbacks.Add(WsfObserver::TaskCanceled(&aSimulation)
                     .Connect(
                        [this](double aSimTime, const WsfTask* aTaskPtr)
                        {
                           AddSimEvent(ut::make_unique<InteractionEvent>(aTaskPtr->GetAssignerPlatformIndex(),
                                                                         aTaskPtr->GetAssigneePlatformIndex(),
                                                                         false,
                                                                         wkf::InteractionPrefData::cTASK,
                                                                         aTaskPtr->GetTaskId()));
                        }));

   mCallbacks.Add(WsfObserver::TaskCompleted(&aSimulation)
                     .Connect(
                        [this](double aSimTime, const WsfTask* aTaskPtr, WsfStringId)
                        {
                           AddSimEvent(ut::make_unique<InteractionEvent>(aTaskPtr->GetAssignerPlatformIndex(),
                                                                         aTaskPtr->GetAssigneePlatformIndex(),
                                                                         false,
                                                                         wkf::InteractionPrefData::cTASK,
                                                                         aTaskPtr->GetTaskId()));
                        }));

   //****** Detections

   mCallbacks.Add(WsfObserver::SensorDetectionChanged(&aSimulation)
                     .Connect(
                        [this](double aSimTime, WsfSensor* aSensorPtr, size_t aTargetIndex, WsfSensorResult& aResult)
                        {
                           FORMAT_TIME_STRING(aSimTime) << " with " << aSensorPtr->GetName()
                                                        << " (mode: " << aSensorPtr->GetCurrentModeName() << ")";
                           AddSimEvent(ut::make_unique<InteractionEvent>(aSensorPtr->GetPlatform()->GetIndex(),
                                                                         aTargetIndex,
                                                                         aResult.Detected(),
                                                                         wkf::InteractionPrefData::cDETECT,
                                                                         aSensorPtr->GetUniqueId(),
                                                                         oss.str()));
                        }));

   //****** Weapon Fire

   mCallbacks.Add(WsfObserver::WeaponFired(&aSimulation)
                     .Connect(
                        [this](double aSimTime, const WsfWeaponEngagement* aEngagementPtr, const WsfTrack* aTargetTrackPtr)
                        {
                           if (aEngagementPtr)
                           {
                              FORMAT_TIME_STRING(aSimTime) << " with " << aEngagementPtr->GetWeaponSystemName();
                              AddSimEvent(ut::make_unique<InteractionEvent>(aEngagementPtr->GetFiringPlatformIndex(),
                                                                            aTargetTrackPtr ?
                                                                               aTargetTrackPtr->GetTargetIndex() :
                                                                               aEngagementPtr->GetTargetPlatformIndex(),
                                                                            true,
                                                                            wkf::InteractionPrefData::cFIRE,
                                                                            aEngagementPtr->GetSerialNumber(),
                                                                            oss.str()));
                           }
                        }));

   mCallbacks.Add(WsfObserver::WeaponTerminated(&aSimulation)
                     .Connect(
                        [this](double aSimTime, const WsfWeaponEngagement* aEngagementPtr)
                        {
                           if (aEngagementPtr)
                           {
                              AddSimEvent(ut::make_unique<InteractionEvent>(aEngagementPtr->GetFiringPlatformIndex(),
                                                                            aEngagementPtr->GetTargetPlatformIndex(),
                                                                            false,
                                                                            wkf::InteractionPrefData::cFIRE,
                                                                            aEngagementPtr->GetSerialNumber()));
                           }
                        }));

   //****** Kills

   mCallbacks.Add(
      WsfObserver::WeaponHit(&aSimulation)
         .Connect(
            [this](double aSimTime, const WsfWeaponEngagement* aEngagementPtr, WsfPlatform* aTargetPtr)
            {
               if ((aEngagementPtr) && (aTargetPtr->GetDamageFactor() == 1.0))
               {
                  FORMAT_TIME_STRING(aSimTime) << " with weapon " << aEngagementPtr->GetWeaponSystemName()
                                               << " (effect: " << aEngagementPtr->GetWeaponEffects()->GetType() << ")";
                  unsigned int sn     = aEngagementPtr->GetSerialNumber();
                  size_t       firing = aEngagementPtr->GetFiringPlatformIndex();
                  size_t       target = aTargetPtr->GetIndex();
                  AddSimEvent(
                     ut::make_unique<InteractionEvent>(firing, target, true, wkf::InteractionPrefData::cKILL, sn, oss.str()));

                  // Add the timeout event
                  WsfSimulation* simPtr = aEngagementPtr->GetSimulation();
                  simPtr->AddEvent(ut::make_unique<WsfOneShotEvent>(
                     simPtr->GetSimTime() + mTimeout,
                     [=]() {
                        AddSimEvent(
                           ut::make_unique<InteractionEvent>(firing, target, false, wkf::InteractionPrefData::cKILL, sn));
                     }));
               }
            }));

   //****** Cyber

   mCallbacks.Add(WsfObserver::CyberScanInitiated(&aSimulation)
                     .Connect(
                        [this](double aSimTime, const wsf::cyber::Engagement& aEngagement)
                        {
                           FORMAT_TIME_STRING(aSimTime);
                           AddSimEvent(ut::make_unique<InteractionEvent>(aEngagement.GetAttackerIndex(),
                                                                         aEngagement.GetVictimIndex(),
                                                                         true,
                                                                         "CyberScan",
                                                                         aEngagement.GetKey(),
                                                                         oss.str()));
                        }));

   mCallbacks.Add(WsfObserver::CyberScanFailed(&aSimulation)
                     .Connect(
                        [this](double aSimTime, const wsf::cyber::Engagement& aEngagement)
                        {
                           AddSimEvent(ut::make_unique<InteractionEvent>(aEngagement.GetAttackerIndex(),
                                                                         aEngagement.GetVictimIndex(),
                                                                         false,
                                                                         "CyberScan",
                                                                         aEngagement.GetKey()));
                        }));

   mCallbacks.Add(
      WsfObserver::CyberScanSucceeded(&aSimulation)
         .Connect(
            [this](double aSimTime, const wsf::cyber::Engagement& aEngagement)
            {
               size_t attacker = aEngagement.GetAttackerIndex();
               size_t victim   = aEngagement.GetVictimIndex();
               size_t id       = aEngagement.GetKey();
               AddSimEvent(ut::make_unique<InteractionEvent>(attacker, victim, false, "CyberScan", id));

               AddSimEvent(ut::make_unique<InteractionEvent>(attacker, victim, true, "CyberCompromise", id));
               // Add the compromise icon timeout event
               aEngagement.GetSimulation().AddEvent(ut::make_unique<WsfOneShotEvent>(
                  aSimTime + mTimeout,
                  [=]()
                  { AddSimEvent(ut::make_unique<InteractionEvent>(attacker, victim, false, "CyberCompromise", id)); }));
            }));

   mCallbacks.Add(
      WsfObserver::CyberScanDetected(&aSimulation)
         .Connect(
            [this](double aSimTime, const wsf::cyber::Engagement& aEngagement)
            {
               size_t attacker = aEngagement.GetAttackerIndex();
               size_t victim   = aEngagement.GetVictimIndex();
               size_t id       = aEngagement.GetKey();
               AddSimEvent(ut::make_unique<InteractionEvent>(attacker, victim, true, "CyberDetect", id));
               // Add the detection icon timeout event
               aEngagement.GetSimulation().AddEvent(ut::make_unique<WsfOneShotEvent>(
                  aSimTime + mTimeout,
                  [=]() { AddSimEvent(ut::make_unique<InteractionEvent>(attacker, victim, false, "CyberDetect", id)); }));
            }));

   mCallbacks.Add(
      WsfObserver::CyberScanAttributed(&aSimulation)
         .Connect(
            [this](double aSimTime, const wsf::cyber::Engagement& aEngagement)
            {
               size_t attacker = aEngagement.GetAttackerIndex();
               size_t victim   = aEngagement.GetVictimIndex();
               size_t id       = aEngagement.GetKey();
               AddSimEvent(ut::make_unique<InteractionEvent>(victim, attacker, true, "CyberAttributed", id));
               // Add the detection icon timeout event
               aEngagement.GetSimulation().AddEvent(ut::make_unique<WsfOneShotEvent>(
                  aSimTime + mTimeout,
                  [=]()
                  { AddSimEvent(ut::make_unique<InteractionEvent>(victim, attacker, false, "CyberAttributed", id)); }));
            }));

   mCallbacks.Add(WsfObserver::CyberAttackInitiated(&aSimulation)
                     .Connect(
                        [this](double aSimTime, const wsf::cyber::Engagement& aEngagement)
                        {
                           FORMAT_TIME_STRING(aSimTime);
                           AddSimEvent(ut::make_unique<InteractionEvent>(aEngagement.GetAttackerIndex(),
                                                                         aEngagement.GetVictimIndex(),
                                                                         true,
                                                                         "CyberAttack",
                                                                         aEngagement.GetKey(),
                                                                         oss.str()));
                        }));

   mCallbacks.Add(WsfObserver::CyberAttackFailed(&aSimulation)
                     .Connect(
                        [this](double aSimTime, const wsf::cyber::Engagement& aEngagement)
                        {
                           AddSimEvent(ut::make_unique<InteractionEvent>(aEngagement.GetAttackerIndex(),
                                                                         aEngagement.GetVictimIndex(),
                                                                         false,
                                                                         "CyberAttack",
                                                                         aEngagement.GetKey()));
                        }));

   mCallbacks.Add(
      WsfObserver::CyberAttackSucceeded(&aSimulation)
         .Connect(
            [this](double aSimTime, const wsf::cyber::Engagement& aEngagement)
            {
               size_t attacker = aEngagement.GetAttackerIndex();
               size_t victim   = aEngagement.GetVictimIndex();
               size_t id       = aEngagement.GetKey();
               AddSimEvent(ut::make_unique<InteractionEvent>(attacker, victim, false, "CyberAttack", id));
               AddSimEvent(ut::make_unique<InteractionEvent>(attacker, victim, true, "CyberDamage", id));
               // Add the damaged icon timeout event
               aEngagement.GetSimulation().AddEvent(ut::make_unique<WsfOneShotEvent>(
                  aSimTime + mTimeout,
                  [=]() { AddSimEvent(ut::make_unique<InteractionEvent>(attacker, victim, false, "CyberDamage", id)); }));
            }));

   mCallbacks.Add(
      WsfObserver::CyberAttackDetected(&aSimulation)
         .Connect(
            [this](double aSimTime, const wsf::cyber::Engagement& aEngagement)
            {
               size_t attacker = aEngagement.GetAttackerIndex();
               size_t victim   = aEngagement.GetVictimIndex();
               size_t id       = aEngagement.GetKey();
               AddSimEvent(ut::make_unique<InteractionEvent>(attacker, victim, true, "CyberDetect", id));
               // Add the attack detection icon timeout event
               aEngagement.GetSimulation().AddEvent(ut::make_unique<WsfOneShotEvent>(
                  aSimTime + mTimeout,
                  [=]() { AddSimEvent(ut::make_unique<InteractionEvent>(attacker, victim, false, "CyberDetect", id)); }));
            }));

   mCallbacks.Add(
      WsfObserver::CyberAttackAttributed(&aSimulation)
         .Connect(
            [this](double aSimTime, const wsf::cyber::Engagement& aEngagement)
            {
               size_t attacker = aEngagement.GetAttackerIndex();
               size_t victim   = aEngagement.GetVictimIndex();
               size_t id       = aEngagement.GetKey();
               AddSimEvent(ut::make_unique<InteractionEvent>(victim, attacker, true, "CyberAttributed", id));
               // Add the attack detection icon timeout event
               aEngagement.GetSimulation().AddEvent(ut::make_unique<WsfOneShotEvent>(
                  aSimTime + mTimeout,
                  [=]()
                  { AddSimEvent(ut::make_unique<InteractionEvent>(victim, attacker, false, "CyberAttributed", id)); }));
            }));

   mCallbacks.Add(
      WsfObserver::CyberAttackRecovery(&aSimulation)
         .Connect(
            [this](double aSimTime, const wsf::cyber::Engagement& aEngagement)
            {
               if (aEngagement.IsVictimImmune())
               {
                  size_t attacker = aEngagement.GetAttackerIndex();
                  size_t victim   = aEngagement.GetVictimIndex();
                  size_t id       = aEngagement.GetKey();
                  AddSimEvent(ut::make_unique<InteractionEvent>(attacker, victim, true, "CyberImmune", id));
                  // Add the immunity icon timeout event
                  aEngagement.GetSimulation().AddEvent(ut::make_unique<WsfOneShotEvent>(
                     aSimTime + mTimeout,
                     [=]()
                     { AddSimEvent(ut::make_unique<InteractionEvent>(attacker, victim, false, "CyberImmune", id)); }));
               }
            }));
}

bool WkInteractions::SimInterface::MessageId::operator<(const MessageId& aRHS) const
{
   if (aRHS.mSerialNumber == mSerialNumber)
   {
      if (aRHS.mDstAddr == mDstAddr)
      {
         return (mSrcAddr < aRHS.mSrcAddr);
      }
      else
      {
         return (mDstAddr < aRHS.mDstAddr);
      }
   }
   else
   {
      return mSerialNumber < aRHS.mSerialNumber;
   }
}
