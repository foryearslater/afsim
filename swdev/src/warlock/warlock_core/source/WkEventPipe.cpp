// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2020 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "WkEventPipe.hpp"

#include "UtMemory.hpp"
#include "UtPackReflect.hpp"
#include "WkEventPipeClasses.hpp"
#include "WkEventPipeClassesRegister.hpp"
#include "WkEventPipeSchema.hpp"
#include "WkObserver.hpp"
#include "WsfEventPipeClasses.hpp"
#include "WsfSimulation.hpp"

void wk::EventPipe::RegisterEvents(WsfEventPipeExtension& aEventPipeExtension)
{
   aEventPipeExtension.AddSchema(cWkEventPipe_SCHEMA);
   aEventPipeExtension.RegisterEventOption("USER_ACTION", true);
   aEventPipeExtension.RegisterEventOption("HUD_DATA", true);
   aEventPipeExtension.RegisterEventOption("CHAT_MESSAGE", true);

   // Register the callback that gets invoked by WsfEventPipeInterface::AddedToSimulation to register
   // our message types with the serializer.
   aEventPipeExtension.AddCallback(aEventPipeExtension.RegisterExtensionMessages.Connect(&EventPipe::RegisterMessages));
}

//! A callback from WsfEventPipeInterface::AddedToSimulation to register our message types with the serializer.
void wk::EventPipe::RegisterMessages(UtPackSerializer* aSerializerPtr)
{
   WsfEventPipe::UtPack_register_all_wk_events_types(*aSerializerPtr);
}

wk::EventPipe::EventPipe(WsfEventPipeInterface* aEventPipeInterfacePtr)
   : mSimulation(aEventPipeInterfacePtr->GetSimulation())
   , mEventPipePtr(aEventPipeInterfacePtr)
{
   cUSER_ACTION  = aEventPipeInterfacePtr->GetEventId("USER_ACTION");
   cHUD_DATA     = aEventPipeInterfacePtr->GetEventId("HUD_DATA");
   cCHAT_MESSAGE = aEventPipeInterfacePtr->GetEventId("CHAT_MESSAGE");
}

void wk::EventPipe::UpdateSubscriptions(const WsfEventPipeOptions& aNewCombinedOptions,
                                        const WsfEventPipeOptions& aPreviousCombinedOptions)
{
   if (aNewCombinedOptions.IsOptionEnabled(cUSER_ACTION))
   {
      if (!aPreviousCombinedOptions.IsOptionEnabled(cUSER_ACTION))
      {
         mUserActionCallback +=
            WkObserver::SimulationUserAction(mSimulation).Connect(&EventPipe::SimulationUserAction, this);
      }
   }
   else if (aPreviousCombinedOptions.IsOptionEnabled(cUSER_ACTION))
   {
      mUserActionCallback.Clear();
   }

   if (aNewCombinedOptions.IsOptionEnabled(cHUD_DATA))
   {
      if (!aPreviousCombinedOptions.IsOptionEnabled(cHUD_DATA))
      {
         mHUD_DataCallback += WkObserver::SimulationHUD_Data(mSimulation).Connect(&EventPipe::SimulationHUD_Data, this);
      }
   }
   else if (aPreviousCombinedOptions.IsOptionEnabled(cHUD_DATA))
   {
      mHUD_DataCallback.Clear();
   }

   if (aNewCombinedOptions.IsOptionEnabled(cCHAT_MESSAGE))
   {
      if (!aPreviousCombinedOptions.IsOptionEnabled(cCHAT_MESSAGE))
      {
         mChatMessageCallback +=
            WkObserver::SimulationChatMessage(mSimulation).Connect(&EventPipe::SimulationChatMessage, this);
      }
   }
   else if (aPreviousCombinedOptions.IsOptionEnabled(cCHAT_MESSAGE))
   {
      mChatMessageCallback.Clear();
   }
}

void wk::EventPipe::SimulationUserAction(const std::string& aUserAction, const WsfPlatform* aPlatformPtr)
{
   if (mEventPipePtr != nullptr)
   {
      auto msgPtr = ut::make_unique<WsfEventPipe::MsgUserAction>();
      msgPtr->platformIndexValid(false);
      msgPtr->text(aUserAction);
      if (aPlatformPtr != nullptr)
      {
         msgPtr->platformIndex(mEventPipePtr->Platform(*aPlatformPtr));
         msgPtr->platformIndexValid(true);
      }
      mEventPipePtr->SendImmediate(mSimulation.GetSimTime(), false, std::move(msgPtr));
   }
}

void wk::EventPipe::SimulationChatMessage(const std::string& aName,
                                          const std::string& aChannel,
                                          const std::string& aText,
                                          bool               aReceived)
{
   if (mEventPipePtr != nullptr)
   {
      auto msgPtr = ut::make_unique<WsfEventPipe::MsgChatMessage>();
      msgPtr->name(aName);
      msgPtr->channel(aChannel);
      msgPtr->text(aText);
      mEventPipePtr->SendImmediate(mSimulation.GetSimTime(), aReceived, std::move(msgPtr));
   }
}

void wk::EventPipe::Send(double aSimTime, const WsfPlatform& aPlatform, std::unique_ptr<WsfEventPipe::MsgBase> aMessagePtr)
{
   mEventPipePtr->SendDependent(aSimTime, aPlatform, std::move(aMessagePtr));
}

void wk::EventPipe::SimulationHUD_Data(double aSimTime, WsfPlatform* aPlatformPtr, unsigned int aHUD_Mode)
{
   if (mEventPipePtr != nullptr && aPlatformPtr != nullptr)
   {
      auto msgPtr = ut::make_unique<WsfEventPipe::MsgHUD_Data>();
      msgPtr->platformIndex(static_cast<unsigned int>(aPlatformPtr->GetIndex()));
      msgPtr->hudMode(aHUD_Mode);
      Send(aSimTime, *aPlatformPtr, std::move(msgPtr));
   }
}

// =================================================================================================
//! Return a reference to the simulation extension.
// static
wk::EventPipeInterface& wk::EventPipeInterface::Get(const WsfSimulation& aSimulation)
{
   WsfSimulationExtension* extensionPtr = aSimulation.FindExtension("wk_eventpipe");
   return static_cast<EventPipeInterface&>(*extensionPtr);
}

wk::EventPipeInterface::EventPipeInterface()
   : mEventPipePtr(nullptr)
{
}

// =================================================================================================
void wk::EventPipeInterface::AddedToSimulation()
{
   // If the event_pipe extension is available, hook in the handers for our events.
   WsfEventPipeInterface* eventPipePtr = WsfEventPipeInterface::Find(GetSimulation());
   if (eventPipePtr != nullptr)
   {
      mEventPipePtr = ut::make_unique<EventPipe>(eventPipePtr);
      eventPipePtr->AddLogger(mEventPipePtr.get());
   }
}
