// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2018 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef ApplicationLauncherSimInterface_HPP
#define ApplicationLauncherSimInterface_HPP

#include "WkSimInterface.hpp"

class WsfSimulation;

namespace WkApplicationLauncher
{
class SimInterface : public warlock::SimInterfaceBase
{
public:
   SimInterface(const QString& aPluginName);

   const std::string& GetOutputFile() const;

private:
   void SimulationInitializing(const WsfSimulation& aSimulation) override;

   std::string mOutputFile;
};
} // namespace WkApplicationLauncher

#endif
