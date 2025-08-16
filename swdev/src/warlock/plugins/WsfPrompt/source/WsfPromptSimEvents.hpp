// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2018 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef PROMPTSIMEVENTS_HPP
#define PROMPTSIMEVENTS_HPP

#include "WkScriptSimInterface.hpp"
#include "WsfScriptPrompt.hpp"

namespace WkWsfPrompt
{
class PromptDialog;
using Dialogs               = QList<QPointer<PromptDialog>>;
using ScriptSimInterfacePtr = std::shared_ptr<warlock::ScriptSimInterface>;

class PromptEvent : public warlock::SimEvent
{
public:
   PromptEvent()
      : warlock::SimEvent(false)
   {
   }

   virtual void Process(Dialogs& aDialogs, ScriptSimInterfacePtr aScriptSimInterfacePtr) = 0;
};

class DisplayEvent : public PromptEvent
{
public:
   DisplayEvent(const WsfScriptPromptClass::PromptData& aPromptData)
      : PromptEvent()
      , mPromptData(aPromptData)
   {
   }

   void Process(Dialogs& aDialogs, ScriptSimInterfacePtr aScriptSimInterfacePtr) override;

protected:
   WsfScriptPromptClass::PromptData mPromptData;
};

class CloseEvent : public PromptEvent
{
public:
   CloseEvent()
      : PromptEvent()
   {
   }

   void Process(Dialogs& aDialogs, ScriptSimInterfacePtr aScriptSimInterfacePtr) override;
};
} // namespace WkWsfPrompt

#endif
