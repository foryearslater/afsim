//*****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2021 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
//*****************************************************************************

#ifndef ORBITALDATAPLOTUPDATERS_HPP
#define ORBITALDATAPLOTUPDATERS_HPP

#include "WkPlotUpdater.hpp"

class WsfSpaceMoverBase;
class WsfSimulation;
class UtOrbitalElements;

namespace WkOrbitalData
{

class PlotUpdater : public warlock::PlotUpdater
{
public:
   PlotUpdater(const std::string& aId, unsigned int aSeriesNum)
      : warlock::PlotUpdater(aId, aSeriesNum)
      , mLastUpdate(-1.0)
   {
   }

protected:
   void           ReadData(const WsfSimulation& aSimulation) override final;
   virtual double ReadDataP(const UtOrbitalElements& aElements) = 0;

private:
   double mLastUpdate;
};

class EccentricityPlotUpdater : public PlotUpdater
{
public:
   EccentricityPlotUpdater(const std::string& aId, unsigned int aSeriesNum)
      : PlotUpdater(aId, aSeriesNum)
   {
   }

protected:
   double ReadDataP(const UtOrbitalElements& aElements) override;
   void   UnitChangedCB();
};

class SemiMajorAxisPlotUpdater : public PlotUpdater
{
public:
   SemiMajorAxisPlotUpdater(const std::string& aId, unsigned int aSeriesNum);

protected:
   double ReadDataP(const UtOrbitalElements& aElements) override;
   void   UnitChangedCB();
};

class InclinationPlotUpdater : public PlotUpdater
{
public:
   InclinationPlotUpdater(const std::string& aId, unsigned int aSeriesNum);

protected:
   double ReadDataP(const UtOrbitalElements& aElements) override;
   void   UnitChangedCB();
};

class RAAN_PlotUpdater : public PlotUpdater
{
public:
   RAAN_PlotUpdater(const std::string& aId, unsigned int aSeriesNum);

protected:
   double ReadDataP(const UtOrbitalElements& aElements) override;
   void   UnitChangedCB();
};

class ArgOfPeriapsisPlotUpdater : public PlotUpdater
{
public:
   ArgOfPeriapsisPlotUpdater(const std::string& aId, unsigned int aSeriesNum);

protected:
   double ReadDataP(const UtOrbitalElements& aElements) override;
   void   UnitChangedCB();
};

class TrueAnomalyPlotUpdater : public PlotUpdater
{
public:
   TrueAnomalyPlotUpdater(const std::string& aId, unsigned int aSeriesNum);

protected:
   double ReadDataP(const UtOrbitalElements& aElements) override;
   void   UnitChangedCB();
};
} // namespace WkOrbitalData

#endif
