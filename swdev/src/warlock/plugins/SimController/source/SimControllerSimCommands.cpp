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

#include "SimControllerSimCommands.hpp"

#include "DisControlEnums.hpp"
#include "UtMath.hpp"
#include "UtMemory.hpp"
#include "UtTime.hpp"
#include "WkObserver.hpp"
#include "WsfClockSource.hpp"
#include "WsfDateTime.hpp"
#include "WsfSimulation.hpp"
#include "dis/WsfDisInterface.hpp"
#include "dis/WsfDisStartResume.hpp"
#include "dis/WsfDisStopFreeze.hpp"
#include "xio/WsfXIO_Connection.hpp"
#include "xio/WsfXIO_Interface.hpp"
#include "xio_sim/WsfXIO_Extension.hpp"

void WkSimController::SimCommand::SendXIO_Command(WsfSimulation&                        aSimulation,
                                                  WsfXIO_SimTimeCommandPkt::CommandType aType,
                                                  double                                aValue /*= 0.0*/)
{
   WsfXIO_Interface* xio = WsfXIO_Extension::Find(aSimulation);
   if (xio)
   {
      WsfXIO_SimTimeCommandPkt pkt;
      pkt.mCommandType = aType;
      pkt.mSimTime     = aValue;

      for (auto connection : xio->GetReliableConnections())
      {
         connection->Send(pkt);
      }
   }
}

void WkSimController::PauseCommand::Process(WsfSimulation& aSimulation)
{
   if (mSendDIS)
   {
      WsfDisInterface* dis = WsfDisInterface::Find(aSimulation);
      if (dis != nullptr)
      {
         auto pdu = ut::make_unique<WsfDisStopFreeze>(dis);
         pdu->SetReason(DisEnum::Control::Reason::RECESS);
         dis->PutPdu(aSimulation.GetSimTime(), std::move(pdu));
      }
   }
   SendXIO_Command(aSimulation, WsfXIO_SimTimeCommandPkt::cPAUSE);

   WkObserver::SimulationUserAction(aSimulation)("Simulation Paused", nullptr);

   aSimulation.Pause();
}

void WkSimController::ResumeCommand::Process(WsfSimulation& aSimulation)
{
   if (mSendDIS)
   {
      WsfDisInterface* dis = WsfDisInterface::Find(aSimulation);
      if (dis != nullptr)
      {
         dis->PutPdu(aSimulation.GetSimTime(), ut::make_unique<WsfDisStartResume>(dis));
      }
   }
   SendXIO_Command(aSimulation, WsfXIO_SimTimeCommandPkt::cRESUME);

   WkObserver::SimulationUserAction(aSimulation)("Simulation Resumed", nullptr);

   aSimulation.Resume();
}

void WkSimController::TerminateCommand::Process(WsfSimulation& aSimulation)
{
   WkObserver::SimulationUserAction(aSimulation)("Simulation Terminated", nullptr);
   aSimulation.RequestTermination();
}

void WkSimController::ResetCommand::Process(WsfSimulation& aSimulation)
{
   // Currently XIO does not support this command,
   // so only the current simulation will reset, not all connected simulations

   WkObserver::SimulationUserAction(aSimulation)("Simulation Reset", nullptr);
   aSimulation.RequestReset();
}

void WkSimController::SetClockRateCommand::Process(WsfSimulation& aSimulation)
{
   SendXIO_Command(aSimulation, WsfXIO_SimTimeCommandPkt::cSET_CLOCK_RATE, mClockRate);

   WkObserver::SimulationUserAction(aSimulation)("Simulation Clock Rate Changed", nullptr);

   aSimulation.SetClockRate(mClockRate);
}

void WkSimController::AdvanceToTimeCommand::Process(WsfSimulation& aSimulation)
{
   // Only support advancing to times in the future.
   //  We do not want to set the simulation clock to a time that has already past. AFSIM does not support this.
   if (mSimTime > aSimulation.GetSimTime())
   {
      // The XIO advance time command, advances the clock by X seconds instead of advancing until time Y like the Sim
      // Controller does.
      //  Therefor compute the difference in the time and pass that value.
      SendXIO_Command(aSimulation, WsfXIO_SimTimeCommandPkt::cADVANCE_TIME, mSimTime - aSimulation.GetSimTime());

      // For lambda capture
      bool   pause   = aSimulation.GetClockSource()->IsStopped();
      double simTime = mSimTime;

      aSimulation.AddEvent(ut::make_unique<WsfOneShotEvent>(mSimTime,
                                                            [=, &aSimulation]()
                                                            {
                                                               aSimulation.SetRealtime(simTime, true);
                                                               // Need to call resume due to the real-time clock being
                                                               // deleted and re-created, and the new clock potentially
                                                               // expecting a Resume() command.
                                                               pause ? aSimulation.Pause() : aSimulation.Resume();
                                                            }));

      WkObserver::SimulationUserAction(
         aSimulation)("Simulation Time Advanced to t = " + UtTime::ToString(mSimTime, UtTime::FmtHMS), nullptr);

      aSimulation.SetRealtime(aSimulation.GetSimTime(), false);
   }
}

void WkSimController::UpdateXioThresholdsCommand::Process(WsfSimulation& aSimulation)
{
   WsfXIO_Interface* xio = WsfXIO_Extension::Find(aSimulation);
   if (xio)
   {
      xio->mFallingBehindThreshold = mFallingBehindThreshold;
   }
}
