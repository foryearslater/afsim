// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2017 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#ifndef TRACKUPDATERS_HPP
#define TRACKUPDATERS_HPP

#include "WkPlotUpdater.hpp"
#include "WkfEnvironment.hpp"
#include "WkfUnitsObject.hpp"
#include "WsfLocalTrack.hpp"

class WsfSimulation;

namespace WkTracks
{
class DataUpdater : public warlock::PlotUpdater
{
public:
   DataUpdater(const std::string& aPlatformName, int aTrackId, unsigned int aSeriesNum)
      : warlock::PlotUpdater(aPlatformName, aSeriesNum)
      , mLocalTrackId(aTrackId)
      , mLastUpdate(-1.0)
   {
   }

protected:
   void           ReadData(const WsfSimulation& aSimulation) override final;
   virtual double ReadDataP(const WsfLocalTrack* aTrack) = 0;

   int    mLocalTrackId;
   double mLastUpdate;
};

class AltitudeUpdater : public DataUpdater
{
public:
   AltitudeUpdater(const std::string& aPlatformName, int aTrackId, unsigned int aSeriesNum);

protected:
   double ReadDataP(const WsfLocalTrack* aTrack) override;
};

class BearingUpdater : public DataUpdater
{
public:
   BearingUpdater(const std::string& aPlatformName, int aTrackId, unsigned int aSeriesNum);

protected:
   double ReadDataP(const WsfLocalTrack* aTrack) override;
};

class ElevationUpdater : public DataUpdater
{
public:
   ElevationUpdater(const std::string& aPlatformName, int aTrackId, unsigned int aSeriesNum);

protected:
   double ReadDataP(const WsfLocalTrack* aTrack) override;
};

class HeadingUpdater : public DataUpdater
{
public:
   HeadingUpdater(const std::string& aPlatformName, int aTrackId, unsigned int aSeriesNum);

protected:
   double ReadDataP(const WsfLocalTrack* aTrack) override;
};

class LatitudeUpdater : public DataUpdater
{
public:
   LatitudeUpdater(const std::string& aPlatformName, int aTrackId, unsigned int aSeriesNum);

protected:
   double ReadDataP(const WsfLocalTrack* aTrack) override;
};

class LongitudeUpdater : public DataUpdater
{
public:
   LongitudeUpdater(const std::string& aPlatformName, int aTrackId, unsigned int aSeriesNum);

protected:
   double ReadDataP(const WsfLocalTrack* aTrack) override;
};

class RangeUpdater : public DataUpdater
{
public:
   RangeUpdater(const std::string& aPlatformName, int aTrackId, unsigned int aSeriesNum);

protected:
   double ReadDataP(const WsfLocalTrack* aTrack) override;
};

class SpeedUpdater : public DataUpdater
{
public:
   SpeedUpdater(const std::string& aPlatformName, int aTrackId, unsigned int aSeriesNum);

protected:
   double ReadDataP(const WsfLocalTrack* aTrack) override;
};

class MachUpdater : public DataUpdater
{
public:
   MachUpdater(const std::string& aPlatformName, int aTrackId, unsigned int aSeriesNum);

protected:
   double ReadDataP(const WsfLocalTrack* aTrack) override;
};
} // namespace WkTracks
#endif
