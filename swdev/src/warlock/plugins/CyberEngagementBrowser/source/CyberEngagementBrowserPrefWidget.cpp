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
#include "CyberEngagementBrowserPrefWidget.hpp"

#include "CyberEngagementBrowserPrefObject.hpp"

WkCyberEngagementBrowser::PrefWidget::PrefWidget()
   : wkf::PrefWidgetT<PrefObject>()
{
   mUI.setupUi(this);
}

void WkCyberEngagementBrowser::PrefWidget::WritePreferenceData(PrefData& aPrefData)
{
   aPrefData.mPurgeTime = mUI.mPurgeTimeSpinbox->value();
   aPrefData.mPurge     = mUI.mPurgeGroupBox->isChecked();
}

void WkCyberEngagementBrowser::PrefWidget::ReadPreferenceData(const PrefData& aPrefData)
{
   mUI.mPurgeTimeSpinbox->setValue(aPrefData.mPurgeTime);
   mUI.mPurgeGroupBox->setChecked(aPrefData.mPurge);
}
