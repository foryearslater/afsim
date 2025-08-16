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

#include "WsfPromptObserver.hpp"

#include "WsfSimulation.hpp"

// static
//! Given a simulation object, return its WsfPromptObserver.
//! @param aSimulationPtr A pointer to the simulation object owning the observer.
//! @return The WsfPromptObserver instance owned by aSimulationPtr.
//! @note This is a convenience method.
WsfPromptObserver& WsfPromptObserver::Get(const WsfSimulation* aSimulationPtr)
{
   return *(static_cast<WsfPromptObserver*>(aSimulationPtr->FindExtension("prompt_observer")));
}

// A call has been made to display a WsfPrompt to the user.
//! @param aSimulationPtr A pointer to the simulation object owning the observer.
//! @return The prompt displayed callback object owned by the simulation.
WsfObserver::PromptDisplayedCallback& WsfObserver::PromptDisplayed(const WsfSimulation* aSimulationPtr)
{
   return WsfPromptObserver::Get(aSimulationPtr).PromptDisplayed;
}
