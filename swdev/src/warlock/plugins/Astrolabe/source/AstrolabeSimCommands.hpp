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

#ifndef ASTROLABESIMCOMMANDS_HPP
#define ASTROLABESIMCOMMANDS_HPP

#include <memory>

#include "AstrolabeSimInterface.hpp"

class WsfSimulation;
#include "OrbitalMissionData.hpp"
#include "WsfOrbitalMissionSequence.hpp"

namespace WkAstrolabe
{

//! A Command to cancel the maneuver sequence on a platform.
class CancelManeuversCommand : public warlock::SimCommand
{
public:
   explicit CancelManeuversCommand(const QString& aPlatformName)
      : warlock::SimCommand{false}
      , mPlatformName{aPlatformName.toStdString()}
   {
   }

   ~CancelManeuversCommand() override = default;

   void Process(WsfSimulation& aSimulation) override;

private:
   std::string mPlatformName{};
};

//! A Command to assign a mission sequence to a platform.
class InjectMissionSequenceCommand : public warlock::SimCommand
{
public:
   InjectMissionSequenceCommand(const QString& aPlatformName, const wsfg::spaceg::OrbitalMissionData& aMission)
      : warlock::SimCommand{false}
      , mPlatformName{aPlatformName.toStdString()}
      , mMission{aMission}
   {
   }

   ~InjectMissionSequenceCommand() override = default;

   void Process(WsfSimulation& aSimulation) override;

private:
   std::string                      mPlatformName;
   wsfg::spaceg::OrbitalMissionData mMission;
};

//! A Command to run verification on a mission sequence for a platform.
class VerifyMissionCommand : public SimInterface::Command
{
public:
   VerifyMissionCommand(const QString&                          aPlatformName,
                        const wsfg::spaceg::OrbitalMissionData& aData,
                        SimInterface*                           aSimInterfacePtr)
      : Command{aSimInterfacePtr, true}
      , mPlatformName{aPlatformName.toStdString()}
      , mData{aData}
   {
   }

   ~VerifyMissionCommand() override = default;

   void Process(WsfSimulation& aSimulation) override;

private:
   std::string                      mPlatformName;
   wsfg::spaceg::OrbitalMissionData mData;
};

//! A Command to create a WsfOrbitalMissionSequence from the mission data, which is then returned to the GUI.
class CreateMissionCommand : public SimInterface::Command
{
public:
   CreateMissionCommand(const wsfg::spaceg::OrbitalMissionData& aMission, bool aVerify, SimInterface* aSimInterfacePtr)
      : Command{aSimInterfacePtr, true}
      , mMission{aMission}
      , mVerify{aVerify}
   {
   }

   ~CreateMissionCommand() override = default;

   void Process(WsfSimulation& aSimulation) override;

private:
   wsfg::spaceg::OrbitalMissionData mMission;
   bool                             mVerify;
};

} // namespace WkAstrolabe

#endif // ASTROLABESIMCOMMANDS_HPP
