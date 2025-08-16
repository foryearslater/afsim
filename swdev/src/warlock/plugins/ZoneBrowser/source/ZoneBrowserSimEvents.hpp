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
#ifndef WkSimEventsZoneBrowser_HPP
#define WkSimEventsZoneBrowser_HPP

#include <QColor>

#include "WkSimInterface.hpp"
#include "zone_browser/WkfZoneBrowserDataContainer.hpp"
#include "zone_browser/WkfZoneBrowserDockWidget.hpp"

namespace WkZoneBrowser
{
class ZoneEvent : public warlock::SimEvent
{
public:
   virtual void Process(wkf::ZoneBrowserDataContainer& aData) = 0;
};

class NewZoneEvent : public ZoneEvent
{
public:
   NewZoneEvent()
      : mZoneSetData("", "")
   {
   }
   explicit NewZoneEvent(const wkf::ZoneSetData& aZoneSetData)
      : mZoneSetData(aZoneSetData)
   {
   }

   void Process(wkf::ZoneBrowserDataContainer& aData) override { aData.AddZoneSet(mZoneSetData); }

private:
   wkf::ZoneSetData mZoneSetData;
};

class DataCopyCompleteEvent : public ZoneEvent
{
public:
   DataCopyCompleteEvent()
      : ZoneEvent()
   {
   }

   void Process(wkf::ZoneBrowserDataContainer& aData) override { aData.NotifyWidget(); }
};

class PlatformDeletedEvent : public ZoneEvent
{
public:
   explicit PlatformDeletedEvent(const std::string& aPlatformName)
      : ZoneEvent()
      , mPlatformName(aPlatformName)
   {
   }

   void Process(wkf::ZoneBrowserDataContainer& aData) override { aData.RemovePlatform(mPlatformName); }

private:
   std::string mPlatformName;
};

class SimulationCompleteEvent : public ZoneEvent
{
public:
   SimulationCompleteEvent()
      : ZoneEvent()
   {
   }

   void Process(wkf::ZoneBrowserDataContainer& aData) override { aData.ClearData(); }
};

class UpdateZoneColor : public ZoneEvent
{
public:
   UpdateZoneColor(const std::string& aPlatformName,
                   const std::string& aZoneName,
                   const QColor&      aFillColor,
                   const QColor&      aLineColor)
      : ZoneEvent()
      , mPlatform(aPlatformName)
      , mZone(aZoneName)
      , mFillColor(aFillColor)
      , mLineColor(aLineColor)
   {
   }

   void Process(wkf::ZoneBrowserDataContainer& aData) override;

private:
   std::string mPlatform;
   std::string mZone;
   QColor      mFillColor;
   QColor      mLineColor;
};

class UpdateFillColor : public ZoneEvent
{
public:
   UpdateFillColor(const std::string& aPlatformName, const std::string& aZoneName, const QColor& aColor)
      : ZoneEvent()
      , mPlatform(aPlatformName)
      , mZone(aZoneName)
      , mColor(aColor)
   {
   }

   void Process(wkf::ZoneBrowserDataContainer& aData) override;

private:
   std::string mPlatform;
   std::string mZone;
   QColor      mColor;
};

class UpdateLineColor : public ZoneEvent
{
public:
   UpdateLineColor(const std::string& aPlatformName, const std::string& aZoneName, const QColor& aColor)
      : ZoneEvent()
      , mPlatform(aPlatformName)
      , mZone(aZoneName)
      , mColor(aColor)
   {
   }

   void Process(wkf::ZoneBrowserDataContainer& aData) override;

private:
   std::string mPlatform;
   std::string mZone;
   QColor      mColor;
};
} // namespace WkZoneBrowser

#endif
