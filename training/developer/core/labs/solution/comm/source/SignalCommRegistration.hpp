// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2003-2013 The Boeing Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
// ****************************************************************************
// Updated by Infoscitex, a DCS Company.
// ****************************************************************************

#include "Interface.hpp"
#include "LocationMessage.hpp"
#include "SignalComm.hpp"

class     UtInput;
#include "UtPlugin.hpp"
#include "UtScriptTypes.hpp"
#include "WsfApplication.hpp"
#include "WsfApplicationExtension.hpp"
#include "WsfCommTypes.hpp"
#include "WsfPlugin.hpp"
#include "WsfScenario.hpp"
#include "WsfScenarioExtension.hpp"
#include "WsfSimulation.hpp"
#include "UtMemory.hpp"

class SignalCommRegistration : public WsfScenarioExtension
{
   public:
      SignalCommRegistration() = default;
      bool ProcessInput(UtInput& aInput) override
      {
         // EXERCISE 2 TASK 1
         //! Call the CommLab::Interface prototype's ProcessInput method.
         return mPrototypeInterface.ProcessInput(aInput);
      }
      void AddedToScenario() override
      {
         // Add the new comm type.
         auto signalCommPtr = ut::make_unique<wsf::comm::SignalComm>(GetScenario());
         GetScenario().GetCommTypes().Add(wsf::comm::SignalComm::GetSignalCommClassId(), std::move(signalCommPtr));

         // Add the script classes to the script manager
         GetScenario().GetApplication().GetScriptTypes()->Register(ut::make_unique<ScriptLocationMessageClass>("LocationMessage", GetScenario().GetApplication().GetScriptTypes()));
      }
      void SimulationCreated(WsfSimulation& aSim) override
      {
         // create a new CommLab::Interface simulation extension
         auto InterfacePtr = ut::make_unique<CommLab::Interface>();

         // use the prototype to initialize member variables in the new Interface
         // have to copy members, since ProcessInput was already called on mPrototypeInterface
         // only have to copy two members, since the others are not yet changed from default
         InterfacePtr->SetDebugEnabled(mPrototypeInterface.GetDebugEnabled());
         InterfacePtr->SetPrintMessages(mPrototypeInterface.GetPrintMessages());

         // register the new extension
         aSim.RegisterExtension("comm_lab_interface", std::move(InterfacePtr));
      }
   private:
      CommLab::Interface mPrototypeInterface;
};
