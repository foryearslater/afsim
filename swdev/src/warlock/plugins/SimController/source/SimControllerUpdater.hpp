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

#ifndef SIMCONTROLLERUPDATER_HPP
#define SIMCONTROLLERUPDATER_HPP

#include "UtCalendar.hpp"
#include "WkSimEnvironment.hpp"
#include "WkSimInterface.hpp"
#include "WkfUnitsObject.hpp"
#include "WkfUpdater.hpp"
#include "WsfSimulation.hpp"

namespace WkSimController
{
class Updater : public wkf::Updater, public warlock::SimInterfaceBase
{
public:
   Updater(const QString& aName);
   ~Updater() override
   {
      if (warlock::SimEnvironment::Exists())
      {
         disconnect(&simEnv, nullptr, this, nullptr);
      }
      QMutexLocker locker(&mMutex);
   }
   void         WallClockRead(const WsfSimulation& aSimulation) override;
   virtual void Read(const WsfSimulation& aSimulation) = 0;
};

class SimTimeUpdater : public Updater
{
public:
   SimTimeUpdater();
   ~SimTimeUpdater() override = default;
   QString       GetValueString() const override;
   const QString GetDisplayName() const override { return ""; }
   void          Read(const WsfSimulation& aSimulation) override;

private:
   double mSimTime;
};

class WallTimeUpdater : public Updater
{
public:
   WallTimeUpdater();
   ~WallTimeUpdater() override = default;
   QString       GetValueString() const override;
   void          Read(const WsfSimulation& aSimulation) override;
   const QString GetDisplayName() const override { return ""; }

private:
   void                       DateFormatChanged(wkf::UnitsData::DateFormat aFormat) { mDateFormat = aFormat; }
   double                     mSimTime;
   UtCalendar                 mEpoch;
   wkf::UnitsData::DateFormat mDateFormat;
};
class ClockRateUpdater : public Updater
{
public:
   ClockRateUpdater();
   ~ClockRateUpdater() override = default;
   QString GetValueString() const override;
   void    Read(const WsfSimulation& aSimulation) override;

private:
   double mRate;
};
}; // namespace WkSimController

#endif
