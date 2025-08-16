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
#include "BM_Plugin.hpp"

#include "UtQtMemory.hpp"

WKF_PLUGIN_DEFINE_SYMBOLS(WkBM::Plugin, "Battle Management", "", "warlock");

WkBM::Plugin::Plugin(const QString& aPluginName, const size_t aPluginId) noexcept
   : warlock::PluginT<WkBM::SimInterface, wkf::bm::PluginBase>(aPluginName, aPluginId)
{
   mInterfacePtr->SetEnabled(false);
}

void WkBM::Plugin::GuiUpdate()
{
   mInterfacePtr->ProcessEvents(*this);

   if (mInterfacePtr->IsEnabled())
   {
      UpdatePlots(mInterfacePtr->GetSimTime());
      RedrawPlots();

      RescanPlatformStatus(mInterfacePtr->GetSimTime());
   }
}

void WkBM::Plugin::InitializePlatformStatus(wkf::bm::PlatformStatus& aPlatformStatus)
{
   aPlatformStatus.AddRuleSet(ut::qt::make_qt_ptr<WkBM::RuleSetDamageFactor>(nullptr, this));
   aPlatformStatus.AddRuleSet(ut::qt::make_qt_ptr<WkBM::RuleSetFuel>(nullptr, this));

   aPlatformStatus.ReadUserSettings();
}

void WkBM::Plugin::AddAuxDataRuleSet(wkf::bm::PlatformStatus& aPlatformStatus, QString aKey, wkf::bm::RuleSet::Mode aMode)
{
   mInterfacePtr->AddAuxDataKey(aKey);
   aPlatformStatus.AddRuleSet(ut::qt::make_qt_ptr<WkBM::AuxDataRuleSet>(nullptr, this, aKey, aMode));
}

void WkBM::Plugin::AddResourceAuxDataRuleSet(wkf::bm::PlatformStatus& aPlatformStatus, QString aNumerator, QString aDenominator)
{
   mInterfacePtr->AddAuxDataKey(aNumerator);
   mInterfacePtr->AddAuxDataKey(aDenominator);
   aPlatformStatus.AddRuleSet(ut::qt::make_qt_ptr<WkBM::AuxDataResourceRuleSet>(nullptr, this, aNumerator, aDenominator));
}

void WkBM::Plugin::NewPlotCreated()
{
   mInterfacePtr->SetEnabled(true);
}
