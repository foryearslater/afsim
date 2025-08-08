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

#ifndef REGISTERUDP_OBSERVER_HPP
#define REGISTERUDP_OBSERVER_HPP

#include "UDP_Observer.hpp"

#include "WsfScenarioExtension.hpp"
#include "WsfSimulation.hpp"
#include "UtMemory.hpp"

class RegisterUDP_Observer : public WsfScenarioExtension
{
   public:

      ~RegisterUDP_Observer() noexcept override = default;

      void SimulationCreated(WsfSimulation& aSimulation) override
      {
         // EXERCISE 1 TASK 3
         // Call the simulation's RegisterExtension method.
         // Name the extension "udp_observer."
         // Provide a unique_ptr to a copy of the prototype UDP_Observer.
         // PLACE YOUR CODE HERE

      }
      bool ProcessInput(UtInput& aInput) override
      {
         // EXERCISE 1 TASK 4
         // Call the prototype UDP_Observer's ProcessInput method.
         // PLACE YOUR CODE HERE

      }

      UDP_Observer mPrototype;
};

#endif
