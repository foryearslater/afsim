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

#include "WkObserver.hpp"

#define WK_OBSERVER_CALLBACK_DEFINE(EVENT)                                                        \
   WkObserver::EVENT##Callback& WkObserver::EVENT(const WsfSimulation& aSimulation)               \
   {                                                                                              \
      return static_cast<WkSimulationObserver*>(aSimulation.FindExtension("wk_observer"))->EVENT; \
   }

WK_OBSERVER_CALLBACK_DEFINE(SimulationHUD_Data)
WK_OBSERVER_CALLBACK_DEFINE(SimulationUserAction)
WK_OBSERVER_CALLBACK_DEFINE(SimulationChatMessage)
