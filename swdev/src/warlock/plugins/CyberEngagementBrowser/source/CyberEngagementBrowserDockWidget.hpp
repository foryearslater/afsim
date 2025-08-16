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

#ifndef CYBERENGAGEMENTBROWSERDOCKWIDGET_HPP
#define CYBERENGAGEMENTBROWSERDOCKWIDGET_HPP

#include <QDockWidget>

#include "CyberEngagementBrowserDataContainer.hpp"
#include "CyberEngagementBrowserSimInterface.hpp"
#include "ui_CyberEngagementBrowserDockWidget.h"

namespace WkCyberEngagementBrowser
{
class DockWidget : public QDockWidget
{
public:
   DockWidget(SimInterface*   aInterfacePtr,
              DataContainer&  aCyberEngagementBrowserData,
              QWidget*        parent = nullptr,
              Qt::WindowFlags f      = Qt::WindowFlags());

   void ClearTable() { mUI.mEngagementTable->clear(); }
   void ShowContextMenu(const QPoint& pos);
   void Update(bool aPurge, int aPurgeTime);

private:
   void SetColumnHeaders(const QStringList& aHeaders);
   void SetBackgroundColor(QTreeWidgetItem* aItem, QColor aColor);
   void UpdateTable();
   void AddEventToTable(const DataContainer::CyberEvent& aEvent);
   void RemoveEventFromTable(const DataContainer::CyberEvent& aEvent);
   void HideRows();

   Ui::CyberEngagementBrowserDockWidget mUI;
   DataContainer&                       mCyberEngagementBrowserData;
   SimInterface*                        mSimInterfacePtr;
};
} // namespace WkCyberEngagementBrowser

#endif
