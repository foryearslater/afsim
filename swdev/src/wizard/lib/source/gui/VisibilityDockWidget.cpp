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

#include "VisibilityDockWidget.hpp"

#include "WkfEnvironment.hpp"
#include "WkfPlugin.hpp"
#include "WkfPluginManager.hpp"
#include "visibility/WkfTypeVisibilityPrefWidget.hpp"

wizard::VisibilityDockWidget::VisibilityDockWidget(QWidget* aParent, Qt::WindowFlags f)
   : wkf::VisibilityDockWidget(aParent, f)
{
   mUI.mComboBox->setDisabled(true);
   mUI.mComboBox->setVisible(false);
   mTypeWidget->setDisabled(true);
   mTypeWidget->setVisible(false);
}
