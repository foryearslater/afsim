// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2003-2013 The Boeing Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
// ****************************************************************************
// Updated by Infoscitex, a DCS Company.
// ****************************************************************************

#ifndef MATLABBALLISTICMOVER_HPP
#define MATLABBALLISTICMOVER_HPP

#ifndef NOMINMAX
#define NOMINMAX
#endif

// Base class
#include "mover/WsfMover.hpp"

// MATLAB generated
#include "libAFSIM_Mover.h"

// Forward declarations
class UtInput;

//! This class represents a ballistic mover.
class MATLABBallisticMover : public WsfMover
{
   public:

      //! Constructor
      explicit MATLABBallisticMover(WsfScenario& aScenario);

      MATLABBallisticMover& operator=(const MATLABBallisticMover&) = delete;

      //! Virtual destructor
      ~MATLABBallisticMover() noexcept override = default;

      //! Clone this object.
      //! @return a pointer to the new object.
      WsfMover* Clone() const override;

      //! Initialize this mover.
      bool Initialize(double aSimTime) override;

      //! Process the input block for this mover.
      bool ProcessInput(UtInput& aInput) override;

      //! Simulation time has advanced; update this mover.
      void Update(double aSimTime) override;

   protected:

      //! A stage represents one stage in the vehicle.
      class Stage
      {
         public:
            Stage();

            bool ProcessInput(UtInput& aInput);

            //! Mass of just the fuel.
            double mFuelMass;

            //! Mass of the whole stage.
            double mTotalMass;

            //! The thrust (N)
            double mThrust;

            //! The engine burn time (sec)
            double mThrustDuration;
      };

      //! Copy Constructor
      MATLABBallisticMover(const MATLABBallisticMover& aSrc);

   private:

      //! @brief  This method returns the hit ground time.
      //!
      //!         This method gets the data from a MATLAB mwArray.
      //! @return Time that the threat missile hits the ground (sec).
      double GetHitGroundTime();

      //! @brief  Updates threat location, velocity, etc. using MATLAB values.
      void UpdatePlatform(double aSimTime);

      //! @brief  Get initial altitude of threat based on input values and terrain.
      void GetInitialAltitude();

      //! @brief Fills the MATLAB mwArray using input values.
      void GetBoosterParams();

      //! @brief Used to output data when debugging.
      void OutputDiagnostic(double aSimTime);

      // Input to MATLAB functions
      mwArray                 mwInputLLA;            //! Initial threat (latitude, longitude, altitude) (size=3)
      mwArray                 mwInputOrientation;    //! Initial threat orientation (heading, pitch, roll) (deg) (size=3)
      mwArray                 mwInputTime;           //! Simulation time (sec) (size=1)
      mwArray                 mwInputBoosterParams;  //!< Threat booster parameters (size=18)
                                                     //!<  1) 1st stage burn time (sec)
                                                     //!<  2) 2nd stage burn time (sec)
                                                     //!<  3) 3rd stage burn time (sec)
                                                     //!<  4) Zero-lift drag coefficient (C_d0)
                                                     //!<  5) Aero reference area (m^2)
                                                     //!<  6) 1st stage mass (kg)
                                                     //!<  7) 2nd stage mass (kg)
                                                     //!<  8) 3rd stage mass (kg)
                                                     //!<  9) 1st stage fuel mass (kg)
                                                     //!< 10) 2nd stage fuel mass (kg)
                                                     //!< 11) 3rd stage fuel mass (kg)
                                                     //!< 12) Payload mass (kg)
                                                     //!< 13) Maximum dynamic pressure (kg/m-s^2)
                                                     //!< 14) Time it takes to pitch off vertical (sec)
                                                     //!< 15) 1st stage thrust (N)
                                                     //!< 16) 2nd stage thrust (N)
                                                     //!< 17) 3rd stage thrust (N)
                                                     //!< 18) Time vehicle remains vertical at launch (sec)

      // Values from AFNES input files
      double                  mInputAlt;             //! Altitude (m)
      bool                    mInputAltAGL;          //! True if mInputAlt is AGL not MSL
      std::vector<Stage>      mStageList;            //!< The list of stages
      bool                    mExplicitStageUsed;    //!< 'true' if a 'stage' command was used
      bool                    mImplicitStageUsed;    //!< 'true' if a stage was defined without using 'stage'

      // Booster parameters from AFNES input files
      double                  mCd0;                  //! Zero-lift drag coefficient (C_d0)
      double                  mReferenceArea;        //! Aero reference area (m^2)
      double                  mMassPayload;          //! Mass of the payload (kg)
      double                  mMaxq;                 //! Max dynamic pressure (kg/m-s^2)
      double                  mPitchInterval;        //! Time it takes to pitch off vertical (sec)
      double                  mVerticalTime;         //! Time vehicle remains vertical at launch (sec)

      // Output from MATLAB functions
      mwArray                 mwHitGroundTime;       //! Time the threat hits the ground (sec) (size=1)
      mwArray                 mwState;               //!< Threat state vector (size=22)
                                                     //!<  1) ECI pos x (km)
                                                     //!<  2) ECI vel x (km/s)
                                                     //!<  3) ECI pos y (km)
                                                     //!<  4) ECI vel y (km/s)
                                                     //!<  5) ECI pos z (km)
                                                     //!<  6) ECI vel z (km/s)
                                                     //!<  7) Total mass (kg)
                                                     //!<  8) Latitude
                                                     //!<  9) Longitude
                                                     //!< 10) Altitude (km)
                                                     //!< 11) Great circle range (km)
                                                     //!< 12) Thrust magnitude
                                                     //!< 13) Gravity magnitude
                                                     //!< 14) Drag magnitude
                                                     //!< 15) alpha
                                                     //!< 16) AOA
                                                     //!< 17) ECEF pos x (km)
                                                     //!< 18) ECEF pos y (km)
                                                     //!< 19) ECEF pos z (km)
                                                     //!< 20) ECEF vel x (km/s)
                                                     //!< 21) ECEF vel y (km/s)
                                                     //!< 22) ECEF vel z (km/s)
                                                     //!< 23) Time
                                                     //!< 24) Stage

      static const int        cBOOSTER_PARAMS_SIZE;  //! Size of the booster parameter mwArray.
      static const int        cSTATE_VECTOR_SIZE;    //! Size of the state vector mwArray.
};

#endif