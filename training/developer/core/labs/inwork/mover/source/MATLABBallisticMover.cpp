// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2003-2013 The Boeing Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
// ****************************************************************************
// Updated by Infoscitex, a DCS Company.
// ****************************************************************************

// Application specific
#include "MATLABBallisticMover.hpp"

// Utilities
#include "UtCast.hpp"
#include "UtException.hpp"
#include "UtInput.hpp"
#include "UtInputBlock.hpp"
#include "UtLog.hpp"
#include "UtMath.hpp"
#include "UtTime.hxx"
#include "UtVec3.hpp"

// AFSIM
#include "WsfPlatform.hpp"
#include "WsfSimulation.hpp"
#include "WsfTerrain.hpp"

const int MATLABBallisticMover::cBOOSTER_PARAMS_SIZE = 18;
const int MATLABBallisticMover::cSTATE_VECTOR_SIZE   = 24;

// ****************************************************************************
// Public

// ============================================================================
// Constructor and destructor
MATLABBallisticMover::MATLABBallisticMover(WsfScenario& aScenario)
 : WsfMover(aScenario),
   mwInputLLA(3, 1, mxDOUBLE_CLASS, mxREAL),
   // EXERCISE 2 TASK 1
   // PLACE YOUR CODE HERE
   mwInputTime(1, 1, mxDOUBLE_CLASS, mxREAL),
   mwInputBoosterParams(cBOOSTER_PARAMS_SIZE, 1, mxDOUBLE_CLASS, mxREAL),
   mInputAlt(0.0),
   mInputAltAGL(false),
   mStageList(),
   mExplicitStageUsed(false),
   mImplicitStageUsed(false),
   mCd0(0.0),
   mReferenceArea(0.0),
   mMassPayload(0.0),
   mMaxq(0.0),
   mPitchInterval(1.0),
   mVerticalTime(0.0),
   mwHitGroundTime(1, 1, mxDOUBLE_CLASS, mxREAL),
   mwState(cSTATE_VECTOR_SIZE, 1, mxDOUBLE_CLASS, mxREAL)
{
   // Provide a default (single) stage in which to place input values.
   mStageList.push_back(Stage());

   // Verify the marray initialization task was complete.
   try
   {
      double dOrientation[3] = {0.0};
      mwInputOrientation.GetData(dOrientation, 3);
   }
   catch(...)
   {
      ut::log::error() << "MATLABBallisticMover: Please complete Exercise 2 - Task 1!";
      throw UtException("MATLABBallisticMover: Please complete Exercise 2 - Task 1!");
   }
}

// ============================================================================
// Virtual
WsfMover* MATLABBallisticMover::Clone() const
{
   return new MATLABBallisticMover(*this);
}

// ============================================================================
// Virtual
bool MATLABBallisticMover::Initialize(double aSimTime)
{
   // Get the altitude
   GetInitialAltitude();

   // Get the booster parameters
   GetBoosterParams();

   // EXERCISE 2 TASK 4
   // Call the MATLAB to initialize
   // PLACE YOUR CODE HERE

   // EXERCISE 2 TASK 5
   // Set orientation
   // PLACE YOUR CODE HERE
   
   
   // Initialize the base class
   return WsfMover::Initialize(aSimTime);
}

// ============================================================================
// Virtual
bool MATLABBallisticMover::ProcessInput(UtInput& aInput)
{
   // Set the default value
   bool myCommand(true);

   std::string command(aInput.GetCommand());
   if ((! mExplicitStageUsed) &&
       mStageList[0].ProcessInput(aInput))
   {
      mImplicitStageUsed = true;
   }
   else if ((! mImplicitStageUsed) &&
            (command == "stage"))
   {
      UtInputBlock inputBlock(aInput);

      int stageNumber;
      aInput.ReadValue(stageNumber);
      aInput.ValueInClosedRange(static_cast<double>(stageNumber),
                                1.0,
                                static_cast<double>(mStageList.size() + 1));
      if (stageNumber > ut::cast_to_int(mStageList.size()))
      {
         mStageList.push_back(Stage());
      }
      while (inputBlock.ReadCommand())
      {
         if (! mStageList[stageNumber - 1].ProcessInput(aInput))
         {
            throw UtInput::UnknownCommand(aInput);
         }
      }
      mExplicitStageUsed = true;
   }
   else if ((command == "reference_area") ||
              (command == "effective_area"))
   {
      aInput.ReadValueOfType(mReferenceArea, UtInput::cAREA);
      aInput.ValueGreater(mReferenceArea, 0.0);
   }
   else if (command == "diameter")
   {
      double diameter;
      aInput.ReadValueOfType(diameter, UtInput::cLENGTH);
      aInput.ValueGreater(diameter, 0.0);
      double radius = 0.5 * diameter;
      mReferenceArea = UtMath::cPI * radius * radius;
   }
   else if ((command == "zero_lift_cd") ||
              (command == "drag_coeff"))
   {
      aInput.ReadValue(mCd0);
      aInput.ValueGreaterOrEqual(mCd0, 0.0);
   }
   else if (command == "mass_payload")
   {
      // EXERCISE 2 TASK 2a
      // PLACE YOUR CODE HERE
   }
   else if (command == "maxq")
   { // kg/m-s^2
      aInput.ReadValue(mMaxq);
   }
   else if (command == "pitch_interval")
   {
      // EXERCISE 2 TASK 2b
      // PLACE YOUR CODE HERE
   }
   else if (command == "vertical_time")
   {
      aInput.ReadValueOfType(mVerticalTime, UtInput::cTIME);
      aInput.ValueGreaterOrEqual(mVerticalTime, 0.0);
   }
   else if (command == "position")
   {
      double dInputLLA[3] = {0.0};

      aInput.ReadValueOfType(dInputLLA[0], UtInput::cLATITUDE);
      aInput.ReadValueOfType(dInputLLA[1], UtInput::cLONGITUDE);

      mwInputLLA.SetData(dInputLLA, 3);
   }
   else if (command == "altitude")
   {
      aInput.ReadValueOfType(mInputAlt, UtInput::cLENGTH);
      std::string altRef;
      aInput.ReadCommand(altRef);
      if (altRef == "agl")
      {
         mInputAltAGL = true;
      }
      else if (altRef == "msl")
      {
         mInputAltAGL = false;
      }
      else
      {
         aInput.PushBack(altRef);
      }
   }
   else if (command == "heading")
   {
      double dInputOrientation[3] = {0.0};
      mwInputOrientation.GetData(dInputOrientation, 3);

      aInput.ReadValueOfType(dInputOrientation[0], UtInput::cANGLE);
      aInput.ValueInClosedRange(dInputOrientation[0], 0.0, UtMath::cTWO_PI);

      // MATLAB expects degrees
      dInputOrientation[0] *= UtMath::cDEG_PER_RAD;
      mwInputOrientation.SetData(dInputOrientation, 3);
   }
   else if (command == "pitch")
   {
      // Measure from vertical
      double dInputOrientation[3] = {0.0};
      mwInputOrientation.GetData(dInputOrientation, 3);

      aInput.ReadValueOfType(dInputOrientation[1], UtInput::cANGLE);
      aInput.ValueInClosedRange(dInputOrientation[1], 0.0, UtMath::cPI_OVER_2);

      // MATLAB expects degrees
      dInputOrientation[1] *= UtMath::cDEG_PER_RAD;
      mwInputOrientation.SetData(dInputOrientation, 3);
   }
   else
   {
      myCommand = WsfMover::ProcessInput(aInput);
   }

   return myCommand;
}

// ============================================================================
// Virtual
void MATLABBallisticMover::Update(double aSimTime)
{
   if ((aSimTime - mLastUpdateTime) > mUpdateTimeTolerance)
   {
      // Set the input parameter - time
      mwInputTime.SetData(&aSimTime, 1);

      // EXERCISE 2 TASK 6
      // -- call MATLAB update function
      // PLACE YOUR CODE HERE
   }

   // Update the platform location, velocity, orientation, etc.
   UpdatePlatform(aSimTime);

   if (DebugEnabled())
   {
      OutputDiagnostic(aSimTime);
   }

   // Check to see if the missile has hit the ground
   if (GetHitGroundTime() > 0)
   {
      if (DebugEnabled())
      {
         auto out = ut::log::debug() << "Missile hit ground.";
         out.AddNote() << "T = " << GetHitGroundTime();
         out.AddNote() << "Removing platform.";
      }
      GetSimulation()->DeletePlatform(aSimTime, GetPlatform());
   }

   // Call base class implementation last
   WsfMover::Update(aSimTime);
}

// ****************************************************************************
// Protected

// ============================================================================
// Copy Constructor
MATLABBallisticMover::MATLABBallisticMover(const MATLABBallisticMover& aSrc)
   : WsfMover(aSrc),
     mwInputLLA(aSrc.mwInputLLA.Clone()),
     mwInputOrientation(aSrc.mwInputOrientation.Clone()),
     mwInputTime(aSrc.mwInputTime.Clone()),
     mwInputBoosterParams(aSrc.mwInputBoosterParams.Clone()),
     mInputAlt(aSrc.mInputAlt),
     mInputAltAGL(aSrc.mInputAltAGL),
     mStageList(aSrc.mStageList),
     mExplicitStageUsed(aSrc.mExplicitStageUsed),
     mImplicitStageUsed(aSrc.mImplicitStageUsed),
     mCd0(aSrc.mCd0),
     mReferenceArea(aSrc.mReferenceArea),
     mMassPayload(aSrc.mMassPayload),
     mMaxq(aSrc.mMaxq),
     mPitchInterval(aSrc.mPitchInterval),
     mVerticalTime(aSrc.mVerticalTime),
     mwHitGroundTime(aSrc.mwHitGroundTime.Clone()),
     mwState(aSrc.mwState.Clone())
{
}

// ****************************************************************************
// Private

// ============================================================================
double MATLABBallisticMover::GetHitGroundTime()
{
   double dHitGroundTime(0.0);
   mwHitGroundTime.GetData(&dHitGroundTime, 1);
   return dHitGroundTime;
}

// ============================================================================
void MATLABBallisticMover::UpdatePlatform(double aSimTime)
{
   // Get the MATLAB result
   double dState[cSTATE_VECTOR_SIZE] = {0.0};
   mwState.GetData(dState, cSTATE_VECTOR_SIZE);

   if (DebugEnabled())
   {
      auto out = ut::log::debug() << "MATLAB Ballistic Mover updating platform.";
      
      out.AddNote() << "T = " << aSimTime << " [" << UtTime(aSimTime, UtTime::FmtHMS + 1) << "]";
      out.AddNote() << "Platform: " << GetPlatform()->GetName();
      { // RAII block
         auto note = out.AddNote() << "ECI Position (km):";
         note.AddNote() << "X: " << dState[0];
         note.AddNote() << "Y: " << dState[2];
         note.AddNote() << "Z: " << dState[4];
      }
      { // RAII block
         auto note = out.AddNote() << "ECI Velocity (km/s):";
         note.AddNote() << "X: " << dState[1];
         note.AddNote() << "Y: " << dState[3];
         note.AddNote() << "Z: " << dState[5];
      }
      out.AddNote() << "Mass: " << dState[6] << " kg";
      out.AddNote() << "Latitude: " << dState[7];
      out.AddNote() << "Longitude: " << dState[8];
      out.AddNote() << "Altitude: " << dState[9] << " km";
      out.AddNote() << "Great Circle Range: " << dState[10] << " km";
      out.AddNote() << "Thrust: " << dState[11];
      out.AddNote() << "Gravity: " << dState[12];
      out.AddNote() << "Drag: " << dState[13];
      out.AddNote() << "Alpha: " << dState[14];
      out.AddNote() << "AOA: " << dState[15];
      { // RAII block
         auto note = out.AddNote() << "ECEF Position (km):";
         note.AddNote() << "X: " << dState[16];
         note.AddNote() << "Y: " << dState[17];
         note.AddNote() << "Z: " << dState[18];
      }
      { // RAII block
         auto note = out.AddNote() << "ECEF Velocity (km/s):";
         note.AddNote() << "X: " << dState[19];
         note.AddNote() << "Y: " << dState[20];
         note.AddNote() << "Z: " << dState[21];
      }
      out.AddNote() << "Time: " << dState[22];
      out.AddNote() << "Stage: " << dState[23];
    }

   // Update platform with new ECI location
   double newLocECI[3] = {0.0};
   UtVec3d::Set(newLocECI, dState[0], dState[2], dState[4]);
   GetPlatform()->SetLocationECI(newLocECI);

   // EXERCISE 2 TASK 7
   // Update platform with new ECI velocity
   // PLACE YOUR CODE HERE

   // Update platform orientation if vertical time exceeded
   if (aSimTime >= mVerticalTime)
   {
      double newVelNED[3] = {0.0};
      GetPlatform()->GetVelocityNED(newVelNED);

      double vel = UtVec3d::Magnitude(newVelNED);
      if (vel > 0.001)
      {
         double heading = ::atan2(newVelNED[1], newVelNED[0]);
         double pitch   = ::asin(-newVelNED[2] / vel);
         double roll    = 0.0;
         GetPlatform()->SetOrientationNED(heading, pitch, roll);
      }
   }
}

// ============================================================================
void MATLABBallisticMover::GetInitialAltitude()
{
   double dLLA[3] = {0.0};
   mwInputLLA.GetData(dLLA, 3);

   if (mInputAltAGL)
   {
      float terrainHeight;
      wsf::Terrain terrain(GetPlatform()->GetTerrain());
      terrain.GetElevInterp(dLLA[0], dLLA[1], terrainHeight);
      dLLA[2] += terrainHeight;
   }
   mwInputLLA.SetData(dLLA, 3);
}

// ============================================================================
void MATLABBallisticMover::GetBoosterParams()
{
   // Initialize the booster parameters to zero
   double dBooster[cBOOSTER_PARAMS_SIZE] = {0.0};

   // Starting locations within the dBooster array
   int nBurnTimeIndex(0);
   int nTotalMass(5);
   int nFuelMass(8);
   int nThrust(14);

   // Get the stage data; stop at 3 stages
   int nStages = ut::cast_to_int(mStageList.size());
   if (nStages > 3)
   {
      nStages = 3;
   }

   for(int i = 0; i < nStages; ++i)
   {
      // burn_time
      dBooster[nBurnTimeIndex + i] = mStageList[i].mThrustDuration;

      // total_mass
      dBooster[nTotalMass + i] = mStageList[i].mTotalMass;

      // fuel_mass
      dBooster[nFuelMass + i] = mStageList[i].mFuelMass;

      // thrust
      dBooster[nThrust + i] = mStageList[i].mThrust;
   }

   // drag_coeff
   dBooster[3] = mCd0;

   // eff_area
   dBooster[4] = mReferenceArea;

   // mass_payload; add to the final stage total mass
   dBooster[11] = mMassPayload;

   // maxq
   dBooster[12] = mMaxq;

   // pitch_interval
   dBooster[13] = mPitchInterval;

   // vert_time
   dBooster[17] = mVerticalTime;

   // Set the data in the MATLAB array
   mwInputBoosterParams.SetData(dBooster, cBOOSTER_PARAMS_SIZE);
}

// ============================================================================
void MATLABBallisticMover::OutputDiagnostic(double aSimTime)
{
   auto out = ut::log::debug() << "MATLAB Ballistic Mover diagnostics:";
   out.AddNote() << "T = " << aSimTime << " [" << UtTime(aSimTime, UtTime::FmtHMS + 1) << "]";
   out.AddNote() << "Platform: " << GetPlatform()->GetName();
   { // RAII block
      double dECI[3] = {0.0};
      GetPlatform()->GetLocationECI(dECI);
      
      auto note = out.AddNote() << "ECI Position (m):";
      note.AddNote() << "X: " << dECI[0];
      note.AddNote() << "Y: " << dECI[1];
      note.AddNote() << "Z: " << dECI[2];
   }
   { // RAII block
      double dLLA[3] = {0.0};
      GetPlatform()->GetLocationLLA(dLLA[0], dLLA[1], dLLA[2]);
      
      auto note = out.AddNote() << "LLA Position:";
      note.AddNote() << "Lat: " << dLLA[0];
      note.AddNote() << "Lon: " << dLLA[1];
      note.AddNote() << "Alt: " << dLLA[2] << " m" << (mInputAltAGL ? " AGL" : "");
   }
   { // RAII block
      double dWCS[3] = {0.0};
      GetPlatform()->GetLocationWCS(dWCS);
      auto note = out.AddNote() << "WCS Position:";
      note.AddNote() << "X: " << dWCS[0];
      note.AddNote() << "Y: " << dWCS[1];
      note.AddNote() << "Z: " << dWCS[2];
   }
   
   if (aSimTime <= 0.0)
   {
      // Orientation
      double dOrientation[3] = {0.0};
      mwInputOrientation.GetData(dOrientation, 3);
      
      out.AddNote() << "Yaw: " << dOrientation[0];
      out.AddNote() << "Pitch: " << dOrientation[1];
      out.AddNote() << "Roll: " << dOrientation[2];
   }
   
   out.AddNote() << "Speed: " << GetPlatform()->GetSpeed() << " m/s";
}

// ============================================================================
// ============================================================================
MATLABBallisticMover::Stage::Stage()
   : mFuelMass(0.0)
   , mTotalMass(0.0)
   , mThrust(0.0)
   , mThrustDuration(0.0)
{
}

// ============================================================================
bool MATLABBallisticMover::Stage::ProcessInput(UtInput& aInput)
{
   bool myCommand(true);
   std::string command(aInput.GetCommand());
   if ((command == "total_mass") ||
       (command == "launch_mass"))
   {
      // EXERCISE 2 TASK 3
      // PLACE YOUR CODE HERE
      
   }
   else if (command == "fuel_mass")
   {
      aInput.ReadValueOfType(mFuelMass, UtInput::cMASS);
      aInput.ValueGreater(mFuelMass, 0.0);
   }
   else if (command == "thrust")
   {
      aInput.ReadValueOfType(mThrust, UtInput::cFORCE);
      aInput.ValueGreater(mThrust, 0.0);
   }
   else if ((command == "thrust_duration") ||
            (command == "burn_time"))
   {
      aInput.ReadValueOfType(mThrustDuration, UtInput::cTIME);
      aInput.ValueGreaterOrEqual(mThrustDuration, 0.0);
   }
   else
   {
      myCommand = false;
   }

   return myCommand;
}
