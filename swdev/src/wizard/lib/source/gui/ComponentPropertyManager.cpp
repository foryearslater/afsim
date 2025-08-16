// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2003-2015 The Boeing Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "ComponentPropertyManager.hpp"

#include <QCryptographicHash>
#include <QGraphicsSceneMouseEvent>

#include "UtRandom.hpp"
#include "WsfPProxyBasicValue.hpp"
#include "WsfPProxyObjectMap.hpp"
#include "WsfPProxyStructType.hpp"

wizard::ComponentPropertyManager* wizard::ComponentPropertyManager::sFactoryPtr = nullptr;

wizard::ComponentPropertyManager::ComponentPropertyManager()
{
   assert(sFactoryPtr == nullptr);
   sFactoryPtr = this;

   mPartIcons["WSF_TASK_PROCESSOR"]    = "checklist";
   mPartIcons["Processor"]             = "gears";
   mPartIcons["Platform"]              = "f-18";
   mPartIcons["Mover"]                 = "horse";
   mPartIcons["WSF_RADAR_SENSOR"]      = "radar";
   mPartIcons["WSF_ACOUSTIC_SENSOR"]   = "microphone";
   mPartIcons["WSF_IRST_SENSOR"]       = "video_camera";
   mPartIcons["WSF_EOIR_SENSOR"]       = "video_camera";
   mPartIcons["WSF_SOSM_SENSOR"]       = "video_camera";
   mPartIcons["WSF_OPTICAL_SENSOR"]    = "show";
   mPartIcons["Sensor"]                = "camera";
   mPartIcons["Weapon"]                = "gun";
   mPartIcons["Comm"]                  = "hand_radio";
   mPartIcons["Fuel"]                  = "gas";
   mPartIcons["WSF_EXPLICIT_WEAPON"]   = "missile";
   mPartIcons["WSF_AIR_MOVER"]         = "airplane";
   mPartIcons["WSF_BRAWLER_MOVER"]     = "airplane";
   mPartIcons["WSF_P6DOF_MOVER"]       = "airplane";
   mPartIcons["WSF_FORMATION_FLYER"]   = "airplane";
   mPartIcons["WSF_KINEMATIC_MOVER"]   = "airplane";
   mPartIcons["WSF_GROUND_MOVER"]      = "tank";
   mPartIcons["WSF_ROAD_MOVER"]        = "tank";
   mPartIcons["WSF_SURFACE_MOVER"]     = "boat";
   mPartIcons["WSF_SUBSURFACE_MOVER"]  = "submarine";
   mPartIcons["WSF_SPACE_MOVER"]       = "satellite";
   mPartIcons["WSF_NORAD_SPACE_MOVER"] = "satellite";
   mPartIcons["WSF_FIRES_MOVER"]       = "missile";
   mPartIcons["WSF_GUIDED_MOVER"]      = "missile";
   mPartIcons["WSF_PARABOLIC_MOVER"]   = "missile";
   mPartIcons["WSF_TBM_MOVER"]         = "missile";
   mPartIcons["WSF_UNGUIDED_MOVER"]    = "dropbomb";
   mPartIcons["WSF_ROTORCRAFT_MOVER"]  = "helicopter";
   mPartIcons["WSF_TOWED_MOVER"]       = "trailer";
   mPartIcons["WSF_WEAPON_FUSE"]       = "fuse";
   mPartIcons["WSF_LASER_WEAPON"]      = "laser";
   mPartIcons["WSF_CUED_LASER_WEAPON"] = "laser";
   mPartIcons["WSF_CHAFF_WEAPON"]      = "hide";
   mPartIcons["WSF_RF_JAMMER"]         = "hide";


   mSpatialDomainIcons["land"]       = "tank";
   mSpatialDomainIcons["air"]        = "airplane";
   mSpatialDomainIcons["surface"]    = "boat";
   mSpatialDomainIcons["subsurface"] = "submarine";
   mSpatialDomainIcons["space"]      = "satellite";

   mSideColors["red"]        = QColor("#FF3031");
   mSideColors["blue"]       = QColor("#00A8DC");
   mSideColors["green"]      = QColor("#00E200");
   mSideColors["yellow"]     = QColor("#FFFF00");
   mSideColors["orange"]     = QColor("#FF4F00");
   mSideColors["indigo"]     = QColor("#4A009F");
   mSideColors["violet"]     = QColor("#C080FF");
   mSideColors["purple"]     = QColor("#6000C0");
   mSideColors["gray"]       = QColor("#666666");
   mSideColors["tan"]        = QColor("#B68538");
   mSideColors["dark_green"] = QColor("#004D00");
   mSideColors["magenta"]    = QColor("#FF00FF");
   mSideColors["brown"]      = QColor("#3D2100");
}


QString wizard::ComponentPropertyManager::FindPartIcon(WsfPProxyValue aValue)
{
   if (aValue.IsStruct())
   {
      WsfPProxyStructType* structPtr = (WsfPProxyStructType*)aValue.GetType();
      while (structPtr)
      {
         // Pick the platform icon based on spaital domain or mover icon
         if (structPtr->mTypeName == "Platform")
         {
            WsfPProxyBasicValue domainVal(aValue["spatialDomain"]);
            if (!domainVal.IsUnset())
            {
               QString domain = QString::fromStdString(domainVal.ToString());
               if (mSpatialDomainIcons.find(domain) != mSpatialDomainIcons.end())
                  return mSpatialDomainIcons[domain];
            }
            WsfPProxyStructValue mover = aValue["movers"]["mover"];
            if (mover.IsValid() && !mover.IsUnset())
            {
               QString moverIcon = FindPartIcon(mover);
               if (moverIcon != mPartIcons["Mover"])
                  return moverIcon;
            }
            if (!aValue["weapons"].GetObjectMap()->GetValues().empty())
            {
               return "cannon";
            }
            else if (!aValue["sensors"].GetObjectMap()->GetValues().empty())
            {
               return "radar";
            }
            /*            else if (!aValue["comms"].GetObjectMap()->GetValues().empty())
                        {
                           return "radio_tower";
                        }*/ // couldn't get a good icon, here
            return "box_house";
            // return mPartIcons[structPtr->mTypeName];
         }

         if (mPartIcons.find(structPtr->mTypeName) != mPartIcons.end())
         {
            return mPartIcons[structPtr->mTypeName];
         }
         structPtr = structPtr->mBasePtr;
      }
   }
   return "";
}

void wizard::ComponentPropertyManager::CreateInstance()
{
   new ComponentPropertyManager;
}

void wizard::ComponentPropertyManager::DestroyInstance()
{
   delete sFactoryPtr;
   sFactoryPtr = nullptr;
}
