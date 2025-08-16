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

#include "LogServerPrefWidget.hpp"

#include <QIntValidator>

LogServer::PrefWidget::PrefWidget(QWidget* parent /*= nullptr*/)
   : PrefWidgetT<PrefObject>(parent)
{
   mUI.setupUi(this);
   // port range
   QValidator* portValidator = new QIntValidator(1, 65535, mUI.mLogServerPortLineEdit);
   mUI.mLogServerPortLineEdit->setValidator(portValidator);
}

QString LogServer::PrefWidget::GetCategoryHint() const
{
   return "Simulation Logs";
}

void LogServer::PrefWidget::showEvent(QShowEvent* aEvent)
{
   // This is necessary because the pref widget isn't constructed until after settings are loaded into pref object
   ReadPreferenceData(mPrefObjectPtr->GetPreferences());
}

void LogServer::PrefWidget::ReadPreferenceData(const PrefData& aPrefData)
{
   mUI.mLogServerPortLineEdit->setText(QString::number(GetPreferenceObject()->GetLogServerPort()));
}

void LogServer::PrefWidget::WritePreferenceData(PrefData& aPrefData)
{
   aPrefData.mLogServerPort = mUI.mLogServerPortLineEdit->text().toInt();
}
