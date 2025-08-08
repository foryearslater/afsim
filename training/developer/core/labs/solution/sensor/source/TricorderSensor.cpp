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

#include "TricorderSensor.hpp"

#include <algorithm>

// Utilities
#include "UtInput.hpp"
#include "UtLog.hpp"
#include "UtMemory.hpp"
#include "UtScriptClass.hpp"

// WSF
#include "WsfAttributeContainer.hpp"
#include "WsfDefaultSensorScheduler.hpp"
#include "WsfDefaultSensorTracker.hpp"
#include "sensor/WsfSensorModeList.hpp"
#include "WsfEM_Rcvr.hpp"
#include "WsfEM_Xmtr.hpp"
#include "WsfSensorObserver.hpp"
#include "WsfPlatform.hpp"
#include "script/WsfScriptManager.hpp"
#include "WsfSensorResult.hpp"
#include "WsfSimulation.hpp"

// ****************************************************************************
// Public

// ============================================================================
// Constructor and destructor
TricorderSensor::TricorderSensor(WsfScenario& aScenario)
   : WsfSensor(aScenario),
     mTricorderModeList()
{
   // Set the class of the sensor. This is a passive multi-spectral sensor
   SetClass(cPASSIVE | cRADIO | cINFRARED | cVISUAL | cACOUSTIC );

   // EXERCISE 2 TASK 1a
   // Assign a new mode list, parameterizing it with this
   // sensor-specific mode template (constructed with a new TricorderMode)
   SetModeList(ut::make_unique<WsfSensorModeList>(new TricorderMode));

   // EXERCISE 2 TASK 1b
   // Assign the sensor scheduler to be the default sensor scheduler
   SetScheduler(ut::make_unique<WsfDefaultSensorScheduler>());

   // EXERCISE 2 TASK 1c
   // Assign the sensor tracker to be the default sensor tracker (constructed
   // with a reference to the scenario)
   SetTracker(ut::make_unique<WsfDefaultSensorTracker>(aScenario));

}

// ============================================================================
// Virtual
WsfSensor* TricorderSensor::Clone() const
{
   return new TricorderSensor(*this);
}

// ============================================================================
// Virtual
//! Initializes the Tricorder Sensor; called by WsfSimulation.
//!
//! @param aSimTime       [input] The current simulation time.
//! @return 'true' if initialization was successful.
bool TricorderSensor::Initialize(double aSimTime)
{
   // Must call base class first!
   bool ok = WsfSensor::Initialize(aSimTime);

   // Reduce future dynamic casting by extracting derived class mode pointers.
   mModeListPtr->GetDerivedModeList(mTricorderModeList);

   return ok;
}

// ============================================================================
// Virtual
//! Process input.
//!
//! @param aInput    [input]  The input stream.
//!
//! @return 'true' if the command was recognized (and processed) or 'false'
//! if the command was not one recognized by this class.
bool TricorderSensor::ProcessInput(UtInput& aInput)
{
   // Call the base class implementation
   return WsfSensor::ProcessInput(aInput);
}

// ============================================================================
// Virtual
//! Called by the simulation object to update the Tricorder Sensor
//! @param aSimTime       [input] The current simulation time.
void TricorderSensor::Update(double aSimTime)
{
   // Bypass updates if not time for an update.  This avoids unnecessary device updates.
   // (A little slop is allowed to make sure event-driven chances occur as scheduled)
   if (mNextUpdateTime <= (aSimTime + 1.0E-5))
   {
      WsfSensor::Update(aSimTime);                    // Ensure my position is current
      PerformScheduledDetections(aSimTime);           // Perform any required detection attempts
   }
}

// ============================================================================
//! See the base class for documentation of the arguments and return value.
bool TricorderSensor::AttemptToDetect(double             aSimTime,
                                      WsfPlatform*       aTargetPtr,
                                      Settings&          aSettings,
                                      WsfSensorResult&   aResult)
{
   // Call the base class implementation
   return WsfSensor::AttemptToDetect(aSimTime, aTargetPtr, aSettings, aResult);
}

// ============================================================================
//! Returns the number of "life form types" defined in the given mode.
//!
//! @param aModeNameId    [input]  The string ID of a given mode.
//!
//! @return number of "life form types"
size_t TricorderSensor::GetLifeFormTypeCount(WsfStringId aModeNameId)
{
   size_t count = 0;
   size_t modeIndex = mModeListPtr->GetModeByName(aModeNameId);
   if (modeIndex < mModeListPtr->GetModeCount())
   {
      TricorderMode* modePtr = dynamic_cast<TricorderMode*>(GetModeEntry(modeIndex));
      count = modePtr->mLifeFormTypes.GetCategoryList().size();
   }
   return count;
}

// ============================================================================
//! Returns the entry ID of a "life form types" entry defined in the given mode.
//!
//! @param aModeNameId    [input]  The string ID of a given mode.
//! @param aEntry         [input]  The index of the "life form type" entry
//!
//! @return the string ID of the given entry
WsfStringId TricorderSensor::GetLifeFormTypeEntry(WsfStringId  aModeNameId,
                                                  unsigned int aEntry)
{
   WsfStringId entryID = nullptr;
   size_t modeIndex = mModeListPtr->GetModeByName(aModeNameId);
   if (modeIndex < mModeListPtr->GetModeCount())
   {
      TricorderMode* modePtr = dynamic_cast<TricorderMode*>(GetModeEntry(modeIndex));
      size_t count = modePtr->mLifeFormTypes.GetCategoryList().size();
      if (aEntry < count)
      {
         entryID = modePtr->mLifeFormTypes.GetCategoryList()[aEntry];
      }
   }
   return entryID;
}

// ****************************************************************************
// Protected

// ============================================================================
//!Copy constructor used by Clone()
TricorderSensor::TricorderSensor(const TricorderSensor& aSrc)
   : WsfSensor(aSrc)
   , mTricorderModeList()
{
}

// ****************************************************************************
// Private

// ***************************************************************************
// ***************************************************************************
// Nested class TricorderSensor::TricorderMode

TricorderSensor::TricorderMode::TricorderMode()
   : WsfSensorMode()
   , mLifeFormTypes()
{
   // Derived sensor modes should call SetCapabities(...) to register what
   // they detect and report out in measurements and tracks.
   SetCapabilities(cRANGE | cTYPE | cOTHER);
}

// ============================================================================
TricorderSensor::TricorderMode::TricorderMode(const TricorderMode& aSrc)
   : WsfSensorMode(aSrc)
   , mLifeFormTypes(aSrc.mLifeFormTypes)
{
}

TricorderSensor::TricorderMode& TricorderSensor::TricorderMode::operator=(const TricorderMode& aSrc)
{
   mLifeFormTypes = aSrc.mLifeFormTypes;
   return *this;
}

// ============================================================================
WsfMode* TricorderSensor::TricorderMode::Clone() const
{
   return new TricorderMode(*this);
}

// ============================================================================
bool TricorderSensor::TricorderMode::Initialize(double     aSimTime,
                                                WsfSensor* aSensorPtr)
{
   // Call the base class implementation
   return WsfSensorMode::Initialize(aSimTime);
}

// ============================================================================
//virtual
bool TricorderSensor::TricorderMode::ProcessInput(UtInput& aInput)
{
   bool myCommand = true;
   std::string command(aInput.GetCommand());
   if (command == "life_form_type")
   {
      // EXERCISE 2 TASK 3
      WsfStringId lifeFormType;
      aInput.ReadValue(lifeFormType);
      mLifeFormTypes.JoinCategory(lifeFormType);
   }
   else
   {
      myCommand = WsfSensorMode::ProcessInput(aInput);
   }

   return myCommand;
}

// ============================================================================
bool TricorderSensor::TricorderMode::AttemptToDetect(double             aSimTime,
                                                     WsfPlatform*       aTargetPtr,
                                                     Settings&          aSettings,
                                                     WsfSensorResult&   aResult)
{
   // EXERCISE 2 TASK 5
   // Check for a life form
   bool detected = IsLifeForm(aTargetPtr);

   aResult.Reset(aSettings);
   if (GetSensor()->DebugEnabled())
   {
      auto out = ut::log::debug() << "Sensor attempting to detect Target.";
      out.AddNote() << "T = " << aSimTime;
      out.AddNote() << "Platform: " << GetPlatform()->GetName();
      out.AddNote() << "Sensor: " << GetSensor()->GetName();
      out.AddNote() << "Target: " << aTargetPtr->GetName();
   }

   if (detected)
   {
      // Set probability of detection to 1.0; tricorder will detect all life forms
      aResult.mPd = 1.0;
   }

   // Get the range and unit vector from the receiver to the target.
   aTargetPtr->Update(aSimTime);                 // Ensure the target position is current
   GetSensor()->GetPlatform()->GetLocationWCS(aResult.mRcvrLoc.mLocWCS);
   aTargetPtr->GetLocationWCS(aResult.mTgtLoc.mLocWCS);
   UtVec3d::Subtract(aResult.mRcvrToTgt.mTrueUnitVecWCS, aResult.mTgtLoc.mLocWCS, aResult.mRcvrLoc.mLocWCS);
   aResult.mRcvrToTgt.mRange = UtVec3d::Normalize(aResult.mRcvrToTgt.mTrueUnitVecWCS);

   // Notify observers about the sensor detection attempt
   WsfObserver::SensorDetectionAttempt(GetSimulation())(aSimTime, GetSensor(), aTargetPtr, aResult);

   return detected;
}

// ============================================================================
void TricorderSensor::TricorderMode::UpdateTrack(double             aSimTime,
                                                 WsfTrack*          aTrackPtr,
                                                 WsfPlatform*       aTargetPtr,
                                                 WsfSensorResult&   aResult)
{
   //
   // Life form type
   //

   // If the sensor is reporting type
   if (ReportsType())
   {
      // Get the life form type
      WsfAttributeContainer& auxDataTarget = aTargetPtr->GetAuxData();
      if (auxDataTarget.AttributeExists("LIFE_FORM_REPORTED_TYPE"))
      {
         // Set the type data in the track
         WsfStringId typeID = WsfStringId(auxDataTarget.GetString("LIFE_FORM_REPORTED_TYPE"));
         aResult.mMeasurement.SetTypeId(typeID);
         aResult.mMeasurement.SetTypeIdValid(true);
      }
   }

   // Call base class' method.  This will set valid measurement data (typeId) in the track.
   WsfSensorMode::UpdateTrack(aSimTime, aTrackPtr, aTargetPtr, aResult);

   //
   // Life form health
   //

   // EXERCISE 2 TASK 7
   // Set the "track quality" WsfTrack attribute with the life reading
   aTrackPtr->SetTrackQuality(GetLifeReading(aTargetPtr));
}

// ============================================================================
void TricorderSensor::TricorderMode::ApplyMeasurementErrors(WsfSensorResult& aResult)
{
   // Just because we don't want to override the base class implementation
}

// ============================================================================
double TricorderSensor::TricorderMode::GetLifeReading(WsfPlatform* aTargetPtr)
{
   // EXERCISE 2 TASK 6
   // Use the damage factor of the life form for the health reading
   // Use std::min and std::max to ensure value is between 0 and 1.
   return std::min(1.0, std::max(0.0, 1.0 - aTargetPtr->GetDamageFactor()));
}

// ============================================================================
bool TricorderSensor::TricorderMode::IsLifeForm(WsfPlatform* aTargetPtr)
{
   for (const auto& category : mLifeFormTypes.GetCategoryList())
   {
      // EXERCISE 2 TASK 4a
      // Check platform type
      if (aTargetPtr->IsA_TypeOf(category))
      {
         return true;
      }
   }

   // EXERCISE 2 TASK 4b
   // Check for category on the platform
   if (mLifeFormTypes.Intersects(aTargetPtr->GetCategories()))
   {
      return true;
   }

   return false;
}

// ***************************************************************************
//virtual
//! Provide the name of the corresponding script class so that the
//! scripting system can implement type casting properly.
const char* TricorderSensor::GetScriptClassName() const
{
   return "TricorderSensor";
}

// ***************************************************************************
#include "script/WsfScriptContext.hpp"

ScriptTricorderSensorClass::ScriptTricorderSensorClass(const std::string& aClassName,
                                                       UtScriptTypes*     aScriptTypesPtr)
   : WsfScriptSensorClass(aClassName, aScriptTypesPtr)
{
   SetClassName("TricorderSensor");

   AddMethod(ut::make_unique<LifeFormTypeCount_1>("LifeFormTypeCount"));   // LifeFormTypeCount()
   AddMethod(ut::make_unique<LifeFormTypeCount_2>("LifeFormTypeCount"));   // LifeFormTypeCount(string)

   AddMethod(ut::make_unique<LifeFormTypeEntry_1>("LifeFormTypeEntry"));   // LifeFormTypeEntry(int)

   // EXERCISE 3 TASK 3
   AddMethod(ut::make_unique<LifeFormTypeEntry_2>("LifeFormTypeEntry"));   // LifeFormTypeEntry(string, int)

}

// int lifeFormCount = <x>.LifeFormTypeCount();
UT_DEFINE_SCRIPT_METHOD(ScriptTricorderSensorClass, TricorderSensor, LifeFormTypeCount_1, 0, "int", "")
{
   // Use the current mode
   WsfStringId modeNameId = aObjectPtr->GetCurrentModeName();
   // Get the number of life form types for this mode
   int lifeFormTypeCount = static_cast<int>(aObjectPtr->GetLifeFormTypeCount(modeNameId));
   // Return the count
   aReturnVal.SetInt(lifeFormTypeCount);
}

// int lifeFormCount = <x>.LifeFormTypeCount(string aModeName);
UT_DEFINE_SCRIPT_METHOD(ScriptTricorderSensorClass, TricorderSensor, LifeFormTypeCount_2, 1, "int", "string")
{
   // Argument 1: string aModeName

   // Use the mode name provided and convert to string ID
   WsfStringId modeNameId = WsfStringId(aVarArgs[0].GetString());
   // Get the number of life form types for this mode
   int lifeFormTypeCount = static_cast<int>(aObjectPtr->GetLifeFormTypeCount(modeNameId));
   // Return the count
   aReturnVal.SetInt(lifeFormTypeCount);
}

// int lifeFormEntry = <x>.LifeFormTypeEntry(int aEntryIndex);
UT_DEFINE_SCRIPT_METHOD(ScriptTricorderSensorClass, TricorderSensor, LifeFormTypeEntry_1, 1, "string", "int")
{
   // Argument 1: int aEntryIndex
   unsigned index = (unsigned)aVarArgs[0].GetInt();

   // Use the current mode
   WsfStringId modeNameId = aObjectPtr->GetCurrentModeName();

   if (index >= aObjectPtr->GetLifeFormTypeCount(modeNameId))
   {
      UT_SCRIPT_ABORT("Bad mode name");
   }

   WsfStringId lifeFormEntry = aObjectPtr->GetLifeFormTypeEntry(modeNameId, index);
   if (lifeFormEntry == nullptr)
   {
      UT_SCRIPT_ABORT("Bad index");
   }
   else
   {
      aReturnVal.SetString(lifeFormEntry.GetString());
   }
}

// int lifeFormEntry = <x>.LifeFormTypeEntry(string aModeName, int aEntryIndex);
UT_DEFINE_SCRIPT_METHOD(ScriptTricorderSensorClass, TricorderSensor, LifeFormTypeEntry_2, 2, "string", "string, int")
{
   // Argument 1: string aModeName
   // Argument 2: int    aEntryIndex

   // Use the mode name provided and convert to string ID
   WsfStringId modeNameId = WsfStringId(aVarArgs[0].GetString());
   unsigned index = (unsigned)aVarArgs[1].GetInt();

   if (index >= aObjectPtr->GetLifeFormTypeCount(modeNameId))
   {
      // EXERCISE 3 TASK 2a
      UT_SCRIPT_ABORT("Bad mode name");
   }

   WsfStringId lifeFormEntry = aObjectPtr->GetLifeFormTypeEntry(modeNameId, index);
   if (lifeFormEntry == nullptr)
   {
      // EXERCISE 3 TASK 2b
      UT_SCRIPT_ABORT("Bad index");
   }
   else
   {
      aReturnVal.SetString(lifeFormEntry.GetString());
   }
}
