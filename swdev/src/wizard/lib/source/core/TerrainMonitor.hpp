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

#ifndef TERRAINMONITOR_HPP
#define TERRAINMONITOR_HPP

#include "ProxyChange.hpp"
#include "UtCallbackHolder.hpp"
#include "VaCallbackHolder.hpp"
#include "WkfPlatform.hpp"
#include "WsfPM_Root.hpp"
#include "WsfPProxy.hpp"

namespace wizard
{
class TerrainMonitor
{
public:
   TerrainMonitor();
   ~TerrainMonitor() = default;

private:
   void OnProxyAvailable(WsfPProxy* aProxyPtr);
   void PlatformsAddedHandler(const wkf::PlatformList& aPlatformPtr);
   void PropertiesChangedHandler(wkf::Platform* aPlatformPtr);
   void TerrainChange(const ProxyChange& aProxyChange);
   void PopulateResources();

   UtCallbackHolder        mCallbacks;
   vespa::VaCallbackHolder mVaCallbacks;
   WsfPM_Root              mProxyRoot;
};
} // namespace wizard

#endif
