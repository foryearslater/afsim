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

#include "SA_DisplayPlugin.hpp"

#include <QString>

WKF_PLUGIN_DEFINE_SYMBOLS(WkSA_Display::Plugin,
                          "Situation Awareness Display",
                          "The SA Display plugin provides a custom-display of SA Processor data, inlcuding perception "
                          "of assets, bogies, and bandits, perceived groups, and truth.",
                          "warlock",
                          false) // Plugin not loaded by default

WkSA_Display::Plugin::Plugin(const QString& aPluginName, size_t aUniqueID)
   : warlock::PluginT<SimInterface, wkf::SA_Display::PluginBase>(aPluginName, aUniqueID, eSECONDARY_VIEWER)
{
   mInterfacePtr->SetEnabled(false);
}

void WkSA_Display::Plugin::GuiUpdate()
{
   mInterfacePtr->ProcessEvents(mDataContainer);

   for (auto& sad : mSA_Displays)
   {
      if (mInterfacePtr->IsConnectionDestroyed(sad.first))
      {
         sad.second->ActivateKillFrame(true);
      }
   }

   PluginBase::GuiUpdate();
}

void WkSA_Display::Plugin::NewDisplay(wkf::Platform* aPlatform)
{
   // The Sim interface is disabled until needed
   mInterfacePtr->SetEnabled(true);
   mInterfacePtr->AddPlatformOfInterest(aPlatform->GetIndex());
}

void WkSA_Display::Plugin::RemoveDisplay(size_t aIndex)
{
   mInterfacePtr->RemovePlatformOfInterest(aIndex);
}

bool WkSA_Display::Plugin::HasSituationAwarenessProcessor(wkf::Platform* aPlatform)
{
   return mInterfacePtr->HasSituationAwarenessProcessor(aPlatform->GetIndex());
}
