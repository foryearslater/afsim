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

#ifndef P6DOF_DATAPLATFORMDATA_HPP
#define P6DOF_DATAPLATFORMDATA_HPP

#include <set>
#include <string>

namespace WkP6DOF_Data
{

class DataContainer
{
public:
   DataContainer()  = default;
   ~DataContainer() = default;

   struct PlatformData
   {
      std::string name;

      double      altitude_m;
      double      vertSpeed_fpm;
      double      KCAS;
      double      mach;
      double      dynamicPressure_psf;
      double      heading_rad;
      double      pitch_rad;
      double      roll_rad;
      double      nx;
      double      ny;
      double      gLoad;
      double      gAvail;
      double      alpha_rad;
      double      beta_rad;
      double      alphaDot_rps;
      double      betaDot_rps;
      double      yawRate_rps;
      double      pitchRate_rps;
      double      rollRate_rps;
      std::string activePilot;
      double      lift_nt;
      double      drag_nt;
      double      thrust_nt;
      double      fuelFlow_pph;
      double      fuelTotalCapacity_kg;
      double      fuelRemaining_kg;
      double      fuelInternal_kg;
      double      fuelExternal_kg;
      double      stickBack;
      double      stickRight;
      double      rudderRight;
      double      throttle;
      bool        afterburner_on;
      double      speedbrakeCommand;
      double      speedbrakeAngle_rad;
      double      totalWeight_kg;
      bool        weightOnWheels;

      void Clear() { name = ""; }
      bool Valid() const { return !name.empty(); }
   };

   void SetPlatformOfInterest(const std::string& aPlatformName) { mPlatformOfInterest = aPlatformName; }

   const PlatformData& GetPlatformData() const { return mData; }
   void                SetPlatformData(const PlatformData& aData)
   {
      if (aData.name == mPlatformOfInterest)
      {
         mData = aData;
      }
      else
      {
         mData.Clear();
      }
   }

   void AddP6dofPlatform(const std::string& aPlatformName) { mP6dofPlatforms.insert(aPlatformName); }
   void RemoveP6dofPlatform(const std::string& aPlatformName) { mP6dofPlatforms.erase(aPlatformName); }
   bool IsP6dofPlatform(const std::string& aPlatformName) const { return mP6dofPlatforms.count(aPlatformName) > 0; }
   void ClearPlatforms() { mP6dofPlatforms.clear(); }

private:
   std::string           mPlatformOfInterest;
   PlatformData          mData;
   std::set<std::string> mP6dofPlatforms;
};

} // namespace WkP6DOF_Data

#endif
