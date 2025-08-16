// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2017 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#ifndef WkNetworkBrowserDockWidget_HPP
#define WkNetworkBrowserDockWidget_HPP

#include <QDockWidget>

#include "NetworkBrowserDataContainer.hpp"
#include "ui_NetworkBrowserDockWidget.h"

namespace WkNetworkBrowser
{
class DockWidget : public QDockWidget
{
   Q_OBJECT
public:
   DockWidget(const DataContainer& aNetworkData, QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
   ~DockWidget() override = default;

private:
   void Clear();
   void RebuildDisplay();

   const DataContainer& mNetworkData;

   Ui::NetworkBrowserDockWidget mUi;
};
} // namespace WkNetworkBrowser

#endif
