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

#include "OrbitalSequenceToInput.hpp"

#include "UtStringUtil.hpp"
#include "WsfLibrationPoint.hpp"
#include "WsfOrbitalMissionSequence.hpp"
#include "WsfOrbitalReferenceFrame.hpp"
#include "WsfTargetPointOptions.hpp"

namespace SpaceTools
{
void OrbitalSequenceToInput::VisitMissionSequence(WsfOrbitalMissionSequence* aSequencePtr)
{
   const WsfOrbitalMissionSequence::MissionEvents& events = aSequencePtr->GetSequence();

   BeginSequence();
   ProduceConstraint(aSequencePtr->GetCondition());
   mStream << "\n";
   for (auto& eventPtr : events)
   {
      eventPtr->Accept(this);
      mStream << "\n";
   }
   EndSequence();
}

void OrbitalSequenceToInput::VisitChangeEccentricity(WsfOrbitalManeuvers::ChangeEccentricity* aEventPtr)
{
   BeginManeuver(aEventPtr);

   ProduceConstraint(aEventPtr->GetCondition());
   ProduceFinite(aEventPtr);

   ProduceIndent();
   mStream << "eccentricity " << aEventPtr->GetEccentricity() << "\n";

   EndManeuver();
}

void OrbitalSequenceToInput::VisitChangeInclination(WsfOrbitalManeuvers::ChangeInclination* aEventPtr)
{
   BeginManeuver(aEventPtr);

   ProduceConstraint(aEventPtr->GetCondition());
   ProduceFinite(aEventPtr);

   ProduceIndent();
   mStream << "inclination " << aEventPtr->GetInclination() << "\n";

   EndManeuver();
}

void OrbitalSequenceToInput::VisitChangeRAAN(WsfOrbitalManeuvers::ChangeRAAN* aEventPtr)
{
   BeginManeuver(aEventPtr);

   ProduceConstraint(aEventPtr->GetCondition());
   ProduceFinite(aEventPtr);

   ProduceIndent();
   mStream << "raan " << aEventPtr->GetRAAN() << "\n";

   EndManeuver();
}

void OrbitalSequenceToInput::VisitChangeRAAN_Inclination(WsfOrbitalManeuvers::ChangeRAAN_Inclination* aEventPtr)
{
   BeginManeuver(aEventPtr);

   ProduceConstraint(aEventPtr->GetCondition());
   ProduceFinite(aEventPtr);

   ProduceIndent();
   mStream << "raan " << aEventPtr->GetRAAN() << "\n";
   ProduceIndent();
   mStream << "inclination " << aEventPtr->GetInclination() << "\n";

   EndManeuver();
}

void OrbitalSequenceToInput::VisitChangeSemiMajorAxis(WsfOrbitalManeuvers::ChangeSemiMajorAxis* aEventPtr)
{
   BeginManeuver(aEventPtr);

   ProduceConstraint(aEventPtr->GetCondition());
   ProduceFinite(aEventPtr);

   ProduceIndent();
   switch (aEventPtr->GetInputType())
   {
   case WsfOrbitalManeuvers::ChangeSemiMajorAxis::InputOption::cFINAL_SMA:
      mStream << "semi_major_axis " << aEventPtr->GetFinalSMA() << "\n";
      break;
   case WsfOrbitalManeuvers::ChangeSemiMajorAxis::InputOption::cAPOAPSIS_ALT:
      mStream << "apoapsis_altitude " << aEventPtr->GetApoapsisAltitude() << "\n";
      break;
   case WsfOrbitalManeuvers::ChangeSemiMajorAxis::InputOption::cPERIAPSIS_ALT:
      mStream << "periapsis_altitude " << aEventPtr->GetPeriapsisAltitude() << "\n";
      break;
   case WsfOrbitalManeuvers::ChangeSemiMajorAxis::InputOption::cRADIUS:
      mStream << "radius " << aEventPtr->GetFinalSMA() << "\n";
      break;
   }

   EndManeuver();
}

void OrbitalSequenceToInput::VisitCircularize(WsfOrbitalManeuvers::Circularize* aEventPtr)
{
   BeginManeuver(aEventPtr);

   ProduceConstraint(aEventPtr->GetCondition());
   ProduceFinite(aEventPtr);

   EndManeuver();
}

void OrbitalSequenceToInput::VisitCompoundManeuver(WsfOrbitalManeuvers::CompoundManeuver* aEventPtr)
{
   BeginManeuver(aEventPtr);

   aEventPtr->GetFirstEvent()->Accept(this);
   aEventPtr->GetSecondEvent()->Accept(this);

   EndManeuver();
}

void OrbitalSequenceToInput::VisitDeltaV(WsfOrbitalManeuvers::DeltaV* aEventPtr)
{
   BeginManeuver(aEventPtr);

   ProduceConstraint(aEventPtr->GetCondition());
   ProduceFinite(aEventPtr);

   ProduceIndent();
   mStream << "delta_v " << wsf::space::OrbitalReferenceFrameToIdentifier(aEventPtr->GetDeltaV_Frame());
   mStream << " " << aEventPtr->GetDeltaV_Component(0);
   mStream << " " << aEventPtr->GetDeltaV_Component(1);
   mStream << " " << aEventPtr->GetDeltaV_Component(2) << "\n";

   EndManeuver();
}

void OrbitalSequenceToInput::VisitDrift(WsfOrbitalManeuvers::Drift* aEventPtr)
{
   BeginManeuver(aEventPtr);

   ProduceConstraint(aEventPtr->GetCondition());

   ProduceIndent();
   mStream << "drift_rate " << aEventPtr->GetDriftRate() << "\n";

   ProduceTargetingEvent(aEventPtr);

   EndManeuver();
}

void OrbitalSequenceToInput::VisitHohmannTransfer(WsfOrbitalManeuvers::HohmannTransfer* aEventPtr)
{
   BeginManeuver(aEventPtr);

   ProduceConstraint(aEventPtr->GetCondition());
   ProduceFinite(aEventPtr);

   ProduceIndent();
   if (aEventPtr->IsRadiusInput())
   {
      mStream << "final_radius " << aEventPtr->GetFinalRadius() << "\n";
   }
   else
   {
      mStream << "final_semi_major_axis " << aEventPtr->GetFinalSMA() << "\n";
   }

   EndManeuver();
}

void OrbitalSequenceToInput::VisitIntercept(WsfOrbitalManeuvers::Intercept* aEventPtr)
{
   BeginManeuver(aEventPtr);

   ProduceConstraint(aEventPtr->GetCondition());

   ProduceTargetingEvent(aEventPtr);
   ProduceTargetPointBlock(aEventPtr->GetTargetPointOptions());

   EndManeuver();
}

void OrbitalSequenceToInput::VisitMatchVelocity(WsfOrbitalManeuvers::MatchVelocity* aEventPtr)
{
   BeginManeuver(aEventPtr);

   ProduceConstraint(aEventPtr->GetCondition());
   ProduceFinite(aEventPtr);

   ProduceIndent();
   mStream << "platform " << aEventPtr->GetPlatformName().GetString() << "\n";

   EndManeuver();
}

void OrbitalSequenceToInput::VisitNaturalMotionCircumnavigation(WsfOrbitalManeuvers::NaturalMotionCircumnavigation* aEventPtr)
{
   BeginManeuver(aEventPtr);

   ProduceConstraint(aEventPtr->GetCondition());

   ProduceIndent();
   mStream << "target_platform " << aEventPtr->GetPlatformName().GetString() << "\n";

   ProduceIndent();
   mStream << "orbit_size " << aEventPtr->GetOrbitSize() << "\n";

   ProduceIndent();
   mStream << "orbit_phase " << aEventPtr->GetOrbitPhase() << "\n";

   ProduceIndent();
   mStream << "out_of_plane_amplitude " << aEventPtr->GetOutOfPlaneAmplitude() << "\n";

   ProduceIndent();
   mStream << "out_of_plane_phase " << aEventPtr->GetOutOfPlanePhase() << "\n";

   ProduceTargetingEvent(aEventPtr);

   EndManeuver();
}

void OrbitalSequenceToInput::VisitNormal(WsfOrbitalManeuvers::Normal* aEventPtr)
{
   BeginManeuver(aEventPtr);

   ProduceConstraint(aEventPtr->GetCondition());
   ProduceFinite(aEventPtr);

   ProduceIndent();
   mStream << "delta_v " << aEventPtr->GetManeuverDeltaV() << "\n";

   EndManeuver();
}

void OrbitalSequenceToInput::VisitRendezvous(WsfOrbitalManeuvers::Rendezvous* aEventPtr)
{
   BeginManeuver(aEventPtr);

   ProduceConstraint(aEventPtr->GetCondition());

   ProduceTargetingEvent(aEventPtr);
   ProduceTargetPointBlock(aEventPtr->GetTargetPointOptions());

   EndManeuver();
}

void OrbitalSequenceToInput::VisitTangent(WsfOrbitalManeuvers::Tangent* aEventPtr)
{
   BeginManeuver(aEventPtr);

   ProduceConstraint(aEventPtr->GetCondition());
   ProduceFinite(aEventPtr);

   ProduceIndent();
   mStream << "delta_v " << aEventPtr->GetManeuverDeltaV() << "\n";

   EndManeuver();
}

void OrbitalSequenceToInput::VisitTarget(WsfOrbitalManeuvers::Target* aEventPtr)
{
   BeginManeuver(aEventPtr);

   ProduceConstraint(aEventPtr->GetCondition());

   ProduceTargetingEvent(aEventPtr);
   ProduceTargetPointBlock(aEventPtr->GetTargetPointOptions());

   EndManeuver();
}

void OrbitalSequenceToInput::VisitTeardrop(WsfOrbitalManeuvers::Teardrop* aEventPtr)
{
   BeginManeuver(aEventPtr);

   ProduceConstraint(aEventPtr->GetCondition());

   ProduceIndent();
   mStream << "target_platform " << aEventPtr->GetPlatformName().GetString() << "\n";

   ProduceIndent();
   mStream << "radial_offset_at_poca " << aEventPtr->GetRadialOffsetAtPOCA() << "\n";

   ProduceIndent();
   mStream << "period " << aEventPtr->GetTeardropPeriod() << "\n";

   ProduceIndent();
   mStream << "time_to_poca " << aEventPtr->GetTimeToPOCA() << "\n";

   ProduceIndent();
   mStream << "repetitions " << aEventPtr->GetRepetitions() << "\n";

   ProduceTargetingEvent(aEventPtr);

   EndManeuver();
}

void OrbitalSequenceToInput::VisitOrbitalScriptEvent(WsfOrbitalScriptEvent* aEventPtr)
{
   BeginEvent(aEventPtr);

   ProduceConstraint(aEventPtr->GetCondition());
   if (aEventPtr->IsFinite())
   {
      ProduceIndent();
      mStream << "finite\n";
   }
   if (aEventPtr->GetDuration() != 0.0)
   {
      ProduceIndent();
      mStream << "duration " << aEventPtr->GetDuration() << "\n";
   }
   if (aEventPtr->GetUpdateInterval() != 0.0)
   {
      ProduceIndent();
      mStream << "update_interval " << aEventPtr->GetUpdateInterval() << "\n";
   }

   ProduceScriptedBlock("on_initialize", aEventPtr->GetOnInitializeInput());
   ProduceScriptedBlock("on_update", aEventPtr->GetOnUpdateInput());
   ProduceScriptedBlock("is_complete", aEventPtr->GetIsCompleteInput());
   ProduceScriptedBlock("on_complete", aEventPtr->GetOnCompleteInput());

   EndEvent();
}

void OrbitalSequenceToInput::VisitOrbitalStagingEvent(WsfOrbitalStagingEvent* aEventPtr)
{
   BeginEvent(aEventPtr);

   ProduceConstraint(aEventPtr->GetCondition());
   ProduceFinite(aEventPtr);

   EndEvent();
}

void OrbitalSequenceToInput::VisitOrbitalAttitudeChangeEvent(WsfOrbitalAttitudeChangeEvent* aEventPtr)
{
   BeginEvent(aEventPtr);

   ProduceConstraint(aEventPtr->GetCondition());
   ProduceFinite(aEventPtr);

   wsf::space::Orientation* orientationPtr = aEventPtr->GetOrientation();
   if (orientationPtr)
   {
      ProduceIndent();

      // Every orientation type has an orientation type string.
      mStream << "orientation " << orientationPtr->GetType();

      // Some orientation types require a platform name, or a geo point name (but not both).
      wsf::space::OrientationEntity* entityReferencedPtr = dynamic_cast<wsf::space::OrientationEntity*>(orientationPtr);
      if (entityReferencedPtr)
      {
         mStream << " " << entityReferencedPtr->GetEntityName();
      }
      else
      {
         wsf::space::OrientationGeoPoint* geoRefPtr = dynamic_cast<wsf::space::OrientationGeoPoint*>(orientationPtr);
         if (geoRefPtr)
         {
            mStream << " " << geoRefPtr->GetGeoPointName();
         }
      }

      mStream << "\n";
   }

   EndEvent();
}

void OrbitalSequenceToInput::BeginSequence()
{
   ProduceSequenceBegin();
   IncreaseIndent();
}

void OrbitalSequenceToInput::EndSequence()
{
   DecreaseIndent();
   ProduceSequenceEnd();
}

void OrbitalSequenceToInput::BeginEvent(const WsfOrbitalEvent* aEventPtr)
{
   ProduceEventBegin(aEventPtr);
   IncreaseIndent();
}

void OrbitalSequenceToInput::EndEvent()
{
   DecreaseIndent();
   ProduceEventEnd();
}

void OrbitalSequenceToInput::BeginManeuver(const WsfOrbitalEvent* aEventPtr)
{
   ProduceManeuverBegin(aEventPtr);
   IncreaseIndent();
}

void OrbitalSequenceToInput::EndManeuver()
{
   DecreaseIndent();
   ProduceManeuverEnd();
}

void OrbitalSequenceToInput::ProduceIndent()
{
   for (int i = 0; i < mIndentSize * mIndentLevel; ++i)
   {
      mStream << " ";
   }
}

void OrbitalSequenceToInput::ProduceSequenceBegin()
{
   ProduceIndent();
   mStream << "mission_sequence\n";
}

void OrbitalSequenceToInput::ProduceSequenceEnd()
{
   ProduceIndent();
   mStream << "end_mission_sequence";
   if (mTrailingNewline)
   {
      mStream << "\n";
   }
}

void OrbitalSequenceToInput::ProduceEventBegin(const WsfOrbitalEvent* aEventPtr)
{
   ProduceIndent();
   std::string eventType = aEventPtr->GetType();
   UtStringUtil::ToLower(eventType);
   mStream << "event " << eventType << "\n";
}

void OrbitalSequenceToInput::ProduceEventEnd()
{
   ProduceIndent();
   mStream << "end_event\n";
}

void OrbitalSequenceToInput::ProduceManeuverBegin(const WsfOrbitalEvent* aEventPtr)
{
   ProduceIndent();
   std::string maneuverType = aEventPtr->GetType();
   UtStringUtil::ToLower(maneuverType);
   mStream << "maneuver " << maneuverType << "\n";
}

void OrbitalSequenceToInput::ProduceManeuverEnd()
{
   ProduceIndent();
   mStream << "end_maneuver\n";
}

void OrbitalSequenceToInput::ProduceConstraint(wsf::space::OrbitalPropagatorCondition& aCondition)
{
   ConditionProducer vtor(*this);
   aCondition.Accept(vtor);
}

void OrbitalSequenceToInput::ProduceFinite(const WsfOrbitalEvent* aEventPtr)
{
   if (aEventPtr->IsFinite())
   {
      ProduceIndent();
      mStream << "finite\n";

      auto duration = aEventPtr->GetDuration();
      if (duration.GetAsUnit() != 0.0)
      {
         ProduceIndent();
         mStream << "duration " << aEventPtr->GetDuration() << "\n";
      }

      ProduceIndent();
      mStream << "update_interval " << aEventPtr->GetUpdateInterval() << "\n";
   }
}

void OrbitalSequenceToInput::ProduceScriptedBlock(const std::string& aBlock, const std::string& aContent)
{
   if (!aContent.empty())
   {
      ProduceIndent();
      mStream << aBlock << "\n";
      IncreaseIndent();
      ProduceIndent();
      mStream << aContent << "\n";
      DecreaseIndent();
      ProduceIndent();
      mStream << "end_" << aBlock << "\n";
   }
}

void OrbitalSequenceToInput::ProduceTargetPointBlock(const wsf::space::TargetPointOptions& aOptions)
{
   ProduceIndent();
   mStream << "target\n";
   IncreaseIndent();

   if (aOptions.HasPlatformTarget())
   {
      ProduceIndent();
      mStream << "platform " << aOptions.GetPlatformName() << "\n";
   }
   else if (aOptions.HasLibrationPointTarget())
   {
      ProduceIndent();
      mStream << "libration_point ";
      mStream << wsf::space::LibrationPoint::GetIdentifierFromSystem(aOptions.GetLP_System()) << " ";
      mStream << wsf::space::LibrationPoint::GetIdentifierFromPoint(aOptions.GetLP_Point()) << "\n";
   }
   else if (aOptions.HasKinematicStateTarget())
   {
      ProduceIndent();
      UtVec3d position = aOptions.GetKinematicStatePositionInUnits();
      UtVec3d velocity = aOptions.GetKinematicStateVelocityInUnits();
      if (UtMath::NearlyZero(velocity.MagnitudeSquared()))
      {
         mStream << "eci_position ";
         mStream << position[0] << " " << position[1] << " " << position[2] << " ";
         mStream << aOptions.GetKinematicStatePositionUnits() << "\n";
      }
      else
      {
         mStream << "eci_state ";
         mStream << position[0] << " " << position[1] << " " << position[2] << " ";
         mStream << aOptions.GetKinematicStatePositionUnits() << " ";
         mStream << velocity[0] << " " << velocity[1] << " " << velocity[2] << " ";
         mStream << aOptions.GetKinematicStateVelocityUnits() << "\n";
      }
   }
   // Track targets are invalid for this use case. There is no way to render that into valid input.

   if (aOptions.HasPositionOffset())
   {
      UtVec3d offset = aOptions.GetPositionOffsetInUnits();
      ProduceIndent();
      mStream << "position_offset ";
      mStream << wsf::space::OrbitalReferenceFrameToIdentifier(aOptions.GetPositionOffsetFrame()) << " ";
      mStream << offset[0] << " " << offset[1] << " " << offset[2] << " " << aOptions.GetPositionOffsetUnits() << "\n";
   }

   if (aOptions.HasVelocityOffset())
   {
      UtVec3d offset = aOptions.GetVelocityOffsetInUnits();
      ProduceIndent();
      mStream << "velocity_offset ";
      mStream << wsf::space::OrbitalReferenceFrameToIdentifier(aOptions.GetVelocityOffsetFrame()) << " ";
      mStream << offset[0] << " " << offset[1] << " " << offset[2] << " " << aOptions.GetVelocityOffsetUnits() << "\n";
   }

   if (aOptions.HasOffsetTime())
   {
      ProduceIndent();
      mStream << "offset_time " << aOptions.GetOffsetTime() << "\n";
   }

   if (aOptions.HasLagTime())
   {
      ProduceIndent();
      mStream << "lag_time " << aOptions.GetLagTime() << "\n";
   }

   DecreaseIndent();
   ProduceIndent();
   mStream << "end_target\n";
}

void OrbitalSequenceToInput::ProduceOptimizeCost(const wsf::space::OrbitalTargetingCost* aCostPtr)
{
   const auto* blendedPtr = dynamic_cast<const wsf::space::BlendedCost*>(aCostPtr);
   if (blendedPtr)
   {
      ProduceIndent();
      mStream << "optimize_cost blended ";
      mStream << blendedPtr->GetA_Value() << " ";
      mStream << blendedPtr->GetB_Value() << " ";
      mStream << blendedPtr->GetC_Value() << "\n";
   }
}

void OrbitalSequenceToInput::ConditionProducer::Accept(wsf::space::ApoapsisCondition& aCondition)
{
   ProduceCommon(aCondition);
   mSequenceToInput.mStream << aCondition.cTYPE << "\n";
}

void OrbitalSequenceToInput::ConditionProducer::Accept(wsf::space::AscendingNodeCondition& aCondition)
{
   ProduceCommon(aCondition);
   mSequenceToInput.mStream << aCondition.cTYPE << "\n";
}

void OrbitalSequenceToInput::ConditionProducer::Accept(wsf::space::AscendingRadiusCondition& aCondition)
{
   ProduceCommon(aCondition);
   mSequenceToInput.mStream << aCondition.cTYPE << " " << aCondition.GetRadius() << "\n";
}

void OrbitalSequenceToInput::ConditionProducer::Accept(wsf::space::DescendingNodeCondition& aCondition)
{
   ProduceCommon(aCondition);
   mSequenceToInput.mStream << aCondition.cTYPE << "\n";
}

void OrbitalSequenceToInput::ConditionProducer::Accept(wsf::space::DescendingRadiusCondition& aCondition)
{
   ProduceCommon(aCondition);
   mSequenceToInput.mStream << aCondition.cTYPE << " " << aCondition.GetRadius() << "\n";
}

void OrbitalSequenceToInput::ConditionProducer::Accept(wsf::space::EclipseEntryCondition& aCondition)
{
   ProduceCommon(aCondition);
   mSequenceToInput.mStream << aCondition.cTYPE << "\n";
}

void OrbitalSequenceToInput::ConditionProducer::Accept(wsf::space::EclipseExitCondition& aCondition)
{
   ProduceCommon(aCondition);
   mSequenceToInput.mStream << aCondition.cTYPE << "\n";
}

void OrbitalSequenceToInput::ConditionProducer::Accept(wsf::space::NoneCondition& aCondition)
{
   // Do nothing.
}

void OrbitalSequenceToInput::ConditionProducer::Accept(wsf::space::NorthernIntersectionCondition& aCondition)
{
   ProduceCommon(aCondition);
   mSequenceToInput.mStream << aCondition.cTYPE << "\n";
}

void OrbitalSequenceToInput::ConditionProducer::Accept(wsf::space::PeriapsisCondition& aCondition)
{
   ProduceCommon(aCondition);
   mSequenceToInput.mStream << aCondition.cTYPE << "\n";
}

void OrbitalSequenceToInput::ConditionProducer::Accept(wsf::space::RelativeTimeCondition& aCondition)
{
   ProduceCommon(aCondition);
   mSequenceToInput.mStream << aCondition.cTYPE << " " << aCondition.GetOffsetTime() << "\n";
}

void OrbitalSequenceToInput::ConditionProducer::Accept(wsf::space::SouthernIntersectionCondition& aCondition)
{
   ProduceCommon(aCondition);
   mSequenceToInput.mStream << aCondition.cTYPE << "\n";
}

void OrbitalSequenceToInput::ConditionProducer::ProduceCommon(wsf::space::OrbitalPropagatorCondition& aCondition)
{
   mSequenceToInput.ProduceIndent();
   mSequenceToInput.mStream << "execute_at ";
   if (aCondition.GetOrbitNumber() != 0)
   {
      mSequenceToInput.mStream << "orbit " << aCondition.GetOrbitNumber() << " ";
   }
}

} // namespace SpaceTools
