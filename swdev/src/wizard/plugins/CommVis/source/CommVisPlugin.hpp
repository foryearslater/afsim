// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2021 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef WIZCOMMVISPLUGIN_HPP
#define WIZCOMMVISPLUGIN_HPP

#include "CommVisDialog.hpp"
#include "Plugin.hpp"
#include "WkfPlugin.hpp"
#include "comm_vis/WkfCommVisDataContainer.hpp"

namespace WizCommVis
{
class Plugin : public wizard::Plugin
{
   Q_OBJECT

public:
   Plugin(const QString& aPluginName, const size_t aUniqueId);

   QList<wkf::Tip> GetTips() const override;

private:
   void Start();
   void OnProxyAvailable(WsfPProxy* aProxyPtr);

   void InitializeNetworks();
   void InitializeComms();

   void              HandleProxyChanged(const wizard::ProxyChange& aProxyChange);
   void              ReInitialize();
   const std::string ResolveCommNetworkName(const std::string& aNetworkAddress,
                                            const std::string& aPlatformName,
                                            const std::string& aCommName) const;
   const std::string GetCommNetworkType(const std::string& aNetworkName) const;

   ut::qt::UiPointer<WizCommVis::CommVisDialog> mDialog;
   wkf::DataContainer                           mCommData;
   WsfPM_Root                                   mProxyRoot;
   UtCallbackHolder                             mProxyCallbacks;
};
} // namespace WizCommVis

#endif
