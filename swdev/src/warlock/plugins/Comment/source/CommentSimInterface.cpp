// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2016 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#include "CommentSimInterface.hpp"

#include "UtMemory.hpp"
#include "WsfPlatform.hpp"
#include "WsfPlatformObserver.hpp"
#include "WsfSimulation.hpp"
#include "comment/WkfCommentForwardingService.hpp"


WkComment::SimInterface::SimInterface(const QString& aPluginName)
   : warlock::SimInterfaceT<CommentSimEvent>(aPluginName)
{
}

void WkComment::SimInterface::SimulationInitializing(const WsfSimulation& aSimulation)
{
   if (!IsEnabled())
   {
      return;
   }

   AddSimEvent(ut::make_unique<SimIntializedEvent>());

   QMutexLocker locker(&mMutex);

   wkf::CommentForwardingService::CommentClear();

   mCallbacks.Clear();

   mCallbacks.Add(WsfObserver::Comment(&aSimulation)
                     .Connect([this](double aSimTime, WsfPlatform* aPlatformPtr, const std::string& aMessage)
                              { AddSimEvent(ut::make_unique<AddEvent>(aSimTime, aPlatformPtr->GetName(), aMessage)); }));
}
