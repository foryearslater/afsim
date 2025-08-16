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
#ifndef PLATFORMDATAPLOTUPDATERS_HPP
#define PLATFORMDATAPLOTUPDATERS_HPP

#include <string>

#include "WkPlotUpdater.hpp"
#include "WkfEnvironment.hpp"
#include "WkfUnitsObject.hpp"

class WsfPlatform;
class WsfSimulation;

namespace WkPlatformData
{
class PlotUpdater : public warlock::PlotUpdater
{
public:
   PlotUpdater(const std::string& aId, unsigned int aSeriesNum)
      : warlock::PlotUpdater(aId, aSeriesNum)
      , mLastUpdateTime(-1.0)
   {
   }

protected:
   virtual double ReadDataP(WsfPlatform* aPlatform) = 0;
   void           ReadData(const WsfSimulation& aSimulation) override final;
   double         mLastUpdateTime;
};

class AltitudePlotUpdater : public PlotUpdater
{
public:
   AltitudePlotUpdater(const std::string& aId, unsigned int aSeriesNum);

protected:
   double ReadDataP(WsfPlatform* aPlatform) override;
   void   UnitChangedCB();
};

class LatitudePlotUpdater : public PlotUpdater
{
public:
   LatitudePlotUpdater(const std::string& aId, unsigned int aSeriesNum);

protected:
   double ReadDataP(WsfPlatform* aPlatform) override;
   void   UnitChangedCB();
};

class LongitudePlotUpdater : public PlotUpdater
{
public:
   LongitudePlotUpdater(const std::string& aId, unsigned int aSeriesNum);

protected:
   double ReadDataP(WsfPlatform* aPlatform) override;
   void   UnitChangedCB();
};

class HeadingPlotUpdater : public PlotUpdater
{
public:
   HeadingPlotUpdater(const std::string& aId, unsigned int aSeriesNum);

protected:
   double ReadDataP(WsfPlatform* aPlatform) override;
   void   UnitChangedCB();
};

class PitchPlotUpdater : public PlotUpdater
{
public:
   PitchPlotUpdater(const std::string& aId, unsigned int aSeriesNum);

protected:
   double ReadDataP(WsfPlatform* aPlatform) override;
   void   UnitChangedCB();
};

class RollPlotUpdater : public PlotUpdater
{
public:
   RollPlotUpdater(const std::string& aId, unsigned int aSeriesNum);

protected:
   double ReadDataP(WsfPlatform* aPlatform) override;
   void   UnitChangedCB();
};

class SpeedPlotUpdater : public PlotUpdater
{
public:
   SpeedPlotUpdater(const std::string& aId, unsigned int aSeriesNum);

protected:
   double ReadDataP(WsfPlatform* aPlatform) override;
   void   UnitChangedCB();
};

class MachPlotUpdater : public PlotUpdater
{
public:
   MachPlotUpdater(const std::string& aId, unsigned int aSeriesNum);

protected:
   double ReadDataP(WsfPlatform* aPlatform) override;
   void   UnitChangedCB();
};

class DamageFactorPlotUpdater : public PlotUpdater
{
public:
   DamageFactorPlotUpdater(const std::string& aId, unsigned int aSeriesNum)
      : PlotUpdater(aId, aSeriesNum)
   {
   }

protected:
   double ReadDataP(WsfPlatform* aPlatform) override;
};

class FuelPlotUpdater : public PlotUpdater
{
public:
   FuelPlotUpdater(const std::string& aId, unsigned int aSeriesNum)
      : PlotUpdater(aId, aSeriesNum)
   {
   }

protected:
   double ReadDataP(WsfPlatform* aPlatform) override;
};

class AuxDataPlotUpdater : public PlotUpdater
{
public:
   AuxDataPlotUpdater(const std::string& aId, const std::string& aDataField, unsigned int aSeriesNum)
      : PlotUpdater(aId, aSeriesNum)
      , mDataField(aDataField)
   {
   }

protected:
   double      ReadDataP(WsfPlatform* aPlatform) override;
   std::string mDataField;
};
} // namespace WkPlatformData
#endif
