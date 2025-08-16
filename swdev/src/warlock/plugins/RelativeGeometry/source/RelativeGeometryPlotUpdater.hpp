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

#ifndef RELATIVEGEOMETRYPLOTUPDATER_HPP
#define RELATIVEGEOMETRYPLOTUPDATER_HPP

#include "WkPlotUpdater.hpp"

class UtEntity;

namespace RelativeGeometry
{
class PlotUpdater : public warlock::PlotUpdater
{
public:
   PlotUpdater(const QString& aId, const QString& aFromPlatform, const QString& aToPlatform);
   ~PlotUpdater() override = default;

private:
   void   ReadData(const WsfSimulation& aSimulation) override final;
   double ReadDataP(const WsfPlatform& aFrom, const WsfPlatform& aTo);

   double      mLastUpdateTime;
   QString     mDatum;
   std::string mToPlatformName;
};
} // namespace RelativeGeometry

#endif
