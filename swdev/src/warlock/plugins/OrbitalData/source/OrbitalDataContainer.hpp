//*****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2021 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
//*****************************************************************************

#ifndef ORBITALDATACONTAINER_HPP
#define ORBITALDATACONTAINER_HPP

#include <set>
#include <string>

namespace WkOrbitalData
{

class DataContainer
{
public:
   DataContainer()  = default;
   ~DataContainer() = default;

   struct OrbitalData
   {
      std::string mName           = "";
      double      mEccentricity   = 0.0; // unitless
      double      mSemiMajorAxis  = 0.0; // length
      double      mInclination    = 0.0; // angle
      double      mRAAN           = 0.0; // angle
      double      mArgOfPeriapsis = 0.0; // angle
      double      mTrueAnomaly    = 0.0; // angle

      bool Valid() const { return !mName.empty(); }
      void Clear() { mName = ""; }
   };

   void SetPlatformOfInterest(const std::string& aPlatformName) { mPlatformOfInterest = aPlatformName; }

   const OrbitalData& GetPlatformData() const { return mData; }
   void               SetPlatformData(const OrbitalData& aData)
   {
      if (aData.mName == mPlatformOfInterest)
      {
         mData = aData;
      }
      else
      {
         mData.Clear();
      }
   }

   void AddOrbitalPlatform(const std::string& aPlatformName) { mOrbitalPlatforms.insert(aPlatformName); }
   void RemoveOrbitalPlatform(const std::string& aPlatformName) { mOrbitalPlatforms.erase(aPlatformName); }
   bool IsOrbitalPlatform(const std::string& aPlatformName) const { return mOrbitalPlatforms.count(aPlatformName) > 0; }
   void ClearPlatforms() { mOrbitalPlatforms.clear(); }

private:
   std::string           mPlatformOfInterest;
   OrbitalData           mData;
   std::set<std::string> mOrbitalPlatforms;
};

} // namespace WkOrbitalData

#endif
