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

#ifndef WARLOCK_TRAINING_DOCKWIDGET_HPP
#define WARLOCK_TRAINING_DOCKWIDGET_HPP

#include <QDockWidget>

#include "WkfPlatform.hpp"

#include "DataContainer.hpp"
#include "PrefObject.hpp"
#include "SimInterface.hpp"

#include "ui_WarlockTrainingDockWidget.h"

namespace WarlockTraining
{
   //! This represents the specific dockable widget associated with our plugin.
   class DockWidget : public QDockWidget
   {
      // This line is required for Qt signals to be properly emitted from this class.
      Q_OBJECT

   public:

      DockWidget(SimInterface&   aSimInterface,
                 DataContainer&  aDataContainer,
                 PrefObject*     aPrefObject,
                 Qt::WindowFlags aWindowFlags = Qt::WindowFlags());

      void TurnToHeading(double aHeading);

   private:

      void PlatformOfInterestChanged(wkf::Platform* aPlatform);
      void PreferencesChanged(const PrefData& aPrefData);
      void UpdateDisplay();

      std::string mPlatformOfInterest;

      SimInterface& mSimInterface;
      DataContainer& mDataContainer;
      //! This contains all of the components that will appear on the screen.
      //! The Ui::WarlockTraining class is generated from the file "SimListenerDockWidget.ui".
      //! Its name comes from the "objectName" field.
      Ui::WarlockTraining mUI;
   };
}

#endif
