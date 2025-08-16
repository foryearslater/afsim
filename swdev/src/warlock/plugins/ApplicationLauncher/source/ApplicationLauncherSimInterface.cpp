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
#include "ApplicationLauncherSimInterface.hpp"

#include "WsfEventPipe.hpp"
#include "WsfSimulation.hpp"

WkApplicationLauncher::SimInterface::SimInterface(const QString& aPluginName)
   : warlock::SimInterfaceBase(aPluginName)
   , mOutputFile("")
{
}

void WkApplicationLauncher::SimInterface::SimulationInitializing(const WsfSimulation& aSimulation)
{
   WsfSimulationExtension* simExtension = aSimulation.FindExtension("event_pipe");
   WsfEventPipeInterface*  eventPipe    = dynamic_cast<WsfEventPipeInterface*>(simExtension);
   if (eventPipe)
   {
      QMutexLocker locker(&mMutex);
      mOutputFile = eventPipe->GetOutputFileName();
   }
}

const std::string& WkApplicationLauncher::SimInterface::GetOutputFile() const
{
   QMutexLocker locker(&mMutex);
   return mOutputFile;
}
