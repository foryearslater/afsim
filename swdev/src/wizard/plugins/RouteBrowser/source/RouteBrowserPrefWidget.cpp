// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2019 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "RouteBrowserPrefWidget.hpp"

#include <QCheckBox>

#include "Util.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"

RouteBrowser::PrefWidget::PrefWidget(QWidget* parent /*= nullptr*/)
   : PrefWidgetT<PrefObject>(parent)
{
   mUI.setupUi(this);

   connect(mUI.mRouteNameCheckbox,
           &QCheckBox::toggled,
           this,
           [=](bool aChecked) { mUI.mRouteNameLineEdit->setEnabled(aChecked); });
   connect(mUI.mWaypointLabelCheckbox,
           &QCheckBox::toggled,
           this,
           [=](bool aChecked) { mUI.mWaypointLabelLineEdit->setEnabled(aChecked); });
}

void RouteBrowser::PrefWidget::showEvent(QShowEvent* aEvent)
{
   // This is necessary because the pref widget isn't constructed until after settings are loaded into pref object
   ReadPreferenceData(mPrefObjectPtr->GetPreferences());
}

void RouteBrowser::PrefWidget::ReadPreferenceData(const PrefData& aPrefData)
{
   mUI.mShowGlobalRoutesCheckbox->setChecked(GetPreferenceObject()->GetShowGlobalRoutes());
   mUI.mRouteNameCheckbox->setChecked(GetPreferenceObject()->GetUsingDefaultRouteNames());
   mUI.mWaypointLabelCheckbox->setChecked(GetPreferenceObject()->GetUsingDefaultWaypointLabels());

   mUI.mRouteNameLineEdit->setText(GetPreferenceObject()->GetDefaultRouteName());
   mUI.mWaypointLabelLineEdit->setText(GetPreferenceObject()->GetDefaultWaypointLabel());
}

void RouteBrowser::PrefWidget::WritePreferenceData(PrefData& aPrefData)
{
   aPrefData.mShowGlobalRoutes        = mUI.mShowGlobalRoutesCheckbox->isChecked();
   aPrefData.mUseDefaultRouteName     = mUI.mRouteNameCheckbox->isChecked();
   aPrefData.mUseDefaultWaypointLabel = mUI.mWaypointLabelCheckbox->isChecked();
   aPrefData.mDefaultRouteName        = mUI.mRouteNameLineEdit->text();
   aPrefData.mDefaultWaypointLabel    = mUI.mWaypointLabelLineEdit->text();
}
