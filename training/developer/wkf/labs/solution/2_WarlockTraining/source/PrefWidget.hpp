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

#ifndef PROPERTIES_PREFWIDGET_HPP
#define PROPERTIES_PREFWIDGET_HPP

#include "WkfPrefWidget.hpp"

#include "PrefObject.hpp"

#include "ui_WarlockTrainingPrefWidget.h"

namespace WarlockTraining
{
   //! This represents the specific preferences widget associated with our plugin.
   //! Inheriting from wkf::PrefWidgetT<T> tells the class what type of PrefObject it is dealing with.
   class PrefWidget : public wkf::PrefWidgetT<PrefObject>
   {
   public:
      explicit PrefWidget(QWidget* aParent = nullptr);

   private:
      Ui::WarlockTrainingPrefs mUI;

      //! Updates the widget with the current preferences.
      //! @param aPrefData This is the current preferences.
      void ReadPreferenceData(const PrefData& aPrefData) override;

      //! Writes the preferences shown in the widget to the current preferences.
      //! @param aPrefData This is the current preferences.
      void WritePreferenceData(PrefData& aPrefData) override;
   };
}

#endif
