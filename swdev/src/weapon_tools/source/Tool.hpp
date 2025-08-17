// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2003-2015 The Boeing Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef TOOL_HPP
#define TOOL_HPP

#include "WeaponToolsExport.hpp"

class UtInput;
#include "WsfExplicitWeapon.hpp"
class WsfGuidanceComputer;
class WsfPlatform;
#include "WeaponObserver.hpp"
#include "WsfObject.hpp"
#include "WsfStringId.hpp"
class ToolManager;

//! Tool is a base class for a processor that does some utility function and reports when it is done.
//! Tool is a processor (similar to WsfProcessor, but distinct from it, since it does not reside on a
//! WsfPlatform) that does some utility function.  The look and feel of this utility is modeled upon the
//! WSF core framework.  The intent is that a single "main" function will instantiate and control one or
//! more Tools, and carry out a WsfSimulation, advancing time until each of the instantiated Tools have
//! reported that they are done.  The earliest use of this class is in repeatedly firing explicit weapons
//! of interest, under completely controlled engagement conditions, to gather metrics that quantify the
//! weapon's performance and behavior (defining lethality tables, Launch Acceptable Regions (LARs), times
//! of flight, etc.).  The input stream (see ProcessInput()) will specify where the Tool activity takes
//! place (in latitude, longitude), the explicit weapon type to experiment upon, and other necessary
//! information.

class WT_EXPORT Tool : public WsfObject
{
public:
   Tool(const Tool& aSrc);
   Tool& operator=(const Tool&) = delete;
   ~Tool() override;
   Tool*        Clone() const override = 0;
   virtual void Update(double aSimTime);
   virtual bool Initialize(WsfSimulation& aSimulation);
   bool         ProcessInput(UtInput& aInput) override;

   //! name Callbacks from WeaponObserver (lets the tool have access to simulation callbacks)
   //@{
   virtual void MoverBurnout(double aSimTime, WsfMover* aMoverPtr);
   virtual void GuidanceComputerPhaseChanged(double aSimTime, WsfGuidanceComputer* aComputerPtr);
   virtual void PlatformAdded(double aSimTime, WsfPlatform* aPlatformPtr);
   //@}

   WsfStringId WeaponPlatformTypeId() const;

   double             FrameStep() const { return mFrameStep; }
   bool               IsInitialized() const { return mInitialized; }
   WsfPlatform*       GetLauncherPlatform() const { return mLaunchPlatPtr; }
   WsfExplicitWeapon* GetLauncherWeapon() const { return mLaunchWpnPtr; }

protected:
   WsfSimulation* GetSimulation() const { return mSimulationPtr; }
   WsfScenario&   GetScenario() const { return *mScenarioPtr; }
   ToolManager*   GetManager() const;
   Tool(WsfScenario& aScenario);

   void UnInitialize();

   //! Return a flag to indicate that this Tool has completed its work.
   bool IsDone() const { return mDone; }

   //! Return a flag to indicate that debugging is enabled.
   bool DebugEnabled() const { return mDebug; }

   //! Method called to determine what the output objects and file names are called.
   virtual void ResolveNames();

   //! Method called to fire a new weapon.  Pure virtual base method must be implemented in derived classes.
   //!   @param aSimTime The current simulated time.
   virtual bool FireNewWeapon(double aSimTime) = 0;

   virtual void ErrorBailout(){};

   WsfScenario*       mScenarioPtr;
   WsfSimulation*     mSimulationPtr;
   WeaponObserver     mObserver;
   WsfPlatform*       mLaunchPlatPtr;
   WsfExplicitWeapon* mLaunchWpnPtr;

   double mFrameStep;
   double mLastUpdate;
   double mLatDeg;
   double mLonDeg;
   double mAlt;
   double mHeadingRad;

   bool mDone;
   bool mNotifiedDone;
   bool mInitialized;
   bool mDebug;
   bool mTerminateOnLaunchFailure;

   std::string mToolProduces;
   std::string mFileExtension;
   WsfStringId mWeaponEffectTypeId;
   WsfStringId mLaunchPlatformTypeId;
   WsfStringId mTargetPlatformTypeId;
   WsfStringId mWeaponNameId;
   std::string mOutputFileName;
   std::string mOutputObjectName;
};

#endif
