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

#ifndef WKOBSERVER_HPP
#define WKOBSERVER_HPP

#include "warlock_core_export.h"

#include <string>

#include "UtCallback.hpp"
#include "WsfSimulation.hpp"
#include "WsfSimulationExtension.hpp"

namespace WkObserver
{
using SimulationHUD_DataCallback    = UtCallbackListN<void(double, WsfPlatform*, unsigned int)>;
using SimulationUserActionCallback  = UtCallbackListN<void(const std::string&, const WsfPlatform*)>;
using SimulationChatMessageCallback = UtCallbackListN<void(const std::string&, // user name
                                                           const std::string&, // channel
                                                           const std::string&, // text
                                                           bool)>;             // received

WARLOCK_CORE_EXPORT SimulationHUD_DataCallback&    SimulationHUD_Data(const WsfSimulation& aSimulationPtr);
WARLOCK_CORE_EXPORT SimulationUserActionCallback&  SimulationUserAction(const WsfSimulation& aSimulationPtr);
WARLOCK_CORE_EXPORT SimulationChatMessageCallback& SimulationChatMessage(const WsfSimulation& aSimulationPtr);
} // namespace WkObserver

class WARLOCK_CORE_EXPORT WkSimulationObserver : public WsfSimulationExtension
{
public:
   WkObserver::SimulationHUD_DataCallback SimulationHUD_Data;

   //! Notifies subscribers when a user takes an action in the simulation.
   //! @param aActionString The name of the action taken by the user.
   //! @param aPlatform The platform on which the user took action.
   WkObserver::SimulationUserActionCallback SimulationUserAction;

   WkObserver::SimulationChatMessageCallback SimulationChatMessage;
};

#endif
