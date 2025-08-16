// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2021 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef WK_NETWORK_PREF_WIDGET_HPP
#define WK_NETWORK_PREF_WIDGET_HPP

#include "WkNetworkPrefObject.hpp"
#include "WkfPrefWidget.hpp"
#include "ui_WkNetworkPrefWidget.h"

namespace warlock
{
namespace net
{
class WARLOCK_CORE_EXPORT PrefWidget final : public wkf::PrefWidgetT<PrefObject>
{
   Q_OBJECT

public:
   explicit PrefWidget(QWidget* aParent);
   ~PrefWidget() override = default;

   QString GetCategoryHint() const override { return "Application"; }

   void showEvent(QShowEvent* aEvent) override;

private:
   void ReadPreferenceData(const PrefData& aPrefData) override;
   void WritePreferenceData(PrefData& aPrefData) override;

   void OnAddInterfaceClicked();
   void PopulateInterfaces(QString aInterface);

   Ui::WkNetworkPrefWidget mUi;
};
} // namespace net
} // namespace warlock

#endif
