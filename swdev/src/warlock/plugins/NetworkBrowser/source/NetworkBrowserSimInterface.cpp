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
#include "NetworkBrowserSimInterface.hpp"

#include "Dis.hpp"
#include "NetworkBrowserDataContainer.hpp"
#include "UtCast.hpp"
#include "UtMemory.hpp"
#include "WsfSimulation.hpp"
#include "dis/WsfDisInterface.hpp"
#include "ext/WsfExtInterface.hpp"
#include "xio/WsfXIO_Connection.hpp"
#include "xio/WsfXIO_Interface.hpp"
#include "xio_sim/WsfXIO_Extension.hpp"

WkNetworkBrowser::SimInterface::SimInterface(const QString& aPluginName)
   : warlock::SimInterfaceT<NetworkEvent>(aPluginName)
{
}

void WkNetworkBrowser::SimInterface::SimulationStarting(const WsfSimulation& aSimulation)
{
   unsigned short                              app      = 0;
   unsigned short                              site     = 0;
   unsigned short                              exercise = 0;
   std::vector<DataContainer::DisConnection>   disConnections;
   std::map<int, DataContainer::XioConnection> xioConnections;

   WsfDisInterface* disInterface = WsfDisInterface::Find(aSimulation);
   if (disInterface != nullptr)
   {
      WsfExtInterface* extInterface = disInterface->GetExtInterface();
      if (extInterface != nullptr)
      {
         app      = extInterface->mApplicationId;
         site     = extInterface->mSiteId;
         exercise = Dis::GetExercise();
      }

      std::vector<WsfDisInput::DeviceConfiguration>& configs = disInterface->GetDeviceConfigurations();
      for (const auto& config : configs)
      {
         if (config.mDeviceType != WsfDisDevice::cDEVICE_NONE)
         {
            DataContainer::DisConnection temp;
            temp.ipAddress        = config.mAddress;
            temp.interfaceAddress = config.mInterfaceAddress;
            temp.sendPort         = ut::safe_cast<unsigned int, int>(config.mSendPort);
            temp.rcvPort          = ut::safe_cast<unsigned int, int>(config.mRecvPort);
            temp.type             = config.mDeviceType;
            disConnections.push_back(temp);
         }
      }
   }

   WsfXIO_Interface* xioInterface = WsfXIO_Extension::Find(aSimulation);
   if (xioInterface != nullptr)
   {
      for (const auto& target : xioInterface->mUDP_Targets)
      {
         DataContainer::XioConnection temp;
         temp.ipAddress                       = target.mAddress;
         temp.interfaceAddress                = target.mInterface;
         temp.rcvPort                         = ut::safe_cast<unsigned int, int>(target.mRecvPort);
         temp.sendPort                        = ut::safe_cast<unsigned int, int>(target.mSendPort);
         temp.type                            = target.mType;
         xioConnections[target.mConnectionId] = temp;
      }
   }
   AddSimEvent(ut::make_unique<InitialEvent>(site, app, exercise, disConnections, xioConnections));
}
