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

#include "SignalComm.hpp"

// Application specific
#include "DataLinkLocationMessage.hpp"
#include "Interface.hpp"
#include "LocationMessage.hpp"

#include "UtLog.hpp"
#include "UtMemory.hpp"

// WSF
#include "WsfAttributeContainer.hpp"
#include "WsfCommApplicationLayer.hpp"
#include "WsfCommDatalinkLayer.hpp"
#include "WsfCommNetworkLayer.hpp"
#include "WsfCommTransportLayer.hpp"
#include "WsfCommLayer.hpp"
#include "WsfCommMessage.hpp"
#include "WsfCommPhysicalLayer.hpp"
#include "WsfCommProtocolLegacy.hpp"
#include "comm/WsfCommResult.hpp"
#include "WsfMessage.hpp"
#include "WsfMessageTable.hpp"
#include "observer/WsfCommObserver.hpp"
#include "WsfPlatform.hpp"
#include "WsfSimulation.hpp"

// ****************************************************************************
// Public

namespace wsf
{
namespace comm
{
// ============================================================================
// Constructor and destructor
SignalComm::SignalComm(WsfScenario& aScenario)
   : Comm(aScenario),
     mSourceTrackNumber(1),
     mInterfacePtr(nullptr),
     mCallbacks()
{
   SetClassId(GetSignalCommClassId());

   // Build the protocol stack from lower to upper layers
  mProtocolStack.AddLayer(ut::make_unique<Layer>(Layer::LayerType::cPHYSICAL, new PhysicalLayer));
  mProtocolStack.AddLayer(ut::make_unique<Layer>(Layer::LayerType::cDATALINK, new DatalinkLayer));
  mProtocolStack.AddLayer(ut::make_unique<Layer>(Layer::LayerType::cNETWORK, new NetworkLayer));
  mProtocolStack.AddLayer(ut::make_unique<Layer>(Layer::LayerType::cTRANSPORT, new TransportLayer));
  mProtocolStack.AddLayer(ut::make_unique<Layer>(Layer::LayerType::cAPPLICATION, new ApplicationLayer));
}

//! Return the class ID associated with objects of this type.
//! @return the string ID of the class name for objects of this class.
//static
WsfStringId SignalComm::GetSignalCommClassId()
{
   static WsfStringId classId("SIGNAL_COMM");
   return classId;
}

// ============================================================================
// Virtual
Comm* SignalComm::Clone() const
{
   return new SignalComm(*this);
}

// ============================================================================
bool SignalComm::ProcessInput(UtInput& aInput)
{
   bool   myCommand = true;
   std::string command(aInput.GetCommand());
   if (command == "source_track_number")
   {
      int offset;
      aInput.ReadValue(offset);
      mSourceTrackNumber = static_cast<unsigned int>(offset);
   }
   else
   {
      myCommand = Comm::ProcessInput(aInput);
   }
   return myCommand;
}

// ============================================================================
bool SignalComm::Initialize(double aSimTime)
{
   bool ok = Comm::Initialize(aSimTime);
   if (ok)
   {
      mInterfacePtr = dynamic_cast<CommLab::Interface*>(&GetSimulation()->GetExtension("comm_lab_interface"));
      assert(mInterfacePtr != nullptr);

      // Subscribe to location messages received over DIS.
      mCallbacks += CommLab::Interface::LocationMessageReceived.Connect(&SignalComm::LocationMessageReceived, this);

      // Set aux data on the platform (Used when constructing a LocationMessage with a platform pointer argument).
      GetPlatform()->GetAuxData().AssignInt("SOURCE_TRACK_NUMBER", mSourceTrackNumber);
   }
   return ok;
}

bool SignalComm::Receive(double aSimTime, Comm* aCommPtr, Message& aMessage)
{
   if (!IsTurnedOn())                 // Don't receive if the device isn't turned on
   {
      return false;
   }

   if (DebugEnabled())
   {
      auto out = ut::log::debug() << "Comm receiving message.";
      out.AddNote() << "T = " << aSimTime;
      out.AddNote() << "Comm: " << GetFullName();
      out.AddNote() << "Message: " << *aMessage.SourceMessage();
   }

   bool messageReceived = false;

   // Perform a comm filter check
   if (Component::Receive(*this, aSimTime, aCommPtr, aMessage))
   {
      // EXERCISE 3 TASK 4a
      // Pass the message to the stack to see if it can be processed
      // PLACE YOUR CODE HERE
      if (messageReceived)
      {
         // EXERCISE 3 TASK 4b
         // Using the owning simulation object accessor,
         // Notify simulation observers that a message has been received
         // PLACE YOUR CODE HERE

         // EXERCISE 3 TASK 4c
         // Forward the message to any on-board recipients (internal links)
         // PLACE YOUR CODE HERE
      }
      //! Only throw a notification of message discarded if the message was intended for us and failed.
      if (!messageReceived && aMessage.SourceMessage()->GetDstAddr() == GetAddress())
      {
         // This message is specifically for a protocol stack failure
         WsfObserver::MessageDiscarded(GetSimulation())(aSimTime, aCommPtr, *aMessage.SourceMessage(), "layer_receive_failure");
      }
   }

   return messageReceived;
}

bool SignalComm::Send(double aSimTime, std::unique_ptr<WsfMessage> aMessagePtr, const Address& aAddress)
{
   if (!IsTurnedOn()) // Don't send if the device isn't turned on
   {
      return false;
   }

   bool messageSent = false;

   // Perform a comm filter check
   if (Component::Send(*this, aSimTime, *aMessagePtr, aAddress))
   {
      //! This object is only valid in the scope of this method call, and will
      //! deallocate upon returning. If any object (layer, event, etc.) in the
      //! stack call chain requires an extended lifetime of this object, it is
      //! their responsibility to create such an object and manage it.
      Message message(std::move(aMessagePtr));

      message.SourceMessage()->SetDstAddr(aAddress);
      message.SourceMessage()->SetSrcAddr(GetAddress());

      // Use properties from the message table
      GetScenario().GetMessageTable()->SetMessageProp(GetTypeId(), *message.SourceMessage());

      // EXERCISE 3 TASK 5a
	  // Using the owning simulation accessor, notify observers that a message has been transmitted
	  // PLACE YOUR CODE HERE
	  
      // EXERCISE 3 TASK 5b
	  // Using our comm's protocol stack, send the message
	  // PLACE YOUR CODE HERE
	  
	  // EXERCISE 3 TASK 5c
      // Using the CommLab::Interface attribute, send the message over DIS
      // PLACE YOUR CODE HERE
   }

   return messageSent;
}

//! Implement a callback function for LocationMessageReceived
void SignalComm::LocationMessageReceived(double                     aSimTime,
                                         DataLink::LocationMessage* aLocMsgPtr)
{
   LocationMessage locationMessage(aLocMsgPtr);
   SendMessage(aSimTime, locationMessage);
}

//! Get the name of the script class associated with this class.
//! This is necessary for proper downcasts in the scripting language.
//virtual
const char* SignalComm::GetScriptClassName() const
{
   return "SignalComm";
}

}
}
