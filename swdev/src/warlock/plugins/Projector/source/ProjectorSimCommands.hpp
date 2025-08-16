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

#ifndef PROJECTORSIMCOMMANDS_HPP
#define PROJECTORSIMCOMMANDS_HPP

#include <string>

class WsfSimulation;

#include "ProjectorSimInterface.hpp"

namespace Projector
{
class InstallProjectorUpdaterCommand : public SimInterface::Command
{
public:
   InstallProjectorUpdaterCommand(SimInterface*      aSimInterfacePtr,
                                  const std::string& aPlatformName,
                                  const std::string& aSensorName,
                                  bool               aUseWallClock = false)
      : Command(aSimInterfacePtr, aUseWallClock)
      , mLocalSimInterface(aSimInterfacePtr)
      , mPlatformName{aPlatformName}
      , mSensorName{aSensorName}
   {
   }

   void Process(WsfSimulation& aSimulation) override;

private:
   SimInterface* mLocalSimInterface;
   std::string   mPlatformName;
   std::string   mSensorName;
};

class UninstallProjectorUpdaterCommand : public SimInterface::Command
{
public:
   UninstallProjectorUpdaterCommand(SimInterface*      aSimInterfacePtr,
                                    const std::string& aPlatformName,
                                    const std::string& aSensorName,
                                    bool               aUseWallClock = false)
      : Command(aSimInterfacePtr, aUseWallClock)
      , mLocalSimInterface(aSimInterfacePtr)
      , mPlatformName{aPlatformName}
      , mSensorName{aSensorName}
   {
   }

   void Process(WsfSimulation& aSimulation) override;

private:
   SimInterface* mLocalSimInterface;
   std::string   mPlatformName;
   std::string   mSensorName;
};
} // namespace Projector

#endif // PROJECTORSIMCOMMANDS_HPP
