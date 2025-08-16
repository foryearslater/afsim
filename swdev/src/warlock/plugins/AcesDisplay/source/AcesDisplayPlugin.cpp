// ****************************************************************************
// CUI//REL TO USA ONLY
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2021 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "AcesDisplayPlugin.hpp"

#include <QString>

WKF_PLUGIN_DEFINE_SYMBOLS(WkAcesDisplay::Plugin,
                          "Aces Display",
                          "The Aces Display plugin provides a custom-display of SA Processor data, inlcuding "
                          "perception of assets, bogies, and bandits, perceived groups, and truth.",
                          "warlock",
                          false) // Plugin not loaded by default

WkAcesDisplay::Plugin::Plugin(const QString& aPluginName, size_t aUniqueID)
   : warlock::PluginT<SimInterface, wkf::AcesDisplay::PluginBase>(aPluginName, aUniqueID, eSECONDARY_VIEWER)
{
   mInterfacePtr->SetEnabled(false);

   // This updates the target platform list, removing any deselected platforms
   // and adding any newly selected ones.
   connect(&wkfEnv,
           &wkf::Environment::PlatformSelectionChanged,
           this,
           [&](const wkf::PlatformList& aSelected, const wkf::PlatformList& aDeselected)
           {
              // Only handle platform selection change if we have a
              // dockwidget open. Handle everything on a base reference
              // list, then set the focus displays list from it.
              if (mDisplayInFocus != 0)
              {
                 // Add any newly selected platforms
                 for (const auto& selectedPlatform : aSelected)
                 {
                    mTargetPlatforms[0].push_back(selectedPlatform->GetIndex());
                 }

                 // Remove any deselected platforms
                 for (const auto& deselectedPlatform : aDeselected)
                 {
                    // Then remove any targets that need removed
                    auto target =
                       std::find(mTargetPlatforms[0].begin(), mTargetPlatforms[0].end(), deselectedPlatform->GetIndex());
                    if (target != mTargetPlatforms[0].end())
                    {
                       mTargetPlatforms[0].erase(target);
                    }
                 }

                 // Set the current displays list to the reference list, and
                 // remove the focus displays index from its own list if found
                 mTargetPlatforms[mDisplayInFocus] = mTargetPlatforms[0];
                 auto focusIndex                   = std::find(mTargetPlatforms[mDisplayInFocus].begin(),
                                             mTargetPlatforms[mDisplayInFocus].end(),
                                             mDisplayInFocus);
                 if (focusIndex != mTargetPlatforms[mDisplayInFocus].end())
                 {
                    mTargetPlatforms[mDisplayInFocus].erase(focusIndex);
                 }

                 // Set the list for the active dockwidget so that
                 // it can draw the engagement data
                 emit SetTargetPlatforms(mTargetPlatforms[mDisplayInFocus]);

                 // Set the target platforms for the interface so it
                 // knows what platform indices to gather engagement
                 // data for
                 mInterfacePtr->SetTargetPlatforms(mTargetPlatforms);
              }
           });
}

void WkAcesDisplay::Plugin::GuiUpdate()
{
   // Update the data container
   mInterfacePtr->ProcessEvents(mDataContainer);

   // Check if the connection to any display has been destroyed
   for (auto& display : mDisplays)
   {
      if (mInterfacePtr->IsConnectionDestroyed(display.first))
      {
         display.second->ActivateKillFrame(true);
      }
   }

   PluginBase::GuiUpdate();
}

void WkAcesDisplay::Plugin::NewDisplay(wkf::Platform* aPlatform)
{
   PluginBase::NewDisplay(aPlatform);

   // The Sim interface is disabled until needed
   mInterfacePtr->SetEnabled(true);
   mInterfacePtr->AddPlatformOfInterest(aPlatform->GetIndex());
}

void WkAcesDisplay::Plugin::RemoveDisplay(size_t aIndex)
{
   PluginBase::RemoveDisplay(aIndex);

   mInterfacePtr->RemovePlatformOfInterest(aIndex);
}
