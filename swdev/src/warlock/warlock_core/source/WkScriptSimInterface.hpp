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
#ifndef WkScriptSimInterface_hpp
#define WkScriptSimInterface_hpp

#include "warlock_core_export.h"

#include <list>
#include <queue>
#include <sstream>

#include <QList>
#include <QString>

#include "UtCallbackHolder.hpp"
#include "UtLLAPos.hpp"
#include "UtScript.hpp"
#include "UtScriptClass.hpp"
#include "UtScriptData.hpp"
#include "UtScriptScope.hpp"
#include "WkScriptTypes.hpp"
#include "WkSimInterface.hpp"
#include "WsfGeoPoint.hpp"
#include "WsfPlatform.hpp"
#include "WsfTrack.hpp"
#include "xio/WsfXIO_PacketRegistry.hpp"

namespace warlock
{
class WARLOCK_CORE_EXPORT ScriptSimInterface : public warlock::SimInterfaceBase
{
   Q_OBJECT

public:
   ScriptSimInterface(const QString& aPluginName);

   // Issues a request for XIO script information on remote platforms,
   //  if the requests platform has not already notified us of the scripts the platform has.
   void RequestXIO_ScriptInfo(size_t aPlatformIndex);

   // Execute the script described by the ScriptInstance argument.
   // Returns an Id value, that is provided to the user when a script's return value is returned.
   int ExecuteScript(const script::Instance& aInstance);

   // Accessor to get the ScriptData from aScriptName in the global context.
   bool GetGlobalScriptData(const std::string& aScriptName, script::Data& aData) const;
   // Accessor to get the ScriptData from aScriptName in the specified platform's context.
   bool GetPlatformScriptData(const std::string& aPlatformName, const std::string& aScriptName, script::Data& aData) const;
   // Accessor to get all ScriptData in the global context
   void GetAllGlobalScriptData(script::DataMap& aData) const;
   // Accessor to get all ScriptData in the specified platform's context
   bool GetAllPlatformScriptData(const std::string& aName, script::DataMap& aData) const;

signals:
   void ReturnValueReady(int aScriptId, const script::ReturnValue& aReturnValue);
   void RemotePlatformUpdated(const QString& aPlatformName);

protected:
   void PlatformAdded(double aSimTime, WsfPlatform& aPlatform) override;
   void SimulationInitializing(const WsfSimulation& aSimulation) override;
   void SimulationStarting(const WsfSimulation& aSimulation) override;
   void SimulationComplete(const WsfSimulation& aSimulation) override;

   void Clear();

   void HandleScriptListPacket(WsfXIO_ScriptListPkt& aPkt);

   // These are not public because they do not lock the mutex,
   // therefore these are for use only within this class.
   bool GetPlatformScriptData(size_t aPlatformIndex, const std::string& aScriptName, warlock::script::Data& aScriptData) const;
   bool GetAllPlatformScriptData(size_t aPlatformIndex, script::DataMap& aData) const;

   // Callback holder to be able to process XIO Script Info packets.
   UtCallbackHolder mCallbacks;

   // ScriptDataMap for scripts that are in the global context
   script::DataMap mGlobalScripts;
   // Map to store name to index for platforms
   std::map<std::string, size_t> mPlatformNameToIdMap;
   // Map with platform index and the ScriptDataMap for the platform
   std::map<size_t, script::DataMap> mPlatformScripts;
   // Unique Id return to user when they execute a script, can be used to identify the script's return value
   int mUniqueScriptInstanceId;

private:
   // WARNING: Only use this mSimulation object on the simulation thread (not GUI).
   //  Use of this should be minimal, it is the work around for not receiving the
   //  simulation when processing XIO packets such as WsfXIO_ScriptListPkt.
   const WsfSimulation* mSimulation;
};

class WARLOCK_CORE_EXPORT ScriptExecuteCommand : public SimCommand
{
public:
   ScriptExecuteCommand(ScriptSimInterface* aSimInterface, const script::Instance& aInstance, int aId)
      : mInterface(aSimInterface)
      , mScript(aInstance)
      , mId(aId)
   {
   }

   void Process(WsfSimulation& aSimulation) override;

private:
   ScriptSimInterface* mInterface;
   script::Instance    mScript;
   int                 mId;
};

class WARLOCK_CORE_EXPORT ScriptInfoRequestCommand : public SimCommand
{
public:
   ScriptInfoRequestCommand(size_t aPlatformIndex)
      : mIndex(aPlatformIndex)
   {
   }

   void Process(WsfSimulation& aSimulation) override;

private:
   size_t mIndex;
};
} // namespace warlock

#endif
