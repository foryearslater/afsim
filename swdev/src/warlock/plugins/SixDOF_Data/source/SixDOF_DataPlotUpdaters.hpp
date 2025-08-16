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

#ifndef SIXDOFDATAPLOTUPDATERS_HPP
#define SIXDOFDATAPLOTUPDATERS_HPP

#include "WkPlotUpdater.hpp"
#include "WkfEnvironment.hpp"
#include "WkfUnitsObject.hpp"

namespace wsf
{
namespace six_dof
{
class Mover;
}
} // namespace wsf
class WsfSimulation;

namespace WkSixDOF_Data
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
   void           ReadData(const WsfSimulation& aSimulation) final;
   virtual double ReadDataP(wsf::six_dof::Mover* aMover) = 0;

private:
   double mLastUpdate;
};

class AltitudePlotUpdater : public PlotUpdater
{
public:
   AltitudePlotUpdater(const std::string& aId, unsigned int aSeriesNum);

protected:
   double ReadDataP(wsf::six_dof::Mover* aMover) override;
   void   UnitChangedCB();
};

class VertSpeedPlotUpdater : public PlotUpdater
{
public:
   VertSpeedPlotUpdater(const std::string& aId, unsigned int aSeriesNum)
      : PlotUpdater(aId, aSeriesNum)
   {
   }

protected:
   double ReadDataP(wsf::six_dof::Mover* aMover) override;
};

class CasPlotUpdater : public PlotUpdater
{
public:
   CasPlotUpdater(const std::string& aId, unsigned int aSeriesNum)
      : PlotUpdater(aId, aSeriesNum)
   {
   }

protected:
   double ReadDataP(wsf::six_dof::Mover* aMover) override;
};

class MachPlotUpdater : public PlotUpdater
{
public:
   MachPlotUpdater(const std::string& aId, unsigned int aSeriesNum)
      : PlotUpdater(aId, aSeriesNum)
   {
   }

protected:
   double ReadDataP(wsf::six_dof::Mover* aMover) override;
};

class DynamicPressurePlotUpdater : public PlotUpdater
{
public:
   DynamicPressurePlotUpdater(const std::string& aId, unsigned int aSeriesNum)
      : PlotUpdater(aId, aSeriesNum)
   {
   }

protected:
   double ReadDataP(wsf::six_dof::Mover* aMover) override;
};

class HeadingPlotUpdater : public PlotUpdater
{
public:
   HeadingPlotUpdater(const std::string& aId, unsigned int aSeriesNum);

protected:
   double ReadDataP(wsf::six_dof::Mover* aMover) override;
   void   UnitChangedCB();
};

class PitchPlotUpdater : public PlotUpdater
{
public:
   PitchPlotUpdater(const std::string& aId, unsigned int aSeriesNum);

protected:
   double ReadDataP(wsf::six_dof::Mover* aMover) override;
   void   UnitChangedCB();
};

class RollPlotUpdater : public PlotUpdater
{
public:
   RollPlotUpdater(const std::string& aId, unsigned int aSeriesNum);

protected:
   double ReadDataP(wsf::six_dof::Mover* aMover) override;
   void   UnitChangedCB();
};

class NxPlotUpdater : public PlotUpdater
{
public:
   NxPlotUpdater(const std::string& aId, unsigned int aSeriesNum)
      : PlotUpdater(aId, aSeriesNum)
   {
   }

protected:
   double ReadDataP(wsf::six_dof::Mover* aMover) override;
};

class NyPlotUpdater : public PlotUpdater
{
public:
   NyPlotUpdater(const std::string& aId, unsigned int aSeriesNum)
      : PlotUpdater(aId, aSeriesNum)
   {
   }

protected:
   double ReadDataP(wsf::six_dof::Mover* aMover) override;
};

class GLoadPlotUpdater : public PlotUpdater
{
public:
   GLoadPlotUpdater(const std::string& aId, unsigned int aSeriesNum)
      : PlotUpdater(aId, aSeriesNum)
   {
   }

protected:
   double ReadDataP(wsf::six_dof::Mover* aMover) override;
};

class GAvailPlotUpdater : public PlotUpdater
{
public:
   GAvailPlotUpdater(const std::string& aId, unsigned int aSeriesNum)
      : PlotUpdater(aId, aSeriesNum)
   {
   }

protected:
   double ReadDataP(wsf::six_dof::Mover* aMover) override;
};

class AlphaPlotUpdater : public PlotUpdater
{
public:
   AlphaPlotUpdater(const std::string& aId, unsigned int aSeriesNum);

protected:
   double ReadDataP(wsf::six_dof::Mover* aMover) override;
   void   UnitChangedCB();
};

class BetaPlotUpdater : public PlotUpdater
{
public:
   BetaPlotUpdater(const std::string& aId, unsigned int aSeriesNum);

protected:
   double ReadDataP(wsf::six_dof::Mover* aMover) override;
   void   UnitChangedCB();
};

class AlphaDotPlotUpdater : public PlotUpdater
{
public:
   AlphaDotPlotUpdater(const std::string& aId, unsigned int aSeriesNum);

protected:
   double ReadDataP(wsf::six_dof::Mover* aMover) override;
   void   UnitChangedCB();
};

class BetaDotPlotUpdater : public PlotUpdater
{
public:
   BetaDotPlotUpdater(const std::string& aId, unsigned int aSeriesNum);

protected:
   double ReadDataP(wsf::six_dof::Mover* aMover) override;
   void   UnitChangedCB();
};

class YawRatePlotUpdater : public PlotUpdater
{
public:
   YawRatePlotUpdater(const std::string& aId, unsigned int aSeriesNum);

protected:
   double ReadDataP(wsf::six_dof::Mover* aMover) override;
   void   UnitChangedCB();
};

class PitchRatePlotUpdater : public PlotUpdater
{
public:
   PitchRatePlotUpdater(const std::string& aId, unsigned int aSeriesNum);

protected:
   double ReadDataP(wsf::six_dof::Mover* aMover) override;
   void   UnitChangedCB();
};

class RollRatePlotUpdater : public PlotUpdater
{
public:
   RollRatePlotUpdater(const std::string& aId, unsigned int aSeriesNum);

protected:
   double ReadDataP(wsf::six_dof::Mover* aMover) override;
   void   UnitChangedCB();
};

class LiftPlotUpdater : public PlotUpdater
{
public:
   LiftPlotUpdater(const std::string& aId, unsigned int aSeriesNum);

protected:
   double ReadDataP(wsf::six_dof::Mover* aMover) override;
   void   UnitChangedCB();
};

class DragPlotUpdater : public PlotUpdater
{
public:
   DragPlotUpdater(const std::string& aId, unsigned int aSeriesNum);

protected:
   double ReadDataP(wsf::six_dof::Mover* aMover) override;
   void   UnitChangedCB();
};

class ThrustPlotUpdater : public PlotUpdater
{
public:
   ThrustPlotUpdater(const std::string& aId, unsigned int aSeriesNum);

protected:
   double ReadDataP(wsf::six_dof::Mover* aMover) override;
   void   UnitChangedCB();
};

class FuelFlowPlotUpdater : public PlotUpdater
{
public:
   FuelFlowPlotUpdater(const std::string& aId, unsigned int aSeriesNum)
      : PlotUpdater(aId, aSeriesNum)
   {
   }

protected:
   double ReadDataP(wsf::six_dof::Mover* aMover) override;
};

class FuelCapacityPlotUpdater : public PlotUpdater
{
public:
   FuelCapacityPlotUpdater(const std::string& aId, unsigned int aSeriesNum);

protected:
   double ReadDataP(wsf::six_dof::Mover* aMover) override;
   void   UnitChangedCB();
};

class FuelRemainingPlotUpdater : public PlotUpdater
{
public:
   FuelRemainingPlotUpdater(const std::string& aId, unsigned int aSeriesNum);

protected:
   double ReadDataP(wsf::six_dof::Mover* aMover) override;
   void   UnitChangedCB();
};

class FuelInternalPlotUpdater : public PlotUpdater
{
public:
   FuelInternalPlotUpdater(const std::string& aId, unsigned int aSeriesNum);

protected:
   double ReadDataP(wsf::six_dof::Mover* aMover) override;
   void   UnitChangedCB();
};

class FuelExternalPlotUpdater : public PlotUpdater
{
public:
   FuelExternalPlotUpdater(const std::string& aId, unsigned int aSeriesNum);

protected:
   double ReadDataP(wsf::six_dof::Mover* aMover) override;
   void   UnitChangedCB();
};

class StickBackPlotUpdater : public PlotUpdater
{
public:
   StickBackPlotUpdater(const std::string& aId, unsigned int aSeriesNum)
      : PlotUpdater(aId, aSeriesNum)
   {
   }

protected:
   double ReadDataP(wsf::six_dof::Mover* aMover) override;
};

class StickRightPlotUpdater : public PlotUpdater
{
public:
   StickRightPlotUpdater(const std::string& aId, unsigned int aSeriesNum)
      : PlotUpdater(aId, aSeriesNum)
   {
   }

protected:
   double ReadDataP(wsf::six_dof::Mover* aMover) override;
};

class RudderRightPlotUpdater : public PlotUpdater
{
public:
   RudderRightPlotUpdater(const std::string& aId, unsigned int aSeriesNum)
      : PlotUpdater(aId, aSeriesNum)
   {
   }

protected:
   double ReadDataP(wsf::six_dof::Mover* aMover) override;
};

class ThrottlePlotUpdater : public PlotUpdater
{
public:
   ThrottlePlotUpdater(const std::string& aId, unsigned int aSeriesNum)
      : PlotUpdater(aId, aSeriesNum)
   {
   }

protected:
   double ReadDataP(wsf::six_dof::Mover* aMover) override;
};

class SpeedbrakeCommandPlotUpdater : public PlotUpdater
{
public:
   SpeedbrakeCommandPlotUpdater(const std::string& aId, unsigned int aSeriesNum)
      : PlotUpdater(aId, aSeriesNum)
   {
   }

protected:
   double ReadDataP(wsf::six_dof::Mover* aMover) override;
};

class SpeedbrakeAnglePlotUpdater : public PlotUpdater
{
public:
   SpeedbrakeAnglePlotUpdater(const std::string& aId, unsigned int aSeriesNum);

protected:
   double ReadDataP(wsf::six_dof::Mover* aMover) override;
   void   UnitChangedCB();
};

class TotalWeightPlotUpdater : public PlotUpdater
{
public:
   TotalWeightPlotUpdater(const std::string& aId, unsigned int aSeriesNum);

protected:
   double ReadDataP(wsf::six_dof::Mover* aMover) override;
   void   UnitChangedCB();
};

} // namespace WkSixDOF_Data

#endif
