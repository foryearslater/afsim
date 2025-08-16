// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2018 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef WKTRACKVISIBILITYPREFWIDGET_HPP
#define WKTRACKVISIBILITYPREFWIDGET_HPP

#include "warlock_core_export.h"

#include "visibility/WkfTrackVisibilityPrefWidget.hpp"

namespace warlock
{
class WARLOCK_CORE_EXPORT TrackVisibilityPrefWidget : public wkf::TrackVisibilityPrefWidget
{
public:
   TrackVisibilityPrefWidget(QWidget* aParent = nullptr)
      : wkf::TrackVisibilityPrefWidget(aParent)
   {
      mUi.showNetworkedSimulationsTracks->show();
   }
};
} // namespace warlock
#endif
