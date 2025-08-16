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

#ifndef JOYSTICKDATACONTAINER_HPP
#define JOYSTICKDATACONTAINER_HPP

#include <set>
#include <string>

#include <QObject>

class JoystickDataContainer : public QObject
{
   Q_OBJECT

public:
   enum eHudMode
   {
      eHUD_NAV_MODE = 0,
      eHUD_WPN_MODE = 1
   };

   // MFDs are disabled as of now, they will be moved to HDD.
   // enum eMfdMode
   //{
   //   eMFD_RADAR = 0,
   //   eMFD_RWR = 1,
   //   eMFD_IRST = 2,
   //   eMFD_FLIR = 3,
   //   eMFD_DATALINK = 4,
   //   eMFD_WEAPONS = 5,
   //   eMFD_FUEL = 6,
   //   eMFD_ENGINES = 7
   //};

   struct JoystickData
   {
      JoystickData()                         = default;
      JoystickData(const JoystickData& aSrc) = default;

      double      bingoFuelQuantity_lbs = 0.0;
      bool        tgtDesignationLft     = false;
      bool        tgtDesignationRgt     = false;
      bool        tgtDesignationUp      = false;
      bool        tgtDesignationDn      = false;
      bool        ecmXmit               = false;
      std::string activeWeapon;
      std::string activeWeaponType;
      int         numGunRnds           = 0;
      bool        shootFlagActive      = false;
      bool        masterArmEnabled     = false;
      int         numChaff             = 0;
      int         numFlares            = 0;
      bool        bingoFlagActive      = false;
      bool        jammingDetected      = false;
      bool        breakX_Flag          = false;
      bool        masterWarningEnabled = false;
      double      radarMinAz_deg       = 0.0;
      double      radarMaxAz_deg       = 0.0;
      double      radarMinEl_deg       = 0.0;
      double      radarMaxEl_deg       = 0.0;
      int         radarNumScanBars     = 0;
      int         radarCurrentBar      = 0;
      int         radarHighPRF         = 0;

      // MFDs are disabled as of now, they will be moved to HDD.
      // eMfdMode       mfdLftMode = eMFD_RADAR;
      // eMfdMode       mfdRgtMode = eMFD_RWR;
      // size_t         mfdActive = 0;

      // Planned improvement
      // double         radarCuedAz_deg = 0.0;
      // double         radarCuedEl_deg = 0.0;
      // double         radarMaxRange_nm = 0.0;
   };

   JoystickDataContainer() = default;
   JoystickDataContainer(const JoystickDataContainer& aSrc);

   /** Returns true if platform uses a P6DOF Mover, false otherwise.
    * @param aPlatformName name of the platform to check.
    */
   bool IsPlatformP6DOF_Entity(const QString& aPlatformName) const;

   /** Returns true if the simulation has been initialized/reset, false otherwise. */
   bool GetSimulationInitialized() const;

   /** Set if the simulation is initialized or not.  Values reset on true.
    * @param aValue value to set.
    */
   void SetSimulationInitialized(bool aValue);

   /** Called when a platform is added.
    * @param aPlatformName name of the platform being added.
    */
   void PlatformAdded(const std::string& aPlatformName);

   /** Called when a platform is deleted.
    * @param aPlatformName name of the platform being deleted.
    */
   void PlatformDeleted(const std::string& aPlatformName);

   /** Return the name of the currently connected platform. */
   const std::string& GetConnectedPlatform() const { return mConnectedPlatform; }

   /** Set the name of the currently connected platform.
    * @param aPlatformName name of the platform to connect.
    */
   void SetConnectedPlatform(const std::string& aPlatformName);

   /** Release the currently connected platform. */
   void ReleaseConnectedPlatform();

   /** Return a const reference to the joystick data. */
   const JoystickData& GetJoystickData() const { return mJoystickData; }

   /** Set the joystick data.
    * @param aJoystickData data to set.
    */
   void SetJoystickData(const JoystickData& aJoystickData);

signals:
   void JoystickDataChanged();

private:
   bool mSimulationInitialized = false;  ///< True if the simulation has been initialized (reset), false otherwise.
   std::string       mConnectedPlatform; ///< Name of the connected platform.
   JoystickData      mJoystickData;      ///< Data container for joystick data.
   std::set<QString> mSetOfP6DofMovers;  ///< Set of entities that have P6Dof Movers.
};

#endif // JOYSTICKDATACONTAINER_HPP
