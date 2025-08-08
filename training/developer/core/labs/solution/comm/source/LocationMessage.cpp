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

#include "LocationMessage.hpp"

// Application Specific
#include "DataLinkLocationMessage.hpp"

// Utilities
#include "UtMath.hpp"
#include "UtMemory.hpp"

// WSF
#include "WsfAttributeContainer.hpp"
#include "WsfPlatform.hpp"
#include "script/WsfScriptContext.hpp"

// ****************************************************************************
// Public

LocationMessage::LocationMessage()
   : WsfMessage(GetTypeId()),
     mSourceTrackNumber(0),
     mLatitude(0.0),
     mLongitude(0.0),
     mAltitude(0.0),
     mCourse(0.0),
     mSpeed(0.0)
{
}

// ============================================================================
//! Constructor (create a message)
//! @param aPlatformPtr The originator of the message.
LocationMessage::LocationMessage(WsfPlatform* aPlatformPtr)
   : WsfMessage(GetTypeId(), nullptr, aPlatformPtr),
     mSourceTrackNumber(0),
     mLatitude(0.0),
     mLongitude(0.0),
     mAltitude(0.0),
     // EXERCISE 3 TASK 3a
     mCourse(0.0),
     mSpeed(0.0)
{
   // Set the source track number
   mSourceTrackNumber = aPlatformPtr->GetAuxData().GetInt("SOURCE_TRACK_NUMBER");

   // EXERCISE 3 TASK 3b
   // Get location
   aPlatformPtr->GetLocationLLA(mLatitude, mLongitude, mAltitude);

   // Get course
   double velNED[3] = { 0.0 };
   aPlatformPtr->GetVelocityNED(velNED);
   mCourse = atan2(velNED[1], velNED[0]);

   // Get speed
   mSpeed = aPlatformPtr->GetSpeed();
}

// ============================================================================
//! Constructor (create a message)
//! @param aLocMsgPtr The analog of this message sent over DIS signal PDU.
LocationMessage::LocationMessage(DataLink::LocationMessage* aLocMsgPtr)
   : WsfMessage(GetTypeId())
   , mSourceTrackNumber(aLocMsgPtr->mSourceTrackNumber)
   , mLatitude(aLocMsgPtr->mLatitude)
   , mLongitude(aLocMsgPtr->mLongitude)
   , mAltitude(aLocMsgPtr->mAltitude)
   , mCourse(aLocMsgPtr->mCourse)
   , mSpeed(aLocMsgPtr->mSpeed)
{
}

// ============================================================================
//! Create a clone of the this message (the 'virtual copy constructor').
//! @return The pointer to the copy of this message.
//virtual
WsfMessage* LocationMessage::Clone() const
{
   return new LocationMessage(*this);
}

// ============================================================================
//! Get the type ID associated with this message.
//! @return The string ID of this message.
//static
WsfStringId LocationMessage::GetTypeId()
{
   static const WsfStringId typeId("LOCATION_MESSAGE");
   return typeId;
}

//! Get the name of the script class associated with this class.
//! This is necessary for proper downcasts in the scripting language.
//virtual
const char* LocationMessage::GetScriptClassName() const
{
   return "LocationMessage";
}

// ***************************************************************************
//! Create the 'class' object for the script system.
//! This is invoked once by WsfScriptManager to create the 'class' object that defines
//! the interface to instances of this class from the script system.
ScriptLocationMessageClass::ScriptLocationMessageClass(const std::string& aClassName,
                                                       UtScriptTypes*     aScriptTypesPtr)
   : WsfScriptMessageClass(aClassName, aScriptTypesPtr)
{
   SetClassName("LocationMessage");

   mConstructible = true;
   mCloneable = true;

   AddMethod(ut::make_unique<SourceTrackNumber>());
   AddMethod(ut::make_unique<Latitude>());
   AddMethod(ut::make_unique<Longitude>());
   AddMethod(ut::make_unique<Altitude>());
   // EXERCISE 4 TASK 2
   // add calls to AddMethod for the script methods Course and Speed
   AddMethod(ut::make_unique<Course>());
   AddMethod(ut::make_unique<Speed>());
}

//virtual
void* ScriptLocationMessageClass::Create(const UtScriptContext& aContext)
{
   WsfPlatform* platformPtr = WsfScriptContext::GetPLATFORM(aContext);
   return new LocationMessage(platformPtr);
}

//virtual
void* ScriptLocationMessageClass::Clone(void* aObjectPtr)
{
   return static_cast<LocationMessage*>(aObjectPtr)->Clone();
}

//virtual
void ScriptLocationMessageClass::Destroy(void* aObjectPtr)
{
   delete static_cast<LocationMessage*>(aObjectPtr);
}

UT_DEFINE_SCRIPT_METHOD(ScriptLocationMessageClass, LocationMessage, SourceTrackNumber, 0, "int", "")
{
  aReturnVal.SetInt(aObjectPtr->GetSourceTrackNumber());
}

UT_DEFINE_SCRIPT_METHOD(ScriptLocationMessageClass, LocationMessage, Latitude, 0, "double", "")
{
  aReturnVal.SetDouble(aObjectPtr->GetLatitude());
}

UT_DEFINE_SCRIPT_METHOD(ScriptLocationMessageClass, LocationMessage, Longitude, 0, "double", "")
{
   aReturnVal.SetDouble(aObjectPtr->GetLongitude());
}

UT_DEFINE_SCRIPT_METHOD(ScriptLocationMessageClass, LocationMessage, Altitude, 0, "double", "")
{
   aReturnVal.SetDouble(aObjectPtr->GetAltitude());
}

// EXERCISE 4 TASK 3a
UT_DEFINE_SCRIPT_METHOD(ScriptLocationMessageClass, LocationMessage, Course, 0, "double", "")
{
   aReturnVal.SetDouble(aObjectPtr->GetCourse() * UtMath::cDEG_PER_RAD);
}

// EXERCISE 4 TASK 3b
UT_DEFINE_SCRIPT_METHOD(ScriptLocationMessageClass, LocationMessage, Speed, 0, "double", "")
{
   aReturnVal.SetDouble(aObjectPtr->GetSpeed());
}

