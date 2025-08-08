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

#ifndef WARLOCK_TRAINING_SIMINTERFACE_HPP
#define WARLOCK_TRAINING_SIMINTERFACE_HPP

#include <QObject>
#include <QString>

#include "WkSimInterface.hpp"

#include "SimEvents.hpp"

namespace WarlockTraining
{
   //! This represents the specific simulation interface we are creating.
   //! Inheriting from warlock::SimInterfaceT<T> tells the class what type of
   //! event the interface creates.
   class SimInterface : public warlock::SimInterfaceT<EventBase>
   {
      Q_OBJECT
    
   public:
      SimInterface(const QString& aPluginName);

      //! Creates an UpdateEvent.
      //! This function will be updated every time the simulation clock updates.
      //! @param aSimulation This is the simulation whose clock updated.
      void SimulationClockRead(const WsfSimulation& aSimulation) override;
   };
}

#endif
