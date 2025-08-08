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

#ifndef COMM_LAB_MESSAGE_HPP
#define COMM_LAB_MESSAGE_HPP

// WSF
#include "WsfMessage.hpp"

// Forward declarations
class UtScriptClass;
class UtScriptTypes;
class WsfPlatform;

namespace DataLink
{
   class LocationMessage;
}

//! A message providing platform location.
class LocationMessage : public WsfMessage
{
   public:
      LocationMessage();

      explicit LocationMessage(WsfPlatform* aPlatformPtr);
      explicit LocationMessage(DataLink::LocationMessage* aLocMsgPtr);

      WsfMessage* Clone() const override;

      static WsfStringId GetTypeId();

      int GetSourceTrackNumber() const { return mSourceTrackNumber; }
      void SetSourceTrackNumber(int aSourceTrackId) { mSourceTrackNumber = aSourceTrackId; }

      double GetLatitude()  const { return mLatitude; }
      void SetLatitude(double aLatitude)   { mLatitude  = aLatitude; }

      double GetLongitude() const { return mLongitude; }
      void SetLongitude(double aLongitude) { mLongitude = aLongitude; }

      double GetAltitude()  const { return mAltitude; }
      void SetAltitude(double aAltitude)   { mAltitude  = aAltitude; }

      // EXERCISE 3 TASK 2
      // PLACE YOUR CODE HERE



      const char* GetScriptClassName() const override;

   private:

      int      mSourceTrackNumber;
      double   mLatitude;
      double   mLongitude;
      double   mAltitude;
      // EXERCISE 3 TASK 1
      // PLACE YOUR CODE HERE

};


#include "script/WsfScriptMessageClass.hpp"

//! The script interface 'class'
class ScriptLocationMessageClass : public WsfScriptMessageClass
{
   public:
      ScriptLocationMessageClass(const std::string& aClassName,
                                 UtScriptTypes*     aScriptTypePtr);

      void* Create(const UtScriptContext& aContext) override;
      void* Clone(void* aObjectPtr) override;
      void  Destroy(void* aObjectPtr) override;

      UT_DECLARE_SCRIPT_METHOD(SourceTrackNumber);
      UT_DECLARE_SCRIPT_METHOD(Latitude);
      UT_DECLARE_SCRIPT_METHOD(Longitude);
      UT_DECLARE_SCRIPT_METHOD(Altitude);
      // EXERCISE 4 TASK 1
      // PLACE YOUR CODE HERE

};
#endif
