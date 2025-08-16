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

#include "StartTimeWidget.hpp"

#include <QSpinBox>
#include <QTime>
#include <QValidator>

#include "Project.hpp"
#include "TimeHandler.hpp"
#include "UtQt.hpp"
#include "WsfDateTime.hpp"

SpaceTools::StartTimeWidget::StartTimeWidget(QWidget* aParentPtr)
{
   mUI.setupUi(this);

   connect(mUI.mScenarioRadioButton, &QRadioButton::clicked, this, &StartTimeWidget::SetToCurrentScenarioTime);
   connect(mUI.mLatestRadioButton, &QRadioButton::clicked, this, &StartTimeWidget::SetToLatestEpoch);
   connect(mUI.mNowRadioButton, &QRadioButton::clicked, this, &StartTimeWidget::SetToNow);
   connect(mUI.mStartDateEdit, &QDateEdit::dateChanged, this, &StartTimeWidget::UpdateEpoch);
   connect(mUI.mStartTimeEdit, &QTimeEdit::timeChanged, this, &StartTimeWidget::UpdateEpoch);
   connect(mUI.mStartEpochEdit, &QLineEdit::textEdited, this, &StartTimeWidget::UpdateDateTimeBoxes);
   connect(mUI.mUpdateStartTimeButton, &QPushButton::clicked, this, &StartTimeWidget::UpdateStartDateTime);
}

//! Updates the gui to the correct time when the dialog is shown
void SpaceTools::StartTimeWidget::OnShow()
{
   if (mUI.mScenarioRadioButton->isChecked())
   {
      SetToCurrentScenarioTime();
   }
   else if (mUI.mLatestRadioButton->isChecked())
   {
      SetToLatestEpoch();
   }
   else
   {
      SetToNow();
   }
}

void SpaceTools::StartTimeWidget::SetToCurrentScenarioTime()
{
   UpdateGui(wizard::Project::Instance()->GetStartDateTime());
}

void SpaceTools::StartTimeWidget::SetToLatestEpoch()
{
   UpdateGui(TimeHandler::GetLatestEpoch());
}

void SpaceTools::StartTimeWidget::SetToNow()
{
   UpdateGui(UtCalendar(), false, false);
}

void SpaceTools::StartTimeWidget::UpdateEpoch()
{
   if (mUI.mStartDateEdit->hasFocus() || mUI.mStartTimeEdit->hasFocus())
   {
      UtCalendar cal{};
      QDate      date{mUI.mStartDateEdit->date()};
      cal.SetDate(date.year(), date.month(), date.day());

      QTime time{mUI.mStartTimeEdit->time()};
      cal.SetTime(time.hour(), time.minute(), time.second());

      UpdateGui(cal, true, false);
   }
}

void SpaceTools::StartTimeWidget::UpdateDateTimeBoxes()
{
   if (mUI.mStartEpochEdit->hasFocus())
   {
      QString epochText(mUI.mStartEpochEdit->text());
      if (epochText.endsWith('.') || Validate(epochText) != QValidator::Acceptable)
      {
         UtQtSetTextColor(mUI.mStartEpochEdit, Qt::red);
      }
      else
      {
         UtCalendar cal{};
         double     epoch{epochText.toDouble()};
         cal.SetEpoch(epoch);
         UpdateGui(cal, false, true);
      }
   }
}

void SpaceTools::StartTimeWidget::UpdateStartDateTime()
{
   UtCalendar newCal{};

   bool ok{true};
   if (mUI.mSetAsEpoch->isChecked())
   {
      ok = newCal.SetEpoch(mUI.mStartEpochEdit->text().toDouble());
   }
   else
   {
      auto date = mUI.mStartDateEdit->date();
      newCal.SetDate(date.year(), date.month(), date.day());
      auto   time = mUI.mStartTimeEdit->time();
      double sec  = time.second() + (time.msec() / 1000.0000);
      newCal.SetTime(time.hour(), time.minute(), sec);
   }

   if (ok)
   {
      int outputType{TimeHandler::cOUTPUT_DATE_TIME};
      if (mUI.mNowRadioButton->isChecked())
      {
         outputType = TimeHandler::cOUTPUT_TIME_NOW;
      }
      else if (mUI.mSetAsEpoch->isChecked())
      {
         outputType = TimeHandler::cOUTPUT_EPOCH;
      }

      TimeHandler::UpdateStartDateTime(newCal, outputType);
   }
}

void SpaceTools::StartTimeWidget::UpdateGui(const UtCalendar& aCalendar, bool aUpdateEpoch, bool aUpdateDateTime)
{
   UtQtSetTextColor(mUI.mStartEpochEdit, QApplication::palette().text().color());

   bool setToNow{mUI.mNowRadioButton->isChecked()};
   mUI.mSetAsEpoch->setEnabled(!setToNow);
   mUI.mStartDateEdit->setEnabled(!setToNow);
   mUI.mStartEpochEdit->setEnabled(!setToNow);
   mUI.mStartTimeEdit->setEnabled(!setToNow);
   if (aUpdateDateTime)
   {
      QDate date{aCalendar.GetYear(), aCalendar.GetMonth(), aCalendar.GetDay()};
      mUI.mStartDateEdit->setDate(date);

      double seconds      = aCalendar.GetSecond();
      int    milliseconds = std::modf(aCalendar.GetSecond(), &seconds) * 1000;
      int    s            = seconds;
      QTime  time{aCalendar.GetHour(), aCalendar.GetMinute(), s, milliseconds};
      mUI.mStartTimeEdit->setTime(time);
   }
   if (aUpdateEpoch)
   {
      mUI.mStartEpochEdit->setText(QString::number(aCalendar.GetEpoch(), 'f', 8));
   }
}

QValidator::State SpaceTools::StartTimeWidget::Validate(QString& aString)
{
   QValidator::State isValidState(QValidator::Invalid);
   UtCalendar        checkEpoch;
   bool              ok{false};
   double            epoch{aString.toDouble(&ok)};
   if (ok && checkEpoch.SetEpoch(epoch))
   {
      isValidState = QValidator::Acceptable;
   }

   return isValidState;
}
