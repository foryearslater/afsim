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
#ifndef SIMCONTROLLERCOMMAND_HPP
#define SIMCONTROLLERCOMMAND_HPP

#include "WkSimInterface.hpp"
#include "xio/WsfXIO_PacketRegistry.hpp"

namespace WkSimController
{
class SimCommand : public warlock::SimCommand
{
public:
   SimCommand()
      : warlock::SimCommand(true)
   {
   } // Execute these Commands on WallClock timer

protected:
   static void SendXIO_Command(WsfSimulation& aSimulation, WsfXIO_SimTimeCommandPkt::CommandType aType, double aValue = 0.0);
};

class PauseCommand : public SimCommand
{
public:
   PauseCommand(bool aSendDIS)
      : SimCommand()
      , mSendDIS(aSendDIS)
   {
   }

   void Process(WsfSimulation& aSimulation) override;

private:
   bool mSendDIS;
};

class ResumeCommand : public SimCommand
{
public:
   ResumeCommand(bool aSendDIS)
      : SimCommand()
      , mSendDIS(aSendDIS)
   {
   }

   void Process(WsfSimulation& aSimulation) override;

private:
   bool mSendDIS;
};

class TerminateCommand : public SimCommand
{
public:
   void Process(WsfSimulation& aSimulation) override;
};

class ResetCommand : public SimCommand
{
public:
   void Process(WsfSimulation& aSimulation) override;
};

class SetClockRateCommand : public SimCommand
{
public:
   SetClockRateCommand(double aClockRate)
      : SimCommand()
      , mClockRate(aClockRate)
   {
   }

   void Process(WsfSimulation& aSimulation) override;

private:
   double mClockRate;
};

class AdvanceToTimeCommand : public SimCommand
{
public:
   AdvanceToTimeCommand(double aSimTime)
      : SimCommand()
      , mSimTime(aSimTime)
   {
   }

   void Process(WsfSimulation& aSimulation) override;

private:
   double mSimTime;
};

class UpdateXioThresholdsCommand : public SimCommand
{
public:
   explicit UpdateXioThresholdsCommand(double aFallingBehindThreshold) noexcept
      : mFallingBehindThreshold(aFallingBehindThreshold)
   {
   }

   void Process(WsfSimulation& aSimulation) override;

private:
   double mFallingBehindThreshold;
};
} // namespace WkSimController

#endif
