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

#ifndef FLIGHT_CONTROLLER_INTERFACE
#define FLIGHT_CONTROLLER_INTERFACE

#include <string>
#include <QtWidgets>

#include "UtCallbackHolder.hpp"
#include "xio/WsfXIO_Interface.hpp"

class     UtInput;
class     WsfXIO_Connection;
class     WsfXIO_Interface;

//! Provide an example interface to control an air body in another simulation.
//! This class reads control inputs from the keyboard and sends it to the
//! simulation via XIO.
class FlightControllerInterface
{
   public:

      enum Direction
      {
         cNONE,
         cLEFT,
         cRIGHT,
         cUP,
         cDOWN,
         cCENTER
      };

      FlightControllerInterface();

      bool ProcessInput(UtInput& aInput);
      bool Initialize(const std::string& aFileName);
      void GetPlatformList(double aSimTime);
      void AdvanceTime(double aSimTime);
      void Update(double aSimTime);
      void HandleKeyPress(bool aKeyPress, int aKey);

      static bool IsFirstPktReceived() { return mPlatformListPktReceived; }
      static void PlatformListPktReceived()   { mPlatformListPktReceived = true; }

   private:

      double TransferFunction(double aLinearInput);

      std::unique_ptr<WsfXIO_Interface> mXIO_InterfacePtr;
      UtCallbackHolder  mCallbacks;

      Direction mDirection;
      Direction mLastDirection;
      unsigned  mIgnoreNumber;
      unsigned  mDebounce;

      unsigned  mSelectedPlatform;
      int       mSelectedPlatformIndex;
      bool      mDebug;
      bool      mGetPlatformList;

      double    mYawRate;
      double    mPitchRate;
      double    mRollRate;

      double    mYawRateIncrement;
      double    mPitchRateIncrement;
      double    mRollRateIncrement;

      double    mThrottle;
      double    mThrottleIncrement;

      bool UpdateInput(bool aKeyPress, int aKeyStroke);

      static bool mPlatformListPktReceived;
};

#endif
