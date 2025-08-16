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
#ifndef P6DOF_CONTROLLERPREFWIDGET_HPP
#define P6DOF_CONTROLLERPREFWIDGET_HPP

#include "P6DOF_ControllerPrefObject.hpp"
#include "WkfPrefWidget.hpp"

class QRadioButton;
class QTreeWidgetItem;
class QComboBox;

namespace WkP6DOF_Controller
{
// The WkP6DOF_Controller::PrefWidget provides the preferences widget with
// which the users can set their preferences for the P6DofController plug-in.
class PrefWidget : public wkf::PrefWidgetT<PrefObject>
{
public:
   PrefWidget();

   QString GetCategoryHint() const override { return "P6DOF"; }

private:
   void ReadPreferenceData(const PrefData& aPrefData) override;
   void WritePreferenceData(PrefData& aPrefData) override;

   QTreeWidgetItem* mShowHud;
   QTreeWidgetItem* mFirstPerson;
   QTreeWidgetItem* mHDD;
   QRadioButton*    mAutopilot;
   QRadioButton*    mManualPilot;
   QRadioButton*    mSyntheticPilot;
   QRadioButton*    mGuidancePilot;
   QComboBox*       mResolution;
};
} // namespace WkP6DOF_Controller

#endif
