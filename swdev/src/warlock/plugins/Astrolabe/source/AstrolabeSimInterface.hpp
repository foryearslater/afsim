// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2019 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef ASTROLABESIMINTERFACE_HPP
#define ASTROLABESIMINTERFACE_HPP

#include <memory>

#include "AstrolabeSimEvents.hpp"
#include "WkSimInterface.hpp"
#include "WsfScenario.hpp"

namespace WkAstrolabe
{

class SimInterface : public warlock::SimInterfaceT<AstrolabeEvent>
{
public:
   explicit SimInterface(const QString& aPluginName);
   ~SimInterface() override = default;

protected:
   void PlatformInitialized(double aSimTime, WsfPlatform& aPlatform) override;
   void PlatformDeleted(double aSimTime, const WsfPlatform& aPlatform) override;
};

} // namespace WkAstrolabe

#endif // ASTROLABESIMINTERFACE_HPP
