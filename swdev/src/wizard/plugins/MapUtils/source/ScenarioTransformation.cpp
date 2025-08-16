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

#include "ScenarioTransformation.hpp"

#include "ScenarioTransformationIO_Types.hpp"
#include "UtEllipsoidalEarth.hpp"
#include "UtLLAPos.hpp"
#include "UtSphericalEarth.hpp"
#include "VaEntity.hpp"
#include "WkfEnvironment.hpp"
#include "WkfScenario.hpp"
#include "WkfTeamVisibilityPrefObject.hpp"
#include "WkfUnitsObject.hpp"
#include "WkfVtkEnvironment.hpp"
#include "route/WkfAttachmentRoute.hpp"
#include "zone/WkfAttachmentZoneCircle.hpp"
#include "zone/WkfAttachmentZoneEllipse.hpp"
#include "zone/WkfAttachmentZonePolygon.hpp"
#include "zone/WkfZonePoint.hpp"

namespace MapUtils
{
namespace ScenarioTransformation
{

UtLLAPos DistanceToPosition(const UtVec2d& aXY_Distance, const vespa::VaPosition& aCenterRef, const ExtrapolationMode aMode)
{
   UtLLAPos oldCentroid;
   aCenterRef.GetLLA(oldCentroid);
   UtLLAPos newCentroid;
   // latitudinal heading has to be zero
   // longitudinal heading has to be 90
   // thus the atan2 arguments are flipped
   // atan2 returns in radians, so we need to convert to degrees
   UtAngleValue pseudoHeading{std::atan2(aXY_Distance[0], aXY_Distance[1]), 0};
   double       pseudoHeadingInDegrees = pseudoHeading.GetAsUnit(UtUnitAngle::BaseUnit::cDEGREES);
   double       newLat;
   double       newLon;
   if (aMode == ExtrapolationMode::cGREAT_CIRCLE)
   {
      UtSphericalEarth::ExtrapolateGreatCirclePosition(oldCentroid.GetLat(),
                                                       oldCentroid.GetLon(),
                                                       pseudoHeadingInDegrees,
                                                       aXY_Distance.Magnitude(),
                                                       newLat,
                                                       newLon);
   }
   else if (aMode == ExtrapolationMode::cGREAT_ELLIPSE)
   {
      UtEllipsoidalEarth::ExtrapolateGreatEllipsePosition(oldCentroid.GetLat(),
                                                          oldCentroid.GetLon(),
                                                          pseudoHeadingInDegrees,
                                                          aXY_Distance.Magnitude(),
                                                          newLat,
                                                          newLon);
   }
   newCentroid.SetLat({newLat, wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetLatLonFormatting()});
   newCentroid.SetLon({newLon, wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetLatLonFormatting()});
   return newCentroid;
}

bool Translate(const UtLLAPos&        aOldRefLLA,
               const UtLLAPos&        aNewRefLLA,
               vespa::VaPosition*     aPositionPtr,
               const TranslationMode& aMode)
{
   // do not perform the translation without a valid position
   if (aPositionPtr == nullptr)
   {
      return false;
   }
   // this mode preserves relative LLA between positions
   if (aMode == TranslationMode::cRELATIVE_LLA)
   {
      // get the old position's LLA
      UtLLAPos oldPosLLA;
      aPositionPtr->GetLLA(oldPosLLA);
      // compute the difference between the old centroid's LLA and the new centroid's LLA
      UtLLAPos diffLLA = aNewRefLLA;
      diffLLA -= aOldRefLLA;
      // apply the difference to the old position
      UtLLAPos newPosLLA = oldPosLLA;
      newPosLLA += diffLLA;
      // get the new position
      vespa::VaPosition newPos;
      newPos.SetLLA(newPosLLA);
      // update the old position to the new position
      *aPositionPtr = newPos;
   }
   // this mode preserves relative XYA between positions
   else if (aMode == TranslationMode::cRELATIVE_XYA)
   {
      UtVec3d distRef;
      // get the old position's LLA
      UtLLAPos oldPosLLA;
      aPositionPtr->GetLLA(oldPosLLA);
      // compute the distance from the old centroid's LLA to the old position's LLA
      vespa::VaPosition::ConvertLLAToXYA(oldPosLLA, aOldRefLLA, distRef);
      // apply the distance to the new centroid's LLA
      UtLLAPos newPosLLA;
      vespa::VaPosition::ConvertXYAToLLA(distRef, aNewRefLLA, newPosLLA);
      // get the new position
      vespa::VaPosition newPos;
      newPos.SetLLA(newPosLLA);
      // update the old position to the new position
      *aPositionPtr = newPos;
   }
   return true;
}

vespa::VaPosition ComputePositionOfCentroid(const vespa::VaEntityList& aSrc)
{
   std::vector<vespa::VaPosition> positionList;
   for (const auto& entity : aSrc)
   {
      positionList.push_back(entity->GetPosition());
   }
   return vespa::VaPosition::ComputeCentroid(positionList);
}

void ApplyChangesToEntity6DOF(vespa::VaEntity* aSourceEntityPtr, vespa::VaEntity* aDestinationEntityPtr)
{
   if (aSourceEntityPtr != nullptr && aDestinationEntityPtr != nullptr)
   {
      vespa::VaPosition sourcePosition      = aSourceEntityPtr->GetPosition();
      vespa::VaPosition destinationPosition = aDestinationEntityPtr->GetPosition();

      // reset the destination entity's latitude if it differs from the source entity's latitude
      UtLatPos sourceLatitude(sourcePosition.GetLat());
      UtLatPos destinationLatitude(destinationPosition.GetLat());
      if (PluginUtil::AreLatitudesDifferent(sourceLatitude, destinationLatitude))
      {
         destinationPosition.SetLat(sourceLatitude);
      }

      // reset the destination entity's longitude if it differs from the source entity's longitude
      UtLonPos sourceLongitude(sourcePosition.GetLon());
      UtLonPos destinationLongitude(destinationPosition.GetLon());
      if (PluginUtil::AreLongitudesDifferent(sourceLongitude, destinationLongitude))
      {
         destinationPosition.SetLon(sourceLongitude);
      }

      // reset the destination entity's altitude if it differs from the source entity's altitude
      UtLength sourceAltitude(sourcePosition.GetAlt());
      UtLength destinationAltitude(destinationPosition.GetAlt());
      if (PluginUtil::AreAltitudesDifferent(sourceAltitude, destinationAltitude))
      {
         destinationPosition.SetAlt(sourceAltitude);
      }

      // apply any changes to the destination entity's position
      aDestinationEntityPtr->SetPosition(destinationPosition);

      // reset the destination entity's pitch if it differs from the source entity's pitch
      double sourcePitch(aSourceEntityPtr->GetPitch());
      double destinationPitch(aDestinationEntityPtr->GetPitch());
      if (PluginUtil::ArePitchesDifferent(sourcePitch, destinationPitch))
      {
         aDestinationEntityPtr->SetPitch(sourcePitch);
      }

      // reset the destination entity's roll if it differs from the source entity's roll
      double sourceRoll(aSourceEntityPtr->GetRoll());
      double destinationRoll(aDestinationEntityPtr->GetRoll());
      if (PluginUtil::AreRollsDifferent(sourceRoll, destinationRoll))
      {
         aDestinationEntityPtr->SetRoll(sourceRoll);
      }

      // reset the destination entity's yaw if it differs from the source entity's yaw
      double sourceYaw(aSourceEntityPtr->GetHeading());
      double destinationYaw(aDestinationEntityPtr->GetHeading());
      if (PluginUtil::AreYawsDifferent(sourceYaw, destinationYaw))
      {
         aDestinationEntityPtr->SetHeading(sourceYaw);
      }
   }
}

void ApplyRouteChange(vespa::VaAttachment* aSourceRoutePtr, vespa::VaAttachment* aDestinationRoutePtr)
{
   if (aSourceRoutePtr->IsA_TypeOf<wkf::AttachmentRoute>() || aDestinationRoutePtr->IsA_TypeOf<wkf::AttachmentRoute>())
   {
      auto realRoute  = static_cast<wkf::AttachmentRoute*>(aSourceRoutePtr);
      auto ghostRoute = static_cast<wkf::AttachmentRoute*>(aDestinationRoutePtr);

      // extract the waypoints from the real and attachment routes
      std::vector<wkf::Waypoint*> realWaypoints  = realRoute->GetWaypoints();
      std::vector<wkf::Waypoint*> ghostWaypoints = ghostRoute->GetWaypoints();

      for (size_t index = 0; index < realWaypoints.size(); ++index)
      {
         // get the ghost waypoint position
         UtLLAPos ghostWaypointPosition;
         ghostWaypoints[index]->GetLLA(ghostWaypointPosition);
         // set the real waypoint position as the ghost position
         realWaypoints[index]->SetLLA(ghostWaypointPosition);
      }
   }
}

void ApplyCircleZoneChange(vespa::VaAttachment* aSourceZonePtr, vespa::VaAttachment* aDestinationZonePtr)
{
   if (aSourceZonePtr->IsA_TypeOf<wkf::AttachmentZoneCircle>() ||
       aDestinationZonePtr->IsA_TypeOf<wkf::AttachmentZoneCircle>())
   {
      auto realZoneCirclePtr  = static_cast<wkf::AttachmentZoneCircle*>(aSourceZonePtr);
      auto ghostZoneCirclePtr = static_cast<wkf::AttachmentZoneCircle*>(aDestinationZonePtr);

      // set the real zone's position as the ghost zone's position
      realZoneCirclePtr->SetX(ghostZoneCirclePtr->GetX());
      realZoneCirclePtr->SetY(ghostZoneCirclePtr->GetY());
      // reload the zone
      realZoneCirclePtr->Load();
   }
}

void ApplyEllipseZoneChange(vespa::VaAttachment* aSourceZonePtr, vespa::VaAttachment* aDestinationZonePtr)
{
   if (aSourceZonePtr->IsA_TypeOf<wkf::AttachmentZoneEllipse>() ||
       aDestinationZonePtr->IsA_TypeOf<wkf::AttachmentZoneEllipse>())
   {
      auto realZoneEllipsePtr  = static_cast<wkf::AttachmentZoneEllipse*>(aSourceZonePtr);
      auto ghostZoneEllipsePtr = static_cast<wkf::AttachmentZoneEllipse*>(aDestinationZonePtr);

      // set the real zone's position as the ghost zone's position
      realZoneEllipsePtr->SetX(ghostZoneEllipsePtr->GetX());
      realZoneEllipsePtr->SetY(ghostZoneEllipsePtr->GetY());
      realZoneEllipsePtr->SetOrientation(ghostZoneEllipsePtr->GetHeading(),
                                         ghostZoneEllipsePtr->GetPitch(),
                                         ghostZoneEllipsePtr->GetRoll());
      // reload the zone
      realZoneEllipsePtr->Load();
   }
}

void ApplyPolygonZoneChange(vespa::VaAttachment* aSourceZonePtr, vespa::VaAttachment* aDestinationZonePtr)
{
   if (aSourceZonePtr->IsA_TypeOf<wkf::AttachmentZonePolygon>() ||
       aDestinationZonePtr->IsA_TypeOf<wkf::AttachmentZonePolygon>())
   {
      auto realZonePolygonPtr  = static_cast<wkf::AttachmentZonePolygon*>(aSourceZonePtr);
      auto ghostZonePolygonPtr = static_cast<wkf::AttachmentZonePolygon*>(aDestinationZonePtr);

      auto realPointListIter  = realZonePolygonPtr->GetZonePoints().begin();
      auto ghostPointListIter = ghostZonePolygonPtr->GetZonePoints().begin();
      while (realPointListIter != realZonePolygonPtr->GetZonePoints().end() &&
             ghostPointListIter != ghostZonePolygonPtr->GetZonePoints().end())
      {
         // set each real zone point's position as the corresponding ghost zone point's position
         auto              realPoint  = *realPointListIter;
         auto              ghostPoint = *ghostPointListIter;
         vespa::VaPosition ghostPos   = ghostPoint->GetPosition();
         realPoint->SetPosition(ghostPos);
         ++realPointListIter;
         ++ghostPointListIter;
      }
      // reload the zone
      realZonePolygonPtr->Load();
   }
}

} // namespace ScenarioTransformation
} // namespace MapUtils
