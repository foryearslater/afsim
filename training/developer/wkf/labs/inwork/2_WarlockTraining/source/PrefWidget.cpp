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

#include "PrefWidget.hpp"

WarlockTraining::PrefWidget::PrefWidget(QWidget* aParent /*= 0*/)
   : wkf::PrefWidgetT<PrefObject>(aParent)
{
   //! Without this line, nothing will show up in the preferences widget.
   mUI.setupUi(this);
}

void WarlockTraining::PrefWidget::ReadPreferenceData(const PrefData& aPrefData)
{
   // EXERCISE 2 TASK 2a
   // Call setChecked() on altitudeCheckBox & headingCheckBox using the data in aPrefData
   // PLACE YOUR CODE HERE
}

void WarlockTraining::PrefWidget::WritePreferenceData(PrefData& aPrefData)
{
   // EXERCISE 2 TASK 2b
   // Set aPrefData based on the altitude and heading checkBoxes
   // PLACE YOUR CODE HERE
}
