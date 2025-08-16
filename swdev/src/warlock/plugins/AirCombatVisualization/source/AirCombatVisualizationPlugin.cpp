// ****************************************************************************
// CUI//REL TO USA ONLY
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2020 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "AirCombatVisualizationPlugin.hpp"

#include "UtMemory.hpp"
#include "VaCamera.hpp"
#include "VaCameraMotionTethered.hpp"
#include "WkfAction.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"
#include "WkfObserver.hpp"
#include "WkfScenario.hpp"
#include "WkfViewer.hpp"
#include "WkfVtkEnvironment.hpp"
#include "air_combat/WkfAirCombatVisualizationPrefObject.hpp"
#include "misc/WkfImmersiveViewDockWidget.hpp"

WKF_PLUGIN_DEFINE_SYMBOLS(WkAirCombat::Plugin,
                          "Air Combat Visualization",
                          "The Air Combat Visualization plugin adds various GUI elements to the map and tether views "
                          "that support air-to-air combat analysis.",
                          "warlock",
                          false) // Plugin not loaded by default

WkAirCombat::Plugin::Plugin(const QString& aPluginName, const size_t aUniqueId)
   : warlock::PluginT<SimInterface>(aPluginName, aUniqueId)
   , mPrefWidgetPtr(new wkf::AirCombatVisualizationPrefWidget())
   , mEnabled(false)
{
   mDisplayInterfacePtr = ut::make_unique<wkf::AirCombatDisplayInterface>(*mPrefWidgetPtr->GetPreferenceObject());

   connect(&wkfEnv, &wkf::Environment::Initialize, this, [&]() { mEnabled = (vaEnv.GetStandardViewer() != nullptr); });

   connect(
      &wkfEnv,
      &wkf::Environment::PlatformSelectionChanged,
      this,
      [&](const wkf::PlatformList& aSelectedPlatforms, const wkf::PlatformList& aUnselectedPlatforms)
      {
         // For each unselected platform, remove it from the sim interface's target list, remove any
         // target blocks in all overlays that correspond to it, and remove all attachments from it.
         for (const auto& unselectedPlatform : aUnselectedPlatforms)
         {
            if (unselectedPlatform->GetName() == mInterfacePtr->GetSourcePlatformName())
            {
               mInterfacePtr->SetSourcePlatform("");
            }
            else
            {
               // Remove the platform from the sim interface's target list.
               mInterfacePtr->RemoveTargetPlatform(unselectedPlatform->GetName());
            }

            // Remove the platform from all overlay target lists.
            mDisplayInterfacePtr->RemoveManualTargetFromAllOverlays(*unselectedPlatform);

            // Remove any attachments from the platform.
            mDisplayInterfacePtr->RemoveAttachments(*unselectedPlatform);
         }

         // If exactly one platform is selected, that platform becomes the new source platform.
         if (aSelectedPlatforms.size() == 1)
         {
            mInterfacePtr->SetSourcePlatform(wkfEnv.GetSelectedPlatforms()[0]->GetName());
         }

         // For each newly selected platform, if it's viable, create attachments for it.
         for (auto& selectedPlatform : aSelectedPlatforms)
         {
            if (!mDisplayInterfacePtr->IsPlatformDecorated(*selectedPlatform) &&
                mDataContainer.IsPlatformViable(selectedPlatform->GetName()))
            {
               if (selectedPlatform->GetName() != mInterfacePtr->GetSourcePlatformName())
               {
                  mInterfacePtr->AddTargetPlatform(selectedPlatform->GetName());
               }

               mDisplayInterfacePtr->CreateDecoratorAttachment(*selectedPlatform);

               if (selectedPlatform->GetName() != mInterfacePtr->GetSourcePlatformName() &&
                   !mInterfacePtr->GetSourcePlatformName().empty())
               {
                  wkf::Platform* sourcePlatform =
                     vaEnv.GetStandardScenario()->FindPlatformByName(mInterfacePtr->GetSourcePlatformName());
                  if (sourcePlatform != nullptr)
                  {
                     mDisplayInterfacePtr->CreateLineAttachment(*sourcePlatform,
                                                                *selectedPlatform,
                                                                wkf::AirCombatDisplayInterface::InteractionType::cBASE);
                     mDisplayInterfacePtr->CreateLineAttachment(*sourcePlatform,
                                                                *selectedPlatform,
                                                                wkf::AirCombatDisplayInterface::InteractionType::cWEZ);
                     mDisplayInterfacePtr->CreateLineAttachment(*selectedPlatform,
                                                                *sourcePlatform,
                                                                wkf::AirCombatDisplayInterface::InteractionType::cWEZ);
                     mDisplayInterfacePtr->CreateLineAttachment(*sourcePlatform,
                                                                *selectedPlatform,
                                                                wkf::AirCombatDisplayInterface::InteractionType::cDETECTION);
                     mDisplayInterfacePtr->CreateLineAttachment(*selectedPlatform,
                                                                *sourcePlatform,
                                                                wkf::AirCombatDisplayInterface::InteractionType::cDETECTION);

                     // Add the selected platform to the source's targets.
                     mDisplayInterfacePtr->AddManualTargetToOverlay(*sourcePlatform, *selectedPlatform);
                  }
               }
            }
         }
      });
}

void WkAirCombat::Plugin::BuildViewerContextMenu(QMenu* aMenu, vespa::VaViewer* aViewerPtr)
{
   UtVariant v = aViewerPtr->GetData("type");
   if (!v.IsUnset())
   {
      std::string type = v.GetString();
      if (type == "immersive")
      {
         vespa::VaCameraMotionTethered* tether =
            dynamic_cast<vespa::VaCameraMotionTethered*>(aViewerPtr->GetCamera()->GetCameraMotion());
         if (tether != nullptr)
         {
            wkf::Platform* platform = dynamic_cast<wkf::Platform*>(tether->GetEntity());
            if (platform != nullptr)
            {
               if (mDataContainer.IsPlatformViable(platform->GetName()))
               {
                  if (aViewerPtr->FindFirstOverlayOfType<wkf::AirCombatOverlay>() == nullptr)
                  {
                     auto addOverlay = new wkf::Action("Add Air Combat Overlay", aMenu);
                     aMenu->addAction(addOverlay);
                     connect(addOverlay,
                             &QAction::triggered,
                             [aViewerPtr, platform, this]()
                             {
                                mDisplayInterfacePtr->CreateOverlay(*platform, *aViewerPtr);

                                if (mInterfacePtr->GetSourcePlatformName() == platform->GetName())
                                {
                                   for (const auto& targetName : mInterfacePtr->GetTargetPlatformNames())
                                   {
                                      wkf::Platform* target = vaEnv.GetStandardScenario()->FindPlatformByName(targetName);
                                      mDisplayInterfacePtr->AddManualTargetToOverlay(*platform, *target);
                                   }
                                }
                             });
                  }
                  else
                  {
                     auto addOverlay = new wkf::Action("Remove Air Combat Overlay", aMenu);
                     aMenu->addAction(addOverlay);
                     connect(addOverlay,
                             &QAction::triggered,
                             [aViewerPtr, platform, this]()
                             { mDisplayInterfacePtr->RemoveOverlay(*platform, *aViewerPtr); });
                  }
               }
            }
         }
      }
   }
}

// Returns the preference widget.
QList<wkf::PrefWidget*> WkAirCombat::Plugin::GetPreferencesWidgets() const
{
   QList<wkf::PrefWidget*> prefs;
   prefs << mPrefWidgetPtr;
   return prefs;
}

// Update each data object (and their attachments).
void WkAirCombat::Plugin::GuiUpdate()
{
   // Grab the most recent data from the simulation.
   mInterfacePtr->ProcessEvents(mDataContainer);

   if (mEnabled)
   {
      // Use that data to update the display interface.
      mDisplayInterfacePtr->Update(mDataContainer);
   }
}
