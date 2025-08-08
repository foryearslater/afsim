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

#include "Interface.hpp"

#include <algorithm>
#include <assert.h>

// datalink
#include "DataLinkLocationMessage.hpp"
#include "GenMemIO.hpp"

// Utilities
#include "DisIO.hpp"
#include "UtCast.hpp"
#include "UtInputBlock.hpp"
#include "UtLog.hpp"
#include "UtMemory.hpp"

// WSF
#include "dis/WsfDisInterface.hpp"
#include "dis/WsfDisPlatform.hpp"
#include "dis/WsfDisSignal.hpp"
#include "WsfDateTime.hpp"
#include "WsfDisObserver.hpp"
#include "WsfSimulation.hpp"

// Comm Lab
#include "LocationMessage.hpp"

namespace CommLab
{
   // static definition
   Interface::LocationMessageReceivedCallback Interface::LocationMessageReceived;

// ============================================================================
   Interface::Interface()
      : WsfSimulationExtension(),
        mDisPtr(nullptr),
        mDebugEnabled(false),
        mPrintMessages(false)
   {
   }

// ============================================================================
   bool Interface::Initialize()
   {
      mCallbacks += WsfObserver::DisSignalReceived(&GetSimulation()).Connect(&Interface::HandleSignalPDU, this);
      return true;
   }

// ============================================================================
   void Interface::Start()
   {
      // Get the DIS interface
      mDisPtr = dynamic_cast<WsfDisInterface*>(
         &GetSimulation().GetExtension("dis_interface"));
   }

// ============================================================================
   bool Interface::ProcessInput(UtInput&    aInput)
   {
      bool myCommand = false;
      std::string cmd;
      if ("comm_lab_interface" == aInput.GetCommand())
      {
         myCommand = true;

         UtInputBlock block(aInput);
         while (block.ReadCommand(cmd))
         {
            // EXERCISE 2 TASK 2a
            // Write an if statement to check if cmd contains "print_messages" and if true,
            // sets mPrintMessages to true
            // PLACE YOUR CODE HERE
            
            // EXERCISE 2 TASK 2b
            // Write an else if statement to check if cmd contains "debug" and if true,
            // sets mDebug to true
            // PLACE YOUR CODE HERE


            else
            {
               throw UtInput::BadValue(aInput);
            }
         }
      }

      return myCommand;
   }

// ============================================================================
   void Interface::SendMessage(double             aSimTime,
                               WsfPlatform*       aSenderPlatformPtr,
                               int                aSourceTrackNumberOffset,
                               const WsfMessage*  aMessagePtr)
   {
      // EXERCISE 3 TASK 6
      // Check for quick return if this message type is not a LocationMessage.
      // PLACE YOUR CODE HERE

      if (mDisPtr != nullptr) // Must be configured with optional DIS interface
      {
         if (DebugEnabled())
         {
            auto out = ut::log::debug() << "Interface::SendMessage called.";
            out.AddNote() << "T = " << aSimTime;
            out.AddNote() << "Sender: " << aSenderPlatformPtr->GetName();
         }

         // Create the DisSignal PDU
         std::unique_ptr<DisSignal> signalPduPtr = ut::make_unique<DisSignal>();

         // Set the entity ID information
         DisEntityId entityId;
         mDisPtr->GetEntityId(aSenderPlatformPtr, entityId);

         // Set PDU information
         signalPduPtr->SetEntityId(entityId);
         signalPduPtr->SetEncodingScheme(DisSignal::EcRawBinary | 1 );
         signalPduPtr->SetSampleRate(0);
         signalPduPtr->SetSampleCount(0);

         // Set type to be generic IP for now
         signalPduPtr->SetTDLType(WsfDisSignal::EtGenericIP);

         // This is a Comm Lab message so recast before getting message data
         const LocationMessage* msgPtr = dynamic_cast<const LocationMessage*>(aMessagePtr);

         if(msgPtr != nullptr)
         {
            unsigned short sourceTrackOffset = ut::safe_cast<unsigned short, size_t>
                                                   (aSourceTrackNumberOffset + aMessagePtr->GetOriginatorIndex());

            // Fill the message that we will send in the signal PDU
            UtCalendar currentTime;
            GetSimulation().GetDateTime().GetCurrentTime(aSimTime, currentTime);

            DataLink::LocationMessage dlMsg(currentTime, sourceTrackOffset);
            dlMsg.mSourceTrackNumber = msgPtr->GetSourceTrackNumber();
            dlMsg.mLatitude          = msgPtr->GetLatitude();
            dlMsg.mLongitude         = msgPtr->GetLongitude();
            dlMsg.mAltitude          = msgPtr->GetAltitude();
            dlMsg.mCourse            = msgPtr->GetCourse();
            dlMsg.mSpeed             = msgPtr->GetSpeed();

            // Use a GenMemIO object to Pack information into the buffer
            // EXERCISE 3 TASK 7a
            // Use a GenMemIO object as the send buffer
            // Parameterize with type GenBuf::Native and size of the message
            // Use the message's Put method to pack the data into the buffer
            // PLACE YOUR CODE HERE

            // EXERCISE 3 TASK 7b
            // Set the user data in the Signal PDU
            // Parameterize with the buffer's contents
            // and size of the message in bits
            // PLACE YOUR CODE HERE

            // EXERCISE 3 TASK 7c
            // Send the PDU using the DIS interface
            // Use the interface's PutPdu method
            // PLACE YOUR CODE HERE

         }
      }
   }

// ============================================================================
   void Interface::HandleSignalPDU(WsfDisInterface* aInterfacePtr, const WsfDisSignal& aPdu)
   {
      // Quick return for unexpected TDL type
      // EXERCISE 3 TASK 8a
      // Use the PDUs GetTDLType() to check that it is of type
      // WsfDisSignal::EtGenericIP, and if not then return
      // PLACE YOUR CODE HERE
      
      if (mDisPtr != nullptr) // Must be configured with optional DIS interface
      {
         // Get the entity ID of the sender
         DisEntityId entityId;
         entityId = aPdu.GetEntityId();

         // Get the DIS platform
         WsfDisPlatform* disPlatformPtr = mDisPtr->FindDisPlatform(entityId);
         if (disPlatformPtr == nullptr)
         {
            return;
         }

         // Get the platform pointer
         WsfPlatform* platformPtr = disPlatformPtr->GetPlatform();
         if (platformPtr == nullptr)
         {
            return;
         }

         if (DebugEnabled())
         {
            auto out = ut::log::debug() << "Received DIS Signal PDU from Platform.";
            out.AddNote() << "Platform: " << platformPtr->GetName();
            out.AddNote() << "DIS Entity: " << entityId;
         }

         // Extract the data from the PDU
         const unsigned char* signalDataPtr = nullptr;
         short unsigned int dataLength  = 0;
         aPdu.GetData(signalDataPtr, dataLength);
         dataLength /= 8;  // Length in bytes

         // Check the size of the data and return if no data
         if (signalDataPtr == nullptr || dataLength == 0)
         {
            return;
         }

         std::vector<unsigned char> signalData;

         std::copy(signalDataPtr, signalDataPtr + dataLength, std::back_inserter(signalData));

         // EXERCISE 3 TASK 8b
         // Use a GenMemIO object that accesses the data buffer from the DIS Signal PDU
         // Parameterize with the signal data, type with type GenBuf::Native and length of the data
         // PLACE YOUR CODE HERE

         // EXERCISE 3 TASK 8c
         // Utilize the DataLink::Message::Create factory method to read the 
         // DataLink::LocationMessage from the buffer
         // PLACE YOUR CODE HERE

         if (msgPtr != nullptr)
         {
            if (msgPtr->GetType() == DataLink::Message::cLOCATION)
            {
               DataLink::LocationMessage* locMsgPtr = dynamic_cast<DataLink::LocationMessage*>(msgPtr);
               if (mPrintMessages)
               {
                  auto out = ut::log::info() << "Received location message.";
                  out.AddNote() << "Source Track Number: " << msgPtr->mSourceTrackNumber;
                  out.AddNote() << "Latitude: " << locMsgPtr->mLatitude;
                  out.AddNote() << "Longitude: " << locMsgPtr->mLongitude;
                  out.AddNote() << "Altitude: " << locMsgPtr->mAltitude;
                  out.AddNote() << "Course: " << locMsgPtr->mCourse;
                  out.AddNote() << "Speed: " << locMsgPtr->mSpeed;
               }

               // Inform observers that we have a message to process:
               LocationMessageReceived(aPdu.GetBestAvailableTime(GetSimulation().GetSimTime()), locMsgPtr);
            }
            else
            {
               auto out = ut::log::info() << "Received message.";
               out.AddNote() << "Type: " << msgPtr->GetType();
            }
         }

         delete msgPtr; // Done with handling message
      }
   }
}
