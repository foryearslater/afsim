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
#ifndef COMMENTSIMINTERFACE_HPP
#define COMMENTSIMINTERFACE_HPP

#include "CommentSimEvents.hpp"
#include "UtCallbackHolder.hpp"
#include "WkSimInterface.hpp"

namespace WkComment
{
class SimInterface : public warlock::SimInterfaceT<CommentSimEvent>
{
public:
   SimInterface(const QString& aPluginName);
   ~SimInterface() override = default;

private:
   void SimulationInitializing(const WsfSimulation& aSimulation) override;

   UtCallbackHolder mCallbacks;
};
} // namespace WkComment

#endif
