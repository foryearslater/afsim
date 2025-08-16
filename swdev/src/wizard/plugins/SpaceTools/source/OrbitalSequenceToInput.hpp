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

#ifndef ORBITALSEQUENCETOINPUT_HPP
#define ORBITALSEQUENCETOINPUT_HPP

#include <algorithm>
#include <ostream>

#include "WsfOrbitalEventVisitor.hpp"
class WsfOrbitalMissionSequence;
#include "WsfSpaceOrbitalPropagatorConditionVisitor.hpp"

namespace wsf
{
namespace space
{
class TargetPointOptions;

}
} // namespace wsf

namespace SpaceTools
{
//! A visitor that produces the input commands that would generate a given mission sequence.
class OrbitalSequenceToInput : public WsfOrbitalEventVisitor
{
public:
   OrbitalSequenceToInput(std::ostream& aStream, int aIndentLevel, int aIndentSize, bool aTrailingNewline)
      : mStream(aStream)
      , mIndentLevel{aIndentLevel}
      , mIndentSize{aIndentSize}
      , mTrailingNewline{aTrailingNewline}
   {
      mSavedPrecision = mStream.precision(9);
   }
   ~OrbitalSequenceToInput() override { mStream.precision(mSavedPrecision); }

   //! Return the current level of indentation.
   int GetIndentLevel() const { return mIndentLevel; }

   //! Set the current level of indentation.
   void SetIndentLevel(int aLevel) { mIndentLevel = std::max(0, aLevel); }

   //! Increase the level of indentation.
   void IncreaseIndent() { ++mIndentLevel; }

   //! Decrease the level of indentation.
   void DecreaseIndent() { mIndentLevel = std::max(0, mIndentLevel - 1); }

   void VisitMissionSequence(WsfOrbitalMissionSequence* aSequencePtr) override;
   void VisitChangeEccentricity(WsfOrbitalManeuvers::ChangeEccentricity* aEventPtr) override;
   void VisitChangeInclination(WsfOrbitalManeuvers::ChangeInclination* aEventPtr) override;
   void VisitChangeRAAN(WsfOrbitalManeuvers::ChangeRAAN* aEventPtr) override;
   void VisitChangeRAAN_Inclination(WsfOrbitalManeuvers::ChangeRAAN_Inclination* aEventPtr) override;
   void VisitChangeSemiMajorAxis(WsfOrbitalManeuvers::ChangeSemiMajorAxis* aEventPtr) override;
   void VisitCircularize(WsfOrbitalManeuvers::Circularize* aEventPtr) override;
   void VisitCompoundManeuver(WsfOrbitalManeuvers::CompoundManeuver* aEventPtr) override;
   void VisitDeltaV(WsfOrbitalManeuvers::DeltaV* aEventPtr) override;
   void VisitDrift(WsfOrbitalManeuvers::Drift* aEventPtr) override;
   void VisitHohmannTransfer(WsfOrbitalManeuvers::HohmannTransfer* aEventPtr) override;
   void VisitIntercept(WsfOrbitalManeuvers::Intercept* aEventPtr) override;
   void VisitMatchVelocity(WsfOrbitalManeuvers::MatchVelocity* aEventPtr) override;
   void VisitNaturalMotionCircumnavigation(WsfOrbitalManeuvers::NaturalMotionCircumnavigation* aEventPtr) override;
   void VisitNormal(WsfOrbitalManeuvers::Normal* aEventPtr) override;
   void VisitRendezvous(WsfOrbitalManeuvers::Rendezvous* aEventPtr) override;
   void VisitTangent(WsfOrbitalManeuvers::Tangent* aEventPtr) override;
   void VisitTarget(WsfOrbitalManeuvers::Target* aEventPtr) override;
   void VisitTeardrop(WsfOrbitalManeuvers::Teardrop* aEventPtr) override;
   void VisitOrbitalScriptEvent(WsfOrbitalScriptEvent* aEventPtr) override;
   void VisitOrbitalStagingEvent(WsfOrbitalStagingEvent* aEventPtr) override;
   void VisitOrbitalAttitudeChangeEvent(WsfOrbitalAttitudeChangeEvent* aEventPtr) override;

private:
   void BeginSequence();
   void EndSequence();
   void BeginEvent(const WsfOrbitalEvent* aEventPtr);
   void EndEvent();
   void BeginManeuver(const WsfOrbitalEvent* aEventPtr);
   void EndManeuver();

   void ProduceIndent();
   void ProduceSequenceBegin();
   void ProduceSequenceEnd();
   void ProduceEventBegin(const WsfOrbitalEvent* aEventPtr);
   void ProduceEventEnd();
   void ProduceManeuverBegin(const WsfOrbitalEvent* aEventPtr);
   void ProduceManeuverEnd();
   void ProduceConstraint(wsf::space::OrbitalPropagatorCondition& aCondition);
   void ProduceFinite(const WsfOrbitalEvent* aEventPtr);
   void ProduceScriptedBlock(const std::string& aBlock, const std::string& aContent);
   void ProduceTargetPointBlock(const wsf::space::TargetPointOptions& aOptions);
   void ProduceOptimizeCost(const wsf::space::OrbitalTargetingCost* aCostPtr);

   template<typename TargetingEventType>
   void ProduceTargetingEvent(const TargetingEventType* aEventPtr)
   {
      if (aEventPtr->GetDeltaTime() > 0.0)
      {
         ProduceIndent();
         mStream << "delta_time " << aEventPtr->GetDeltaTime() << "\n";
      }
      if (aEventPtr->GetMaximumDeltaT() > 0.0)
      {
         ProduceIndent();
         mStream << "maximum_delta_time " << aEventPtr->GetMaximumDeltaT() << "\n";
      }
      if (aEventPtr->GetMaximumDeltaV() > 0.0)
      {
         ProduceIndent();
         mStream << "maximum_delta_v " << aEventPtr->GetMaximumDeltaV() << "\n";
      }
      switch (aEventPtr->GetOptimizeOption())
      {
      case WsfOrbitalManeuvers::OptimizeOption::cOPTIMIZE_TIME:
         ProduceIndent();
         mStream << "optimize_time\n";
         break;
      case WsfOrbitalManeuvers::OptimizeOption::cOPTIMIZE_DELTA_V:
         ProduceIndent();
         mStream << "optimize_delta_v\n";
         break;
      case WsfOrbitalManeuvers::OptimizeOption::cOPTIMIZE_NONE:
         break;
      case WsfOrbitalManeuvers::OptimizeOption::cOPTIMIZE_COST:
         ProduceOptimizeCost(aEventPtr->GetOptimizationCost());
         break;
      }
      ProduceIndent();
      mStream << "tolerance " << aEventPtr->GetTolerance() << "\n";
   }

   class ConditionProducer : public wsf::space::OrbitalPropagatorConditionVisitor
   {
   public:
      ConditionProducer(OrbitalSequenceToInput& aSequenceToInput)
         : mSequenceToInput(aSequenceToInput)
      {
      }

      void Accept(wsf::space::ApoapsisCondition& aCondition) override;
      void Accept(wsf::space::AscendingNodeCondition& aCondition) override;
      void Accept(wsf::space::AscendingRadiusCondition& aCondition) override;
      void Accept(wsf::space::DescendingNodeCondition& aCondition) override;
      void Accept(wsf::space::DescendingRadiusCondition& aCondition) override;
      void Accept(wsf::space::EclipseEntryCondition& aCondition) override;
      void Accept(wsf::space::EclipseExitCondition& aCondition) override;
      void Accept(wsf::space::NoneCondition& aCondition) override;
      void Accept(wsf::space::NorthernIntersectionCondition& aCondition) override;
      void Accept(wsf::space::PeriapsisCondition& aCondition) override;
      void Accept(wsf::space::RelativeTimeCondition& aCondition) override;
      void Accept(wsf::space::SouthernIntersectionCondition& aCondition) override;

   private:
      void ProduceCommon(wsf::space::OrbitalPropagatorCondition& aCondition);

      OrbitalSequenceToInput& mSequenceToInput;
   };

   std::ostream&   mStream;
   int             mIndentLevel;
   int             mIndentSize;
   bool            mTrailingNewline;
   std::streamsize mSavedPrecision;
};

} // namespace SpaceTools

#endif // ORBITALSEQUENCETOINPUT_HPP
