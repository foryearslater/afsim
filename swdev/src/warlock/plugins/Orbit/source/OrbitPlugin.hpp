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

#ifndef ORBITPLUGIN_HPP
#define ORBITPLUGIN_HPP

#include "OrbitSimInterface.hpp"
#include "WkPlugin.hpp"
#include "orbit/WkfOrbitPluginBase.hpp"

namespace WkOrbit
{

class Plugin : public warlock::PluginT<SimInterface, wkf::OrbitPluginBase>
{
public:
   explicit Plugin(const QString& aPluginName, const size_t& aUniqueId);
   ~Plugin() override = default;

   void SetPlatformOptionState(int aOptionId, bool aState, wkf::Platform* aPlatformPtr) override;

protected:
   void GuiUpdate() override;
};

} // namespace WkOrbit

#endif // ORBITPLUGIN_HPP
