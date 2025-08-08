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

#include "UDP_Observer.hpp"

#include <sstream>

// GenIO
#include "GenSocketManager.hpp"
#include "GenUDP_Connection.hpp"

// Utilities
#include "UtCast.hpp"
#include "UtInputBlock.hpp"
#include "UtLog.hpp"

#include "RegisterUDP_Observer.hpp"

// WSF
#include "WsfApplication.hpp"
#include "WsfApplicationExtension.hpp"
#include "WsfException.hpp"
#include "observer/WsfPlatformObserver.hpp"
#include "observer/WsfTrackObserver.hpp"
#include "WsfPlatform.hpp"
#include "sensor/WsfSensor.hpp"
#include "WsfSimulation.hpp"
#include "WsfTrack.hpp"

// ****************************************************************************
// Public

// ============================================================================
// Constructor and destructor
//! Constructs the UDP_Observer class
UDP_Observer::UDP_Observer()
   : mPort(14421),
     mAddress(),
     mConnectionPtr(nullptr)
{
}

UDP_Observer::UDP_Observer(const UDP_Observer& aSrc)
   : mPort(aSrc.mPort),
     mAddress(aSrc.mAddress),
     mConnectionPtr(nullptr)
{
}

UDP_Observer& UDP_Observer::operator=(const UDP_Observer& aSrc)
{
   mPort = aSrc.mPort;
   mAddress = aSrc.mAddress;
   mConnectionPtr = nullptr;
   return *this;
}

UDP_Observer::~UDP_Observer() noexcept
{
   Disconnect();
}

// ****************************************************************************
// Private

// ============================================================================
//! Initialize the UDP connection.
//! @return False if the connection cannot be established.
bool UDP_Observer::Initialize()
{
   if (mAddress.empty())
   {
      Disconnect();
      return true;
   }
   bool connected = false;

   // Create a new connection and initialize

   // EXERCISE 1 TASK 5
   // Create a new GenUDP_Connection and assign it to class member mConnectionPtr.
   // Initialize the GenUDP_Connection using the Init() method.
   // Refer to the header file GenUDP_Connection.hpp in the
   // genio project of the Visual Studio solution for correct syntax.
   // Ensure local variable 'connected' is set to true if connection is successfully initialized.

   mConnectionPtr = new GenUDP_Connection();
   connected = mConnectionPtr->Init(mAddress, mPort);

   // On successful connection subscribe to additional events
   // See WsfObserver.hpp for all typedefs of all AFSIM-provided available events
   if (connected)
   {
      mCallbacks.Add(WsfObserver::SensorTrackUpdated(&GetSimulation()).Connect(&UDP_Observer::SensorTrackUpdated, this));
      mCallbacks.Add(WsfObserver::SensorTrackInitiated(&GetSimulation()).Connect(&UDP_Observer::SensorTrackUpdated, this));
	  
	  // EXERCISE 1 TASK 6a
	  // Subscribe to the WsfObserver::PlatformAdded callback.
	  // When WsfObserver::PlatformAdded is invoked, we should call UDP_Observer::PlatformAdded
      mCallbacks.Add(WsfObserver::PlatformAdded(&GetSimulation()).Connect(&UDP_Observer::PlatformAdded, this));
	  
	  // EXERCISE 1 TASK 6b
	  // Subscribe to the WsfObserver::PlatformDeleted callback.
 	  // When WsfObserver::PlatformDeleted is invoked, we should call UDP_Observer::PlatformDeleted
      mCallbacks.Add(WsfObserver::PlatformDeleted(&GetSimulation()).Connect(&UDP_Observer::PlatformDeleted, this));
   }
   else
   {
      auto out = ut::log::error() << "Could not connect to socket.";
      out.AddNote() << "Address: " << mAddress;
      out.AddNote() << "Port: " << mPort;
      
      delete mConnectionPtr;
      mConnectionPtr = nullptr;
   }
   return connected;
}

// ============================================================================
//! Process input from the AFSIM input file.
bool UDP_Observer::ProcessInput(UtInput& aInput)
{
   /** We are searching for a command of the form:
   udp_observer
      port      4321
      address   www.google.com
   end_udp_observer  */

   bool myCommand = false;
   // EXERCISE 1 TASK 7 
   // Have the following "if" statement compare the result of
   //                     the aInput.GetCommand() to the string "udp_observer".
   if (aInput.GetCommand() == "udp_observer")
   {
      myCommand = true;

      // UtInputBlock automatically stops when end_udp_observer is found
      UtInputBlock block(aInput);

      std::string command;
      while (block.ReadCommand(command))
      {
         // EXERCISE 1 TASK 8a 
         // Add code to process the keyword for "port" below.
         // [Note: Students can find many AFSIM examples by searching for references to ReadCommand.]
         if (command == "port")
         {
            // Read the value corresponding to the "the_keyword" from aInput
            // Assign values to the UDP_Observer class members mPort.
            aInput.ReadValue(mPort);
         }
         // EXERCISE 1 TASK 8b 
         // Add code to process the keyword for "address" below.
         // [Note: Students can find many AFSIM examples by searching for references to ReadCommand.]
         else if (command == "address")
         {
            // Read the value corresponding to the "another_keyword" from aInput
            // Assign values to the UDP_Observer class members mAddress.
            aInput.ReadValue(mAddress);

         }
         else
         {
            throw UtInput::UnknownCommand(aInput);
         }
      }
   }
   return myCommand;
}

// ============================================================================
//! This method is called in response to a platform being added to the
//! simulation.
void UDP_Observer::PlatformAdded(double       aSimTime,
                                 WsfPlatform* aPlatformPtr)
{
   // EXERCISE 1 TASK 9a
   // Students should inspect WsfPlatform.hpp and the base class WsfObject.hpp.
   // Use the aPlatformPtr argument to access selected methods, capture the
   // data, and write it to a string. Inspect WsfPlatform.hpp and the base class WsfObject.hpp
   // to discover what public methods and data are available.
   // Then send the string with the selected data to the UDP_Observer::SendPacket function
   // that has been provided for this class.
   //
   // [Note: This task can be solved in many ways]
   std::stringstream ss;
   ss << "PlatformAdded," << aSimTime
      << ',' << aPlatformPtr->GetName()
      << ',' << aPlatformPtr->GetType();
   SendPacket(ss.str());
}

// ============================================================================
//! This method is called in response to a platform being removed from the
//! simulation.
void UDP_Observer::PlatformDeleted(double       aSimTime,
                                   WsfPlatform* aPlatformPtr)
{
   // EXERCISE 1 TASK 9b
   // Use TASK 9a as an example
   std::stringstream ss;
   ss << "PlatformDeleted," << aSimTime
      << ',' << aPlatformPtr->GetName()
      << ',' << aPlatformPtr->GetType();
   SendPacket(ss.str());
}

// ============================================================================
//! This method is called in response to a sensor track being initiated or
//! updated.
void UDP_Observer::SensorTrackUpdated(double          aSimTime,
                                      WsfSensor*      aSensorPtr,
                                      const WsfTrack* aTrackPtr)
{
   // EXERCISE 1 TASK 9c
   // Use TASK 8a as an example
   std::stringstream ss;
   double lat, lon, alt;
   aTrackPtr->GetLocationLLA(lat, lon, alt);
   ss << "SensorTrackUpdated," << aSimTime
      << ',' << aSensorPtr->GetName()
      << ',' << aSensorPtr->GetPlatform()->GetIndex()
      << ',' << aTrackPtr->GetTargetIndex()
      << ',' << lat
      << ',' << lon
      << ',' << alt;
   SendPacket(ss.str());
}

// ============================================================================
////! Destroy the socket, and disconnect from all callbacks.
void UDP_Observer::Disconnect()
{
   delete mConnectionPtr;
   mConnectionPtr = nullptr;
   // It is good practice to disconnect from callbacks you don't need to
   // reduce overhead.
   mCallbacks.Clear();
}

// ============================================================================
//! Send a UDP packet given a packet type ID and a buffer of data
void UDP_Observer::SendPacket(const std::string& aMessage)
{
   int bytesWritten = mConnectionPtr->SendBuffer(aMessage.c_str(), ut::cast_to_int(aMessage.length() + 1));
   if (bytesWritten < 0)
   {
      ut::log::error() << "Socket error: " << GenSockets::GenSocketManager::GetLastError();
      Disconnect();
   }
}
