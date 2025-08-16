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

#include "Zone.hpp"

#include "AttachmentZoneCircle.hpp"
#include "AttachmentZoneEllipse.hpp"
#include "AttachmentZonePolygon.hpp"
#include "AttachmentZoneSphere.hpp"
#include "Environment.hpp"
#include "Project.hpp"
#include "ProxyChange.hpp"
#include "ProxyWatcher.hpp"
#include "UtColor.hpp"
#include "UtCompilerAttributes.hpp"
#include "UtEllipsoidalEarth.hpp"
#include "UtOptional.hpp"
#include "VaObserver.hpp"
#include "VaUtils.hpp"
#include "WkfScenario.hpp"
#include "WkfViewer.hpp"
#include "WkfVtkEnvironment.hpp"
#include "ZoneSet.hpp"
#include "zone/WkfAttachmentZoneCircle.hpp"
#include "zone/WkfAttachmentZoneEllipse.hpp"
#include "zone/WkfAttachmentZonePolygon.hpp"
#include "zone/WkfZonePoint.hpp"

// wizard::Zone::Zone(WsfPM_Zone& aZoneNode)
//                   : mZoneProxyNode(aZoneNode)
//{
//    AddZone(aZoneNode);
// }

wizard::Zone::Zone(WsfPM_ZoneDefinition& aZoneNode, ZoneSet* aOwningZoneSetPtr, int aZoneSetIndex)
   : mZoneDefName(aZoneNode.GetName())
   , mZoneDefParentPlatName(aZoneNode.ParentPlatform())
   , mOwningZoneSetPtr(aOwningZoneSetPtr)
   , mZoneSetIndex(aZoneSetIndex)
   , mZoneProxyNode(aZoneNode)

{
   AddZone(aZoneNode);
}

void wizard::Zone::SetCircularZoneData(wizard::AttachmentZoneCircle* aZonePtr, const WsfPM_ZoneDefinition& aZoneDef)
{
   if (!aZonePtr->IsInitialized())
   {
      aZonePtr->Initialize();
      aZonePtr->Load();
   }
   aZonePtr->SetStateVisibility(false);
   aZonePtr->SetAuxData(aZoneDef.AuxData());

   if (mAnchorPtr)
   {
      std::string refPlatform    = aZoneDef.ReferencePlatform();
      bool        hasRefPlatform = !refPlatform.empty();
      aZonePtr->SetRelative(hasRefPlatform);
      mAnchorPtr->SetDraggable(!hasRefPlatform);

      auto   minAlt     = aZoneDef.MinimumAltitude();
      auto   maxAlt     = aZoneDef.MaximumAltitude();
      double maxRadius  = aZoneDef.MaximumRadius().GetValue();
      double minRadius  = aZoneDef.MinimumRadius().GetValue();
      double startAngle = aZoneDef.StartAngle().GetValue();
      double stopAngle  = aZoneDef.StopAngle().GetValue();

      if (maxAlt.IsUnset())
      {
         maxAlt = minAlt;
      }

      aZonePtr->SetBaseHeight(static_cast<double>(minAlt.GetValue()));
      aZonePtr->SetHeight(maxAlt.GetValue() - minAlt.GetValue());
      aZonePtr->SetRadius(maxRadius);
      aZonePtr->SetMinRadius(minRadius);
      aZonePtr->SetBegAngle(UtUnitAngle::ConvertFromStandard(startAngle, UtUnitAngle::cDEGREES));
      aZonePtr->SetEndAngle(UtUnitAngle::ConvertFromStandard(stopAngle, UtUnitAngle::cDEGREES));
   }
}

void wizard::Zone::SetEllipticalZoneData(wizard::AttachmentZoneEllipse* aZonePtr, const WsfPM_ZoneDefinition& aZoneDef)
{
   if (!aZonePtr->IsInitialized())
   {
      aZonePtr->Initialize();
      aZonePtr->Load();
   }
   aZonePtr->SetStateVisibility(false);
   aZonePtr->SetAuxData(aZoneDef.AuxData());

   if (mAnchorPtr)
   {
      std::string refPlatform    = aZoneDef.ReferencePlatform();
      bool        hasRefPlatform = !refPlatform.empty();
      aZonePtr->SetRelative(hasRefPlatform);
      mAnchorPtr->SetDraggable(!hasRefPlatform);

      auto   minAlt           = aZoneDef.MinimumAltitude();
      auto   maxAlt           = aZoneDef.MaximumAltitude();
      double maxRadius        = aZoneDef.MaximumRadius().GetValue();
      double lateralAxis      = aZoneDef.LateralAxis().GetValue();
      double longitudinalAxis = aZoneDef.LongitudinalAxis().GetValue();
      double minRadius        = aZoneDef.MinimumRadius().GetValue();
      double startAngle       = aZoneDef.StartAngle().GetValue();
      double stopAngle        = aZoneDef.StopAngle().GetValue();

      if (maxAlt.IsUnset())
      {
         maxAlt = minAlt;
      }

      aZonePtr->SetBaseHeight(static_cast<double>(minAlt.GetValue()));
      aZonePtr->SetHeight(maxAlt.GetValue() - minAlt.GetValue());
      aZonePtr->SetRadius(maxRadius);
      aZonePtr->SetLatAxisRadius(lateralAxis / 2);
      aZonePtr->SetLonAxisRadius(longitudinalAxis / 2);
      aZonePtr->SetMinRadius(minRadius);
      aZonePtr->SetBegAngle(UtUnitAngle::ConvertFromStandard(startAngle, UtUnitAngle::cDEGREES));
      aZonePtr->SetEndAngle(UtUnitAngle::ConvertFromStandard(stopAngle, UtUnitAngle::cDEGREES));
   }
}

void wizard::Zone::SetSphericalZoneData(wizard::AttachmentZoneSphere* aZonePtr, const WsfPM_ZoneDefinition& aZoneDef)
{
   if (!aZonePtr->IsInitialized())
   {
      aZonePtr->Initialize();
      aZonePtr->Load();
   }
   aZonePtr->SetStateVisibility(false);
   aZonePtr->SetAuxData(aZoneDef.AuxData());

   if (mAnchorPtr)
   {
      auto refPlatform    = aZoneDef.ReferencePlatform();
      bool hasRefPlatform = !refPlatform.empty();
      aZonePtr->SetRelative(hasRefPlatform);
      mAnchorPtr->SetDraggable(!hasRefPlatform);

      auto   minAlt     = aZoneDef.MinimumAltitude();
      auto   maxAlt     = aZoneDef.MaximumAltitude();
      double maxRadius  = aZoneDef.MaximumRadius().GetValue();
      double minRadius  = aZoneDef.MinimumRadius().GetValue();
      double startAngle = aZoneDef.StartAngle().GetValue();
      double stopAngle  = aZoneDef.StopAngle().GetValue();

      if (!minAlt.IsUnset())
      {
         aZonePtr->SetMinAltitudeSet(true);
         aZonePtr->SetMinAltitude(static_cast<double>(minAlt.GetValue()));
      }
      if (!maxAlt.IsUnset())
      {
         aZonePtr->SetMaxAltitudeSet(true);
         aZonePtr->SetMaxAltitude(static_cast<double>(maxAlt.GetValue()));
      }

      aZonePtr->SetRadius(maxRadius);
      aZonePtr->SetMinRadius(minRadius);
      aZonePtr->SetBegAngle(UtUnitAngle::ConvertFromStandard(startAngle, UtUnitAngle::cDEGREES));
      aZonePtr->SetEndAngle(UtUnitAngle::ConvertFromStandard(stopAngle, UtUnitAngle::cDEGREES));
   }
}

void wizard::Zone::SetPolygonalZoneData(wizard::AttachmentZonePolygon* aZonePtr, const WsfPM_ZoneDefinition& aZoneDef)
{
   if (!aZonePtr->IsInitialized())
   {
      aZonePtr->Initialize();
      aZonePtr->Load();
   }
   aZonePtr->SetStateVisibility(false);
   aZonePtr->SetAuxData(aZoneDef.AuxData());

   if (mAnchorPtr)
   {
      bool        isLatLon       = aZoneDef.UseLatLon();
      bool        isMGRS         = aZoneDef.UseMGRS();
      std::string refPlatform    = aZoneDef.ReferencePlatform();
      std::string refZoneName    = aZoneDef.ReferenceZone();
      bool        hasRefPlatform = !refPlatform.empty();
      bool        isAbsolute     = aZoneDef.IsAbsolute();

      auto minAlt = aZoneDef.MinimumAltitude();
      auto maxAlt = aZoneDef.MaximumAltitude();
      if (maxAlt.IsUnset())
      {
         maxAlt = minAlt;
      }

      ut::optional<WsfPM_Zone::Point> optAnchorPt = aZoneDef.ReferencePoint();

      if (IsGlobal())
      {
         vespa::VaPosition position;
         if (hasRefPlatform)
         {
            auto refEnt = vaEnv.FindEntity(refPlatform);
            if (refEnt != nullptr)
            {
               auto           proxyPtr = wizard::ProxyWatcher::GetActiveProxy();
               WsfPM_Root     rootNode{proxyPtr};
               WsfPM_Platform proxyPlatform = rootNode.platforms() + refPlatform;
               if (proxyPlatform.IsValid())
               {
                  auto pos = proxyPlatform.Position();
                  mAnchorPtr->SetPosition(
                     vespa::VaPosition(pos.GetLatitude(), pos.GetLongitude(), proxyPlatform.Altitude().GetValue()));
                  position = vespa::VaPosition(pos.GetLatitude(), pos.GetLongitude(), proxyPlatform.Altitude().GetValue());
               }
               else
               {
                  position = refEnt->GetPosition();
               }
            }
         }
         else
         {
            if (isLatLon || isMGRS) // (absolute coords)
            {
               position.SetAlt(minAlt.GetValue());
               if (optAnchorPt)
               {
                  WsfPM_Zone::Point anchorPt = optAnchorPt.value();
                  position.SetLat(anchorPt[0]);
                  position.SetLon(anchorPt[1]);
               }
            }
            else // is polar/offset (relative coords)
            {
               position.SetAlt(-minAlt.GetValue());
               if (optAnchorPt)
               {
                  WsfPM_Zone::Point anchorPt = optAnchorPt.value();
                  position.SetX(anchorPt[0]);
                  position.SetY(anchorPt[1]);
               }
            }
         }
         // move the anchor entity
         mAnchorPtr->SetPosition(position);
      }

      if (isAbsolute) // (absolute coords)
      {
         aZonePtr->SetRelative(false);
         aZonePtr->SetBaseHeight(static_cast<double>(minAlt.GetValue()));
         aZonePtr->SetHeight(maxAlt.GetValue() - minAlt.GetValue());
         mAnchorPtr->SetDraggable(true);
      }
      else // is polar/offset (relative coords)
      {
         // Altitude and height are negative for XY due to differences in frame of reference
         aZonePtr->SetRelative(true);
         aZonePtr->SetBaseHeight(-static_cast<double>(minAlt.GetValue()));
         aZonePtr->SetHeight(-(maxAlt.GetValue() - minAlt.GetValue()));
         mAnchorPtr->SetDraggable(false);
      }

      // delete all of the zone points and re-add the new proxy values
      auto pointList = aZonePtr->GetZonePoints();
      for (const auto& curPoint : pointList)
      {
         aZonePtr->RemoveZonePoint(curPoint->GetUniqueId());
      }

      std::vector<WsfPM_Zone::Point> pointsVec = aZoneDef.Points();
      for (const auto& point : pointsVec)
      {
         vespa::VaPosition pos;

         if (isAbsolute)
         {
            pos.SetLat(point[0]);
            pos.SetLon(point[1]);
            pos.SetAlt(static_cast<double>(minAlt.GetValue()));
            pos.SetPositionIsXY(false);
         }
         else
         {
            pos.SetX(-point[0]);
            pos.SetY(point[1]);
            pos.SetAlt(-static_cast<double>(minAlt.GetValue()));
            pos.SetPositionIsXY(true);
         }

         auto zonePointPtr = new wkf::ZonePoint(*aZonePtr, pos);
         aZonePtr->AddZonePoint(zonePointPtr, vaEnv.GetStandardViewer());
      }

      aZonePtr->SetClosed(true);
      aZonePtr->SetHideZonePoints(true);
      aZonePtr->SetNeverShowZones(false);
   }
}

// Whenever the proxy zone map changes, update zones based on the change.
void wizard::Zone::OnProxyModified(const ProxyChange& aProxyChange)
{
   switch (aProxyChange.reason())
   {
   case wizard::ProxyChange::Reason::cUPDATED:
   {
      auto       node = aProxyChange.changed();
      WsfPM_Root root = node.Root();
      auto       type = node.GetTypeName();
      while (type.empty() && node.IsValid())
      {
         node = node.GetParent();
         type = node.GetTypeName();
         if (node == root)
         {
            return;
         }
      }
      if (type == "WSF_ZONE_DEFINITION")
      {
         WsfPM_ZoneDefinition updateZoneDef{node};
         UpdateZone(updateZoneDef);
      }
      break;
   }
   case wizard::ProxyChange::Reason::cADDED:
      CPP17_FALLTHROUGH
   case wizard::ProxyChange::Reason::cLIST_DELETION:
      CPP17_FALLTHROUGH
   case wizard::ProxyChange::Reason::cLIST_INSERTION:
      CPP17_FALLTHROUGH
   case wizard::ProxyChange::Reason::cOPTIONS_CHANGED:
      CPP17_FALLTHROUGH
   case wizard::ProxyChange::Reason::cREMOVED:
      CPP17_FALLTHROUGH
   case wizard::ProxyChange::Reason::cRENAMED:
      CPP17_FALLTHROUGH
   case wizard::ProxyChange::Reason::cUNKNOWN:
      break;
   }
}

void wizard::Zone::AddZone(WsfPM_ZoneDefinition& aZoneDef)
{
   if (!vaEnv.GetStandardScenario())
   {
      wkf::Scenario* scenario = (wkf::Scenario*)vaEnv.NewScenario("wkf");
      vaEnv.SetStandardScenario(scenario);
   }

   const std::string parentPlatName = mOwningZoneSetPtr->GetParentPlatformName();
   const std::string zoneSetName    = mOwningZoneSetPtr->GetZoneSetName();

   if (vaEnv.GetStandardScenario() && vaEnv.GetStandardViewer())
   {
      if (aZoneDef.IsDefinition())
      {
         std::string zoneAnchorName;

         if (parentPlatName != "Global Zones" && vaEnv.FindEntity(parentPlatName))
         {
            zoneAnchorName = parentPlatName + "_" + zoneSetName + "_" + std::to_string(mZoneSetIndex);
         }
         else
         {
            zoneAnchorName = "Global Zones_" + zoneSetName + "_" + std::to_string(mZoneSetIndex);
            mIsGlobal      = true;
         }

         auto anchorEnt = vaEnv.FindEntity(zoneAnchorName);
         if (!anchorEnt)
         {
            mAnchorPtr = new vespa::VaEntity(zoneAnchorName);
            if (vaEnv.GetStandardScenario()->AddEntity(mAnchorPtr))
            {
               mAnchorPtr->Initialize();
            }
         }
         else
         {
            mAnchorPtr = anchorEnt;
         }

         if (mAnchorPtr)
         {
            auto zoneType = aZoneDef.GetDefinitionType();
            if (zoneType == WsfPM_ZoneDefinition::cTYPE_NOT_SET)
            {
               // if the type is not set due to lack of name other circumstance attempt to figure out what the zonetype
               // is based on other inputs just as core does...
               if (!aZoneDef.Points().empty())
               {
                  // points were listed, set the type to polygonal
                  zoneType = WsfPM_ZoneDefinition::cPOLYGONAL;
               }
               else if (!aZoneDef.LateralAxis().IsUnset() || !aZoneDef.LongitudinalAxis().IsUnset())
               {
                  // lateral or longitudinal axes are set
                  zoneType = WsfPM_ZoneDefinition::cELLIPTICAL;
               }
            }
            if (zoneType == WsfPM_ZoneDefinition::cCIRCULAR) // || (zoneType == WsfPM_ZoneDefinition::cTYPE_NOT_SET &&
                                                             // zoneSetName == "circular"))
            {
               auto circleZonePtr =
                  vespa::make_attachment<wizard::AttachmentZoneCircle>(*mAnchorPtr, vaEnv.GetStandardViewer());
               SetCircularZoneData(circleZonePtr, aZoneDef);
               mZonePtr = circleZonePtr;
            }
            else if (zoneType == WsfPM_ZoneDefinition::cELLIPTICAL) // || (zoneType == WsfPM_ZoneDefinition::cTYPE_NOT_SET
                                                                    // && zoneSetName == "elliptical"))
            {
               auto ellipseZonePtr =
                  vespa::make_attachment<wizard::AttachmentZoneEllipse>(*mAnchorPtr, vaEnv.GetStandardViewer());
               SetEllipticalZoneData(ellipseZonePtr, aZoneDef);
               mZonePtr = ellipseZonePtr;
            }
            else if (zoneType == WsfPM_ZoneDefinition::cSPHERICAL)
            {
               auto sphereZonePtr =
                  vespa::make_attachment<wizard::AttachmentZoneSphere>(*mAnchorPtr, vaEnv.GetStandardViewer());
               SetSphericalZoneData(sphereZonePtr, aZoneDef);
               mZonePtr = sphereZonePtr;
            }
            else if (zoneType == WsfPM_ZoneDefinition::cPOLYGONAL) // || (zoneType == WsfPM_ZoneDefinition::cTYPE_NOT_SET
                                                                   // && zoneSetName == "polygonal"))
            {
               auto polygonZonePtr =
                  vespa::make_attachment<wizard::AttachmentZonePolygon>(*mAnchorPtr, vaEnv.GetStandardViewer());
               SetPolygonalZoneData(polygonZonePtr, aZoneDef);
               mZonePtr = polygonZonePtr;
            }
            else
            {
               // create a generic zone
               auto zonePtr = vespa::make_attachment<wkf::AttachmentZone>(*mAnchorPtr, vaEnv.GetStandardViewer());
               zonePtr->SetStateVisibility(false);
               mZonePtr = zonePtr;
            }

            // set the position of the zone
            mZonePtr->SetName(zoneSetName);
            SetZonePositionAndHeading(aZoneDef);
            SetZoneColors(aZoneDef);
         }
      } // if (aZone.IsDefinition())
   }    // if (vaEnv.GetStandardScenario())
}

void wizard::Zone::SetZoneColors(const WsfPM_ZoneDefinition& aZoneDefinition)
{
   auto fillColor = aZoneDefinition.GetFillColor();
   if (fillColor.has_value())
   {
      mZonePtr->SetFillColor(fillColor.value());
   }

   auto lineColor = aZoneDefinition.GetLineColor();
   if (lineColor.has_value())
   {
      mZonePtr->SetColor(lineColor.value());
   }
}

void wizard::Zone::SetZonePositionAndHeading(const WsfPM_ZoneDefinition& aZoneDefinition)
{
   double            heading;
   vespa::VaPosition position;
   std::string       refPlatName    = aZoneDefinition.ReferencePlatform();
   std::string       parentPlatName = aZoneDefinition.ParentPlatform();

   vespa::VaEntity* entity = nullptr;
   if (!refPlatName.empty())
   {
      entity = vaEnv.FindEntity(refPlatName);
   }
   else if (!parentPlatName.empty())
   {
      entity = vaEnv.FindEntity(parentPlatName);
   }

   // position: try proxy lookup
   WsfProxy::Position pos = aZoneDefinition.Position();
   if (!pos.IsUnset())
   {
      position.SetLat(pos.GetLatitude());
      position.SetLon(pos.GetLongitude());

      if (aZoneDefinition.GetDefinitionType() == WsfPM_ZoneDefinition::cSPHERICAL)
      {
         WsfProxy::Length alt = aZoneDefinition.AnchorPlatformAltitude();
         position.SetAlt(alt.IsUnset() ? 0 : static_cast<double>(alt.GetValue()));
      }
      else
      {
         position.SetAlt(0);
      }
   }
   // position: try entity lookup
   else if (entity)
   {
      position = entity->GetPosition();
      position.SetAlt(0);
   }
   // position: fall back to reference point
   else
   {
      ut::optional<WsfPM_Zone::Point> optAnchorPt = aZoneDefinition.ReferencePoint();
      if (optAnchorPt)
      {
         WsfPM_Zone::Point anchorPt = optAnchorPt.value();
         position.SetLat(anchorPt[0]);
         position.SetLon(anchorPt[1]);
      }

      position.SetAlt(0);
   }

   // heading: try proxy lookup
   heading = aZoneDefinition.Heading().GetValue();

   mAnchorPtr->SetPosition(position);
   mAnchorPtr->SetHeading(UtUnitAngle::ConvertFromStandard(heading, UtUnitAngle::cRADIANS));
}

void wizard::Zone::RemoveZone()
{
   if (mAnchorPtr && mZonePtr)
   {
      vespa::VaObserver::AttachmentDeleted(mZonePtr, 0);
      mAnchorPtr->RemoveAttachment(mZonePtr->GetUniqueId());
      vaEnv.DeleteEntity(mAnchorPtr);
      mAnchorPtr = nullptr;
      UnlinkProxyCallbacks();
      mZonePtr = nullptr;
   }
}

void wizard::Zone::UpdateZone(WsfPM_ZoneDefinition& aZoneDef)
{
   if (mZonePtr)
   {
      std::string       parentPlatName     = aZoneDef.ParentPlatform();
      const std::string prevParentPlatName = mOwningZoneSetPtr->GetParentPlatformName();

      // if previously there was an attached platform but now the zone has been moved out into a global space:
      // remove the attachment from the current reference platform
      // create a global anchor entity for this zone to attach to
      if (!IsGlobal() && parentPlatName == "Global Zones")
      {
         if (mZonePtr)
         {
            vespa::VaObserver::AttachmentDeleted(mZonePtr, 0);
            mAnchorPtr->RemoveAttachment(mZonePtr->GetUniqueId());
            mAnchorPtr = nullptr;
            mZonePtr   = nullptr;
         }
         AddZone(aZoneDef);
         return;
      }

      // if there was previously no attached platform (i.e. this is was global zone) but now has been added to a
      // reference platform: remove the attachment from the global entity and delete the global entity attach the zone
      // to the reference platform
      else if (IsGlobal() && parentPlatName != "Global Zones")
      {
         if (mZonePtr)
         {
            vespa::VaObserver::AttachmentDeleted(mZonePtr, 0);
            mAnchorPtr->RemoveAttachment(mZonePtr->GetUniqueId());
            vaEnv.DeleteEntity(mAnchorPtr);
            mAnchorPtr = nullptr;
            mZonePtr   = nullptr;
         }
         AddZone(aZoneDef);
         return;
      }

      else if (!IsGlobal() && (prevParentPlatName != parentPlatName))
      {
         // changing from one attached platform to a different one...
         if (mZonePtr)
         {
            vespa::VaObserver::AttachmentDeleted(mZonePtr, 0);
            mAnchorPtr->RemoveAttachment(mZonePtr->GetUniqueId());
            mAnchorPtr = nullptr;
            mZonePtr   = nullptr;
         }
         vespa::VaEntity* parentEnt = vaEnv.FindEntity(parentPlatName);
         if (parentEnt)
         {
            AddZone(aZoneDef);
         }
         return;
      }

      auto parentEnt = vaEnv.FindEntity(prevParentPlatName);
      if (parentEnt != nullptr)
      {
         auto zoneType = aZoneDef.GetDefinitionType();
         if (zoneType == WsfPM_ZoneDefinition::cTYPE_NOT_SET)
         {
            // if the type is not set due to lack of name other circumstance attempt to figure out what the zonetype is
            // based on other inputs just as core does...
            if (!aZoneDef.Points().empty())
            {
               // points were listed, set the type to polygonal
               zoneType = WsfPM_ZoneDefinition::cPOLYGONAL;
            }
            else if (!aZoneDef.LateralAxis().IsUnset() || !aZoneDef.LongitudinalAxis().IsUnset())
            {
               // lateral or longitudinal axes are set
               zoneType = WsfPM_ZoneDefinition::cELLIPTICAL;
            }
         }

         if (zoneType == WsfPM_ZoneDefinition::cCIRCULAR)
         {
            auto* zoneAttachmentPtr = dynamic_cast<wizard::AttachmentZoneCircle*>(mZonePtr);
            if (zoneAttachmentPtr != nullptr)
            {
               bool isVis = mZonePtr->GetStateVisibility();
               SetCircularZoneData(zoneAttachmentPtr, aZoneDef);
               zoneAttachmentPtr->SetStateVisibility(isVis);
            }
            else // changing from one zone type to a different one
            {
               // delete the zone and add new
               RemoveZone();
               AddZone(aZoneDef);
            }
         }

         if (zoneType == WsfPM_ZoneDefinition::cELLIPTICAL)
         {
            auto zoneAttachmentPtr = dynamic_cast<wizard::AttachmentZoneEllipse*>(mZonePtr);
            if (zoneAttachmentPtr != nullptr)
            {
               bool isVis = mZonePtr->GetStateVisibility();
               SetEllipticalZoneData(zoneAttachmentPtr, aZoneDef);
               zoneAttachmentPtr->SetStateVisibility(isVis);
            }
            else // changing from one zone type to a different one
            {
               // delete the zone and add new
               RemoveZone();
               AddZone(aZoneDef);
            }
         }

         if (zoneType == WsfPM_ZoneDefinition::cSPHERICAL)
         {
            auto* zoneAttachmentPtr = dynamic_cast<wizard::AttachmentZoneSphere*>(mZonePtr);
            if (zoneAttachmentPtr != nullptr)
            {
               bool isVis = mZonePtr->GetStateVisibility();
               SetSphericalZoneData(zoneAttachmentPtr, aZoneDef);
               zoneAttachmentPtr->SetStateVisibility(isVis);
            }
            else // changing from one zone type to a different one
            {
               // delete the zone and add new
               RemoveZone();
               AddZone(aZoneDef);
            }
         }

         else if (zoneType == WsfPM_ZoneDefinition::cPOLYGONAL)
         {
            auto zoneAttachmentPtr = dynamic_cast<wizard::AttachmentZonePolygon*>(mZonePtr);
            if (zoneAttachmentPtr != nullptr)
            {
               bool isVis = mZonePtr->GetStateVisibility();
               SetPolygonalZoneData(zoneAttachmentPtr, aZoneDef);
               zoneAttachmentPtr->SetStateVisibility(isVis);
            }
            else // changing from one zone type to a different one
            {
               // delete the zone and add new
               RemoveZone();
               AddZone(aZoneDef);
            }
         }
         else if (zoneType == WsfPM_ZoneDefinition::cTYPE_NOT_SET)
         {
            // delete the zone and add new
            RemoveZone();
            AddZone(aZoneDef);
         }
         SetZonePositionAndHeading(aZoneDef);
      }
   }
   else
   {
      std::string refPlatform = aZoneDef.ReferencePlatform();
      if (refPlatform.empty())
      {
         refPlatform = aZoneDef.GetParent().GetParent().GetName();
      }
      auto parentEnt = vaEnv.FindEntity(refPlatform);
      if (parentEnt != nullptr)
      {
         AddZone(aZoneDef);
      }
   }
}

void wizard::Zone::LinkProxyCallbacks()
{
   auto updateCallbackPtr = wizard::WatchProxy(mZoneProxyNode.GetPath()).AnyUpdate.Connect(&Zone::OnProxyModified, this);
   mUpdateCallbackPtr     = updateCallbackPtr.get();
   mProxyCallbacks.Add(std::move(updateCallbackPtr));
}

void wizard::Zone::UnlinkProxyCallbacks()
{
   mProxyCallbacks.Remove(mUpdateCallbackPtr);
   mUpdateCallbackPtr = nullptr;
}

const std::string wizard::Zone::GetZoneSetName() const
{
   return mOwningZoneSetPtr->GetZoneSetName();
}

const std::string wizard::Zone::GetParentPlatformName() const
{
   return mOwningZoneSetPtr->GetParentPlatformName();
}
