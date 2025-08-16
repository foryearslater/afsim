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

#ifndef STARTTIMEWIDGET_HPP
#define STARTTIMEWIDGET_HPP

#include <QObject>
#include <QString>
#include <QValidator>
#include <QWidget>

#include "UtCalendar.hpp"
#include "ui_StartTimeWidget.h"

namespace SpaceTools
{
//!
class StartTimeWidget : public QWidget
{
public:
   explicit StartTimeWidget(QWidget* aParentPtr);
   ~StartTimeWidget() override = default;

   void OnShow();

private:
   void SetToCurrentScenarioTime();
   void SetToLatestEpoch();
   void SetToNow();
   void UpdateEpoch();
   void UpdateDateTimeBoxes();
   void UpdateStartDateTime();
   void UpdateGui(const UtCalendar& aCalendar, bool aUpdateEpoch = true, bool aUpdateDateTime = true);

   QValidator::State Validate(QString& aString);

   Ui::StartTimeWidget mUI;
};
} // namespace SpaceTools
#endif
