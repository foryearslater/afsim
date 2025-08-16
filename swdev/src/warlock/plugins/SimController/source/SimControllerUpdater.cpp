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

#include "SimControllerUpdater.hpp"

#include <sstream>

#include <QDateTime>

#include "UtTime.hpp"
#include "WkfEnvironment.hpp"
#include "WkfUnitsObject.hpp"
#include "WsfDateTime.hpp"

WkSimController::Updater::Updater(const QString& aName)
   : wkf::Updater(aName, "")
   , warlock::SimInterfaceBase(aName)
{
}

void WkSimController::Updater::WallClockRead(const WsfSimulation& aSimulation)
{
   if (mMutex.tryLock())
   {
      Read(aSimulation);
      mMutex.unlock();
   }
}

WkSimController::SimTimeUpdater::SimTimeUpdater()
   : Updater("Sim Time")
   , mSimTime(0.0)
{
}

void WkSimController::SimTimeUpdater::Read(const WsfSimulation& aSimulation)
{
   mSimTime = aSimulation.GetSimTime();
}

QString WkSimController::SimTimeUpdater::GetValueString() const
{
   std::ostringstream oss;
   oss << UtTime(mSimTime, (wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetTimeFormatting()));
   return oss.str().c_str();
}

WkSimController::WallTimeUpdater::WallTimeUpdater()
   : Updater("Wall Time")
   , mSimTime(0.0)
   , mEpoch()
{
   wkf::UnitsObject* unitPrefs = wkfEnv.GetPreferenceObject<wkf::UnitsObject>();
   mDateFormat                 = unitPrefs->GetDateFormatting();
   connect(unitPrefs, &wkf::UnitsObject::DateFormatChanged, this, &WallTimeUpdater::DateFormatChanged);
}

void WkSimController::WallTimeUpdater::Read(const WsfSimulation& aSimulation)
{
   mSimTime = aSimulation.GetSimTime();
   mEpoch   = aSimulation.GetDateTime().GetCurrentTime(0.0);
}

QString WkSimController::WallTimeUpdater::GetValueString() const
{
   UtCalendar c = mEpoch;
   c.AdvanceTimeBy(mSimTime);
   QDate     date(c.GetYear(), c.GetMonth(), c.GetDay());
   QTime     time(c.GetHour(), c.GetMinute(), c.GetSecond());
   QDateTime wallTime(date, time, Qt::UTC);

   if (mDateFormat == wkf::UnitsData::DATE_ISO)
   {
      return wallTime.toString(Qt::ISODate);
   }
   else if (mDateFormat == wkf::UnitsData::DATE_SHORT)
   {
      return wallTime.toString("MM/dd/yy hh:mm:ss");
   }
   else
   {
      return wallTime.toString("MMM d yyyy hh:mm:ss");
   }
   return wallTime.toString(Qt::ISODate);
}

WkSimController::ClockRateUpdater::ClockRateUpdater()
   : Updater("Clock Rate")
   , mRate(1.0)
{
}

QString WkSimController::ClockRateUpdater::GetValueString() const
{
   return QString("x%1").arg(mRate, 0, 'f', 2);
}

void WkSimController::ClockRateUpdater::Read(const WsfSimulation& aSimulation)
{
   mRate = aSimulation.GetClockRate();
}
