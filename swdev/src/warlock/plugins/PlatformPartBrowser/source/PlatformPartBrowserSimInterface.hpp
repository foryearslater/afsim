// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2016-2017 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#ifndef PLATFORMPARTBROWSERSIMINTERFACE_HPP
#define PLATFORMPARTBROWSERSIMINTERFACE_HPP

#include <QMetaType>
#include <QVariant>

#include "PlatformPartBrowserSimEvents.hpp"
#include "PlatformPartBrowserTypes.hpp"
#include "UtCallbackHolder.hpp"
#include "WkSimInterface.hpp"

class WsfMover;
class WsfPlatformPart;
class WsfRadarSensor;

namespace WkPlatformPartBrowser
{
class SimInterface : public warlock::SimInterfaceT<PlatformPartEvent>
{
   Q_OBJECT
public:
   SimInterface(const QString& aPluginName);
   ~SimInterface() override;

   void SetPlatformName(const std::string& aPlatformName);

protected:
   void SimulationInitializing(const WsfSimulation& aSimulation) override;
   void WallClockRead(const WsfSimulation& aSimulation) override;

private:
   void Clear();

   void PlatformPartChangedHandler(const WsfPlatformPart* aPlatformPart, PropertyName aName, QVariant aVariant);

   static void SetIconType(PlatformPart& aPartInfo, PartType aPartType);

   static void Populate(const WsfPlatformPart* aPlatformPart, PlatformPart& aPartInfo);
   static void PopulatePlatformPart(const WsfPlatformPart* aPlatformPart, PlatformPart& aPartInfo);
   static void PopulateArticulatedPart(const WsfPlatformPart* aPlatformPart, PlatformPart& aPartInfo);
   static void PopulateMover(const WsfPlatformPart* aPlatformPart, PlatformPart& aPartInfo);
   static void PopulateProcessor(const WsfPlatformPart* aPlatformPart, PlatformPart& aPartInfo);
   static void PopulateFuel(const WsfPlatformPart* aPlatformPart, PlatformPart& aPartInfo);
   static void PopulateSensor(const WsfPlatformPart* aPlatformPart, PlatformPart& aPartInfo);
   static void PopulateComm(const WsfPlatformPart* aPlatformPart, PlatformPart& aPartInfo);
   static void PopulateWeapon(const WsfPlatformPart* aPlatformPart, PlatformPart& aPartInfo);
   static void PopulateExplicitWeapon(const WsfPlatformPart* aPlatformPart, PlatformPart& aPartInfo);
   static void PopulateRadar(const WsfRadarSensor* aRadarSensor, PlatformPart& aPartInfo);

   std::string      mPlatformOfInterest; // Name of the selected platform
   bool             mPlatformChanged;    // did the platform of interest change?
   UtCallbackHolder mCallbacks;
};
} // namespace WkPlatformPartBrowser
#endif