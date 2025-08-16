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

#include "ZoneBrowserSimInterface.hpp"

#include "UtMemory.hpp"
#include "UtQt.hpp"
#include "UtVariant.hpp"
#include "WsfAttributeContainer.hpp"
#include "WsfGeoPoint.hpp"
#include "WsfObjectTypeList.hpp"
#include "WsfObjectTypeListBase.hpp"
#include "WsfPlatform.hpp"
#include "WsfPlatformObserver.hpp"
#include "WsfScenario.hpp"
#include "WsfSimulation.hpp"
#include "WsfZone.hpp"
#include "WsfZoneDefinition.hpp"
#include "WsfZoneObserver.hpp"
#include "WsfZoneReference.hpp"
#include "WsfZoneSet.hpp"
#include "WsfZoneTypes.hpp"

namespace
{
std::map<std::string, std::string> MakeAuxDataMap(const UtAttributeContainer& aAuxData)
{
   std::map<std::string, std::string> auxData{};
   UtAttributeBase::ConstIterator     iter(aAuxData);
   while (iter->HasNext())
   {
      const UtAttributeBase& attr = *iter;
      if (attr.IsContainerType())
      {
         auto containerMap = MakeAuxDataMap(static_cast<const UtAttributeContainer&>(attr));
         auxData.insert(containerMap.begin(), containerMap.end());
      }
      const std::string& name = attr.GetName();
      UtVariant          var;
      attr.GetVariant(var);
      auxData.emplace(name, var.ToString());
      iter->Next();
   }
   return auxData;
}
} // namespace

WkZoneBrowser::SimInterface::SimInterface(const QString& aPluginName)
   : warlock::SimInterfaceT<ZoneEvent>(aPluginName)
{
}

void WkZoneBrowser::SimInterface::SimulationComplete(const WsfSimulation& aSimulation)
{
   if (IsEnabled())
   {
      AddSimEvent(ut::make_unique<SimulationCompleteEvent>());
   }
}

void WkZoneBrowser::SimInterface::SimulationInitializing(const WsfSimulation& aSimulation)
{
   if (!IsEnabled())
   {
      return;
   }
   QMutexLocker locker(&mMutex);
   mCallbacks.Clear();
   mCallbacks.Add(WsfObserver::PlatformDeleted(&aSimulation)
                     .Connect(
                        [this](double aSimTime, const WsfPlatform* aPlatformPtr)
                        {
                           if (aPlatformPtr)
                           {
                              AddSimEvent(ut::make_unique<PlatformDeletedEvent>(aPlatformPtr->GetName()));
                           }
                        }));
   mCallbacks.Add(WsfObserver::ZoneFillColorChanged(&aSimulation).Connect(&SimInterface::ZoneFillColorChanged, this));
   mCallbacks.Add(WsfObserver::ZoneLineColorChanged(&aSimulation).Connect(&SimInterface::ZoneLineColorChanged, this));
}

void WkZoneBrowser::SimInterface::SimulationStarting(const WsfSimulation& aSimulation)
{
   // Get Zone Data from the Scenario
   // Global Zones
   std::vector<WsfZone*> globalZoneData;
   WsfZoneTypes&         globalZones = aSimulation.GetScenario().GetZoneTypes();
   globalZones.GetCurrentTypes(globalZoneData);
   for (const auto& zonePtr : globalZoneData)
   {
      auto def = dynamic_cast<WsfZoneDefinition*>(zonePtr);
      auto set = dynamic_cast<WsfZoneSet*>(zonePtr);
      if (def)
      {
         def->UpdateReference(aSimulation);
      }
      if (set)
      {
         for (int i = 0; i < set->GetNumZones(); i++)
         {
            auto zoneDef = dynamic_cast<WsfZoneDefinition*>(set->GetZoneAtIndex(i));
            if (zoneDef)
            {
               zoneDef->UpdateReference(aSimulation);
            }
         }
      }
      InterpretZoneSet(zonePtr);
   }

   // Platform Specific Zones
   for (size_t i = 0; i < aSimulation.GetPlatformCount(); ++i)
   {
      WsfPlatform* platformPtr = aSimulation.GetPlatformEntry(i);
      if (platformPtr)
      {
         const WsfPlatformComponentList& componentList = platformPtr->GetComponents();
         for (const auto& zonePtr : componentList)
         {
            auto def = dynamic_cast<WsfZoneDefinition*>(zonePtr);
            auto set = dynamic_cast<WsfZoneSet*>(zonePtr);
            if (def)
            {
               def->UpdateReference(aSimulation);
            }
            if (set)
            {
               for (int i = 0; i < set->GetNumZones(); i++)
               {
                  auto zoneDef = dynamic_cast<WsfZoneDefinition*>(set->GetZoneAtIndex(i));
                  if (zoneDef)
                  {
                     zoneDef->UpdateReference(aSimulation);
                  }
               }
            }
            InterpretZoneSet(zonePtr);
         }
      }
   }
   AddSimEvent(ut::make_unique<DataCopyCompleteEvent>());
}

void WkZoneBrowser::SimInterface::InterpretZoneSet(WsfPlatformComponent* aZonePtr)
{
   auto GetDefinedValue = [](double aValue, double aUnspecified) { return (aValue != aUnspecified) ? aValue : 0.0; };

   auto AddData = [&, aZonePtr](WsfZoneDefinition* aZoneDefPtr, wkf::ZoneSetData& aZoneSet)
   {
      wkf::CommonZoneVariables commonVariables;
      aZoneDefPtr->GetAltBounds(commonVariables.mMinAltitude, commonVariables.mMaxAltitude);
      commonVariables.mMinAltitudeSet = commonVariables.mMinAltitude != WsfZone::cUNSPECIFIED;
      commonVariables.mMaxAltitudeSet = commonVariables.mMaxAltitude != WsfZone::cUNSPECIFIED;

      if (!commonVariables.mMinAltitudeSet)
      {
         commonVariables.mMinAltitude = 0;
      }
      if (!commonVariables.mMaxAltitudeSet)
      {
         commonVariables.mMaxAltitude = commonVariables.mMinAltitude;
      }

      commonVariables.mReferencePlatformName = aZoneDefPtr->GetReferencePlatformName();
      commonVariables.mReferenceLat          = aZoneDefPtr->GetReferenceLat();
      commonVariables.mReferenceLon          = aZoneDefPtr->GetReferenceLon();

      if (commonVariables.mReferenceLat == WsfZone::cUNSPECIFIED)
      {
         commonVariables.mReferenceLat = 0;
      }
      if (commonVariables.mReferenceLon == WsfZone::cUNSPECIFIED)
      {
         commonVariables.mReferenceLon = 0;
      }

      commonVariables.mLocationDefined = false;

      // Location is defined if lat/lon is defined directly, without a reference platform.
      if (aZoneDefPtr->HasAbsoluteLatLon())
      {
         commonVariables.mLocationDefined = true;
      }

      commonVariables.mHeading = aZoneDefPtr->GetReferenceHeading();
      commonVariables.mAuxData.clear();
      if (aZoneDefPtr->HasAuxData())
      {
         commonVariables.mAuxData = MakeAuxDataMap(aZoneDefPtr->GetAuxDataConst());
      }

      WsfZoneDefinition::ShapeType shapeType = aZoneDefPtr->GetShapeType();
      if (shapeType == WsfZoneDefinition::ShapeType::cCIRCULAR)
      {
         wkf::CircularZoneVariables circleVariables;
         // The sqrt comparison is necessary because for an unspecified radius, GetRadius() is effectively returning
         // sqrt(cUNSPECIFIED)
         circleVariables.mRadius    = GetDefinedValue(aZoneDefPtr->GetRadius(), std::sqrt(WsfZone::cUNSPECIFIED));
         circleVariables.mMinRadius = GetDefinedValue(aZoneDefPtr->GetMinRadius(), std::sqrt(WsfZone::cUNSPECIFIED));
         aZoneDefPtr->GetAngleBounds(circleVariables.mStartAngle, circleVariables.mStopAngle);
         aZoneSet.AddCircularData(commonVariables, circleVariables);
      }
      else if (shapeType == WsfZoneDefinition::ShapeType::cELLIPTICAL)
      {
         wkf::EllipticalZoneVariables ellipticalVariables;
         ellipticalVariables.mLatAxis   = GetDefinedValue(aZoneDefPtr->GetLatAxisLength(), WsfZone::cUNSPECIFIED);
         ellipticalVariables.mLonAxis   = GetDefinedValue(aZoneDefPtr->GetLonAxisLength(), WsfZone::cUNSPECIFIED);
         ellipticalVariables.mMinRadius = GetDefinedValue(aZoneDefPtr->GetMinRadius(), std::sqrt(WsfZone::cUNSPECIFIED));
         aZoneDefPtr->GetAngleBounds(ellipticalVariables.mStartAngle, ellipticalVariables.mStopAngle);
         aZoneSet.AddEllipticalData(commonVariables, ellipticalVariables);
      }
      else if (shapeType == WsfZoneDefinition::ShapeType::cSPHERICAL)
      {
         wkf::SphericalZoneVariables sphericalVariables;
         sphericalVariables.mMinRadius = GetDefinedValue(aZoneDefPtr->GetMinRadius(), std::sqrt(WsfZone::cUNSPECIFIED));
         sphericalVariables.mRadius    = GetDefinedValue(aZoneDefPtr->GetRadius(), std::sqrt(WsfZone::cUNSPECIFIED));
         aZoneDefPtr->GetAngleBounds(sphericalVariables.mStartAngle, sphericalVariables.mStopAngle);
         aZoneSet.AddSphericalData(commonVariables, sphericalVariables);
      }
      else if (shapeType == WsfZoneDefinition::ShapeType::cPOLYGONAL)
      {
         wkf::PolygonalZoneVariables polygonalVariables;
         for (const auto& point : aZoneDefPtr->GetPoints())
         {
            polygonalVariables.mPoints.emplace_back(wkf::ZonePointStruct(point.mX, point.mY));
         }
         polygonalVariables.mUseLatLon = aZoneDefPtr->PointsAreLatLon();
         if (polygonalVariables.mUseLatLon)
         {
            commonVariables.mReferenceLat = aZoneDefPtr->Centroid().GetLat();
            commonVariables.mReferenceLon = aZoneDefPtr->Centroid().GetLon();
         }
         aZoneSet.AddPolygonalData(commonVariables, polygonalVariables);
      }
   };

   WsfZoneDefinition* zoneDefPtr = dynamic_cast<WsfZoneDefinition*>(aZonePtr);
   WsfZoneSet*        zoneSetPtr = dynamic_cast<WsfZoneSet*>(aZonePtr);
   WsfZoneReference*  zoneRefPtr = dynamic_cast<WsfZoneReference*>(aZonePtr);
   if (zoneDefPtr)
   {
      std::string platformName = (zoneDefPtr->GetPlatform()) ? zoneDefPtr->GetPlatform()->GetName() : "Global Zones";
      wkf::ZoneSetData zone(zoneDefPtr->GetName(), platformName);
      AddData(zoneDefPtr, zone);
      zone.SetFillColor(zoneDefPtr->GetFillColor());
      zone.SetLineColor(zoneDefPtr->GetLineColor());
      if (zone.GetZones().size() > 0)
      {
         AddSimEvent(ut::make_unique<NewZoneEvent>(zone));
      }
   }
   else if (zoneSetPtr)
   {
      std::string platformName = (zoneSetPtr->GetPlatform()) ? zoneSetPtr->GetPlatform()->GetName() : "Global Zones";
      wkf::ZoneSetData zoneSet(zoneSetPtr->GetName(), platformName);
      for (int i = 0; i < zoneSetPtr->GetNumZones(); ++i)
      {
         zoneDefPtr = dynamic_cast<WsfZoneDefinition*>(zoneSetPtr->GetZoneAtIndex(i));
         zoneRefPtr = dynamic_cast<WsfZoneReference*>(zoneSetPtr->GetZoneAtIndex(i));
         if (!zoneDefPtr && zoneRefPtr)
         {
            zoneDefPtr = dynamic_cast<WsfZoneDefinition*>(zoneRefPtr->GetReferencedZone());
         }

         if (zoneDefPtr)
         {
            AddData(zoneDefPtr, zoneSet);
         }
      }
      for (int i = 0; i < zoneSetPtr->GetNumExclusionZones(); i++)
      {
         zoneDefPtr = dynamic_cast<WsfZoneDefinition*>(zoneSetPtr->GetExclusionZoneAtIndex(i));
         zoneRefPtr = dynamic_cast<WsfZoneReference*>(zoneSetPtr->GetExclusionZoneAtIndex(i));
         if (!zoneDefPtr && zoneRefPtr)
         {
            zoneDefPtr = dynamic_cast<WsfZoneDefinition*>(zoneRefPtr->GetReferencedZone());
         }

         if (zoneDefPtr)
         {
            AddData(zoneDefPtr, zoneSet);
         }
      }
      zoneSet.SetFillColor(zoneSetPtr->GetFillColor());
      zoneSet.SetLineColor(zoneSetPtr->GetLineColor());
      if (zoneSet.GetZones().size() > 0)
      {
         AddSimEvent(ut::make_unique<NewZoneEvent>(zoneSet));
      }
   }
   else if (zoneRefPtr)
   {
      zoneDefPtr = dynamic_cast<WsfZoneDefinition*>(zoneRefPtr->GetReferencedZone());
      if (zoneDefPtr)
      {
         std::string platformName = (zoneRefPtr->GetPlatform()) ? zoneRefPtr->GetPlatform()->GetName() : "Global Zones";
         wkf::ZoneSetData zone(zoneRefPtr->GetName(), platformName);
         AddData(zoneDefPtr, zone);
         zone.SetFillColor(zoneDefPtr->GetFillColor());
         zone.SetLineColor(zoneDefPtr->GetLineColor());
         if (zone.GetZones().size() > 0)
         {
            AddSimEvent(ut::make_unique<NewZoneEvent>(zone));
         }
      }
   }
}

void WkZoneBrowser::SimInterface::ZoneFillColorChanged(const WsfZone& aZone)
{
   if (aZone.GetFillColor().has_value())
   {
      std::string platformName = (aZone.GetPlatform()) ? aZone.GetPlatform()->GetName() : "Global Zones";
      AddSimEvent(ut::make_unique<UpdateFillColor>(platformName, aZone.GetName(), UtQtColor(aZone.GetFillColor().value())));
   }
}

void WkZoneBrowser::SimInterface::ZoneLineColorChanged(const WsfZone& aZone)
{
   if (aZone.GetLineColor().has_value())
   {
      std::string platformName = (aZone.GetPlatform()) ? aZone.GetPlatform()->GetName() : "Global Zones";
      AddSimEvent(ut::make_unique<UpdateLineColor>(platformName, aZone.GetName(), UtQtColor(aZone.GetLineColor().value())));
   }
}
