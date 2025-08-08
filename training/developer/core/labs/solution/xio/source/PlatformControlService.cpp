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

#include "PlatformControlService.hpp"
#include "FlightControllerConstants.hpp"

// Utilities
#include "UtLog.hpp"

// XIO
#include "xio_sim/WsfXIO_Extension.hpp"
#include "xio/WsfXIO_Interface.hpp"
#include "xio/WsfXIO_Packet.hpp"

// AFSIM
#include "WsfApplication.hpp"
#include "WsfScenario.hpp"
#include "WsfPlatform.hpp"
#include "WsfSimulation.hpp"
#include "WsfP6DOF_Mover.hpp"

// This exercise
#include "FlightControlPkt.hpp"

// ****************************************************************************
// Public

// ============================================================================
// Constructor and destructor
//! Constructs the PlatformControlService class
PlatformControlService::PlatformControlService()
   : mXIO_Ptr(nullptr),
     mControlledPlatformIndex(UINT_MAX)
{
}

// ****************************************************************************
// Private

// ============================================================================
//! Initialize the controller.
void PlatformControlService::Start()
{
   mXIO_Ptr = WsfXIO_Extension::Find(GetSimulation());

   // If the xio interface was not configured the pointer will be zero.
   if (mXIO_Ptr != nullptr)
   {
      // EXERCISE 1 TASK 2a
      // Register our new packet type
      mXIO_Ptr->RegisterPacket("FlightControlPkt", new FlightControlPkt());

      // EXERCISE 1 TASK 2b
      // Subscribe to be notified when a new Flight Control packet is received.
      mCallbacks += mXIO_Ptr->Connect(FlightControlPkt::cPACKET_ID,               // The ID of the packet of interest
                                      &PlatformControlService::ControlPlatform,   // the Function to be called to operate on the packet
                                      this);                                      // The instance that will operate on the packet
   }

   // Make sure the autopilot is initially turned on for all 6-dof movers
   for (unsigned i = 0; i < GetSimulation().GetPlatformCount(); ++i)
   {
      WsfPlatform* platformPtr = GetSimulation().GetPlatformEntry(i);
      WsfP6DOF_Mover* p6DofMoverPtr = dynamic_cast<WsfP6DOF_Mover*>(platformPtr->GetMover());
      if (p6DofMoverPtr != nullptr)
      {
         p6DofMoverPtr->ReleaseDirectControlInput();
         p6DofMoverPtr->EnableAutopilot(true);
      }
   }
}

//! Method called by XIO to control the platform based on received packet values
//! @param aPacket The packet received by XIO and passed to this method.
void PlatformControlService::ControlPlatform(PakPacket& aPacket)
{
   // EXERCISE 2 TASK 1
   // Perform an ID check to verify that the packet's ID corresponds with aPacket's ID
   if (aPacket.ID() == FlightControlPkt::cPACKET_ID)
   {
      FlightControlPkt flightControlPacket = static_cast<FlightControlPkt&>(aPacket);

      if (flightControlPacket.mPlatformIndex == cINVALID_PLATFORM_INDEX)
      {
         WsfPlatform* currentPlatformPtr = GetSimulation().GetPlatformByIndex(
            mControlledPlatformIndex);
         if (currentPlatformPtr != nullptr)
         {
            DeselectPlatform(currentPlatformPtr);
         }
         mControlledPlatformIndex = cNO_PLATFORM_SELECTED;
      }
      else if (flightControlPacket.mPlatformIndex != mControlledPlatformIndex)
      {
         WsfPlatform* currentPlatformPtr = GetSimulation().GetPlatformByIndex(
            mControlledPlatformIndex);

         if (currentPlatformPtr != nullptr)
         {
            DeselectPlatform(currentPlatformPtr);
         }

         mControlledPlatformIndex = flightControlPacket.mPlatformIndex;
         currentPlatformPtr = GetSimulation().GetPlatformByIndex(
            mControlledPlatformIndex);

         if (currentPlatformPtr != nullptr)
         {
            SelectPlatform(currentPlatformPtr);
         }
         else
         {
            auto out = ut::log::warning() << "Could not find platform.";
            out.AddNote() << "Index: " << flightControlPacket.mPlatformIndex;
         }
      }

      if (mControlledPlatformIndex != cNO_PLATFORM_SELECTED)
      {
         // EXERCISE 2 TASK 2
         // Use the simulation object to get the currently selected platform
         // Get the platform's mover and cast it to a WsfP6DOF_Mover
         // Set the control inputs using values from the flight control packet
         // Generate ut::log::info() output indicating a packet was received, and its yaw, 
         // pitch, roll, and throttle values
         WsfPlatform* currentPlatformPtr = GetSimulation().GetPlatformByIndex(mControlledPlatformIndex);
         if (currentPlatformPtr != nullptr)
         {
            WsfP6DOF_Mover* p6DofMoverPtr = dynamic_cast<WsfP6DOF_Mover*>(currentPlatformPtr->GetMover());
            if (p6DofMoverPtr != nullptr)
            {
               p6DofMoverPtr->SetDirectControlInputs(flightControlPacket.mRollRate,
                                                     flightControlPacket.mPitchRate,
                                                     flightControlPacket.mYawRate,
                                                     flightControlPacket.mThrottle);
                                                     
               auto out = ut::log::info() << "Received flight control packet.";
               out.AddNote() << "Yaw: " << flightControlPacket.mYawRate;
               out.AddNote() << "Pitch: " << flightControlPacket.mPitchRate;
               out.AddNote() << "Roll: " << flightControlPacket.mRollRate;
               out.AddNote() << "Throttle: " << flightControlPacket.mThrottle;
            }
         }
      }
   }
}

// private
void PlatformControlService::SelectPlatform(const WsfPlatform * aPlatformPtr)
{
   WsfP6DOF_Mover* p6DofMoverPtr = dynamic_cast<WsfP6DOF_Mover*>(aPlatformPtr->GetMover());

   if (p6DofMoverPtr != nullptr)
   {

      {// RAII block
         auto out = ut::log::info() << "Selecting platform.";
         out.AddNote() << "Platform: " << aPlatformPtr->GetName();
      }
      
      p6DofMoverPtr->EnableControls(true);
      p6DofMoverPtr->EnableAutopilot(false);
      p6DofMoverPtr->TakeDirectControlInput();
   }
   else
   {
      mControlledPlatformIndex = cNO_PLATFORM_SELECTED;
   }
}

// private
void PlatformControlService::DeselectPlatform(const WsfPlatform * aPlatformPtr)
{
   { // RAII block
      auto out = ut::log::info() << "Deselecting platform.";
      out.AddNote() << "Platform: " << aPlatformPtr->GetName();
   }

   WsfP6DOF_Mover* p6DofMoverPtr = dynamic_cast<WsfP6DOF_Mover*>(aPlatformPtr->GetMover());
   if (p6DofMoverPtr != nullptr)
   {
      p6DofMoverPtr->ReleaseDirectControlInput();
      p6DofMoverPtr->EnableAutopilot(true);
   }
}
