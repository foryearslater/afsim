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

#include "WsfScriptPrompt.hpp"

#include "UtMemory.hpp"
#include "UtPlugin.hpp"
#include "UtScriptClassDefine.hpp"
#include "UtScriptData.hpp"
#include "WsfApplication.hpp"
#include "WsfApplicationExtension.hpp"
#include "WsfPlatform.hpp"
#include "WsfPlugin.hpp"
#include "WsfPromptObserver.hpp"
#include "script/WsfScriptDefs.hpp"

namespace
{
class ApplicationExtension : public WsfApplicationExtension
{
public:
   void AddedToApplication(WsfApplication& aApplication) override;
   void SimulationCreated(WsfSimulation& aSimulation) override;
};

void ApplicationExtension::AddedToApplication(WsfApplication& aApplication)
{
   UtScriptTypes* scriptTypesPtr = aApplication.GetScriptTypes();

   // Register new script classes provided by this extension.
   scriptTypesPtr->Register(ut::make_unique<WsfScriptPromptClass>("WsfPrompt", scriptTypesPtr));
}

void ApplicationExtension::SimulationCreated(WsfSimulation& aSimulation)
{
   aSimulation.RegisterExtension("prompt_observer", ut::make_unique<WsfPromptObserver>());
}
} // namespace

//! Registers the wsf_message_prompt extension with the application
//! so it is available for use.
void WSF_PROMPT_EXPORT Register_wsf_prompt(WsfApplication& aApplication)
{
   if (!aApplication.ExtensionIsRegistered("wsf_prompt"))
   {
      aApplication.RegisterFeature("prompt", "wsf_prompt");
      aApplication.RegisterExtension("wsf_prompt", ut::make_unique<ApplicationExtension>());
   }
}

extern "C"
{
   //! This method is called when the plugin is loaded to ensure that the plugin and the executable loading it were
   //! built with the same version of the plugin API.
   UT_PLUGIN_EXPORT void WsfPluginVersion(UtPluginVersion& aVersion)
   {
      aVersion =
         UtPluginVersion(WSF_PLUGIN_API_MAJOR_VERSION, WSF_PLUGIN_API_MINOR_VERSION, WSF_PLUGIN_API_COMPILER_STRING);
   }
   //! This method is called when the plugin is loaded. It must have exactly this signature (name and parameter) to
   //! succeed. As we only have an application reference at load time, one usually registers an application extension,
   //! within which the ScenarioCreated method can be overridden to gain access to a scenario.  If one also needs access
   //! to the simulation, one should also instantiate and register a simulation extension by overriding the
   //! SimulationCreated method in the scenario extension.
   UT_PLUGIN_EXPORT void WsfPluginSetup(WsfApplication& aApplication) { Register_wsf_prompt(aApplication); }
}

WsfScriptPromptClass::WsfScriptPromptClass(const std::string& aClassName, UtScriptTypes* aScriptTypesPtr)
   : UtScriptClass(aClassName, aScriptTypesPtr)
{
   SetClassName("WsfPrompt");

   AddStaticMethod(ut::make_unique<Display_1>("Display"));
   AddStaticMethod(ut::make_unique<Display_2>("Display"));
   AddStaticMethod(ut::make_unique<DisplayAndPause_1>("DisplayAndPause"));
   AddStaticMethod(ut::make_unique<DisplayAndPause_2>("DisplayAndPause"));
}

namespace
{
std::pair<WsfScriptPromptClass::PromptData, bool> FormatScriptData(const UtScriptMethodArgs aVarArgs,
                                                                   const std::string&       aPlatformName,
                                                                   bool                     aBlock)
{
   std::string title   = aVarArgs[0].GetString();
   std::string message = aVarArgs[1].GetString();

   WsfScriptPromptClass::ScriptButtons scriptButtons;
   bool                                success = true;
   if (aVarArgs.size() == 2)
   {
      scriptButtons.emplace_back("OK", "");
   }
   else
   {
      auto scriptsData = *(static_cast<std::vector<UtScriptData>*>(aVarArgs[2].GetPointer()->GetAppObject()));
      auto buttonsData = *(static_cast<std::vector<UtScriptData>*>(aVarArgs[3].GetPointer()->GetAppObject()));

      if (scriptsData.empty() || buttonsData.empty() || (scriptsData.size() != buttonsData.size()))
      {
         success = false;
      }
      else
      {
         for (size_t i = 0; i < buttonsData.size(); ++i)
         {
            scriptButtons.emplace_back(buttonsData.at(i).GetString(), scriptsData.at(i).GetString());
         }
      }
   }

   return std::make_pair(WsfScriptPromptClass::PromptData{aPlatformName, title, message, scriptButtons, aBlock}, success);
}
} // namespace

UT_DEFINE_SCRIPT_METHOD(WsfScriptPromptClass, WsfScriptPromptClass, Display_1, 4, "void", "string, string, Array<string>, Array<string>")
{
   WsfScriptPromptClass::PromptData promptData;
   bool                             valid;
   std::string                      platformName = "";

   WsfPlatform* platform = WsfScriptContext::GetPLATFORM(aContext);
   if (platform != nullptr)
   {
      platformName = platform->GetName();
   }

   std::tie(promptData, valid) = FormatScriptData(aVarArgs, platformName, false);
   if (!valid)
   {
      UT_SCRIPT_ABORT("Script and Button arrays must be non-empty and of the same size");
   }

   WsfObserver::PromptDisplayed(SIMULATION)(promptData);
}

UT_DEFINE_SCRIPT_METHOD(WsfScriptPromptClass, WsfScriptPromptClass, Display_2, 2, "void", "string, string")
{
   WsfScriptPromptClass::PromptData promptData;
   bool                             valid;
   std::string                      platformName = "";

   WsfPlatform* platform = WsfScriptContext::GetPLATFORM(aContext);
   if (platform != nullptr)
   {
      platformName = platform->GetName();
   }

   std::tie(promptData, valid) = FormatScriptData(aVarArgs, platformName, false);
   if (!valid)
   {
      UT_SCRIPT_ABORT("Script and Button arrays must be non-empty and of the same size");
   }

   WsfObserver::PromptDisplayed(SIMULATION)(promptData);
}

UT_DEFINE_SCRIPT_METHOD(WsfScriptPromptClass,
                        WsfScriptPromptClass,
                        DisplayAndPause_1,
                        4,
                        "void",
                        "string, string, Array<string>, Array<string>")
{
   WsfScriptPromptClass::PromptData promptData;
   bool                             valid;
   std::string                      platformName = "";

   WsfPlatform* platform = WsfScriptContext::GetPLATFORM(aContext);
   if (platform != nullptr)
   {
      platformName = platform->GetName();
   }

   std::tie(promptData, valid) = FormatScriptData(aVarArgs, platformName, true);
   if (!valid)
   {
      UT_SCRIPT_ABORT("Script and Button arrays must be non-empty and of the same size");
   }

   WsfObserver::PromptDisplayed(SIMULATION)(promptData);
}

UT_DEFINE_SCRIPT_METHOD(WsfScriptPromptClass, WsfScriptPromptClass, DisplayAndPause_2, 2, "void", "string, string")
{
   WsfScriptPromptClass::PromptData promptData;
   bool                             valid;
   std::string                      platformName = "";

   WsfPlatform* platform = WsfScriptContext::GetPLATFORM(aContext);
   if (platform != nullptr)
   {
      platformName = platform->GetName();
   }

   std::tie(promptData, valid) = FormatScriptData(aVarArgs, platformName, true);
   if (!valid)
   {
      UT_SCRIPT_ABORT("Script and Button arrays must be non-empty and of the same size");
   }

   WsfObserver::PromptDisplayed(SIMULATION)(promptData);
}
