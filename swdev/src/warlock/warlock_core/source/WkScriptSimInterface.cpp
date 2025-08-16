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


#include "WkScriptSimInterface.hpp"

#include "UtMemory.hpp"
#include "UtScriptContext.hpp"
#include "UtScriptFunction.hpp"
#include "UtScriptTypes.hpp"
#include "WkObserver.hpp"
#include "WkSimEnvironment.hpp"
#include "WkXIO_DataContainer.hpp"
#include "WkfQueueableMessageObject.hpp"
#include "WkfTrack.hpp"
#include "WsfComm.hpp"
#include "WsfScenario.hpp"
#include "WsfSensor.hpp"
#include "WsfSimulation.hpp"
#include "WsfTrackList.hpp"
#include "WsfWeapon.hpp"
#include "dis/WsfDisInterface.hpp"
#include "dis/WsfDisPlatform.hpp"
#include "script/WsfScriptContext.hpp"
#include "xio/WsfXIO_Interface.hpp"
#include "xio_sim/WsfXIO_Extension.hpp"
#include "xio_sim/WsfXIO_SimPacketRegistry.hpp"

namespace
{
void GetScriptsFromScope(UtScriptScope* aScope, warlock::script::DataMap& aData)
{
   if (aScope)
   {
      const UtScriptScope::ScriptMap& scripts = aScope->GetScripts();

      for (const auto& iter : scripts)
      {
         UtScript* script = iter.second.mScriptPtr;
         if (script)
         {
            warlock::script::Data data;
            data.name = script->GetName();
            for (size_t i = 0; i < script->GetArgumentCount(); ++i)
            {
               data.argNames.push_back(script->GetArgumentName(i));
               // I think this is the correct location to get Type from.
               data.argTypes.push_back(script->mPrototype->mArgs.at(i).GetString());
            }
            data.returnType = script->GetReturnType();

            aData[data.name] = data;
         }
      }
   }
}

template<typename PlatformPart>
static bool PlatformPartScriptData(UtScriptDataList&                aScriptRefs,
                                   const warlock::script::Argument& aArgument,
                                   const warlock::script::Instance& aScript,
                                   const WsfSimulation&             aSimulation,
                                   const bool                       aScriptLocal)
{
   static_assert(std::is_same<PlatformPart, wsf::comm::Comm>::value || std::is_same<PlatformPart, WsfSensor>::value ||
                    std::is_same<PlatformPart, WsfWeapon>::value,
                 "Must be WsfSensor, WsfWeapon, or wsf::comm::Comm");

   if (aScript.script.context == warlock::script::GLOBAL)
   {
      // ERROR: no platform context
      wkf::QueueableMessageObject::DisplayQueuedMessage(QMessageBox::Warning,
                                                        "Script Error",
                                                        "No platform context for script " +
                                                           QString::fromStdString(aScript.script.name));
      return false;
   }

   auto* contextPlatform = aSimulation.GetPlatformByName(aScript.platformName);
   if (contextPlatform == nullptr)
   {
      // ERROR: can't find context platform
      wkf::QueueableMessageObject::DisplayQueuedMessage(
         QMessageBox::Warning,
         "Script Error",
         QString("Platform (%1) does not exist").arg(QString::fromStdString(aScript.platformName)));
      return false;
   }

   if (aArgument.type == "WsfSensor" || aArgument.type == "WsfWeapon" || aArgument.type == "WsfComm")
   {
      auto partName = aArgument.data.toString().toStdString();

      if (aScriptLocal)
      {
         auto* part = dynamic_cast<PlatformPart*>(contextPlatform->GetComponents().FindComponent(partName));
         aScriptRefs.emplace_back(
            new UtScriptRef(part, aSimulation.GetScenario().GetScriptTypes()->GetClass(aArgument.type)));
      }
      else
      {
         aScriptRefs.emplace_back(partName);
      }
   }
   else if (aArgument.type == "Array<WsfSensor>" || aArgument.type == "Array<WsfWeapon>" ||
            aArgument.type == "Array<WsfComm>")
   {
      UtScriptDataList* arrayPtr = new UtScriptDataList;
      std::string       baseType;

      if (aArgument.type == "Array<WsfSensor>")
      {
         baseType = "WsfSensor";
      }
      else if (aArgument.type == "Array<WsfWeapon>")
      {
         baseType = "WsfWeapon";
      }
      else if (aArgument.type == "Array<WsfComm>")
      {
         baseType = "WsfComm";
      }
      else
      {
         // ERROR
      }

      for (const auto& partName : aArgument.data.value<QVector<QVariant>>())
      {
         auto* part =
            dynamic_cast<PlatformPart*>(contextPlatform->GetComponents().FindComponent(partName.toString().toStdString()));
         if (part == nullptr)
         {
            // ERROR: part not on this platform
            wkf::QueueableMessageObject::DisplayQueuedMessage(QMessageBox::Warning,
                                                              "Script Error",
                                                              QString("Part (%1) does not exist on platform (%2)")
                                                                 .arg(partName.toString(),
                                                                      QString::fromStdString(aScript.platformName)));
         }

         if (aScriptLocal)
         {
            arrayPtr->emplace_back(new UtScriptRef(part, aSimulation.GetScenario().GetScriptTypes()->GetClass(baseType)));
         }
         else
         {
            arrayPtr->emplace_back(part->GetName());
         }
      }

      aScriptRefs.emplace_back(
         new UtScriptRef(arrayPtr, aSimulation.GetScenario().GetScriptTypes()->GetClass(aArgument.type)));
   }
   else
   {
      // ERROR: somehow an invalid type made it in
      wkf::QueueableMessageObject::DisplayQueuedMessage(
         QMessageBox::Warning,
         "Script Error",
         QString("Part (%1) is of invalid type (%2)").arg(aArgument.data.toString(), QString::fromStdString(aArgument.type)));
      return false;
   }

   return true;
}

UtScriptDataList GetUtScriptDataList(const warlock::script::Instance& aScript,
                                     const WsfSimulation&             aSimulation,
                                     bool                             aScriptLocal,
                                     bool&                            aOk)
{
   aOk = true;

   UtScriptDataList retVal;
   for (const auto& iter : aScript.args)
   {
      if (iter.type == "int")
      {
         retVal.emplace_back(static_cast<int>(iter.data.toDouble()));
      }
      else if (iter.type == "double")
      {
         retVal.emplace_back(iter.data.toDouble());
      }
      else if (iter.type == "bool")
      {
         retVal.emplace_back(iter.data.toBool());
      }
      else if (iter.type == "string")
      {
         retVal.emplace_back(iter.data.toString().toStdString());
      }
      else if (iter.type == "WsfGeoPoint")
      {
         QVector<double> vec = iter.data.value<QVector<double>>();
         if (vec.size() == 3) // vector of lat, lon, alt
         {
            auto* geoPoint = new WsfGeoPoint(vec[0], vec[1], vec[2]);
            retVal.emplace_back(new UtScriptRef(geoPoint,
                                                aSimulation.GetScenario().GetScriptTypes()->GetClass("WsfGeoPoint"),
                                                UtScriptRef::MemManagement::cMANAGE));
         }
         else
         {
            wkf::QueueableMessageObject::DisplayQueuedMessage(QMessageBox::Warning,
                                                              "Script Error",
                                                              "Invalid GeoPoint format.");
            aOk = false;
         }
      }
      else if (iter.type == "WsfPlatform")
      {
         auto* platform = aSimulation.GetPlatformByName(iter.data.toString().toStdString());
         if (!platform)
         {
            wkf::QueueableMessageObject::DisplayQueuedMessage(QMessageBox::Warning,
                                                              "Script Error",
                                                              "Platform (" + iter.data.toString() + ") not found.");
            aOk = false;
         }
         else if (!aScriptLocal)
         {
            retVal.emplace_back(platform->GetName());
         }
         else
         {
            retVal.emplace_back(
               new UtScriptRef(platform, aSimulation.GetScenario().GetScriptTypes()->GetClass("WsfPlatform")));
         }
      }
      else if (iter.type == "WsfSensor" || iter.type == "Array<WsfSensor>")
      {
         aOk &= PlatformPartScriptData<WsfSensor>(retVal, iter, aScript, aSimulation, aScriptLocal);
      }
      else if (iter.type == "WsfWeapon" || iter.type == "Array<WsfWeapon>")
      {
         aOk &= PlatformPartScriptData<WsfWeapon>(retVal, iter, aScript, aSimulation, aScriptLocal);
      }
      else if (iter.type == "WsfComm" || iter.type == "Array<WsfComm>")
      {
         aOk &= PlatformPartScriptData<wsf::comm::Comm>(retVal, iter, aScript, aSimulation, aScriptLocal);
      }
      else if (iter.type == "WsfTrack")
      {
         auto               trackId      = iter.data.value<wkf::TrackId>();
         const std::string& platformName = trackId.GetPlatformName();

         auto* platform = aSimulation.GetPlatformByName(platformName);
         if (platform)
         {
            auto* track = platform->GetMasterTrackList().FindTrack(WsfTrackId(platformName, trackId.GetLocalTrackId()));
            if (track)
            {
               retVal.emplace_back(
                  new UtScriptRef(track, aSimulation.GetScenario().GetScriptTypes()->GetClass("WsfTrack")));
            }
            else
            {
               wkf::QueueableMessageObject::DisplayQueuedMessage(QMessageBox::Warning,
                                                                 "Script Error",
                                                                 "Track (" + trackId + ") not found.");
               aOk = false;
            }
         }
         else
         {
            wkf::QueueableMessageObject::DisplayQueuedMessage(QMessageBox::Warning,
                                                              "Script Error",
                                                              QString("Platform (%1) not found.").arg(platformName.c_str()));
            aOk = false;
         }
      }
      else if (iter.type == "Array<int>")
      {
         UtScriptDataList* arrayPtr = new UtScriptDataList;
         for (const auto& i : iter.data.value<QVector<QVariant>>())
         {
            arrayPtr->emplace_back(static_cast<int>(i.toDouble()));
         }
         retVal.emplace_back(new UtScriptRef(arrayPtr, aSimulation.GetScenario().GetScriptTypes()->GetClass("Array<int>")));
      }
      else if (iter.type == "Array<double>")
      {
         UtScriptDataList* arrayPtr = new UtScriptDataList;
         for (const auto& i : iter.data.value<QVector<QVariant>>())
         {
            arrayPtr->emplace_back(i.toDouble());
         }
         retVal.emplace_back(
            new UtScriptRef(arrayPtr, aSimulation.GetScenario().GetScriptTypes()->GetClass("Array<double>")));
      }
      else if (iter.type == "Array<string>")
      {
         UtScriptDataList* arrayPtr = new UtScriptDataList;
         for (const auto& i : iter.data.value<QVector<QVariant>>())
         {
            arrayPtr->push_back(UtScriptData(i.toString().toStdString()));
         }
         retVal.emplace_back(
            new UtScriptRef(arrayPtr, aSimulation.GetScenario().GetScriptTypes()->GetClass("Array<string>")));
      }
      else if (iter.type == "Array<bool>")
      {
         UtScriptDataList* arrayPtr = new UtScriptDataList;
         for (const auto& i : iter.data.value<QVector<QVariant>>())
         {
            arrayPtr->push_back(UtScriptData(i.toBool()));
         }
         retVal.emplace_back(
            new UtScriptRef(arrayPtr, aSimulation.GetScenario().GetScriptTypes()->GetClass("Array<bool>")));
      }
      else if (iter.type == "Array<WsfGeoPoint>")
      {
         UtScriptDataList* arrayPtr = new UtScriptDataList;
         for (const auto& point : iter.data.value<QVector<QVariant>>())
         {
            std::stringstream lla;
            UtLLAPos          llaPos;
            lla << point.toString().toStdString();
            lla >> llaPos;

            if (!lla.fail()) // if LLA is valid
            {
               auto* geoPoint = new WsfGeoPoint(llaPos.GetLat(), llaPos.GetLon(), llaPos.GetAlt());
               arrayPtr->push_back(new UtScriptRef(geoPoint,
                                                   aSimulation.GetScenario().GetScriptTypes()->GetClass("WsfGeoPoint"),
                                                   UtScriptRef::MemManagement::cMANAGE));
            }
            else
            {
               wkf::QueueableMessageObject::DisplayQueuedMessage(QMessageBox::Warning,
                                                                 "Script Error",
                                                                 "Invalid GeoPoint format.");
               aOk = false;
               break;
            }
         }
         retVal.emplace_back(
            new UtScriptRef(arrayPtr, aSimulation.GetScenario().GetScriptTypes()->GetClass("Array<WsfGeoPoint>")));
      }
      else if (iter.type == "Array<WsfPlatform>")
      {
         UtScriptDataList* arrayPtr = new UtScriptDataList;
         for (const auto& platformName : iter.data.value<QVector<QVariant>>())
         {
            auto* platform = aSimulation.GetPlatformByName(platformName.toString().toStdString());
            if (!platform)
            {
               wkf::QueueableMessageObject::DisplayQueuedMessage(QMessageBox::Warning,
                                                                 "Script Error",
                                                                 "Platform (" + platformName.toString() + ") not found.");
               aOk = false;
            }
            else if (!aScriptLocal)
            {
               arrayPtr->emplace_back(platform->GetName());
            }
            else
            {
               arrayPtr->push_back(
                  new UtScriptRef(platform, aSimulation.GetScenario().GetScriptTypes()->GetClass("WsfPlatform")));
            }
         }
         retVal.emplace_back(
            new UtScriptRef(arrayPtr, aSimulation.GetScenario().GetScriptTypes()->GetClass("Array<WsfPlatform>")));
      }
      else if (iter.type == "Array<WsfTrack>")
      {
         UtScriptDataList* arrayPtr = new UtScriptDataList;
         for (const auto& item : iter.data.value<QVector<QVariant>>())
         {
            auto               trackId      = item.value<wkf::TrackId>();
            const std::string& platformName = trackId.GetPlatformName();

            auto* platform = aSimulation.GetPlatformByName(platformName);
            if (platform)
            {
               auto* track = platform->GetMasterTrackList().FindTrack(WsfTrackId(platformName, trackId.GetLocalTrackId()));
               if (track)
               {
                  arrayPtr->push_back(
                     new UtScriptRef(track, aSimulation.GetScenario().GetScriptTypes()->GetClass("WsfTrack")));
               }
               else
               {
                  wkf::QueueableMessageObject::DisplayQueuedMessage(QMessageBox::Warning,
                                                                    "Script Error",
                                                                    "Track (" + trackId + ") not found.");
                  aOk = false;
               }
            }
            else
            {
               wkf::QueueableMessageObject::DisplayQueuedMessage(
                  QMessageBox::Warning,
                  "Script Error",
                  QString("Platform (%1) not found.").arg(platformName.c_str()));
               aOk = false;
            }
         }
         retVal.emplace_back(
            new UtScriptRef(arrayPtr, aSimulation.GetScenario().GetScriptTypes()->GetClass("Array<WsfTrack>")));
      }
      else
      {
         wkf::QueueableMessageObject::DisplayQueuedMessage(QMessageBox::Warning,
                                                           "Script Error",
                                                           "Unknown argument Type " + QString::fromStdString(iter.type));
         aOk = false;
      }
   }
   return retVal;
}
} // namespace

warlock::ScriptSimInterface::ScriptSimInterface(const QString& aPluginName)
   : warlock::SimInterfaceBase(aPluginName)
   , mUniqueScriptInstanceId(1)
   , mSimulation(nullptr)
{
   qRegisterMetaType<warlock::script::ReturnValue>();
   QMetaType::registerConverter<wkf::TrackId, QString>(); // register implicit conversion (using cast operator)
}

void warlock::ScriptSimInterface::RequestXIO_ScriptInfo(size_t aPlatformIndex)
{
   mMutex.lock();
   bool add = (mPlatformScripts.count(aPlatformIndex) == 0);
   mMutex.unlock();

   if (add)
   {
      AddSimCommand(ut::make_unique<ScriptInfoRequestCommand>(aPlatformIndex));
   }
}

int warlock::ScriptSimInterface::ExecuteScript(const script::Instance& aInstance)
{
   AddSimCommand(ut::make_unique<ScriptExecuteCommand>(this, aInstance, mUniqueScriptInstanceId));
   return (mUniqueScriptInstanceId++);
}

bool warlock::ScriptSimInterface::GetGlobalScriptData(const std::string& aScriptName, warlock::script::Data& aData) const
{
   QMutexLocker locker(&mMutex);
   auto         iter = mGlobalScripts.find(aScriptName);
   if (iter != mGlobalScripts.end())
   {
      aData = iter->second;
      return true;
   }
   return false;
}

bool warlock::ScriptSimInterface::GetPlatformScriptData(const std::string&     aPlatformName,
                                                        const std::string&     aScriptName,
                                                        warlock::script::Data& aData) const
{
   QMutexLocker locker(&mMutex);
   // Get Platform Id based on Platform Name
   auto idIter = mPlatformNameToIdMap.find(aPlatformName);
   if (idIter != mPlatformNameToIdMap.end())
   {
      return GetPlatformScriptData(idIter->second, aScriptName, aData);
   }
   return false;
}

bool warlock::ScriptSimInterface::GetPlatformScriptData(size_t                 aPlatformIndex,
                                                        const std::string&     aScriptName,
                                                        warlock::script::Data& aScriptData) const
{
   // If any scripts are present for the specified platform
   auto platformIter = mPlatformScripts.find(aPlatformIndex);
   if (platformIter != mPlatformScripts.end())
   {
      auto scriptIter = platformIter->second.find(aScriptName);
      if (scriptIter != platformIter->second.end())
      {
         aScriptData = scriptIter->second;
         return true;
      }
   }
   return false;
}

void warlock::ScriptSimInterface::GetAllGlobalScriptData(script::DataMap& aData) const
{
   QMutexLocker locker(&mMutex);
   aData = mGlobalScripts;
}

bool warlock::ScriptSimInterface::GetAllPlatformScriptData(const std::string& aName, script::DataMap& aData) const
{
   auto idIter = mPlatformNameToIdMap.find(aName);
   if (idIter != mPlatformNameToIdMap.end())
   {
      return GetAllPlatformScriptData(idIter->second, aData);
   }
   return false;
}

bool warlock::ScriptSimInterface::GetAllPlatformScriptData(size_t aPlatformIndex, script::DataMap& aData) const
{
   auto platformIter = mPlatformScripts.find(aPlatformIndex);
   if (platformIter != mPlatformScripts.end())
   {
      aData = platformIter->second;
      return true;
   }
   return false;
}

void warlock::ScriptSimInterface::PlatformAdded(double aSimTime, WsfPlatform& aPlatform)
{
   QMutexLocker locker(&mMutex);

   mPlatformNameToIdMap[aPlatform.GetName()] = aPlatform.GetIndex();

   UtScriptScope* scope = aPlatform.GetScriptContext().GetContext().GetScope();
   if (scope)
   {
      script::DataMap temp;
      GetScriptsFromScope(scope, temp);

      for (auto& i : temp)
      {
         i.second.context = script::PLATFORM;
      }

      if (!temp.empty())
      {
         mPlatformScripts[aPlatform.GetIndex()] = temp;
      }
   }
}

void warlock::ScriptSimInterface::SimulationInitializing(const WsfSimulation& aSimulation)
{
   Clear();
}

void warlock::ScriptSimInterface::SimulationStarting(const WsfSimulation& aSimulation)
{
   QMutexLocker locker(&mMutex);
   GetScriptsFromScope(aSimulation.GetScriptContext().GetContext().GetScope(), mGlobalScripts);
   for (auto& i : mGlobalScripts)
   {
      i.second.context = script::GLOBAL;
   }

   WsfXIO_Interface* xio = WsfXIO_Extension::Find(aSimulation);
   if (xio != nullptr)
   {
      mCallbacks += xio->Connect(&ScriptSimInterface::HandleScriptListPacket, this);
   }

   mSimulation = &aSimulation;
}

void warlock::ScriptSimInterface::SimulationComplete(const WsfSimulation& aSimulation)
{
   mSimulation = nullptr;
   mCallbacks.Clear();
}

void warlock::ScriptSimInterface::Clear()
{
   QMutexLocker locker(&mMutex);
   mCallbacks.Clear();
   mGlobalScripts.clear();
   mPlatformNameToIdMap.clear();
   mPlatformScripts.clear();
}

void warlock::ScriptSimInterface::HandleScriptListPacket(WsfXIO_ScriptListPkt& aPkt)
{
   auto* senderPtr = dynamic_cast<WsfXIO_Connection*>(aPkt.GetSender());
   if (senderPtr != nullptr)
   {
      WsfDisInterface*     disInterface = WsfDisInterface::Find(*mSimulation);
      WsfXIO_PlatformInfo* info         = simEnv.GetXIO_Info().GetPlatformInfo(senderPtr, aPkt.mPlatformIndex);
      if (info != nullptr && disInterface != nullptr && mSimulation != nullptr)
      {
         WsfDisPlatform* disPlatformPtr = disInterface->FindDisPlatform(info->GetEntityId().Convert<DisEntityId>());
         if (disPlatformPtr != nullptr)
         {
            WsfPlatform* platform = disPlatformPtr->GetPlatform();
            if (platform != nullptr)
            {
               auto         index = platform->GetIndex();
               QMutexLocker locker(&mMutex);
               mPlatformScripts[index].clear();
               for (const auto& script : aPkt.mScripts)
               {
                  script::Data temp;
                  temp.name    = script.mName;
                  temp.context = warlock::script::PLATFORM;
                  for (const auto& arg : script.mArguments)
                  {
                     temp.argTypes.push_back(arg.mArgumentType);
                     temp.argNames.push_back(arg.mArgumentName);
                  }
                  mPlatformScripts[index][script.mName] = temp;
               }
               emit RemotePlatformUpdated(QString::fromStdString(platform->GetName()));
            }
         }
      }
   }
}

void warlock::ScriptExecuteCommand::Process(WsfSimulation& aSimulation)
{
   bool ok = true;
   if (mScript.script.context == script::GLOBAL)
   {
      UtScriptDataList list = GetUtScriptDataList(mScript, aSimulation, true, ok);
      if (ok)
      {
         script::ReturnValue value;
         value.platformName = "";
         value.scriptName   = mScript.script.name;

         if (!mScript.script.name.empty())
         {
            // Notify Observer of User Action. (Platform Script Execution) Action is recorded in Mystic's Event Pipe.
            WkObserver::SimulationUserAction(aSimulation)("Script Executed: " + value.scriptName, nullptr);

            aSimulation.GetScriptContext().ExecuteScript(0.0, value.scriptName, value.returnValue, list);
            emit mInterface->ReturnValueReady(mId, value);
         }
         else if (!mScript.script.text.empty())
         {
            // Notify Observer of User Action. (Platform Script Execution) Action is recorded in Mystic's Event Pipe.
            WkObserver::SimulationUserAction(aSimulation)("Script Executed: (No Function Name)\n" + mScript.script.text,
                                                          nullptr);

            // Provide the text to be used in the ReturnValueReady signal
            value.scriptName = mScript.script.text;

            aSimulation.GetScriptContext().ExecuteScript(0.0, value.returnValue, list, mScript.script.text);
            emit mInterface->ReturnValueReady(mId, value);
         }
      }
   }
   else // context == PLATFORM
   {
      WsfPlatform* platform = aSimulation.GetPlatformByName(mScript.platformName);
      if (platform)
      {
         UtScriptDataList list = GetUtScriptDataList(mScript, aSimulation, !platform->IsExternallyControlled(), ok);
         if (ok)
         {
            if (platform->IsExternallyControlled())
            {
               WsfXIO_PlatformInfo* info = simEnv.GetXIO_Info().GetPlatformInfo(platform);
               if (info != nullptr)
               {
                  WsfXIO_ScriptExecutePkt pkt;
                  pkt.mPlatformIndex       = info->GetIndex();
                  pkt.mArgumentList        = list;
                  pkt.mScriptName          = mScript.script.name;
                  pkt.mScriptText          = mScript.script.text;
                  pkt.mPartType            = 0;
                  pkt.mBaseObject          = nullptr;
                  pkt.mIsApplicationMethod = false;
                  info->GetHostConnection()->Send(pkt);
               }
            }
            else
            {
               script::ReturnValue value;
               value.platformName = mScript.platformName;
               value.scriptName   = mScript.script.name;

               if (!mScript.script.name.empty())
               {
                  // Notify Observer of User Action. (Platform Script Execution) Action is recorded in Mystic's Event Pipe.
                  WkObserver::SimulationUserAction(aSimulation)("Script Executed: " + value.scriptName, platform);

                  platform->ExecuteScript(0.0, value.scriptName, value.returnValue, list);
                  emit mInterface->ReturnValueReady(mId, value);
               }
               else if (!mScript.script.text.empty())
               {
                  // Notify Observer of User Action. (Platform Script Execution) Action is recorded in Mystic's Event Pipe.
                  WkObserver::SimulationUserAction(
                     aSimulation)("Script Executed: (No Function Name)\n" + mScript.script.text, platform);

                  // Provide the text to be used in the ReturnValueReady signal
                  value.scriptName = mScript.script.text;

                  platform->GetScriptContext().ExecuteScript(0.0, value.returnValue, list, mScript.script.text);
                  emit mInterface->ReturnValueReady(mId, value);
               }
            }
         }
      }
      else
      {
         wkf::QueueableMessageObject::DisplayQueuedMessage(QMessageBox::Warning,
                                                           "Script Error",
                                                           "Platform (" + QString::fromStdString(mScript.platformName) +
                                                              ") not found.");
      }
   }
}

void warlock::ScriptInfoRequestCommand::Process(WsfSimulation& aSimulation)
{
   WsfPlatform* platform = aSimulation.GetPlatformByIndex(mIndex);
   if (platform != nullptr)
   {
      WsfXIO_PlatformInfo* info = simEnv.GetXIO_Info().GetPlatformInfo(platform);
      if (info != nullptr)
      {
         WsfXIO_InformationRequestPkt pkt;
         pkt.mType          = WsfXIO_InformationRequestPkt::cSCRIPT_LIST;
         pkt.mPlatformIndex = info->GetIndex();
         info->GetHostConnection()->Send(pkt);
      }
   }
}
