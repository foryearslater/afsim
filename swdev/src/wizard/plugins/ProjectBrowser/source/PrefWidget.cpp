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

#include "PrefWidget.hpp"

ProjectBrowser::PrefWidget::PrefWidget(QWidget* parent /*= nullptr*/)
   : PrefWidgetT<PrefObject>(parent)
{
}

void ProjectBrowser::PrefWidget::showEvent(QShowEvent* aEvent)
{
   // This is necessary because the pref widget isn't constructed until after settings are loaded into pref object
   ReadPreferenceData(mPrefObjectPtr->GetPreferences());
}

void ProjectBrowser::PrefWidget::ReadPreferenceData(const PrefData& aPrefData) {}

void ProjectBrowser::PrefWidget::WritePreferenceData(PrefData& aPrefData) {}
