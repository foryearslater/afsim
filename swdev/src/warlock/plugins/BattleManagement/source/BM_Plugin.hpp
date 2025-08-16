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
#ifndef BM_PLUGIN_HPP
#define BM_PLUGIN_HPP

#include <map>

#include "BM_SimInterface.hpp"
#include "WkPlugin.hpp"
#include "WkfObserver.hpp"
#include "statistics/WkfBM_PlatformDatabase.hpp"
#include "statistics/WkfBM_PluginBase.hpp"

namespace WkBM
{
class Plugin final : public warlock::PluginT<WkBM::SimInterface, wkf::bm::PluginBase>
{
   Q_OBJECT
public:
   Plugin(const QString& aPluginName, const size_t aPluginId) noexcept;

   // intentionally public
   using wkf::bm::PluginBase::mPlatformData;
   std::map<unsigned, PlatformState> mPlatformStates;

   //! Key = platform index
   //! Value = aux data keys and values
   AuxDataMap mAuxData;

protected:
   void GuiUpdate() override;

   void NewPlotCreated() override;

   void InitializePlatformStatus(wkf::bm::PlatformStatus& aPlatformStatus) override;
   void AddAuxDataRuleSet(wkf::bm::PlatformStatus& aPlatformStatus, QString aKey, wkf::bm::RuleSet::Mode aMode) override;
   void AddResourceAuxDataRuleSet(wkf::bm::PlatformStatus& aPlatformStatus, QString aNumerator, QString aDenominator) override;
};
} // namespace WkBM

#endif
