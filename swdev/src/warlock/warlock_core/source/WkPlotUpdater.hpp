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
#ifndef WkPlotUpdater_HPP
#define WkPlotUpdater_HPP

#include <iostream>
#include <list>

#include <QMutex>
#include <QObject>

class UtQtGL2DPlot;
class WsfSimulation;

#include "warlock_core_export.h"

#include "WkSimInterface.hpp"

namespace warlock
{
class WARLOCK_CORE_EXPORT PlotUpdater : public SimInterfaceBase
{
   Q_OBJECT
public:
   PlotUpdater(const std::string& aId, unsigned int aSeriesNum);

   // The following two functions are called on the simulation thread
   void SimulationClockRead(const WsfSimulation& aSimulation) override final;
   void SimulationStarting(const WsfSimulation& aSimulation) override final;

   // The following functions are called on the GUI thread
   // Update() will add data to the plot widget
   void Update(UtQtGL2DPlot& aPlotWidget);
   // Set/Get the Unit of the values stored by this PlotUpdater
   void           SetUnit(int aUnitType, const QString& aUnitName);
   const QString& GetUnitName() const { return mUnitName; }

signals:
   void UnitChanged(const QString& aNewUnitName);

protected:
   // ReadData is volved from the SimulationClockRead() function.
   virtual void ReadData(const WsfSimulation& aSimulation) = 0;

   unsigned int mSeriesNum;
   std::string  mPlatformId;
   int          mUnitType;
   QString      mUnitName;
   bool         mSimStartPending;

   std::list<std::pair<double, double>> mData;
};
} // namespace warlock
#endif
