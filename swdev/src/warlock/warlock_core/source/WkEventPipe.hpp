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

#ifndef WKEVENTPIPE_HPP
#define WKEVENTPIPE_HPP

#include "UtCallbackHolder.hpp"
#include "WsfEventPipe.hpp"
class WsfPlatform;
class WsfSimulation;

#include "warlock_core_export.h"

namespace wk
{
class WARLOCK_CORE_EXPORT EventPipe : public WsfEventPipeLogger
{
public:
   static void RegisterEvents(WsfEventPipeExtension& aEventPipeExtension);
   static void RegisterMessages(UtPackSerializer* aSerializerPtr);
   explicit EventPipe(WsfEventPipeInterface* aEventPipeInterfacePtr);

   void UpdateSubscriptions(const WsfEventPipeOptions& aNewCombinedOptions,
                            const WsfEventPipeOptions& aPreviousCombinedOptions) override;

   void SimulationUserAction(const std::string& aUserAction, const WsfPlatform* aPlatformPtr);

   void SimulationHUD_Data(double aSimTime, WsfPlatform* aPlatformPtr, unsigned int aHUD_Mode);

   void SimulationChatMessage(const std::string& aName, const std::string& aChannel, const std::string& aText, bool aReceived);

private:
   void Send(double aSimTime, const WsfPlatform& aPlatform, std::unique_ptr<WsfEventPipe::MsgBase> aMessagePtr);


   const WsfSimulation&   mSimulation;
   WsfEventPipeInterface* mEventPipePtr;

   UtCallbackHolder mUserActionCallback;
   UtCallbackHolder mHUD_DataCallback;
   UtCallbackHolder mChatMessageCallback;

   int cUSER_ACTION;
   int cHUD_DATA;
   int cCHAT_MESSAGE;
};

class WARLOCK_CORE_EXPORT EventPipeInterface : public WsfSimulationExtension
{
public:
   static EventPipeInterface& Get(const WsfSimulation& aSimulation);
   EventPipeInterface();
   ~EventPipeInterface() override = default;

   void AddedToSimulation() override;

private:
   std::unique_ptr<EventPipe> mEventPipePtr;
};
} // namespace wk
#endif
