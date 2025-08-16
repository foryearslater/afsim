// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2020 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef ZONEBROWSERSIMCOMMANDS_HPP
#define ZONEBROWSERSIMCOMMANDS_HPP

#include "UtOrbitalElements.hpp"
#include "WkSimInterface.hpp"
#include "ZoneBrowserDockWidget.hpp"
#include "ZoneBrowserSimInterface.hpp"

namespace WkZoneBrowser
{
class ZoneCommand : public SimInterface::Command
{
public:
   ZoneCommand(SimInterface* aSimInterfacePtr, const std::string& aPlatform, const std::string& aZone, bool aUseWallClock = false)
      : Command(aSimInterfacePtr, aUseWallClock)
      , mPlatform(aPlatform)
      , mZone(aZone)
   {
   }

   virtual void Process(WsfSimulation& aSimulation){};

protected:
   std::string mPlatform;
   std::string mZone;
};

class UpdateZoneColorCommand : public ZoneCommand
{
public:
   UpdateZoneColorCommand(SimInterface*      aSimInterfacePtr,
                          const std::string& aPlatform,
                          const std::string& aZone,
                          const QColor&      aFillColor,
                          const QColor&      aLineColor,
                          bool               aUseWallClock = false)
      : ZoneCommand(aSimInterfacePtr, aPlatform, aZone, aUseWallClock)
      , mFillColor(aFillColor)
      , mLineColor(aLineColor)
   {
   }

   void Process(WsfSimulation& aSimulation) override;

private:
   QColor mFillColor;
   QColor mLineColor;
};

class SetFillColorCommand : public ZoneCommand
{
public:
   SetFillColorCommand(SimInterface*      aSimInterfacePtr,
                       const std::string& aPlatform,
                       const std::string& aZone,
                       const QColor&      aFillColor,
                       bool               aUseWallClock = false)
      : ZoneCommand(aSimInterfacePtr, aPlatform, aZone, aUseWallClock)
      , mFillColor(aFillColor)
   {
   }

   void Process(WsfSimulation& aSimulation) override;

private:
   QColor mFillColor;
};

class SetLineColorCommand : public ZoneCommand
{
public:
   SetLineColorCommand(SimInterface*      aSimInterfacePtr,
                       const std::string& aPlatform,
                       const std::string& aZone,
                       const QColor&      aLineColor,
                       bool               aUseWallClock = false)
      : ZoneCommand(aSimInterfacePtr, aPlatform, aZone, aUseWallClock)
      , mLineColor(aLineColor)
   {
   }

   void Process(WsfSimulation& aSimulation) override;

private:
   QColor mLineColor;
};
} // namespace WkZoneBrowser

#endif // ZONEBROWSERSIMCOMMANDS_HPP
