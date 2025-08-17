// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2016 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
// ****************************************************************************
// Updated by Infoscitex, a DCS Company
// ****************************************************************************

#ifndef ORBITALLAUNCHCOMPUTERGENERATOR_HPP
#define ORBITALLAUNCHCOMPUTERGENERATOR_HPP

#include <list>
#include <string>
#include <vector>

#include "Tool.hpp"
#include "WeaponToolsExport.hpp"
#include "WsfOrbitalLaunchComputer.hpp"

class WT_EXPORT OrbitalLaunchComputerGenerator : public Tool, public WsfSinglePlatformObserver
{
public:
   using Orbit = WsfOrbitalLaunchComputer::Orbit;

   OrbitalLaunchComputerGenerator(WsfScenario& aScenario);
   OrbitalLaunchComputerGenerator(const OrbitalLaunchComputerGenerator& aSrc);
   OrbitalLaunchComputerGenerator& operator=(const OrbitalLaunchComputerGenerator&) = delete;
   ~OrbitalLaunchComputerGenerator() override                                       = default;

   //! @name weapon_tools 'Tool' methods.
   //@{
   Tool* Clone() const override;
   bool  Initialize(WsfSimulation& aSimulation) override;
   bool  ProcessInput(UtInput& aInput) override;
   void  Update(double aSimTime) override;
   //@}

   //! @name Simulation observers
   //@{
   void MoverBurnout(double aSimTime, WsfMover* aMoverPtr) override;
   void GuidanceComputerPhaseChanged(double aSimTime, WsfGuidanceComputer* aComputerPtr) override;
   void PlatformAdded(double aSimTime, WsfPlatform* aPlatformPtr) override;
   //@}
private:
   void CaptureInsertionData(double aSimTime, WsfPlatform* aPlatformPtr);

   double ComputeFlightPathAngle(WsfPlatform* aPlatformPtr);

   double ComputeOrbitalSpeed(WsfPlatform* aPlatformPtr);

   double ComputeTimeOfFlight(double aTime) const;

   bool FireNewWeapon(double aSimTime) override;

   bool FireNextWeapon(double aSimTime);

   void SummarizePitchRateResults();

   std::string mLEO_DataFileName;

   double mSpeed;
   double mPitch;

   std::vector<double> mLaunchHeadings;
   double              mLaunchHeading;
   size_t              mLaunchHeadingIndex;

   std::vector<double> mLoftAngles;
   double              mLoftAngle; //!< The current loft angle
   size_t              mLoftAngleIndex;

   std::vector<double> mPitchRates;
   double              mPitchRate;
   size_t              mPitchRateIndex;

   Orbit mLEO_Data; //!< Orbit data at insertion for the current launch
   Orbit mBO_Data;  //!< Orbit data at burnout for the current launch

   std::list<Orbit> mLEO_Orbits; //!< Orbit data at insertion for all completed launches
   std::list<Orbit> mBO_Orbits;  //!< Orbit data at burnout for all completed launches

   std::string mPlatformName; //!< Name of launched platform (needed after deleted)
};

#endif
