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
#ifndef PLATFORMBROWSERSIMINTERFACE_HPP
#define PLATFORMBROWSERSIMINTERFACE_HPP

#include "WkSimInterface.hpp"

namespace WkPlatformBrowser
{
class SimInterface : public warlock::SimInterfaceT<warlock::SimEvent>
{
public:
   SimInterface(const QString& aPluginName)
      : warlock::SimInterfaceT<warlock::SimEvent>(aPluginName)
   {
   }
};
} // namespace WkPlatformBrowser
#endif // PLATFORMBROWSERSIMINTERFACE_HPP
