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

#ifndef WKENVIRONMENT_HPP
#define WKENVIRONMENT_HPP

#include "warlock_core_export.h"

#include <memory>

#include <QObject>

#if defined(wkEnv)
#undef wkEnv
#endif
#define wkEnv warlock::AppEnvironment::GetInstance()

namespace warlock
{
namespace net
{
class Network;
}

class WARLOCK_CORE_EXPORT AppEnvironment : public QObject
{
   Q_OBJECT

public:
   static void            Create();
   static AppEnvironment& GetInstance();
   static bool            Exists();
   static void            Shutdown();

   warlock::net::Network& GetNetwork() const;

protected:
   AppEnvironment();
   ~AppEnvironment() override;

   static AppEnvironment* mInstancePtr;

   std::unique_ptr<warlock::net::Network> mNetworkPtr;
};
} // namespace warlock
#endif
