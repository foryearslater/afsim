// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2016 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef WKPREFERENCESDIALOG_HPP
#define WKPREFERENCESDIALOG_HPP

#include "UtQtMemory.hpp"
#include "WkNetworkPrefWidget.hpp"
#include "WkSimPrefWidget.hpp"
#include "WkTrackVisibilityPrefWidget.hpp"
#include "WkfPreferencesDialog.hpp"
#include "visibility/WkfTypeVisibilityPrefWidget.hpp"

namespace warlock
{
class PreferencesDialog : public wkf::PreferencesDialog
{
   Q_OBJECT

public:
   PreferencesDialog(wkf::MainWindow* parent, Qt::WindowFlags f = Qt::WindowFlags())
      : wkf::PreferencesDialog(parent, f)
   {
      mStandardWidgets << ut::qt::make_qt_ptr<SimPrefWidget>(this);
      mStandardWidgets << ut::qt::make_qt_ptr<TrackVisibilityPrefWidget>(this);
      mStandardWidgets << ut::qt::make_qt_ptr<wkf::TypeVisibilityPrefWidget>(this);
      mStandardWidgets << ut::qt::make_qt_ptr<warlock::net::PrefWidget>(this);
   }
};
} // namespace warlock
#endif
