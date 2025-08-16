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
#include "SimControllerStatusWidget.hpp"

#include <cmath>

#include <QDateTime>
#include <QDrag>
#include <QMimeData>
#include <QPixmap>

#ifdef _WIN32
#include <QWinTaskbarButton>
#include <QWinTaskbarProgress>
#endif
#include "SimControllerPrefObject.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"
#include "WkfUnitTypes.hpp"
#include "WsfSimulationObserver.hpp"

WkSimController::StatusWidget::StatusWidget(SimControllerDataContainer& aSimStateData,
                                            const PrefObject*           aPrefObjectPtr,
                                            size_t                      aPluginUniqueId)
   : QWidget()
   , mSimStateData(aSimStateData)
   , mPrefObjectPtr(aPrefObjectPtr)
#ifdef _WIN32
   , mTaskbarProgressPtr(nullptr)
   , mPluginUniqueId(aPluginUniqueId)
#endif
{
   mUi.setupUi(this);

   connect(mPrefObjectPtr, &PrefObject::ShowClockRateChanged, this, &StatusWidget::ShowClockRateHandler);
   connect(mPrefObjectPtr, &PrefObject::ShowProgressBarChanged, this, &StatusWidget::ShowProgressBarHandler);
   connect(mPrefObjectPtr, &PrefObject::ShowSimModeChanged, this, &StatusWidget::ShowSimModeHandler);
   connect(mPrefObjectPtr, &PrefObject::ShowSimStateChanged, this, &StatusWidget::ShowSimStateHandler);
   connect(mPrefObjectPtr, &PrefObject::ShowSimTimeChanged, this, &StatusWidget::ShowSimTimeHandler);
   connect(mPrefObjectPtr, &PrefObject::ShowTimeBehindChanged, this, &StatusWidget::ShowTimeBehindHandler);
   connect(mPrefObjectPtr, &PrefObject::ShowWallTimeChanged, this, &StatusWidget::ShowWallTimeHandler);
   connect(mPrefObjectPtr, &PrefObject::WallTimeSpecChanged, this, &StatusWidget::Update);

   connect(&mSimStateData, &SimControllerDataContainer::Initialized, this, &StatusWidget::SimulationStarting);
   connect(&mSimStateData, &SimControllerDataContainer::SimPausing, this, &StatusWidget::SimulationPausing);
   connect(&mSimStateData, &SimControllerDataContainer::SimResuming, this, &StatusWidget::SimulationResuming);
   connect(&mSimStateData, &SimControllerDataContainer::SimComplete, this, &StatusWidget::SimulationComplete);

   mUi.mClockRateWidget->installEventFilter(this);
   mUi.mSimTimeWidget->installEventFilter(this);
   mUi.mWallTimeWidget->installEventFilter(this);

   wkf::UnitsObject* unitPrefs = wkfEnv.GetPreferenceObject<wkf::UnitsObject>();
   mDateFormat                 = unitPrefs->GetDateFormatting();
   connect(unitPrefs, &wkf::UnitsObject::DateFormatChanged, this, &StatusWidget::DateFormatChanged);
}

void WkSimController::StatusWidget::Update()
{
   // Display Simulation Mode
   if (mPrefObjectPtr->GetShowSimMode())
   {
      if (mSimStateData.GetRealTime())
      {
         mUi.mSimModeWidget->hide();
      }
      else
      {
         mUi.mSimModeLabel->setText("NON-REALTIME");
         mUi.mSimModeLabel->setStyleSheet("#mSimModeLabel { color: rgba(255, 0, 0, 192); }");
         mUi.mSimModeWidget->show();
      }
   }

   // Update Simulation State
   if (mPrefObjectPtr->GetShowSimState())
   {
      if (mSimStateData.GetPaused())
      {
         mUi.mSimStateLabel->setText("PAUSED");
      }
      else
      {
         QString simStateString = QString::fromStdString(mSimStateData.GetStateString());
         if (!simStateString.isEmpty())
         {
            mUi.mSimStateLabel->setText(simStateString);
         }
      }
   }

   // Update Clock Rate
   if (mPrefObjectPtr->GetShowClockRate())
   {
      QString clockRateString = QString("x%1").arg(mSimStateData.GetClockRate(), 0, 'f', 2);

      mUi.mClockRateLabel->setText(clockRateString);
   }

   // Update Simulation Time
   if (mPrefObjectPtr->GetShowSimTime())
   {
      mUi.mSimTimeLabel->SetValue(mSimStateData.GetSimTime());
   }

   // Update Wall Time
   if (mPrefObjectPtr->GetShowWallTime())
   {
      UtCalendar calendar = mSimStateData.GetWallTime();

      QDate date(calendar.GetYear(), calendar.GetMonth(), calendar.GetDay());
      QTime time(calendar.GetHour(), calendar.GetMinute(), calendar.GetSecond());

      QDateTime wallTime(date, time, Qt::UTC);
      if (mPrefObjectPtr->GetWallTimeSpec() == Qt::LocalTime)
      {
         wallTime = wallTime.toLocalTime();
      }
      else if (mPrefObjectPtr->GetWallTimeSpec() == Qt::OffsetFromUTC)
      {
         wallTime = wallTime.toOffsetFromUtc((3600 * mPrefObjectPtr->GetUTC_OffsetHrs()) +
                                             (60 * mPrefObjectPtr->GetUTC_OffsetMins()));
      }
      if (mDateFormat == wkf::UnitsData::DATE_ISO)
      {
         mUi.mWallTimeLabel->setText(wallTime.toString(Qt::ISODate));
      }
      else if (mDateFormat == wkf::UnitsData::DATE_SHORT)
      {
         mUi.mWallTimeLabel->setText(wallTime.toString("MM/dd/yy hh:mm:ss"));
      }
      else
      {
         mUi.mWallTimeLabel->setText(wallTime.toString("MMM d yyyy hh:mm:ss"));
      }
   }

   // Update Progress Bar
   if (mPrefObjectPtr->GetShowProgressBar())
   {
      mUi.mProgressBar->setValue(mSimStateData.GetSimTime());
      mUi.mProgressBar->setMaximum(mSimStateData.GetEndTime());

#ifdef _WIN32
      if (mTaskbarProgressPtr)
      {
         mTaskbarProgressPtr->setValue(mSimStateData.GetSimTime());
         mTaskbarProgressPtr->setMaximum(mSimStateData.GetEndTime());
      }
#endif
   }

   // Update Time Behind
   if (mPrefObjectPtr->GetShowTimeBehind())
   {
      mUi.mTimeBehindWidget->setBackgroundRole(QPalette::Base);

      double timeBehind = mSimStateData.GetTimeBehind();
      mUi.mTimeBehindLabel->setText(QString::number(timeBehind, 'f', 1) + " s");

      QColor timeBehindColor;
      if (timeBehind >= 1.0)
      {
         timeBehindColor = QColor(Qt::red);
      }
      else if (timeBehind > 0.05) // 1 frame (20hz)
      {
         timeBehindColor = QColor(Qt::yellow);
      }
      else
      {
         timeBehindColor = QColor(Qt::green);
      }
      timeBehindColor.setAlpha(85);

      QString timeBehindCSS = QString("#mTimeBehindLabel { background-color: rgba(%1, %2, %3, %4); }");
      mUi.mTimeBehindLabel->setStyleSheet(timeBehindCSS.arg(timeBehindColor.red())
                                             .arg(timeBehindColor.green())
                                             .arg(timeBehindColor.blue())
                                             .arg(timeBehindColor.alpha()));

      mUi.mOtherSimulationsBehindWidget->setBackgroundRole(QPalette::Base);
      if (mSimStateData.GetApplicationsBehind())
      {
         mUi.mOtherSimulationsBehindLabel->setStyleSheet("QLabel { background-color : rgba(255, 0, 0, 85); }");
         QIcon   resourceIcon = QIcon::fromTheme("others_behind");
         QPixmap pixmap(resourceIcon.pixmap(QSize(64, 64)));
         mUi.mOtherSimulationsBehindLabel->setPixmap(pixmap.scaled(mUi.mOtherSimulationsBehindLabel->width(),
                                                                   mUi.mOtherSimulationsBehindLabel->height() * 9 / 10,
                                                                   Qt::KeepAspectRatio));
         mUi.mOtherSimulationsBehindLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
         mUi.mOtherSimulationsBehindWidget->show();
      }
      else
      {
         mUi.mOtherSimulationsBehindWidget->hide();
      }
   }
}

void WkSimController::StatusWidget::SimulationStarting()
{
#ifdef _WIN32
   if (mTaskbarProgressPtr)
   {
      mTaskbarProgressPtr->reset();
      mTaskbarProgressPtr->resume();
   }
#endif
}

void WkSimController::StatusWidget::SimulationPausing()
{
#ifdef _WIN32
   if (mTaskbarProgressPtr)
   {
      mTaskbarProgressPtr->pause();
   }
#endif
}

void WkSimController::StatusWidget::SimulationResuming()
{
#ifdef _WIN32
   if (mTaskbarProgressPtr)
   {
      mTaskbarProgressPtr->resume();
   }
#endif
}

void WkSimController::StatusWidget::SimulationComplete(double aSimTime)
{
#ifdef _WIN32
   if (mTaskbarProgressPtr && aSimTime < mSimStateData.GetEndTime())
   {
      mTaskbarProgressPtr->stop();
   }
#endif
}

void WkSimController::StatusWidget::showEvent(QShowEvent*)
{
#ifdef _WIN32
   if (!mTaskbarProgressPtr)
   {
      QWinTaskbarButton* buttonPtr = new QWinTaskbarButton(this);
      buttonPtr->setWindow(wkfEnv.GetMainWindow()->windowHandle());
      mTaskbarProgressPtr = buttonPtr->progress();
      mTaskbarProgressPtr->setVisible(mPrefObjectPtr->GetShowProgressBar());
   }
#endif
}

void WkSimController::StatusWidget::ShowClockRateHandler(bool aShow)
{
   mUi.mClockRateWidget->setVisible(aShow);
}

void WkSimController::StatusWidget::ShowProgressBarHandler(bool aShow)
{
   mUi.mProgressBar->setVisible(aShow);
#ifdef _WIN32
   if (mTaskbarProgressPtr)
   {
      mTaskbarProgressPtr->setVisible(aShow);
   }
#endif
}

void WkSimController::StatusWidget::ShowSimModeHandler(bool aShow)
{
   mUi.mSimModeWidget->setVisible(aShow);
}

void WkSimController::StatusWidget::ShowSimStateHandler(bool aShow)
{
   mUi.mSimStateWidget->setVisible(aShow);
}

void WkSimController::StatusWidget::ShowSimTimeHandler(bool aShow)
{
   mUi.mSimTimeWidget->setVisible(aShow);
}

void WkSimController::StatusWidget::ShowTimeBehindHandler(bool aShow)
{
   mUi.mTimeBehindWidget->setVisible(aShow);
   mUi.mOtherSimulationsBehindWidget->setVisible(aShow);
}

void WkSimController::StatusWidget::ShowWallTimeHandler(bool aShow)
{
   mUi.mWallTimeWidget->setVisible(aShow);
}

bool WkSimController::StatusWidget::eventFilter(QObject* aObj, QEvent* aEvent)
{
   if (aEvent->type() == QEvent::MouseButtonPress)
   {
      QMimeData*  mimeData = new QMimeData;
      QByteArray  bytes;
      QDataStream out(&bytes, QIODevice::WriteOnly);

      unsigned int qi = static_cast<unsigned int>(mPluginUniqueId);
      out << qi;
      out << QString();
      out << aObj->objectName();
      mimeData->setData(wkf::Updater::cMIME_IDENTIFIER, bytes); // need to give an address to the updater here
      QDrag* drag = new QDrag(this);
      drag->setMimeData(mimeData);
      drag->exec(Qt::DropAction::LinkAction);
      // capture the updater and pass it to whoever receives the drop
      return true;
   }
   return false;
}

void WkSimController::StatusWidget::DateFormatChanged(wkf::UnitsData::DateFormat aFormat)
{
   mDateFormat = aFormat;
   Update();
}
