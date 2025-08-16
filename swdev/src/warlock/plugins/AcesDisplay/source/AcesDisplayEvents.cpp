// ****************************************************************************
// CUI//REL TO USA ONLY
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2021 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "AcesDisplayEvents.hpp"

void WkAcesDisplay::UpdateAcesDataEvent::Process(wkf::AcesDisplay::DataContainer& aData)
{
   aData.SetAcesData(mData);
}

void WkAcesDisplay::SimulationInitializingEvent::Process(wkf::AcesDisplay::DataContainer& aData)
{
   aData.GetAcesTruthPlatforms().clear();
   aData.SetSimulationInitialized(true);
}

void WkAcesDisplay::CommentAddEvent::Process(wkf::AcesDisplay::DataContainer& aData)
{
   aData.AddPlatformComment(mPlatformIndex, mCreationTime, mMessage);
}

void WkAcesDisplay::PlatformWithSAP_AddedEvent::Process(wkf::AcesDisplay::DataContainer& aData)
{
   aData.AddPlatformWithSAP(mPlatformIndex);
}

void WkAcesDisplay::PlatformAddedEvent::Process(wkf::AcesDisplay::DataContainer& aData)
{
   // Create an empty object for the platform
   wkf::AcesDisplay::EntityTruth entity;

   // Fill in the general data needed to draw here.
   double lat, lon, alt;
   mPlatform.GetLocationLLA(lat, lon, alt);
   entity.lat_deg = lat;
   entity.lon_deg = lon;

   double heading_rad, dummypitch, dummyroll;
   mPlatform.GetOrientationNED(heading_rad, dummypitch, dummyroll);
   entity.heading_deg = heading_rad * UtMath::cDEG_PER_RAD;
   entity.altitude_ft = mPlatform.GetAltitude() * UtMath::cFT_PER_M;
   entity.speed_kts   = mPlatform.GetSpeed() * UtMath::cNMPH_PER_MPS;

   entity.index = mPlatform.GetIndex();
   entity.name  = mPlatform.GetName();
   entity.type  = mPlatform.GetType();

   // Get the domain.
   switch (mPlatform.GetSpatialDomain())
   {
   default:
   case WSF_SPATIAL_DOMAIN_UNKNOWN:
      entity.domain = "unknown";
      break;
   case WSF_SPATIAL_DOMAIN_LAND:
      entity.domain = "land";
      break;
   case WSF_SPATIAL_DOMAIN_AIR:
      entity.domain = "air";
      break;
   case WSF_SPATIAL_DOMAIN_SURFACE:
      entity.domain = "surface";
      break;
   case WSF_SPATIAL_DOMAIN_SUBSURFACE:
      entity.domain = "subsurface";
      break;
   case WSF_SPATIAL_DOMAIN_SPACE:
      entity.domain = "space";
      break;
   }

   // Set missile flag to false, but it may become true below
   entity.isMissile = false;

   WsfCategoryList catList = mPlatform.GetCategories();
   if (catList.IsCategoryMember(WsfStringId("missile")))
   {
      entity.isMissile = true;
   }

   entity.entityValid   = true;
   entity.altitudeValid = true;
   entity.speedValid    = true;
   entity.headingValid  = true;

   aData.AddEntityTruth(entity);
}


void WkAcesDisplay::PlatformDeletedEvent::Process(wkf::AcesDisplay::DataContainer& aData)
{
   aData.RemoveEntityTruth(mPlatform.GetIndex());
}
