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

#include "FlightControllerInterface.hpp"
#include "FlightControllerConstants.hpp"

#include <string>

#include "UtInput.hpp"
#include "UtInputFile.hpp"
#include "UtLog.hpp"
#include "UtMemory.hpp"
#include "xio/WsfXIO_InputData.hpp"
#include "xio/WsfXIO_Interface.hpp"
#include "xio/WsfXIO_Request.hpp"
#include "FlightControlPkt.hpp"
#include "FlightControllerPlatformListRequest.hpp"

bool FlightControllerInterface::mPlatformListPktReceived = false;

FlightControllerInterface::FlightControllerInterface()
   : mDirection(cNONE),
     mLastDirection(cNONE),
     mIgnoreNumber(0),
     mDebounce(20),
     mSelectedPlatform(0),
     mSelectedPlatformIndex(0), // No platform selected
     mDebug(false),
     mGetPlatformList(false),
     mYawRate(0.0),
     mPitchRate(0.0),
     mRollRate(0.0),
     mYawRateIncrement(0.1),
     mPitchRateIncrement(0.1),
     mRollRateIncrement(0.1),
     mThrottle(1.0),
     mThrottleIncrement(0.1)
{
}

//! Explicitly request the platform list
void FlightControllerInterface::GetPlatformList(double aSimTime)
{
   { // RAII block
      auto out = ut::log::info() << "GetPlatformList:";
      out.AddNote() << "T = " << aSimTime;
      out.AddNote() << "XIO: " << mXIO_InterfacePtr->GetSimTime();
   }

   mXIO_InterfacePtr->AdvanceTime(aSimTime);
   // create PlatformListUpdated request packet and send it
   WsfXIO_RequestPkt pkt;
   pkt.mSubscriptionType = WsfXIO_RequestPkt::cPLATFORM_LIST;
   mXIO_InterfacePtr->SendToAll(pkt);
}

//! Update the current simulation time
void FlightControllerInterface::AdvanceTime(double aSimTime)
{
   mXIO_InterfacePtr->AdvanceTime(aSimTime);
}

//! Read input from the keyboard and send a packet if needed.
void FlightControllerInterface::Update(double aSimTime)
{
   mXIO_InterfacePtr->AdvanceTime(aSimTime);
   // Issue a message once per second.
   if (((int)(aSimTime * 100.0) % 100) == 0)
   {
      auto out = ut::log::info() << "Update:";
      out.AddNote() << "T = " << aSimTime;
      out.AddNote() << "XIO: " << mXIO_InterfacePtr->GetSimTime();
   }
   HandleKeyPress(false, 0);

   // have to handle Key-P (get platform list) here as GetPlatformList requires sim time
   if (mGetPlatformList)
   {
      mGetPlatformList = false;
      GetPlatformList(aSimTime);
      return;
   }
}

void FlightControllerInterface::HandleKeyPress(bool aKeyPress, int aKey)
{
   bool hasNewInput = UpdateInput(aKeyPress, aKey);
   if (hasNewInput && (mSelectedPlatformIndex != cNO_PLATFORM_SELECTED))
   {
      auto out = ut::log::debug();
      if (mDebug)
      {
         out << "Key pressed: ";
      }
      
      // Send a packet
      FlightControlPkt pkt;
      if (mDirection == cLEFT)
      {
         mRollRate -= mRollRateIncrement;
         mYawRate  -= mYawRateIncrement;
         if (mDebug)
         {
            out << "Turn left";
         }
      }
      else if (mDirection == cRIGHT)
      {
         mRollRate += mRollRateIncrement;
         mYawRate  += mYawRateIncrement;
         if (mDebug)
         {
            out << "Turn right";
         }
      }
      else if (mDirection == cUP)
      {
         mPitchRate -= mPitchRateIncrement;
         if (mDebug)
         {
            out << "Pitch down";
         }
      }
      else if (mDirection == cDOWN)
      {
         mPitchRate += mPitchRateIncrement;
         if (mDebug)
         {
            out << "Pitch up";
         }
      }
      else if (mDirection == cCENTER)
      {
         mRollRate  = 0.0;
         mPitchRate = 0.0;
         mYawRate   = 0.0;
         if (mDebug)
         {
            out << "Null Rates";
         }
      }

      //! Fill the rate values in the packet.
      //! Use degrees for the angular rates.
      pkt.mYawRate   = TransferFunction(mYawRate);
      pkt.mRollRate  = TransferFunction(mRollRate);
      pkt.mPitchRate = TransferFunction(mPitchRate);
      pkt.mThrottle  = mThrottle;

      if (mDebug)
      {
         out.AddNote() << "Yaw:   " << pkt.mYawRate;
         out.AddNote() << "Pitch: " << pkt.mPitchRate;
         out.AddNote() << "Roll:  " << pkt.mRollRate;
      }
      
      out.Send();

      pkt.mPlatformIndex = mSelectedPlatformIndex;
      mXIO_InterfacePtr->SendToAll(pkt);
   }
}

//! Process Input for several attributes
bool FlightControllerInterface::ProcessInput(UtInput& aInput)
{
   bool myCommand = true;
   std::string command;
   aInput.GetCommand(command);
   if (command == "pitch_rate_increment")
   {
      aInput.ReadValue(mPitchRateIncrement);
      aInput.ValueInClosedRange(mPitchRateIncrement, 0.0, 1.0);
   }
   else if (command == "yaw_rate_increment")
   {
      aInput.ReadValue(mYawRateIncrement);
      aInput.ValueInClosedRange(mYawRateIncrement, 0.0, 1.0);
   }
   else if (command == "roll_rate_increment")
   {
      aInput.ReadValue(mRollRateIncrement);
      aInput.ValueInClosedRange(mRollRateIncrement, 0.0, 1.0);
   }
   else if (command == "throttle_increment")
   {
      aInput.ReadValue(mThrottleIncrement);
      aInput.ValueInClosedRange(mThrottleIncrement, 0.0, 1.0);
   }
   else if (command == "debounce")
   {
      aInput.ReadValue(mIgnoreNumber);
   }
   else if (command == "debug")
   {
      mDebug = true;
   }
   else
   {
      myCommand = false;
   }

   return myCommand;
}

//! Provide custom initialization.  Configure the XIO interface, initialize it,
//! register the command packet type, and subscribe for platform list updates.
bool FlightControllerInterface::Initialize(const std::string& aFileName)
{
   bool ok = true;
   // EXERCISE 3 TASK 2
   // Create the XIO Interface and set its application name.
   mXIO_InterfacePtr = ut::make_unique<WsfXIO_Interface>();
   mXIO_InterfacePtr->SetApplicationName("flight_controller");

   try
   {
      UtInput input;
      input.PushInput(ut::make_unique<UtInputFile>(aFileName));
      std::string command;
      while (input.TryReadCommand(command))
      {
         // EXERCISE 3 TASK 3
         // Call ProcessInput and throw Ut::UnknownCommand exception if neither
         // FlightControllerInterface::ProcessInput nor WsfXIO_Interface::Process
         // can process it
         if (ProcessInput(input))
         {
         }
         else if (mXIO_InterfacePtr->ProcessInput(input))
         {
         }
         else
         {
            throw UtInput::UnknownCommand(input);
         }
      }
   }
   catch (UtInputFile::OpenError&)
   {
      auto out = ut::log::error() << "Could not open flight controller configuration file.";
      out.AddNote() << "File: " << aFileName;
      ok = false;
   }
   catch (std::exception& err)
   {
      ut::log::error() << "Exception: " << err.what();
      ok = false;
   }
   catch (...)
   {
      ut::log::error() << "Unknown exception.";
      ok = false;
   }

   // A connection type must be specified in the input file.
   if (ok &&
       (!mXIO_InterfacePtr->IsXIO_Requested()))
   {
      ut::log::error() << "XIO connection not defined.";
      ok = false;
   }
   // If everything is OK initialize our connection.
   if (ok &&
       (! mXIO_InterfacePtr->Initialize()))
   {
      ut::log::error() << "Unable to initialize XIO interface.";
      ok = false;
   }

   if (ok)
   {
      // EXERCISE 3 TASK 4a
      // Use the XIO Interface to Register this New Packet Type
      // The packet should be called "FlightControlPkt"
      mXIO_InterfacePtr->RegisterPacket("FlightControlPkt", new FlightControlPkt());

      // EXERCISE 3 TASK 4b
      // Use the XIO Interface's RequestManager to Register for Platform List Updates
      // Get the RequestManager from the XIO_Interface
      // Call AddRequest with an argument of a FlightContollerPlatformListRequest*
      mXIO_InterfacePtr->GetRequestManager().AddRequest(
        new FlightControllerPlatformListRequest(mXIO_InterfacePtr->GetConnections()[0]));
   }
   else
   {
      ut::log::error() << "Unable to register FlightControlPkt or FlightControllerPlatformListRequest callback.";
   }

   return ok;
}

double FlightControllerInterface::TransferFunction(double aLinearInput)
{
   if (aLinearInput >= 0.0)
   {
      return (1.0 - exp(-aLinearInput));
   }
   else
   {
      return (-1.0 + exp(aLinearInput));
   }
}

//! Reads input from the keyboard and updates class attributes.
bool FlightControllerInterface::UpdateInput(bool aKeyPress, int aKeyStroke)
{
   if(aKeyPress)
   {
      mIgnoreNumber = 0;
      if ((aKeyStroke != 0) && (aKeyStroke != -32))
      {
         if ((aKeyStroke == Qt::Key_Up) || (aKeyStroke == Qt::Key_W))
         {
            mDirection = cUP;
         }
         else if ((aKeyStroke == Qt::Key_Down) || (aKeyStroke == Qt::Key_S))
         {
            mDirection = cDOWN;
         }
         else if ((aKeyStroke == Qt::Key_Left) || (aKeyStroke == Qt::Key_A))
         {
            mDirection = cLEFT;
         }
         else if ((aKeyStroke == Qt::Key_Right) || (aKeyStroke == Qt::Key_D))
         {
            mDirection = cRIGHT;
         }
         else if (aKeyStroke == Qt::Key_L)
         {
            const FlightControllerPlatformListRequest::Platforms& platforms = FlightControllerPlatformListRequest::GetPlatforms();
            auto out = ut::log::info() << "Platform Listing: ";
            for (unsigned i = 0; i < platforms.size(); ++i)
            {
               out.AddNote() << "Index: " << platforms[i].mIndex;
               out.AddNote() << "Name: " << platforms[i].mName.GetString();
            }
         }
         else if (aKeyStroke == Qt::Key_U)
         {
            {
               ut::log::info() << "Requesting Platform List Update";
            }
            mGetPlatformList = true;
         }
         else if ((aKeyStroke == Qt::Key_PageUp) || (aKeyStroke == Qt::Key_BracketRight))
         {
            mThrottle += mThrottleIncrement;
            const static double cMAX_THROTTLE = 2.0;  // including afterburner effect
            if (mThrottle > cMAX_THROTTLE)
            {
               mThrottle = cMAX_THROTTLE;
            }
            ut::log::info() << "Throttle increased to " << mThrottle;
         }
         else if ((aKeyStroke == Qt::Key_PageDown) || (aKeyStroke == Qt::Key_BracketLeft))
         {
            mThrottle -= mThrottleIncrement;
            if (mThrottle < 0.0)
            {
               mThrottle = 0.0;
            }
            ut::log::info() << "Throttle reduced to " << mThrottle;
         }
         else if ((aKeyStroke >= Qt::Key_1) && (aKeyStroke <= Qt::Key_9))
         {
            mSelectedPlatformIndex = aKeyStroke - Qt::Key_1 + 1;
            const FlightControllerPlatformListRequest::Platforms& platforms = FlightControllerPlatformListRequest::GetPlatforms();
            bool found = false;
            for (unsigned i = 0; i < platforms.size(); ++i)
            {
               if (platforms[i].mIndex == mSelectedPlatformIndex)
               {
                  mSelectedPlatform = i;
                  found = true;
                  
                  auto out = ut::log::info() << "Selected platform.";
                  out.AddNote() << "Index: " << platforms[mSelectedPlatform].mIndex;
                  out.AddNote() << "Name: " << platforms[mSelectedPlatform].mName.GetString();
                  
                  break;
               }
            }
            if (! found)
            {
               auto out = ut::log::warning() << "Selected platform does not exist.";
               out.AddNote() << "Platform Index: " << mSelectedPlatformIndex;
               mSelectedPlatformIndex = cNO_PLATFORM_SELECTED;
            }
         }
         else if (aKeyStroke == Qt::Key_Backspace)
         {
            mSelectedPlatformIndex = cINVALID_PLATFORM_INDEX;
            mSelectedPlatform = cNO_PLATFORM_SELECTED;
            ut::log::info() << "All Platforms Deselected.";
         }

         else
         {
            ut::log::info() << "Key pressed: " << aKeyStroke;
         }
      }
   }
   else
   {
      ++mIgnoreNumber;
      if (mIgnoreNumber == mDebounce)
      {
         mDirection = cCENTER;
      }
      else
      {
         mDirection = cNONE;
      }
   }

   bool haveNew = (mLastDirection != mDirection);
   mLastDirection = mDirection;
   return haveNew;
}
