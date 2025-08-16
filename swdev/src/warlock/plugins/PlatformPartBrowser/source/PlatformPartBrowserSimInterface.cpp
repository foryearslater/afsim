// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2016-2017 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "PlatformPartBrowserSimInterface.hpp"

#include <QVariant>

#include "UtMemory.hpp"
#include "WsfArticulatedPart.hpp"
#include "WsfComm.hpp"
#include "WsfCommObserver.hpp"
#include "WsfFuel.hpp"
#include "WsfMilComponentRoles.hpp"
#include "WsfMover.hpp"
#include "WsfMoverObserver.hpp"
#include "WsfPlatform.hpp"
#include "WsfPlatformPart.hpp"
#include "WsfPlatformPartObserver.hpp"
#include "WsfProcessor.hpp"
#include "WsfProcessorObserver.hpp"
#include "WsfRadarSensor.hpp"
#include "WsfSensor.hpp"
#include "WsfSensorModeList.hpp"
#include "WsfSensorObserver.hpp"
#include "WsfSimulation.hpp"
#include "WsfTrack.hpp"
#include "WsfTrackList.hpp"
#include "WsfVisualPart.hpp"
#include "WsfWeapon.hpp"
#include "WsfWeaponObserver.hpp"

WkPlatformPartBrowser::SimInterface::SimInterface(const QString& aPluginName)
   : warlock::SimInterfaceT<PlatformPartEvent>(aPluginName)
   , mPlatformOfInterest("")
   , mPlatformChanged(false)
{
}

WkPlatformPartBrowser::SimInterface::~SimInterface()
{
   QMutexLocker locker(&mMutex);
   Clear();
}

void WkPlatformPartBrowser::SimInterface::SetPlatformName(const std::string& aPlatformName)
{
   QMutexLocker locker(&mMutex);
   if (aPlatformName != mPlatformOfInterest)
   {
      mPlatformChanged = true;
   }
   mPlatformOfInterest = aPlatformName;
}

void WkPlatformPartBrowser::SimInterface::SimulationInitializing(const WsfSimulation& aSimulation)
{
   QMutexLocker locker(&mMutex);
   Clear();

   mCallbacks.Add(
      WsfObserver::SensorTurnedOn(&aSimulation)
         .Connect([this](double aSimTime, WsfSensor* aSensor) { PlatformPartChangedHandler(aSensor, eON, true); }));
   mCallbacks.Add(
      WsfObserver::SensorTurnedOff(&aSimulation)
         .Connect([this](double aSimTime, WsfSensor* aSensor) { PlatformPartChangedHandler(aSensor, eON, false); }));
   mCallbacks.Add(WsfObserver::SensorOperational(&aSimulation)
                     .Connect([this](double aSimTime, WsfSensor* aSensor)
                              { PlatformPartChangedHandler(aSensor, eOPERATIONAL, true); }));
   mCallbacks.Add(WsfObserver::SensorNonOperational(&aSimulation)
                     .Connect([this](double aSimTime, WsfSensor* aSensor)
                              { PlatformPartChangedHandler(aSensor, eOPERATIONAL, false); }));

   mCallbacks.Add(
      WsfObserver::CommTurnedOn(&aSimulation)
         .Connect([this](double aSimTime, wsf::comm::Comm* aComm) { PlatformPartChangedHandler(aComm, eON, true); }));
   mCallbacks.Add(
      WsfObserver::CommTurnedOff(&aSimulation)
         .Connect([this](double aSimTime, wsf::comm::Comm* aComm) { PlatformPartChangedHandler(aComm, eON, false); }));
   mCallbacks.Add(WsfObserver::CommOperational(&aSimulation)
                     .Connect([this](double aSimTime, wsf::comm::Comm* aComm)
                              { PlatformPartChangedHandler(aComm, eOPERATIONAL, true); }));
   mCallbacks.Add(WsfObserver::CommNonOperational(&aSimulation)
                     .Connect([this](double aSimTime, wsf::comm::Comm* aComm)
                              { PlatformPartChangedHandler(aComm, eOPERATIONAL, false); }));

   mCallbacks.Add(
      WsfObserver::MoverTurnedOn(&aSimulation)
         .Connect([this](double aSimTime, WsfMover* aMover) { PlatformPartChangedHandler(aMover, eON, true); }));
   mCallbacks.Add(
      WsfObserver::MoverTurnedOff(&aSimulation)
         .Connect([this](double aSimTime, WsfMover* aMover) { PlatformPartChangedHandler(aMover, eON, false); }));
   mCallbacks.Add(WsfObserver::MoverOperational(&aSimulation)
                     .Connect([this](double aSimTime, WsfMover* aMover)
                              { PlatformPartChangedHandler(aMover, eOPERATIONAL, true); }));
   mCallbacks.Add(WsfObserver::MoverNonOperational(&aSimulation)
                     .Connect([this](double aSimTime, WsfMover* aMover)
                              { PlatformPartChangedHandler(aMover, eOPERATIONAL, false); }));

   mCallbacks.Add(WsfObserver::ProcessorTurnedOn(&aSimulation)
                     .Connect([this](double aSimTime, WsfProcessor* aProcessor)
                              { PlatformPartChangedHandler(aProcessor, eON, true); }));
   mCallbacks.Add(WsfObserver::ProcessorTurnedOff(&aSimulation)
                     .Connect([this](double aSimTime, WsfProcessor* aProcessor)
                              { PlatformPartChangedHandler(aProcessor, eON, false); }));
   mCallbacks.Add(WsfObserver::ProcessorOperational(&aSimulation)
                     .Connect([this](double aSimTime, WsfProcessor* aProcessor)
                              { PlatformPartChangedHandler(aProcessor, eOPERATIONAL, true); }));
   mCallbacks.Add(WsfObserver::ProcessorNonOperational(&aSimulation)
                     .Connect([this](double aSimTime, WsfProcessor* aProcessor)
                              { PlatformPartChangedHandler(aProcessor, eOPERATIONAL, false); }));

   mCallbacks.Add(
      WsfObserver::WeaponTurnedOn(&aSimulation)
         .Connect([this](double aSimTime, WsfWeapon* aWeapon) { PlatformPartChangedHandler(aWeapon, eON, true); }));
   mCallbacks.Add(
      WsfObserver::WeaponTurnedOff(&aSimulation)
         .Connect([this](double aSimTime, WsfWeapon* aWeapon) { PlatformPartChangedHandler(aWeapon, eON, false); }));
   mCallbacks.Add(WsfObserver::WeaponOperational(&aSimulation)
                     .Connect([this](double aSimTime, WsfWeapon* aWeapon)
                              { PlatformPartChangedHandler(aWeapon, eOPERATIONAL, true); }));
   mCallbacks.Add(WsfObserver::WeaponNonOperational(&aSimulation)
                     .Connect([this](double aSimTime, WsfWeapon* aWeapon)
                              { PlatformPartChangedHandler(aWeapon, eOPERATIONAL, false); }));

   mCallbacks.Add(WsfObserver::VisualPartTurnedOn(&aSimulation)
                     .Connect([this](double aSimTime, WsfVisualPart* aVisualPart)
                              { PlatformPartChangedHandler(aVisualPart, eON, true); }));
   mCallbacks.Add(WsfObserver::VisualPartTurnedOff(&aSimulation)
                     .Connect([this](double aSimTime, WsfVisualPart* aVisualPart)
                              { PlatformPartChangedHandler(aVisualPart, eON, false); }));
   mCallbacks.Add(WsfObserver::VisualPartOperational(&aSimulation)
                     .Connect([this](double aSimTime, WsfVisualPart* aVisualPart)
                              { PlatformPartChangedHandler(aVisualPart, eOPERATIONAL, true); }));
   mCallbacks.Add(WsfObserver::VisualPartNonOperational(&aSimulation)
                     .Connect([this](double aSimTime, WsfVisualPart* aVisualPart)
                              { PlatformPartChangedHandler(aVisualPart, eOPERATIONAL, false); }));
}

void WkPlatformPartBrowser::SimInterface::WallClockRead(const WsfSimulation& aSimulation)
{
   if (mMutex.tryLock())
   {
      WsfPlatform* platform = aSimulation.GetPlatformByName(mPlatformOfInterest);
      mMutex.unlock();

      std::vector<PlatformPart> parts;
      if (platform)
      {
         int count = platform->GetComponentCount<WsfPlatformPart>();

         for (int i = 0; i < count; ++i)
         {
            WsfPlatformPart* platformPart = platform->GetComponentEntry<WsfPlatformPart>(i);
            if (platformPart != nullptr)
            {
               parts.emplace_back(platformPart->GetUniqueId(), platformPart->GetName(), platformPart->GetType());
               Populate(platformPart, parts.back());
            }
         }
      }
      if (mPlatformChanged) // we need to clear the data and repopulate it
      {
         mPlatformChanged = false;
         AddSimEvent(ut::make_unique<PartListEvent>(mPlatformOfInterest, parts));
      }
      else // we don't need to clear the data, just update it.
      {
         AddSimEvent(ut::make_unique<PartUpdateEvent>(mPlatformOfInterest, parts));
      }
   }
}

void WkPlatformPartBrowser::SimInterface::Clear()
{
   mCallbacks.Clear();
   mPlatformOfInterest = "";
   mPlatformChanged    = true;
}

void WkPlatformPartBrowser::SimInterface::PlatformPartChangedHandler(const WsfPlatformPart* aPlatformPart,
                                                                     PropertyName           aName,
                                                                     QVariant               aVariant)
{
   AddSimEvent(ut::make_unique<PartPropertyUpdateEvent>(mPlatformOfInterest, aPlatformPart->GetUniqueId(), aName, aVariant));
}

// This function sets the type to be used for the icon
// in such a way that the icon reflects the most specific type possible
// Also see comments above the PartType definition
void WkPlatformPartBrowser::SimInterface::SetIconType(PlatformPart& aPartInfo, PartType aPartType)
{
   if (aPartInfo.GetType() < aPartType) // if type is less specific than aPartType
   {
      aPartInfo.SetType(aPartType);
   }
}

void WkPlatformPartBrowser::SimInterface::Populate(const WsfPlatformPart* aPlatformPart, PlatformPart& aPartInfo)
{
   auto roles = aPlatformPart->GetComponentRoles();
   for (int i = 0; roles[i] != cWSF_COMPONENT_NULL; ++i)
   {
      switch (roles[i])
      {
      case cWSF_COMPONENT_PLATFORM_PART:
         PopulatePlatformPart(aPlatformPart, aPartInfo);
         break;
      case cWSF_COMPONENT_ARTICULATED_PART:
         PopulateArticulatedPart(aPlatformPart, aPartInfo);
         break;
      case cWSF_COMPONENT_MOVER:
         PopulateMover(aPlatformPart, aPartInfo);
         break;
      case cWSF_COMPONENT_PROCESSOR:
         PopulateProcessor(aPlatformPart, aPartInfo);
         break;
      case cWSF_COMPONENT_FUEL:
         PopulateFuel(aPlatformPart, aPartInfo);
         break;
      case cWSF_COMPONENT_SENSOR:
         PopulateSensor(aPlatformPart, aPartInfo);
         break;
      case cWSF_COMPONENT_COMM:
         PopulateComm(aPlatformPart, aPartInfo);
         break;
      case cWSF_COMPONENT_WEAPON:
         PopulateWeapon(aPlatformPart, aPartInfo);
         break;
      }
   }
}

void WkPlatformPartBrowser::SimInterface::PopulatePlatformPart(const WsfPlatformPart* aPlatformPart, PlatformPart& aPartInfo)
{
   aPartInfo.AddProperty(eDEBUG, aPlatformPart->DebugEnabled());
   aPartInfo.AddProperty(eON, aPlatformPart->IsTurnedOn());
   aPartInfo.AddProperty(eOPERATIONAL, aPlatformPart->IsOperational());
   aPartInfo.AddProperty(eBROKEN, aPlatformPart->IsBroken());
}

void WkPlatformPartBrowser::SimInterface::PopulateArticulatedPart(const WsfPlatformPart* aPlatformPart,
                                                                  PlatformPart&          aPartInfo)
{
   const WsfArticulatedPart* articulatedPart = dynamic_cast<const WsfArticulatedPart*>(aPlatformPart);
   if (articulatedPart)
   {
      static const std::map<WsfArticulatedPart::SlewMode, QString> nameMap = {{WsfArticulatedPart::cSLEW_FIXED, "Fixed"},
                                                                              {WsfArticulatedPart::cSLEW_AZ, "Azimuth"},
                                                                              {WsfArticulatedPart::cSLEW_EL, "Elevation"},
                                                                              {WsfArticulatedPart::cSLEW_AZ_EL,
                                                                               "Azimuth, Elevation"}};
      aPartInfo.AddProperty(eROLL, articulatedPart->GetRoll());
      aPartInfo.AddProperty(ePITCH, articulatedPart->GetPitch());
      aPartInfo.AddProperty(eYAW, articulatedPart->GetYaw());
      aPartInfo.AddProperty(eTILT, articulatedPart->GetTilt());
      aPartInfo.AddProperty(eSLEWMODE, nameMap.at(articulatedPart->GetSlewMode()));

      SetIconType(aPartInfo, eARTICULATEDPART);
   }
}

void WkPlatformPartBrowser::SimInterface::PopulateMover(const WsfPlatformPart* aPlatformPart, PlatformPart& aPartInfo)
{
   const WsfMover* mover = dynamic_cast<const WsfMover*>(aPlatformPart);
   if (mover)
   {
      switch (mover->GetSpatialDomain())
      {
      case WSF_SPATIAL_DOMAIN_LAND:
         aPartInfo.SetType(eMOVER_LAND);
         break;
      case WSF_SPATIAL_DOMAIN_AIR:
         aPartInfo.SetType(eMOVER_AIR);
         break;
      case WSF_SPATIAL_DOMAIN_SURFACE:
         aPartInfo.SetType(eMOVER_SURFACE);
         break;
      case WSF_SPATIAL_DOMAIN_SUBSURFACE:
         aPartInfo.SetType(eMOVER_SUBSURFACE);
         break;
      case WSF_SPATIAL_DOMAIN_SPACE:
         aPartInfo.SetType(eMOVER_SPACE);
         break;
      case WSF_SPATIAL_DOMAIN_UNKNOWN:
         break;
      }
   }
}

void WkPlatformPartBrowser::SimInterface::PopulateProcessor(const WsfPlatformPart* aPlatformPart, PlatformPart& aPartInfo)
{
   SetIconType(aPartInfo, ePROCESSOR);
}

void WkPlatformPartBrowser::SimInterface::PopulateFuel(const WsfPlatformPart* aPlatformPart, PlatformPart& aPartInfo)
{
   SetIconType(aPartInfo, eFUEL);
}

void WkPlatformPartBrowser::SimInterface::PopulateSensor(const WsfPlatformPart* aPlatformPart, PlatformPart& aPartInfo)
{
   const WsfSensor* sensor = dynamic_cast<const WsfSensor*>(aPlatformPart);
   if (sensor != nullptr)
   {
      QStringList        modes;
      QStringList        tracks;
      WsfSensorModeList* modeList    = sensor->GetModeList();
      WsfSensorMode*     currentMode = sensor->GetCurrentMode();

      if (modeList != nullptr)
      {
         size_t count = sensor->GetModeCount();
         for (size_t i = 0; i < count; ++i)
         {
            WsfSensorMode* mode = sensor->GetModeEntry(i);
            if (mode != nullptr)
            {
               modes.push_back(mode->GetName().c_str());
            }
         }

         aPartInfo.AddProperty(eCURRENTMODE, currentMode->GetName().c_str());
         aPartInfo.AddProperty(eMODES, modes);
      }
      WsfTrackList trackList;
      sensor->GetActiveTrackList(trackList);

      for (unsigned int i = 0; i < trackList.GetTrackCount(); ++i)
      {
         tracks.push_back(QString::fromStdString(trackList.GetTrackEntry(i)->GetTargetName().GetString()));
      }
      aPartInfo.AddProperty(eTRACKS, tracks);

      SetIconType(aPartInfo, eSENSOR);

      // Check specific subtypes of the Sensor Component Type
      auto* radar = dynamic_cast<const WsfRadarSensor*>(sensor);
      if (radar)
      {
         PopulateRadar(radar, aPartInfo);
      }
   }
}

void WkPlatformPartBrowser::SimInterface::PopulateComm(const WsfPlatformPart* aPlatformPart, PlatformPart& aPartInfo)
{
   SetIconType(aPartInfo, eCOMM);
}

void WkPlatformPartBrowser::SimInterface::PopulateWeapon(const WsfPlatformPart* aPlatformPart, PlatformPart& aPartInfo)
{
   SetIconType(aPartInfo, eWEAPON);

   const WsfWeapon* weapon = dynamic_cast<const WsfWeapon*>(aPlatformPart);
   if (weapon != nullptr)
   {
      if (weapon->IsA_TypeOf("WSF_EXPLICIT_WEAPON"))
      {
         PopulateExplicitWeapon(aPlatformPart, aPartInfo);
      }
   }
}

void WkPlatformPartBrowser::SimInterface::PopulateExplicitWeapon(const WsfPlatformPart* aPlatformPart,
                                                                 PlatformPart&          aPartInfo)
{
   SetIconType(aPartInfo, eWEAPON_EXPLICIT);
}

void WkPlatformPartBrowser::SimInterface::PopulateRadar(const WsfRadarSensor* aRadarSensor, PlatformPart& aPartInfo)
{
   size_t rcvrCount = aRadarSensor->GetEM_RcvrCount();
   size_t xmtrCount = aRadarSensor->GetEM_XmtrCount();

   QList<QVariant> receivers;

   for (size_t r = 0; r < rcvrCount; ++r)
   {
      QList<QVariant> receiverProperties;
      WsfEM_Rcvr&     rcvr = aRadarSensor->GetEM_Rcvr(r);

      WsfEM_Antenna* antenna = rcvr.GetAntenna();
      if (antenna != nullptr)
      {
         double minRange, maxRange, minEl, maxEl, minAz, maxAz;
         antenna->GetRangeLimits(minRange, maxRange);
         antenna->GetElevationFieldOfView(minEl, maxEl);
         antenna->GetAzimuthFieldOfView(minAz, maxAz);

         receiverProperties.push_back(QVariant::fromValue(Property(eRANGELIMITSMIN, minRange)));
         receiverProperties.push_back(QVariant::fromValue(Property(eRANGELIMITSMAX, maxRange)));
         receiverProperties.push_back(QVariant::fromValue(Property(eELEVATIONFOVMIN, minEl)));
         receiverProperties.push_back(QVariant::fromValue(Property(eELEVATIONFOVMAX, maxEl)));
         receiverProperties.push_back(QVariant::fromValue(Property(eAZIMUTHFOVMIN, minAz)));
         receiverProperties.push_back(QVariant::fromValue(Property(eAZIMUTHFOVMAX, maxAz)));
      }

      receiverProperties.push_back(QVariant::fromValue(Property(eBANDWIDTH, rcvr.GetBandwidth())));
      receiverProperties.push_back(QVariant::fromValue(Property(eNOISEPOWER, rcvr.GetNoisePower())));
      receiverProperties.push_back(QVariant::fromValue(Property(eDETECTIONTHRESHOLD, rcvr.GetDetectionThreshold())));

      receivers.push_back(receiverProperties);
   }

   QList<QVariant> transmitters;

   for (size_t t = 0; t < xmtrCount; ++t)
   {
      QList<QVariant> transmitterProperties;
      WsfEM_Xmtr&     xmtr = aRadarSensor->GetEM_Xmtr(t);

      transmitterProperties.push_back(QVariant::fromValue(Property(ePEAKPOWER, xmtr.GetPeakPower())));
      transmitterProperties.push_back(QVariant::fromValue(Property(eAVERAGEPOWER, xmtr.GetAveragePower())));
      transmitterProperties.push_back(QVariant::fromValue(Property(eFREQUENCY, xmtr.GetFrequency())));
      transmitterProperties.push_back(QVariant::fromValue(Property(ePULSEWIDTH, xmtr.GetPulseWidth())));
      transmitterProperties.push_back(QVariant::fromValue(Property(ePRI, xmtr.GetPulseRepetitionInterval())));

      transmitters.push_back(transmitterProperties);
   }

   aPartInfo.AddProperty(eRECEIVERS, receivers);
   aPartInfo.AddProperty(eTRANSMITTERS, transmitters);

   SetIconType(aPartInfo, eRADAR);
}
