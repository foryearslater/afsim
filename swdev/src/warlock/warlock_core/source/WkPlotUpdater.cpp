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
#include "WkPlotUpdater.hpp"

#include <QMutexLocker>

#include "UtQtGL2DPlot.hpp"
#include "WkSimEnvironment.hpp"
#include "WkfMainWindow.hpp"
#include "WsfClockSource.hpp"
#include "WsfSimulation.hpp"

warlock::PlotUpdater::PlotUpdater(const std::string& aPlatformId, unsigned int aSeriesNum)
   : SimInterfaceBase("WarlockPlotUpdater")
   , mSeriesNum(aSeriesNum)
   , mPlatformId(aPlatformId)
   , mUnitType(0)
   , mUnitName("")
   , mSimStartPending(false)
{
}

void warlock::PlotUpdater::SimulationClockRead(const WsfSimulation& aSimulation)
{
   QMutexLocker locker(&mMutex);
   ReadData(aSimulation);
}

void warlock::PlotUpdater::SimulationStarting(const WsfSimulation& aSimulation)
{
   QMutexLocker locker(&mMutex);
   mData.clear();
   mSimStartPending = true;
}

void warlock::PlotUpdater::SetUnit(int aUnitType, const QString& aNewUnitName)
{
   bool unitChanged = false;
   // scope the mutex locker so the mutex is release as soon as it is safe to do so
   {
      QMutexLocker locker(&mMutex);
      // Clear Pending Data when the unit changes
      mData.clear();
      unitChanged = (aUnitType != mUnitType);
      mUnitType   = aUnitType;
   }

   // Outside of the mutex lock
   mUnitName = aNewUnitName;
   if (unitChanged)
   {
      emit UnitChanged(aNewUnitName);
   }
}

void warlock::PlotUpdater::Update(UtQtGL2DPlot& aPlot)
{
   QMutexLocker locker(&mMutex);
   if (mSimStartPending)
   {
      aPlot.ClearData();
      mSimStartPending = false;
   }
   for (auto& iter : mData)
   {
      aPlot.AddPoint(iter.first, iter.second, mSeriesNum);
   }
   mData.clear();
}
