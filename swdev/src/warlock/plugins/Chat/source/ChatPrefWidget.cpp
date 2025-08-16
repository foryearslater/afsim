// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2016 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#include "ChatPrefWidget.hpp"

#include "ChatPrefObject.hpp"
#include "UtQtNetwork.hpp"

Chat::PrefWidget::PrefWidget()
   : wkf::PrefWidgetT<PrefObject>()
{
   mUi.setupUi(this);
}

void Chat::PrefWidget::WritePreferenceData(PrefData& aPrefData)
{
   aPrefData.mUserName  = mUi.mUserName->text();
   aPrefData.mWallTimes = mUi.mTimeStamps->isChecked();
   aPrefData.mSimTimes  = mUi.mSimTimes->isChecked();
}

void Chat::PrefWidget::ReadPreferenceData(const PrefData& aPrefData)
{
   mUi.mUserName->setText(aPrefData.mUserName);
   mUi.mTimeStamps->setChecked(aPrefData.mWallTimes);
   mUi.mSimTimes->setChecked(aPrefData.mSimTimes);
}
