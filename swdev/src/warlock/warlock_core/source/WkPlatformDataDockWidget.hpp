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
#ifndef WKPLATFORMDATADOCKWIDGET_HPP
#define WKPLATFORMDATADOCKWIDGET_HPP

#include "WkPlugin.hpp"
#include "WkfPlatformDataDockWidget.hpp"
#include "data_ring/WkfAttachmentDataRing.hpp"

namespace warlock
{
class PlatformDataDockWidget : public wkf::PlatformDataDockWidget
{
public:
   PlatformDataDockWidget(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

protected:
   void PlotActionHandler();
   void AddRingActionHandler();
   void RemoveRingActionHandler();

   void BuildMenuForItem(QMenu& aMenu, QTreeWidgetItem& aItem) override;
   void AddRingItem(QMenu& aMenu, QTreeWidgetItem& aItem);

   Plugin*  mContextMenuItemPlugin;
   QString  mContextMenuItemId;
   unsigned mItemContext = 0;
};
} // namespace warlock
#endif
