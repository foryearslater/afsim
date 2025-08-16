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

#ifndef SDL_JOYSTICKMANAGER_HPP
#define SDL_JOYSTICKMANAGER_HPP

#include <list>
#include <map>
#include <set>

#include <QDialog>
#include <QString>
#include <SDL2/SDL.h>

#include "SDL_Manager.hpp"
#include "UtWallClock.hpp"

class QCheckBox;
class QComboBox;
class QFile;
class QLineEdit;
class QProgressBar;
class QXmlStreamReader;

namespace sdl
{
class JoystickManagerSpy;
// sdl::JoystickManager organizes/manages gaming joysticks and controllers
// (and even keyboards) through the use of Simple Direct Media Layer (SDL).
class JoystickManager : public QObject
{
public:
   JoystickManager(Manager& aManager, QWidget* aMainWindow);
   ~JoystickManager() override;

   void GetDevices(std::list<std::string>& aDeviceList);

   // This finds the best device in the device name list and assigns it
   // to the mUsedDevices map
   bool UseBestDevice(int aDeviceId, std::list<std::string> aDeviceNames);

   // This finds a defined device, and assigns it to the user deviceId.
   // It just uses the first one it finds.
   bool UseDefinedDevice(int aDeviceId);

   // This releases all devices
   void ReleaseDevice(int aDeviceId);

   std::string GetDeviceName(int aDeviceId);
   bool        HasDevice(int aDeviceId);

   void Update();

   float GetValue(int aDeviceId, const std::string& aName);

   // Note, except in the case of toggle buttons, this is a useless thing to do
   void SetValue(int aDeviceId, const std::string& aName, float aValue);

   // This returns the devices recognized by SDL with indices.
   // This is really meant for debugging -- mapped devices are used in
   // nominal circumstances.
   std::map<int, std::string> GetAvailableDeviceIdStrings();

   // This identifies which capabilities are available on a device.
   // It is used in debugging
   void QueryComponentsByIndex(int aIndex, int& aButtons, int& aAxes, int& aHats, int& aBalls);

   // This provides the state of a component.
   // This is used in debugging (and is terribly inefficient)
   int GetButtonStateByIndex(int aIndex, int aButton);

   int GetAxisStateByIndex(int aIndex, int aAxis);
   int GetHatStateByIndex(int aIndex, int aHat);

   std::pair<int, int> GetBallStateByIndex(int aIndex, int aBall);

   void OpenSDL_DeviceSpy(QWidget* aParent);

   JoystickManagerSpy* GetOpenSDL_DeviceSpy() { return mDeviceMonitorPtr; }

   void HideSDL_DeviceSpy();

   const std::vector<std::string>& GetScriptQueue() const { return mScriptQueue; }
   void                            ClearScriptQueue() { mScriptQueue.clear(); }

   struct JoystickValueSourceDefinition
   {
      int                     mSubdevice;
      int                     mType;
      int                     mIndex;
      float                   mDeadZone;
      int                     mAxis; // Used for POV/hat buttons
      int                     mMult;
      std::pair<float, float> mRange;
      std::vector<float>      mToggleList;
   };

   struct JoystickDeviceDefinition
   {
      std::map<int, std::list<std::string>>                           mIdList;
      std::map<std::string, std::list<JoystickValueSourceDefinition>> mValueSourceMap;
      std::map<std::string, std::list<JoystickValueSourceDefinition>> mScriptSourceMap;
      std::map<std::string, bool>                                     mScriptExecuteStateMap;
   };

   struct ToggleState
   {
      int    mDown;
      int    mIndex;
      double mLastToggleTime;
   };

   struct UsedDevice
   {
      std::map<int, SDL_Joystick*>               mSubDeviceList;
      std::string                                mDeviceType;
      JoystickDeviceDefinition                   mDeviceDefinition;
      std::map<std::string, float>               mValueMap;
      std::map<std::pair<int, int>, ToggleState> mToggleState;
   };

   enum JoystickComponentType
   {
      TYPE_BUTTON,
      TYPE_AXIS,
      TYPE_TOGGLE_BUTTON,
      TYPE_HAT,
      TYPE_STATE_KEY,
      TYPE_TOGGLE_KEY
   };

private:
   bool eventFilter(QObject* aObjPtr, QEvent* aEventPtr) override;

   float GetSourceValue(SDL_Joystick*                        aSubdevices,
                        const JoystickValueSourceDefinition& aDefinition,
                        ToggleState&                         aToggleState);

   void XmlReadDevice(QXmlStreamReader& aRXML);

   void XmlReadComponents(QXmlStreamReader& aRXML, std::list<JoystickValueSourceDefinition>& aValueSourceList);

   void XmlReadNext(QXmlStreamReader& aRXML);

   void XmlReadFile(QFile& aFile);

   QString XmlGetStringAttribute(QXmlStreamReader& aRXML, const QString& aName);

   int XmlGetIntAttribute(QXmlStreamReader& aRXML, const QString& aName);

   float XmlGetFloatAttribute(QXmlStreamReader& aRXML, const QString& aName);

   std::vector<float> XmlGetFloatListAttribute(QXmlStreamReader& aRXML, const QString& aName, float aFirst, float aSecond);

   std::pair<float, float> XmlGetFloatPairAttribute(QXmlStreamReader& aRXML, const QString& aName, float aMin, float aMax);

   char XmlGetQtKeyAttribute(QXmlStreamReader& aRXML, const QString& aName);

   std::map<std::string, JoystickDeviceDefinition> mDeviceDefinitionMap;
   std::map<int, UsedDevice>                       mUsedDevices;
   UtWallClock                                     mClock;
   std::set<char>                                  mKeyListener;
   std::vector<std::string>                        mScriptQueue;
   Manager&                                        mManager;
   JoystickManagerSpy*                             mDeviceMonitorPtr;
};

class JoystickManagerSpy : public QDialog
{
   Q_OBJECT
public:
   JoystickManagerSpy(JoystickManager* aMgr, QWidget* aParent);
   ~JoystickManagerSpy() override;

   void timerEvent(QTimerEvent* aEvent) override;
   void closeEvent(QCloseEvent* aEvent) override;
signals:
   void WindowIsClosing();

private:
   JoystickManager*             mManager;
   QComboBox*                   mDevice;
   int                          mActiveIndex;
   SDL_Joystick*                mJoystick;
   QWidget*                     mControlArea;
   std::map<int, QCheckBox*>    mButtonMap;
   std::map<int, QProgressBar*> mAxesMap;
   std::map<int, QLineEdit*>    mHatMap;
};
} // namespace sdl
#endif // SDL_JOYSTICKMANAGER_HPP
