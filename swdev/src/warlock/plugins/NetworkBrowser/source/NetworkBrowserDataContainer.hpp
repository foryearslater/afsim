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
#ifndef NetworkBrowserDataContainer_HPP
#define NetworkBrowserDataContainer_HPP

#include <map>
#include <string>
#include <vector>

#include <QObject>

#include "dis/WsfDisInput.hpp"
#include "xio/WsfXIO_InputData.hpp"

namespace WkNetworkBrowser
{
class DataContainer : public QObject
{
   Q_OBJECT
public:
   struct DisConnection
   {
      WsfDisDevice::DeviceType type             = WsfDisDevice::cDEVICE_NONE;
      std::string              ipAddress        = {};
      std::string              interfaceAddress = {};
      unsigned int             sendPort         = 0;
      unsigned int             rcvPort          = 0;
   };

   struct XioConnection
   {
      WsfXIO_InputData::UDP_Type type             = WsfXIO_InputData::cBROADCAST;
      std::string                ipAddress        = {};
      std::string                interfaceAddress = {};
      unsigned int               sendPort         = 0;
      unsigned int               rcvPort          = 0;
   };

   DataContainer() = default;

   unsigned short                      GetSiteId() const { return mSiteId; }
   unsigned short                      GetApplicationId() const { return mApplicationId; }
   unsigned short                      GetExerciseId() const { return mExerciseId; }
   const std::vector<DisConnection>&   GetDisConnections() const { return mDisConnections; }
   const std::map<int, XioConnection>& GetXioConnections() const { return mXioConnections; }

   void SetInfo(unsigned short                      aSiteId,
                unsigned short                      aAppId,
                unsigned short                      aExerciseId,
                const std::vector<DisConnection>&   aDisConnections,
                const std::map<int, XioConnection>& aXioConnections)
   {
      mSiteId         = aSiteId;
      mApplicationId  = aAppId;
      mExerciseId     = aExerciseId;
      mDisConnections = aDisConnections;
      mXioConnections = aXioConnections;
      emit InfoUpdated();
   }

signals:
   void InfoUpdated();

private:
   unsigned short               mSiteId         = {0};
   unsigned short               mApplicationId  = {0};
   unsigned short               mExerciseId     = {0};
   std::vector<DisConnection>   mDisConnections = {};
   std::map<int, XioConnection> mXioConnections = {};
};
} // namespace WkNetworkBrowser
#endif
