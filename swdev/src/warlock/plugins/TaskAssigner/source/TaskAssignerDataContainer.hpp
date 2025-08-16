// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2018 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef TASKASSIGNERDATACONTAINER_HPP
#define TASKASSIGNERDATACONTAINER_HPP

#include <map>
#include <string>
#include <vector>

#include <QObject>

namespace WkTaskAssigner
{
class ObjectData
{
public:
   ObjectData()
      : mName()
      , mType()
   {
   }

   ObjectData(const std::string& aName, const std::string& aType)
      : mName(aName)
      , mType(aType)
   {
   }

   const std::string& Name() const { return mName; }
   const std::string& Type() const { return mType; }

private:
   std::string mName;
   std::string mType;
};
using ObjectList = std::vector<ObjectData>;

class CommandChainData
{
public:
   CommandChainData()
      : mSelf("", "")
   {
   }

   void Clear()
   {
      mSelf = ObjectData();
      mCommandChainNames.clear();
      mCommanders.clear();
      mPeers.clear();
      mSubordinates.clear();
   }

   ObjectData                        mSelf;
   std::vector<std::string>          mCommandChainNames;
   std::map<std::string, ObjectData> mCommanders;
   std::map<std::string, ObjectList> mPeers;
   std::map<std::string, ObjectList> mSubordinates;
};

class JammerData : public ObjectData
{
public:
   JammerData(const std::string& aName, const std::string& aType)
      : ObjectData(aName, aType)
   {
   }
};
using JammerList = std::vector<JammerData>;

class ProcessorData : public ObjectData
{
public:
   ProcessorData(const std::string& aName, const std::string& aType)
      : ObjectData(aName, aType)
   {
   }
};
using ProcessorList = std::vector<ProcessorData>;

class SensorData : public ObjectData
{
public:
   SensorData(const std::string& aName, const std::string& aType, const std::vector<std::string>& aModes)
      : ObjectData(aName, aType)
      , mModes(aModes)
   {
   }

   std::vector<std::string> mModes;
};
using SensorList = std::vector<SensorData>;

class WeaponData : public ObjectData
{
public:
   WeaponData(unsigned int aUniqueID, const std::string& aName, const std::string& aType, double aQuantity)
      : ObjectData(aName, aType)
      , mQuantity(aQuantity)
      , mID(aUniqueID)
   {
   }

   double       mQuantity;
   unsigned int mID;
};
using WeaponList = std::vector<WeaponData>;

class DataContainer : public QObject
{
   Q_OBJECT

public:
   DataContainer()
      : mValid(false)
   {
   }

   bool IsValid() { return mValid; }

   const std::string& GetPlatformOfInterest() const { return mPlatformOfInterest; }
   void               SetPlatformOfInterest(const std::string& aPlatformName);

   const CommandChainData& GetCommandChainData() const { return mCommandChainData; }
   const ObjectList&       GetCommunicationDevicesData() const { return mCommunicationDevices; }

   const JammerList*    GetJammerData(const std::string& aPlatformName) const;
   const ProcessorList* GetProcessorData(const std::string& aPlatformName) const;
   const ProcessorList* GetTaskProcessorData(const std::string& aPlatformName) const;
   const SensorList*    GetSensorData(const std::string& aPlatformName) const;
   const WeaponList*    GetWeaponData(const std::string& aPlatformName) const;

   void SetData(const std::string&                          aPlatformName,
                const CommandChainData&                     aCommandChainData,
                const ObjectList&                           aCommunicationDevicesData,
                const std::map<std::string, JammerList>&    aJammerData,
                const std::map<std::string, ProcessorList>& aProcessorData,
                const std::map<std::string, ProcessorList>& aTaskProcessorData,
                const std::map<std::string, SensorList>&    aSensorData,
                const std::map<std::string, WeaponList>&    aWeaponData);

   void RemovePlatform(const std::string& aPlatformName);

signals:
   void PlatformDeleted(const std::string& aPlatformName);

protected:
   void SetCommandChainData(const CommandChainData& aCommandChainData);
   void SetCommunicationDevicesData(const ObjectList& aCommunicationDevicesData);
   void SetJammerData(const std::map<std::string, JammerList>& aJammerData);
   void SetProcessorData(const std::map<std::string, ProcessorList>& aProcessorData);
   void SetTaskProcessorData(const std::map<std::string, ProcessorList>& aTaskProcessorData);
   void SetSensorData(const std::map<std::string, SensorList>& aSensorData);
   void SetWeaponData(const std::map<std::string, WeaponList>& aWeaponData);

private:
   bool        mValid;
   std::string mPlatformOfInterest;

   CommandChainData mCommandChainData;
   ObjectList       mCommunicationDevices;

   // Platform components
   std::map<std::string, JammerList>    mJammers;
   std::map<std::string, ProcessorList> mProcessors;
   std::map<std::string, ProcessorList> mTaskProcessors;
   std::map<std::string, SensorList>    mSensors;
   std::map<std::string, WeaponList>    mWeapons;
};
} // namespace WkTaskAssigner
#endif