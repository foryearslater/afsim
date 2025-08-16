// ****************************************************************************
// CUI//REL TO USA ONLY
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2003-2015 The Boeing Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "TableWidget.hpp"

#include <QApplication>
#include <QWidget>

#include "Dialog.hpp"
#include "TableRow.hpp"

Engage::TableWidget::TableWidget()
   : mDialogPtr(nullptr)
{
   setDragEnabled(true);
   acceptDrops();

   foreach (QWidget* widget, QApplication::allWidgets())
   {
      Dialog* mwPtr = dynamic_cast<Dialog*>(widget);
      if (mwPtr)
      {
         mDialogPtr = mwPtr;
         break;
      }
   }
}

Engage::TableWidget::TableWidget(QWidget* aParentPtr)
   : QTableWidget(aParentPtr)
   , mDialogPtr(nullptr)
{
   setDragEnabled(true);
   acceptDrops();

   foreach (QWidget* widget, QApplication::allWidgets())
   {
      Dialog* mwPtr = dynamic_cast<Dialog*>(widget);
      if (mwPtr)
      {
         mDialogPtr = mwPtr;
         break;
      }
   }
}

void Engage::TableWidget::AddRow(Dialog* aDialogPtr)
{
   TableRow* newRow = new TableRow(this, aDialogPtr);
   mTableRows.push_back(newRow);
}
