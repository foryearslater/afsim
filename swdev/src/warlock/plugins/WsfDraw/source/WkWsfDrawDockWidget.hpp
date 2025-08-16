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

#ifndef WKWSFDRAWDOCKWIDGET_HPP
#define WKWSFDRAWDOCKWIDGET_HPP

#include "warlock_core_export.h"

#include "ui_WkfWsfDrawDockWidget.h"
#include "wsf_draw/WkfWsfDrawDockWidget.hpp"

namespace warlock
{
class WsfDrawDockWidget : public wkf::WsfDrawDockWidget
{
public:
   WsfDrawDockWidget()
      : wkf::WsfDrawDockWidget()
   {
      mUi.networkCheckBox->show();
   }
};
} // namespace warlock
#endif
