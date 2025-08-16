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
#ifndef CyberEngagementBrowserPrefWidget_hpp
#define CyberEngagementBrowserPrefWidget_hpp

#include "CyberEngagementBrowserPrefObject.hpp"
#include "WkfPrefWidget.hpp"
#include "ui_CyberEngagementBrowserPrefWidget.h"

namespace WkCyberEngagementBrowser
{
class PrefWidget : public wkf::PrefWidgetT<PrefObject>
{
public:
   PrefWidget();

private:
   void ReadPreferenceData(const PrefData& aPrefData) override;
   void WritePreferenceData(PrefData& aPrefData) override;

   Ui::CyberEngagementBrowserPrefWidget mUI;
};
} // namespace WkCyberEngagementBrowser
#endif
