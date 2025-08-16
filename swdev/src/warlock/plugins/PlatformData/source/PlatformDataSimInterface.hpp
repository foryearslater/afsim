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
#ifndef PLATFORMDATAINTERFACE_HPP
#define PLATFORMDATAINTERFACE_HPP

#include <QObject>

class WsfSimulation;

#include "PlatformDataSimEvents.hpp"
#include "WkSimInterface.hpp"

namespace WkPlatformData
{
class SimInterface : public warlock::SimInterfaceT<PlatformDataEvent>
{
   Q_OBJECT

public:
   SimInterface(const QString& aPluginName);

   void SetPlatformOfInterest(const std::string& aPlatformName);

   // Executed on the simulation thread to read and write data from/to the simulation
   void WallClockRead(const WsfSimulation& aSimulation) override;

private:
   std::string mPlatformOfInterest;
};
} // namespace WkPlatformData
#endif // PLATFORMDATAINTERFACE_HPP
