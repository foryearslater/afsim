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

#ifndef PLATFORMBROWSERINTERFACE_HPP
#define PLATFORMBROWSERINTERFACE_HPP

#include "PlatformBrowserSimInterface.hpp"
#include "VaCallbackHolder.hpp"
#include "WkfGeneralPrefObject.hpp"
#include "WkfPlatform.hpp"
#include "WkfScenario.hpp"
#include "platform_browser/WkfPlatformBrowserInterface.hpp"

namespace WkPlatformBrowser
{
class Interface : public wkf::PlatformBrowserInterface
{
public:
   Interface(SimInterface* aSimInterface);
   ~Interface() override = default;

   void Update();

   void DeletePlatform(const QString& aPlatformName);
   void DeleteCurrentlySelectedPlatforms();

private:
   void ScenarioRemovedCB(wkf::Scenario* aScenarioPtr);

   vespa::VaCallbackHolder mCallbacks;
   SimInterface*           mSimInterface;
};
} // namespace WkPlatformBrowser

#endif
