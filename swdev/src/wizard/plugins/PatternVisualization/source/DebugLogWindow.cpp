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

#include "DebugLogWindow.hpp"

#include "DebugLogModel.hpp"
#include "UtMemory.hpp"

namespace PatternVisualizer
{

DebugLogWindow::DebugLogWindow(QWidget* aParent)
   : QWidget{aParent, Qt::Window}
   , mDebugLogModel{ut::make_unique<DebugLogModel>()}
{
   mUI.setupUi(this);

   mUI.logTableView->setModel(mDebugLogModel.get());

   mUI.logTableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents); // Time
   mUI.logTableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents); // Category
   mUI.logTableView->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);          // Message
   mUI.logTableView->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents); // File
   mUI.logTableView->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents); // Line
}

DebugLogWindow::~DebugLogWindow() {}

void DebugLogWindow::hideEvent(QHideEvent* aEvent)
{
   QWidget::hideEvent(aEvent);
   emit VisibilityChanged();
}

void DebugLogWindow::showEvent(QShowEvent* aEvent)
{
   mUI.logTableView->resizeRowsToContents();
   QWidget::showEvent(aEvent);
   emit VisibilityChanged();
}

} // namespace PatternVisualizer
