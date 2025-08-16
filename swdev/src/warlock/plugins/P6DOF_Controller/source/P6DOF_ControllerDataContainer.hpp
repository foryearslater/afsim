// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2019 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef P6DOF_CONTROLLERDATACONTAINER_HPP
#define P6DOF_CONTROLLERDATACONTAINER_HPP

#include <string>
#include <unordered_set>

#include <QObject>

#include "UtCalendar.hpp"
#include "WsfSimulation.hpp"

namespace WkP6DOF_Controller
{
// The WkP6DOF_Controller::P6DOF_ControllerDataContainer contains most of the
// state data of the P6DofController (and the controlled platform). This
// provides thread-safe communications using SimEvents from the SimInterface.
class P6DOF_ControllerDataContainer : public QObject
{
   Q_OBJECT

public:
   enum eTrackSide
   {
      eUNDEFINED         = 0,
      eFRIENDLY          = 1,
      eNEUTRAL           = 2,
      ePOSSIBLE_HOSTILE  = 3,
      eCONFIRMED_HOSTILE = 4
   };

   struct TrackData
   {
      double      tgtBearing_rad   = 0.0;
      double      tgtAz_rad        = 0.0;
      double      tgtEl_rad        = 0.0;
      double      tgtRng_m         = 0.0;
      double      tgtSpd_kts       = 0.0;
      double      tgtAlt_ft        = 0.0;
      double      tgtHdg_rad       = 0.0;
      double      tgtRangeRate_mps = 0.0;
      bool        tgtHasIffReply   = false;
      double      tgtLat           = 0.0;
      double      tgtLon           = 0.0;
      double      tgtAlt_m         = 0.0;
      std::string tgtIdString;
      bool        tgtBearingValid = false;
      bool        tgtAzValid      = false;
      bool        tgtElValid      = false;
      bool        tgtRngValid     = false;
      bool        tgtSpdValid     = false;
      bool        tgtAltValid     = false;
      bool        tgtHdgValid     = false;
      bool        tgtRngRateValid = false;
      bool        tgtIdValid      = false;
      bool        tgtIffValid     = false;
      bool        tgtLLA_Valid    = false;
      int         tgtPackedData   = 0;
      std::string tgtPlatformName;
   };

   struct WeaponData
   {
      std::string weaponTypeName;
      std::string displayName;
      int         numRoundsRemaining;
   };

   struct WaypointData
   {
      double lat   = 0.0;
      double lon   = 0.0;
      double alt_m = 0.0;
   };

   enum eHudMode
   {
      eHUD_UNDEFINED = 0,
      eHUD_NAV_WAYPT = 1,
      eHUD_NAV_LAND  = 2,
      eHUD_NAV_TAXI  = 3,
      eHUD_FUEL      = 4,
      eHUD_ENG       = 5,
      eHUD_GND       = 6,
      eHUD_GUN       = 7,
      eHUD_MSL       = 8,
      eHUD_TEST      = 9,
      eHUD_CONTROLS  = 10,
      eHUD_WEAPON    = 11
   };

   enum eMfdMode
   {
      eMFD_RADAR    = 0,
      eMFD_RWR      = 1,
      eMFD_IRST     = 2,
      eMFD_FLIR     = 3,
      eMFD_DATALINK = 4,
      eMFD_WEAPONS  = 5,
      eMFD_FUEL     = 6,
      eMFD_ENGINES  = 7
   };

   // This is the main data structure used in the plug-in
   struct PlatformData
   {
      PlatformData() = default;

      PlatformData(const PlatformData& aSrc) = default;

      double simTime                                  = 0.0;
      double heading_deg                              = 0.0;
      double pitch_deg                                = 0.0;
      double roll_deg                                 = 0.0;
      double yawRate_dps                              = 0.0;
      double pitchRate_dps                            = 0.0;
      double rollRate_dps                             = 0.0;
      double alpha_deg                                = 0.0;
      double beta_deg                                 = 0.0;
      double alphaDot_dps                             = 0.0;
      double betaDot_dps                              = 0.0;
      double lat_deg                                  = 0.0;
      double lon_deg                                  = 0.0;
      double altitudeMSL_ft                           = 0.0;
      double radarAltitude_ft                         = 0.0;
      double verticalSpeed_fpm                        = 0.0;
      double kcas                                     = 0.0;
      double ktas                                     = 0.0;
      double mach                                     = 0.0;
      double internalTotalFuelTankCapacity_lbs        = 0.0;
      double totalFuelTankCapacity_lbs                = 0.0;
      double currentInternalTotalFuelTankQuantity_lbs = 0.0;
      double currentTotalFuelTankQuantity_lbs         = 0.0;
      double bingoFuelQuantity_lbs                    = 0.0;
      double currentWeight_lbs                        = 0.0;
      double emptyWeight_lbs                          = 0.0;
      double nx_g                                     = 0.0;
      double ny_g                                     = 0.0;
      double nz_g                                     = 0.0;
      double lat                                      = 0.0;
      double lon                                      = 0.0;

      // Control inputs
      double stickBackControllerPosition           = 0.0;
      double stickRightControllerPosition          = 0.0;
      double rudderRightControllerPosition         = 0.0;
      double throttleMilitaryControllerPosition    = 0.0;
      double throttleAfterburnerControllerPosition = 0.0;
      double speedBrakesControllerPosition         = 0.0;
      double landingGearControllerPosition         = 0.0;
      double flapsControllerPosition               = 0.0;
      double spoilersControllerPosition            = 0.0;

      // Flight control surfaces
      double stabLeft_deg      = 0.0;
      double stabRight_deg     = 0.0;
      double rudder_deg        = 0.0;
      double rudderLft_deg     = 0.0;
      double rudderRgt_deg     = 0.0;
      double aileronLft_deg    = 0.0;
      double aileronRgt_deg    = 0.0;
      double flapLft_deg       = 0.0;
      double flapRgt_deg       = 0.0;
      double spoilerLft_deg    = 0.0;
      double spoilerRgt_deg    = 0.0;
      double landingGear_deg   = 0.0;
      double speedBrakes_deg   = 0.0;
      double noseWheel_deg     = 0.0;
      double nws_angle_deg     = 0.0;
      double wheelBrakeRight   = 0.0;
      double wheelBrakeLeft    = 0.0;
      bool   twinRuddersValid  = false;
      bool   singleRudderValid = false;
      bool   aileronsValid     = false;
      bool   flapsValid        = false;
      bool   spoilersValid     = false;
      bool   speedbrakesValid  = false;

      // Misc controls - Outputs
      bool triggerPulled                  = false; // Planned improvement: trigger and button are obsolete,
      bool buttonPressed                  = false; // so replace them as needed
      bool weaponSelectTogglePressed      = false;
      bool dispenseChaffPressed           = false;
      bool dispenseFlaresPressed          = false;
      bool dispenseCountermeasuresPressed = false;
      bool jettisonExternalTanksPressed   = false;

      bool ecmPower = false;
      bool ecmAuto  = false;

      // Misc controls - Local
      bool tgtDesignationLft = false;
      bool tgtDesignationRgt = false;
      bool tgtDesignationUp  = false;
      bool tgtDesignationDn  = false;

      // Misc controls - Inputs
      bool ecmXmit = false;

      std::string radarMode;
      std::string rwrMode;
      std::string irstMode;
      std::string flirMode;

      // Weapons data
      std::vector<WeaponData> weaponList;

      // This is the weapon-type name
      std::string activeWeaponType;

      // This is the weapon display name including the number of rounds
      std::string activeWeapon;

      int    numGunRnds           = 0;
      bool   shootFlagActive      = false;
      bool   masterArmEnabled     = false;
      int    numChaff             = 0;
      int    numFlares            = 0;
      bool   bingoFlagActive      = false;
      bool   jammingDetected      = false;
      bool   breakX_Flag          = false;
      bool   masterWarningEnabled = false;
      double yawCommandG          = 0.0;
      double pitchCommandG        = 0.0;
      double throttleCommand      = 0.0;
      double rangeMax_nm          = 0.0;
      double rangeNE_nm           = 0.0;
      double rangeMin_nm          = 0.0;
      double rangeCurrent_nm      = 0.0;
      double rangeClosure_kts     = 0.0;
      double timeToIntercept_sec  = 0.0;
      double timeToActive_sec     = 0.0;
      double targetMach           = 0.0;
      double targetaspect_deg     = 0.0;
      double maxG_Allowable       = 0.0;
      double maxG_Available       = 0.0;
      double maxG_Attained        = 0.0;

      // Track data items
      std::vector<TrackData> radarTracks;
      std::vector<TrackData> rwrTracks;
      std::vector<TrackData> irstTracks;
      std::vector<TrackData> flirTracks;
      std::vector<TrackData> masterTracks;

      // Current (primary) target
      TrackData currentTarget;
      bool      currentTargetValid = false;

      // Sensor data
      double radarMinAz_deg   = 0.0;
      double radarMaxAz_deg   = 0.0;
      double radarMinEl_deg   = 0.0;
      double radarMaxEl_deg   = 0.0;
      double radarCuedAz_deg  = 0.0;
      double radarCuedEl_deg  = 0.0;
      double radarMaxRange_nm = 0.0;
      int    radarNumScanBars = 0;
      int    radarCurrentBar  = 0;
      int    radarHighPRF     = 0;

      // MFD data
      eMfdMode mfdLftMode = eMFD_RADAR;
      eMfdMode mfdRgtMode = eMFD_RWR;
      size_t   mfdActive  = 0;

      // Nav data items
      std::vector<WaypointData> waypoints;
      size_t                    numWaypoints          = 0;
      size_t                    currentWaypoint       = 0;
      double                    headingBug_deg        = 0.0;
      bool                      headingBugValid       = false;
      bool                      wayptDataValid        = false;
      double                    distanceToWaypoint_nm = 0.0;
      double                    timeToWaypoint_sec    = 0.0;
      double                    localizerRight_deg    = 0.0;
      bool                      localizerValid        = 0.0;
      double                    glideslopeAbove_deg   = 0.0;
      bool                      glideslopeValid       = false;
      std::string               markerSymbol;
      bool                      markerValid = false;

      // Landing gear items
      bool   nwsEnabled                      = false;
      bool   weightOnWheelsFlag              = false;
      bool   weightOnNoseWheelFlag           = false;
      double noseGearUncompressedLength_ft   = 0.0;
      double noseGearMaximumCompression_ft   = 0.0;
      double noseGearCurrentCompression_ft   = 0.0;
      double noseGearNormalizedBrakingValue  = 0.0;
      double noseGearSteeringAngle_deg       = 0.0;
      bool   noseGearInContactWithGround     = false;
      bool   noseGearMaxCompressionExceeded  = false;
      double leftGearUncompressedLength_ft   = 0.0;
      double leftGearMaximumCompression_ft   = 0.0;
      double leftGearCurrentCompression_ft   = 0.0;
      double leftGearNormalizedBrakingValue  = 0.0;
      double leftGearSteeringAngle_deg       = 0.0;
      bool   leftGearInContactWithGround     = false;
      bool   leftGearMaxCompressionExceeded  = false;
      double rightGearUncompressedLength_ft  = 0.0;
      double rightGearMaximumCompression_ft  = 0.0;
      double rightGearCurrentCompression_ft  = 0.0;
      double rightGearNormalizedBrakingValue = 0.0;
      double rightGearSteeringAngle_deg      = 0.0;
      bool   rightGearInContactWithGround    = false;
      bool   rightGearMaxCompressionExceeded = false;

      // Propulsion items
      double thrustVectorYaw_deg            = 0.0;
      double thrustVectorPitch_deg          = 0.0;
      bool   afterburnerOn                  = false;
      double engineThrottleLeverMilitary    = 0.0;
      double engineThrottleLeverAfterburner = 0.0;
      double engineThrust_lbs               = 0.0;
      double engineFuelFlow_pph             = 0.0;

      // Misc items
      std::string pilotType;
   };

   P6DOF_ControllerDataContainer() = default;
   P6DOF_ControllerDataContainer(const P6DOF_ControllerDataContainer& aSrc);

   // Returns true if platform uses a P6DOF Mover
   bool IsPlatformP6DOF_Entity(const QString& aPlatformName) const;

   // Returns true if the simulation has been initialized
   bool GetSimulationInitialized() const;

   // This is used to initialize data when the simulation is initialized.
   // Parameters are only set if aValue is true.
   void SetSimulationInitialized(bool aValue);

   // This should be called when a platform is added
   void PlatformAdded(const std::string& aPlatform);

   // This should be called when a platform is removed/deleted
   void PlatformDeleted(const std::string& aPlatform);

   // This returns the name of the currently connected platform, or an
   // empty string if no platform is connected
   const std::string& GetConnectedPlatform() const { return mConnectedPlatform; }

   // This sets the name of the connected platform
   void SetConnectedPlatform(const std::string& aPlatformName);

   // This is used to "clear" (or release) the connected platform
   void ReleaseConnectedPlatform();

   // This returns true if the connection has been disconnected/destroyed
   bool GetConnectionDestroyed() const { return mConnectionDestroyed; };

   // This returns a const reference to the platform data
   const PlatformData& GetPlatformData() const { return mPlatformData; }

   // This replaces the current platform data with the specified data
   void SetPlatformData(const PlatformData& aPlatformData);

private:
   bool              mSimulationInitialized = false;
   bool              mConnectionDestroyed   = false;
   std::string       mConnectedPlatform;
   PlatformData      mPlatformData;
   std::set<QString> mSetOfP6DofMovers; // Set of entities that have P6Dof Movers
};
} // namespace WkP6DOF_Controller

#endif
