// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2020 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "SDL_JoystickManager.hpp"

#include <iostream>

#include <QCheckBox>
#include <QComboBox>
#include <QDir>
#include <QFile>
#include <QFormLayout>
#include <QKeyEvent>
#include <QLayout>
#include <QLineEdit>
#include <QProgressBar>
#include <QScrollArea>
#include <QTimer>
#include <QXmlStreamReader>

#include "UtPath.hpp"

sdl::JoystickManager::JoystickManager(sdl::Manager& aManager, QWidget* aMainWindow)
   : mManager(aManager)
   , mDeviceMonitorPtr(new JoystickManagerSpy(this, aMainWindow))
{
   // Initialize SDL (required for joystick support)
   if (!mManager.InitSDL())
   {
      std::cout << "ERROR: sdl::JoystickManager::JoystickManager() was unable to initialize SDL!" << std::endl;
      return;
   }

   SDL_JoystickEventState(SDL_QUERY);

   // Read in the XML structure
   UtPath path = UtPath::GetExePath();
   path.Up();
   const char* devDataPath = getenv("SOURCE_ROOT");
   if (nullptr != devDataPath)
   {
      path = std::string(devDataPath) + "/warlock/plugins/P6DOF_Controller/data";
   }
   else
   {
      path = path.GetSystemPath() + "/../resources/data";
   }
   QDir        dir(path.GetSystemPath().c_str());
   QStringList contents = dir.entryList(QStringList("*.xml"));
   for (auto&& it : contents)
   {
      QFile file(QString(path.GetSystemPath().c_str()) + "/" + it);
      if (!file.open(QFile::ReadOnly | QFile::Text))
      {
         std::cout << "Could not open: " << path.GetSystemPath() << std::endl;
      }
      else
      {
         XmlReadFile(file);
      }
   }
}

sdl::JoystickManager::~JoystickManager()
{
   for (auto& device : mUsedDevices)
   {
      for (auto& jt : device.second.mSubDeviceList)
      {
         SDL_JoystickClose(jt.second);
      }
   }

   // Tell the SDL manager that we're ready to quit
   mManager.QuitSDL();

   mUsedDevices.clear();
}

std::map<int, std::string> sdl::JoystickManager::GetAvailableDeviceIdStrings()
{
   std::map<int, std::string> joysticklist;
   int                        nj = SDL_NumJoysticks();
   for (int i = 0; i < nj; ++i)
   {
      SDL_Joystick* js = SDL_JoystickOpen(i);
      joysticklist[i]  = SDL_JoystickName(js);
      SDL_JoystickClose(js);
   }
   return joysticklist;
}

void sdl::JoystickManager::QueryComponentsByIndex(int aIndex, int& aButtons, int& aAxes, int& aHats, int& aBalls)
{
   SDL_Joystick* js = SDL_JoystickOpen(aIndex);
   aButtons         = SDL_JoystickNumButtons(js);
   aAxes            = SDL_JoystickNumAxes(js);
   aHats            = SDL_JoystickNumHats(js);
   aBalls           = SDL_JoystickNumBalls(js);
   SDL_JoystickClose(js);
}

int sdl::JoystickManager::GetButtonStateByIndex(int aIndex, int aButton)
{
   SDL_Joystick* js = SDL_JoystickOpen(aIndex);
   SDL_JoystickUpdate();
   int value = SDL_JoystickGetButton(js, aButton);
   SDL_JoystickClose(js);
   return value;
}

int sdl::JoystickManager::GetAxisStateByIndex(int aIndex, int aAxis)
{
   SDL_Joystick* js = SDL_JoystickOpen(aIndex);
   SDL_JoystickUpdate();
   int value = SDL_JoystickGetAxis(js, aAxis);
   SDL_JoystickClose(js);
   return value;
}

int sdl::JoystickManager::GetHatStateByIndex(int aIndex, int aHat)
{
   SDL_Joystick* js = SDL_JoystickOpen(aIndex);
   SDL_JoystickUpdate();
   int value = SDL_JoystickGetHat(js, aHat);
   SDL_JoystickClose(js);
   return value;
}

std::pair<int, int> sdl::JoystickManager::GetBallStateByIndex(int aIndex, int aBall)
{
   std::pair<int, int> p;
   SDL_Joystick*       js = SDL_JoystickOpen(aIndex);
   SDL_JoystickUpdate();
   SDL_JoystickGetBall(js, aBall, &(p.first), &(p.second));
   SDL_JoystickClose(js);
   return p;
}

bool sdl::JoystickManager::UseBestDevice(int aDeviceId, std::list<std::string> aPreferences)
{
   auto cit = mUsedDevices.find(aDeviceId);
   if (cit != mUsedDevices.end())
   {
      std::cout << "JoystickManager: error, provided user device id is already assigned: " << aDeviceId << std::endl;
      return false;
   }

   std::map<int, std::string> joysticklist;
   int                        nj = SDL_NumJoysticks();
   for (int i = 0; i < nj; ++i)
   {
      SDL_Joystick* js = SDL_JoystickOpen(i);
      joysticklist[i]  = SDL_JoystickName(js);
      SDL_JoystickClose(js);
   }

   std::list<std::string>::iterator it;
   for (it = aPreferences.begin(); it != aPreferences.end(); ++it)
   {
      bool found = false;
      auto jt    = mDeviceDefinitionMap.find(*it);
      if (jt != mDeviceDefinitionMap.end())
      {
         if (jt->first == "keyboard")
         {
            mUsedDevices[aDeviceId].mSubDeviceList.clear();
            mUsedDevices[aDeviceId].mDeviceDefinition = jt->second;
            mUsedDevices[aDeviceId].mDeviceType       = jt->first;
            found                                     = true;
         }
         else
         {
            std::map<int, std::list<std::string>>::iterator idit;
            for (idit = jt->second.mIdList.begin(); idit != jt->second.mIdList.end(); ++idit)
            {
               std::list<std::string>::iterator oneidit;
               for (oneidit = idit->second.begin(); oneidit != idit->second.end(); ++oneidit)
               {
                  std::map<int, std::string>::iterator jlit;
                  for (jlit = joysticklist.begin(); jlit != joysticklist.end(); ++jlit)
                  {
                     QRegExp rx(oneidit->c_str());
                     rx.setPatternSyntax(QRegExp::Wildcard);
                     if (rx.exactMatch(jlit->second.c_str()))
                     {
                        mUsedDevices[aDeviceId].mSubDeviceList[idit->first] = SDL_JoystickOpen(jlit->first);
                        mUsedDevices[aDeviceId].mDeviceDefinition           = jt->second;
                        mUsedDevices[aDeviceId].mDeviceType                 = jt->first;

                        found = true;
                        break; // we are done with this subdevice
                     }
                  }
               }
            }
         }
      }
      if (found)
      {
         return true;
      }
   }
   return false;
}

void sdl::JoystickManager::GetDevices(std::list<std::string>& aDeviceList)
{
   std::map<int, std::string> joysticklist;
   int                        nj = SDL_NumJoysticks();
   for (int i = 0; i < nj; ++i)
   {
      SDL_Joystick* js = SDL_JoystickOpen(i);
      joysticklist[i]  = SDL_JoystickName(js);
      SDL_JoystickClose(js);
   }

   std::map<std::string, JoystickDeviceDefinition>::iterator jt;
   for (jt = mDeviceDefinitionMap.begin(); jt != mDeviceDefinitionMap.end(); ++jt)
   {
      if (jt->first == "keyboard")
      {
         aDeviceList.push_back(jt->first);
      }
      else
      {
         std::map<int, std::list<std::string>>::iterator idit;
         for (idit = jt->second.mIdList.begin(); idit != jt->second.mIdList.end(); ++idit)
         {
            std::list<std::string>::iterator oneidit;
            for (oneidit = idit->second.begin(); oneidit != idit->second.end(); ++oneidit)
            {
               std::map<int, std::string>::iterator jlit;
               for (jlit = joysticklist.begin(); jlit != joysticklist.end(); ++jlit)
               {
                  QRegExp rx(oneidit->c_str());
                  rx.setPatternSyntax(QRegExp::Wildcard);
                  if (rx.exactMatch(jlit->second.c_str()))
                  {
                     aDeviceList.push_back(jt->first);
                  }
               }
            }
         }
      }
   }
}

bool sdl::JoystickManager::UseDefinedDevice(int aDeviceId)
{
   auto cit = mUsedDevices.find(aDeviceId);
   if (cit != mUsedDevices.end())
   {
      std::cout << "JoystickManager: error, provided user device id is already assigned: " << aDeviceId << std::endl;
      return false;
   }

   std::map<int, std::string> joysticklist;
   int                        nj = SDL_NumJoysticks();
   for (int i = 0; i < nj; ++i)
   {
      SDL_Joystick* js = SDL_JoystickOpen(i);
      joysticklist[i]  = SDL_JoystickName(js);
      SDL_JoystickClose(js);
   }

   bool                                                      found = false;
   std::map<std::string, JoystickDeviceDefinition>::iterator jt;
   for (jt = mDeviceDefinitionMap.begin(); jt != mDeviceDefinitionMap.end(); ++jt)
   {
      std::map<int, std::list<std::string>>::iterator idit;
      for (idit = jt->second.mIdList.begin(); idit != jt->second.mIdList.end(); ++idit)
      {
         std::list<std::string>::iterator oneidit;
         for (oneidit = idit->second.begin(); oneidit != idit->second.end(); ++oneidit)
         {
            std::map<int, std::string>::iterator jlit;
            for (jlit = joysticklist.begin(); jlit != joysticklist.end(); ++jlit)
            {
               QRegExp rx(oneidit->c_str());
               rx.setPatternSyntax(QRegExp::Wildcard);
               if (rx.exactMatch(jlit->second.c_str()))
               {
                  mUsedDevices[aDeviceId].mSubDeviceList[idit->first] = SDL_JoystickOpen(jlit->first);
                  mUsedDevices[aDeviceId].mDeviceDefinition           = jt->second;
                  mUsedDevices[aDeviceId].mDeviceType                 = jt->first;

                  found = true;
                  break; // we are done with this subdevice
               }
            }
         }
      }
   }
   if (found)
   {
      return true;
   }

   auto kt = mDeviceDefinitionMap.find("keyboard");
   if (kt != mDeviceDefinitionMap.end())
   {
      mUsedDevices[aDeviceId].mSubDeviceList.clear();
      mUsedDevices[aDeviceId].mDeviceDefinition = kt->second;
      mUsedDevices[aDeviceId].mDeviceType       = kt->first;
      return true;
   }

   // Try the keyboard?
   return false;
}

void sdl::JoystickManager::ReleaseDevice(int aDeviceId)
{
   auto it = mUsedDevices.find(aDeviceId);
   if (it != mUsedDevices.end())
   {
      for (auto& jt : it->second.mSubDeviceList)
      {
         SDL_JoystickClose(jt.second);
      }
      mUsedDevices.erase(it);
   }
}

std::string sdl::JoystickManager::GetDeviceName(int aDeviceId)
{
   auto it = mUsedDevices.find(aDeviceId);
   if (it != mUsedDevices.end())
   {
      return it->second.mDeviceType;
   }
   return "ERROR";
}

bool sdl::JoystickManager::HasDevice(int aDeviceId)
{
   auto it = mUsedDevices.find(aDeviceId);
   if (it != mUsedDevices.end())
   {
      return true;
   }
   return false;
}

void sdl::JoystickManager::Update()
{
   SDL_JoystickUpdate();

   for (auto& ud : mUsedDevices)
   {
      UsedDevice& device = ud.second;
      for (const auto& jvs : device.mDeviceDefinition.mValueSourceMap)
      {
         device.mValueMap[jvs.first] = 0.0f;
         float multer                = 1.0f;
         for (const auto& jvsd : jvs.second)
         {
            int   subdevice   = jvsd.mSubdevice;
            float sourceValue = GetSourceValue(device.mSubDeviceList[subdevice],
                                               jvsd,
                                               device.mToggleState[std::make_pair(subdevice, jvsd.mIndex)]);
            if (jvsd.mMult == 1)
            {
               multer *= sourceValue;
            }
            else
            {
               device.mValueMap[jvs.first] += sourceValue;
            }
         }
         device.mValueMap[jvs.first] *= multer;
      }
      for (const auto& jss : device.mDeviceDefinition.mScriptSourceMap)
      {
         // Currently, we will only handle discrete events that trigger above 0.5 and reset below

         float value  = 0.0f;
         float multer = 1.0f;
         for (const auto& jvsd : jss.second)
         {
            int   subdevice   = jvsd.mSubdevice;
            float sourceValue = GetSourceValue(device.mSubDeviceList[subdevice],
                                               jvsd,
                                               device.mToggleState[std::make_pair(subdevice, jvsd.mIndex)]);
            if (jvsd.mMult == 1)
            {
               multer *= sourceValue;
            }
            else
            {
               value += sourceValue;
            }
         }
         value *= multer;
         if (value > 0.5f)
         {
            if (!device.mDeviceDefinition.mScriptExecuteStateMap[jss.first])
            {
               device.mDeviceDefinition.mScriptExecuteStateMap[jss.first] = true;
               mScriptQueue.push_back(jss.first);
               // execute
            }
         }
         else
         {
            if (device.mDeviceDefinition.mScriptExecuteStateMap[jss.first])
            {
               device.mDeviceDefinition.mScriptExecuteStateMap[jss.first] = false;
            }
         }
      }
   }
   mKeyListener.clear();
   // Fill the active devices value map
}

float sdl::JoystickManager::GetValue(int aDeviceId, const std::string& aName)
{
   auto it = mUsedDevices.find(aDeviceId);
   if (it != mUsedDevices.end())
   {
      return it->second.mValueMap[aName];
   }
   else
   {
      static float sEmpty = 0.0f;
      return sEmpty;
   }
}

// NOTE: Except in the case of toggle_buttons, this will be overwritten by the device on the next update!
void sdl::JoystickManager::SetValue(int aDeviceId, const std::string& aName, float aValue)
{
   auto it = mUsedDevices.find(aDeviceId);
   if (it != mUsedDevices.end())
   {
      it->second.mValueMap[aName] = aValue;
   }
}

float sdl::JoystickManager::GetSourceValue(SDL_Joystick*                        aSubdevice,
                                           const JoystickValueSourceDefinition& aDefinition,
                                           ToggleState&                         aToggleState)
{
   float value = 0;
   if (aDefinition.mType == TYPE_BUTTON)
   {
      value = SDL_JoystickGetButton(aSubdevice, aDefinition.mIndex);
      value = value * (aDefinition.mRange.second - aDefinition.mRange.first) + aDefinition.mRange.first;
   }
   else if (aDefinition.mType == TYPE_HAT)
   {
      int ivalue = SDL_JoystickGetHat(aSubdevice, aDefinition.mIndex);
      if (aDefinition.mAxis == 0)
      {
         if (ivalue & 8)
         {
            value = -1;
         }
         else if (ivalue & 2)
         {
            value = 1;
         }
      }
      else
      {
         if (ivalue & 4)
         {
            value = -1;
         }
         else if (ivalue & 1)
         {
            value = 1;
         }
      }
      value = (value + 1.0f) / 2.0f * (aDefinition.mRange.second - aDefinition.mRange.first) + aDefinition.mRange.first;
   }
   else if (aDefinition.mType == TYPE_AXIS)
   {
      value = SDL_JoystickGetAxis(aSubdevice, aDefinition.mIndex) / 32768.0f;

      if (fabs(value) > aDefinition.mDeadZone)
      {
         if (value > 0)
         {
            value = (value - aDefinition.mDeadZone) / (1.0f - aDefinition.mDeadZone);
         }
         else
         {
            value = (value + aDefinition.mDeadZone) / (1.0f - aDefinition.mDeadZone);
         }
      }
      else
      {
         value = 0.0f;
      }
      value = (value + 1.0f) / 2.0f * (aDefinition.mRange.second - aDefinition.mRange.first) + aDefinition.mRange.first;
   }
   else if (aDefinition.mType == TYPE_TOGGLE_BUTTON)
   {
      int state = SDL_JoystickGetButton(aSubdevice, aDefinition.mIndex);
      if ((state == 1) && (aToggleState.mDown == 0))
      {
         double time = mClock.GetClock();
         if (time > 0.2 + aToggleState.mLastToggleTime)
         {
            aToggleState.mIndex          = (aToggleState.mIndex + 1) % aDefinition.mToggleList.size();
            aToggleState.mLastToggleTime = time;
         }
      }
      aToggleState.mDown = state;
      value              = aDefinition.mToggleList[aToggleState.mIndex];
   }
   if (aDefinition.mType == TYPE_STATE_KEY)
   {
      if (mKeyListener.find(aDefinition.mIndex) != mKeyListener.end())
      {
         value = aDefinition.mRange.second;
      }
      else
      {
         value = aDefinition.mRange.first;
      }
   }
   else if (aDefinition.mType == TYPE_TOGGLE_KEY)
   {
      if (mKeyListener.find(aDefinition.mIndex) != mKeyListener.end())
      {
         aToggleState.mIndex = (aToggleState.mIndex + 1) % aDefinition.mToggleList.size();
      }
      value = aDefinition.mToggleList[aToggleState.mIndex];
   }
   return value;
}

void sdl::JoystickManager::XmlReadFile(QFile& aFile)
{
   QXmlStreamReader rxml;
   rxml.setDevice(&aFile);
   XmlReadNext(rxml);
   while (!rxml.atEnd())
   {
      if (rxml.isStartElement())
      {
         if (rxml.name() == "device")
         {
            XmlReadDevice(rxml);
         }
         XmlReadNext(rxml);
      }
      else
      {
         XmlReadNext(rxml);
      }
   }
   aFile.close();
   if (rxml.hasError())
   {
      std::cerr << "Error: Failed to parse controllers.xml file: " << qPrintable(rxml.errorString()) << std::endl;
   }
   else if (aFile.error() != QFile::NoError)
   {
      std::cerr << "Error: Cannot controllers.xml file: " << qPrintable(aFile.errorString()) << std::endl;
   }
}

void sdl::JoystickManager::XmlReadDevice(QXmlStreamReader& aRXML)
{
   QString devName = XmlGetStringAttribute(aRXML, "name");
   XmlReadNext(aRXML);
   JoystickDeviceDefinition device;

   while ((!aRXML.atEnd()) && (!aRXML.isEndElement()))
   {
      if (aRXML.isStartElement())
      {
         if (aRXML.name() == "id_string")
         {
            QString id_string = XmlGetStringAttribute(aRXML, "name");
            int     subDevice = XmlGetIntAttribute(aRXML, "subdevice");
            device.mIdList[subDevice].push_back(id_string.toStdString());
            XmlReadNext(aRXML);
         }
         else if (aRXML.name() == "value")
         {
            QString vname = XmlGetStringAttribute(aRXML, "name");
            XmlReadNext(aRXML);
            XmlReadComponents(aRXML, device.mValueSourceMap[vname.toStdString()]);
         }
         else if (aRXML.name() == "script_value")
         {
            QString vname = XmlGetStringAttribute(aRXML, "name");
            XmlReadNext(aRXML);
            XmlReadComponents(aRXML, device.mScriptSourceMap[vname.toStdString()]);
            device.mScriptExecuteStateMap[vname.toStdString()] = false;
         }
      }
      XmlReadNext(aRXML);
   }
   mDeviceDefinitionMap[devName.toStdString()] = device;
}

void sdl::JoystickManager::XmlReadComponents(QXmlStreamReader&                         aRXML,
                                             std::list<JoystickValueSourceDefinition>& aValueSourceList)
{
   while ((!aRXML.atEnd()) && (!aRXML.isEndElement()))
   {
      if (aRXML.isStartElement())
      {
         if (aRXML.name() == "button")
         {
            JoystickValueSourceDefinition jvs;
            jvs.mType      = TYPE_BUTTON;
            jvs.mIndex     = XmlGetIntAttribute(aRXML, QString("name"));
            jvs.mSubdevice = XmlGetIntAttribute(aRXML, QString("subdevice"));
            jvs.mDeadZone  = 0.0;
            jvs.mRange     = XmlGetFloatPairAttribute(aRXML, QString("value"), 0, 1);
            jvs.mMult      = XmlGetIntAttribute(aRXML, QString("mult"));
            jvs.mAxis      = 0;
            aValueSourceList.push_back(jvs);
            XmlReadNext(aRXML);
         }
         else if (aRXML.name() == "axis")
         {
            JoystickValueSourceDefinition jvs;
            jvs.mType      = TYPE_AXIS;
            jvs.mIndex     = XmlGetIntAttribute(aRXML, QString("name"));
            jvs.mSubdevice = XmlGetIntAttribute(aRXML, QString("subdevice"));
            jvs.mDeadZone  = XmlGetFloatAttribute(aRXML, QString("deadzone"));
            jvs.mRange     = XmlGetFloatPairAttribute(aRXML, QString("value"), -1, 1);
            jvs.mMult      = XmlGetIntAttribute(aRXML, QString("mult"));
            jvs.mAxis      = 0;
            aValueSourceList.push_back(jvs);
            XmlReadNext(aRXML);
         }
         else if (aRXML.name() == "toggle_button")
         {
            JoystickValueSourceDefinition jvs;
            jvs.mType       = TYPE_TOGGLE_BUTTON;
            jvs.mIndex      = XmlGetIntAttribute(aRXML, QString("name"));
            jvs.mSubdevice  = XmlGetIntAttribute(aRXML, QString("subdevice"));
            jvs.mDeadZone   = 0.0;
            jvs.mToggleList = XmlGetFloatListAttribute(aRXML, QString("value"), 0, 1);
            jvs.mMult       = XmlGetIntAttribute(aRXML, QString("mult"));
            jvs.mAxis       = 0;
            aValueSourceList.push_back(jvs);
            XmlReadNext(aRXML);
         }
         else if (aRXML.name() == "hat")
         {
            JoystickValueSourceDefinition jvs;
            jvs.mType       = TYPE_HAT;
            jvs.mIndex      = XmlGetIntAttribute(aRXML, QString("name"));
            jvs.mSubdevice  = XmlGetIntAttribute(aRXML, QString("subdevice"));
            jvs.mDeadZone   = 0.0;
            jvs.mMult       = XmlGetIntAttribute(aRXML, QString("mult"));
            QString hataxis = XmlGetStringAttribute(aRXML, QString("axis")).toLower();
            if ((hataxis == "h") || (hataxis == "horz") || (hataxis == "horizontal"))
            {
               jvs.mAxis = 0;
            }
            else
            {
               jvs.mAxis = 1;
            }
            jvs.mRange = XmlGetFloatPairAttribute(aRXML, QString("value"), -1, 1);
            aValueSourceList.push_back(jvs);
            XmlReadNext(aRXML);
         }
         else if (aRXML.name() == "state_key")
         {
            JoystickValueSourceDefinition jvs;
            jvs.mType      = TYPE_STATE_KEY;
            jvs.mIndex     = (int)XmlGetQtKeyAttribute(aRXML, QString("name"));
            jvs.mSubdevice = -1;
            jvs.mDeadZone  = 0.0;
            jvs.mMult      = XmlGetIntAttribute(aRXML, QString("mult"));
            jvs.mAxis      = 0;
            jvs.mRange     = XmlGetFloatPairAttribute(aRXML, QString("value"), 0, 1);
            aValueSourceList.push_back(jvs);
            XmlReadNext(aRXML);
         }
         else if (aRXML.name() == "toggle_key")
         {
            JoystickValueSourceDefinition jvs;
            jvs.mType       = TYPE_TOGGLE_KEY;
            jvs.mIndex      = (int)XmlGetQtKeyAttribute(aRXML, QString("name"));
            jvs.mSubdevice  = -1;
            jvs.mDeadZone   = 0.0;
            jvs.mMult       = XmlGetIntAttribute(aRXML, QString("mult"));
            jvs.mAxis       = 0;
            jvs.mToggleList = XmlGetFloatListAttribute(aRXML, QString("value"), 0, 1);
            aValueSourceList.push_back(jvs);
            XmlReadNext(aRXML);
         }
      }
      XmlReadNext(aRXML);
   }
}

QString sdl::JoystickManager::XmlGetStringAttribute(QXmlStreamReader& aRXML, const QString& aName)
{
   foreach (const QXmlStreamAttribute& attr, aRXML.attributes())
   {
      if (attr.name().toString() == aName)
      {
         return attr.value().toString();
      }
   }
   return "dummy";
}

int sdl::JoystickManager::XmlGetIntAttribute(QXmlStreamReader& aRXML, const QString& aName)
{
   foreach (const QXmlStreamAttribute& attr, aRXML.attributes())
   {
      if (attr.name().toString() == aName)
      {
         return attr.value().toInt();
      }
   }
   return 0;
}

char sdl::JoystickManager::XmlGetQtKeyAttribute(QXmlStreamReader& aRXML, const QString& aName)
{
   foreach (const QXmlStreamAttribute& attr, aRXML.attributes())
   {
      if (attr.name().toString() == aName)
      {
         return QKeySequence::fromString(attr.value().toString())[0];
      }
   }
   return 0;
}

float sdl::JoystickManager::XmlGetFloatAttribute(QXmlStreamReader& aRXML, const QString& aName)
{
   foreach (const QXmlStreamAttribute& attr, aRXML.attributes())
   {
      if (attr.name().toString() == aName)
      {
         return attr.value().toFloat();
      }
   }
   return 0.0f;
}

std::pair<float, float>
sdl::JoystickManager::XmlGetFloatPairAttribute(QXmlStreamReader& aRXML, const QString& aName, float aMin, float aMax)
{
   foreach (const QXmlStreamAttribute& attr, aRXML.attributes())
   {
      if (attr.name().toString() == aName)
      {
         QString     ps  = attr.value().toString();
         QStringList psl = ps.split(',');

         return std::pair<float, float>(psl[0].toFloat(), psl[1].toFloat());
      }
   }
   return std::pair<float, float>(aMin, aMax);
}

std::vector<float> sdl::JoystickManager::XmlGetFloatListAttribute(QXmlStreamReader& aRXML,
                                                                  const QString&    aName,
                                                                  float             aFirst,
                                                                  float             aSecond)
{
   std::vector<float> vec;
   foreach (const QXmlStreamAttribute& attr, aRXML.attributes())
   {
      if (attr.name().toString() == aName)
      {
         QString     ps  = attr.value().toString();
         QStringList psl = ps.split(',');

         foreach (const QString& entry, psl)
         {
            vec.push_back(entry.toFloat());
         }
         return vec;
      }
   }
   vec.push_back(aFirst);
   vec.push_back(aSecond);
   return vec;
}

void sdl::JoystickManager::XmlReadNext(QXmlStreamReader& aRXML)
{
   aRXML.readNext();
}

void sdl::JoystickManager::OpenSDL_DeviceSpy(QWidget* aParent)
{
   mDeviceMonitorPtr->show();
}

void sdl::JoystickManager::HideSDL_DeviceSpy()
{
   mDeviceMonitorPtr->hide();
}

bool sdl::JoystickManager::eventFilter(QObject* aObjPtr, QEvent* aEventPtr)
{
   if (aEventPtr->type() == QEvent::KeyPress)
   {
      QKeyEvent* keyEvent = static_cast<QKeyEvent*>(aEventPtr);
      mKeyListener.insert(keyEvent->key());
      return true;
   }
   else if (aEventPtr->type() == QEvent::ShortcutOverride)
   {
      QKeyEvent* keyEvent = static_cast<QKeyEvent*>(aEventPtr);
      mKeyListener.insert(keyEvent->key());
      return true;
   }
   return QObject::eventFilter(aObjPtr, aEventPtr);
}

sdl::JoystickManagerSpy::JoystickManagerSpy(JoystickManager* aMgr, QWidget* aParent)
   : QDialog(aParent)
   , mManager(aMgr)
   , mActiveIndex(-1)
   , mJoystick(nullptr)
{
   QVBoxLayout* vb = new QVBoxLayout(this);
   setLayout(vb);
   mDevice                                   = new QComboBox(this);
   std::map<int, std::string>           dmap = aMgr->GetAvailableDeviceIdStrings();
   std::map<int, std::string>::iterator it;
   QStringList                          strlist;

   for (it = dmap.begin(); it != dmap.end(); ++it)
   {
      strlist.push_back(it->second.c_str());
   }

   mDevice->addItems(strlist);
   vb->addWidget(mDevice);

   mControlArea = new QWidget(this);
   mControlArea->setLayout(new QFormLayout(mControlArea));
   vb->addWidget(mControlArea);
   QBasicTimer* timer = new QBasicTimer();
   timer->start(100, this);
}

sdl::JoystickManagerSpy::~JoystickManagerSpy()
{
   if (mJoystick != nullptr)
   {
      SDL_JoystickClose(mJoystick);
   }
}

void sdl::JoystickManagerSpy::closeEvent(QCloseEvent* aEvent)
{
   QDialog::closeEvent(aEvent);
   emit WindowIsClosing();
}

void sdl::JoystickManagerSpy::timerEvent(QTimerEvent* aEvent)
{
   if (mDevice->currentIndex() != mActiveIndex)
   {
      if (mJoystick != nullptr)
      {
         SDL_JoystickClose(mJoystick);
      }
      mJoystick    = SDL_JoystickOpen(mDevice->currentIndex());
      mActiveIndex = mDevice->currentIndex();

      while (mControlArea->layout()->count() > 0)
      {
         QLayoutItem* item = mControlArea->layout()->takeAt(0);
         if (item->widget())
         {
            delete item->widget();
         }
         delete item;
      }

      mButtonMap.clear();
      mAxesMap.clear();
      mHatMap.clear();

      int buttons = SDL_JoystickNumButtons(mJoystick);
      for (int i = 0; i < buttons; ++i)
      {
         QCheckBox* cb   = new QCheckBox(mControlArea);
         QString    name = QString("B") + QVariant(i).toString();
         cb->setObjectName(name);
         cb->setEnabled(false);
         ((QFormLayout*)mControlArea->layout())->addRow(name, cb);
         mButtonMap[i] = cb;
      }

      int axes = SDL_JoystickNumAxes(mJoystick);
      for (int i = 0; i < axes; ++i)
      {
         QProgressBar* s    = new QProgressBar(mControlArea);
         QString       name = QString("A") + QVariant(i).toString();
         s->setObjectName(name);
         s->setMinimum(-32768);
         s->setMaximum(32767);
         s->setTextVisible(false);
         ((QFormLayout*)mControlArea->layout())->addRow(name, s);
         mAxesMap[i] = s;
      }

      int hats = SDL_JoystickNumHats(mJoystick);
      for (int i = 0; i < hats; ++i)
      {
         QLineEdit* le   = new QLineEdit(mControlArea);
         QString    name = QString("H") + QVariant(i).toString();
         le->setObjectName(name);
         le->setEnabled(false);
         ((QFormLayout*)mControlArea->layout())->addRow(name, le);
         mHatMap[i] = le;
      }
      mControlArea->repaint(mControlArea->geometry());
   }

   SDL_JoystickUpdate();

   int buttons = SDL_JoystickNumButtons(mJoystick);
   for (int i = 0; i < buttons; ++i)
   {
      if (SDL_JoystickGetButton(mJoystick, i))
      {
         mButtonMap[i]->setChecked(true);
      }
      else
      {
         mButtonMap[i]->setChecked(false);
      }
   }

   int axes = SDL_JoystickNumAxes(mJoystick);
   for (int i = 0; i < axes; ++i)
   {
      mAxesMap[i]->setValue(SDL_JoystickGetAxis(mJoystick, i));
   }

   int hats = SDL_JoystickNumHats(mJoystick);
   for (int i = 0; i < hats; ++i)
   {
      QString str;
      int     v = SDL_JoystickGetHat(mJoystick, i);
      if (v & 4)
      {
         str += "down ";
      }
      else if (v & 1)
      {
         str += "up ";
      }
      if (v & 8)
      {
         str += "left";
      }
      else if (v & 2)
      {
         str += "right";
      }
      mHatMap[i]->setText(str);
   }
}
