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

#ifndef JOYSTICKINTERFACE_HPP
#define JOYSTICKINTERFACE_HPP

#include <QString>

#include "JoystickSimEvents.hpp"

namespace Joystick
{
class SimInterface : public warlock::SimInterfaceT<WkJoystickEvent>
{
public:
   explicit SimInterface(const QString& aPluginName);

   /** Get the sim time. */
   const double& GetSimTime() const { return mSimTime; }

protected:
   /** This slot is triggered when the simulation initializes.
    * @param aSimulation reference to the simulation initializing.
    */
   void SimulationInitializing(const WsfSimulation& aSimulation) override;

   /** This slot is triggered when a platform is added to the scenario.
    * @param aSimTime simtime the platform was added.
    * @param aPlatform reference to the platform being added.
    */
   void PlatformAdded(double aSimTime, WsfPlatform& aPlatform) override;

   /** This slot is triggered when a platform is deleted from the scenario.
    * @param aSimTime simtime the platform was deleted.
    * @param aPlatform reference to the platform being deleted.
    */
   void PlatformDeleted(double aSimTime, const WsfPlatform& aPlatform) override;

   double mSimTime; ///< Simtime, used for the plugin GuiUpdate.  If there is another way to get the simtime in said
                    ///< function, this var can be removed.
};
} // namespace Joystick

#endif // JOYSTICKINTERFACE_HPP
