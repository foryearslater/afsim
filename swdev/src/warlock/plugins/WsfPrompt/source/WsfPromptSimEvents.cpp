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

#include "WsfPromptSimEvents.hpp"

#include <QMessageBox>
#include <QPointer>

#include "WkfEnvironment.hpp"
#include "WsfPromptDialog.hpp"

void WkWsfPrompt::DisplayEvent::Process(Dialogs& aDialogs, ScriptSimInterfacePtr aScriptSimInterfacePtr)
{
   // Check to make sure that the scripts to be called have no arguments.
   for (const auto& scriptButton : mPromptData.scriptButtons)
   {
      warlock::script::Instance scriptInstance;
      scriptInstance.platformName = mPromptData.platformName;
      if (!aScriptSimInterfacePtr->GetPlatformScriptData(mPromptData.platformName, scriptButton.second, scriptInstance.script))
      {
         aScriptSimInterfacePtr->GetGlobalScriptData(scriptButton.second, scriptInstance.script);
      }

      if (!scriptInstance.script.argTypes.empty())
      {
         QMessageBox::warning(nullptr,
                              "WsfPrompt Error",
                              "WsfPrompt.Display is only supported for scripts with no arguments.");
         std::cout << "WsfPrompt.Display is only supported for scripts with no arguments.\n";
         return;
      }
   }

   // Construct and show the dialog. Tell Sim Interface to pause the simulation if this was a blocking call.
   aDialogs.push_back(new PromptDialog(wkfEnv.GetMainWindow(), mPromptData, aScriptSimInterfacePtr));
   aDialogs.back()->show();
}

void WkWsfPrompt::CloseEvent::Process(Dialogs& aDialogs, ScriptSimInterfacePtr /*aScriptSimInterfacePtr*/)
{
   for (QPointer<PromptDialog>& dialog : aDialogs)
   {
      if (dialog)
      {
         dialog->close();
      }
   }
   aDialogs.clear();
}
