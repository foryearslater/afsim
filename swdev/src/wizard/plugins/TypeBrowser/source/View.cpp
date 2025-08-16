// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2003-2015 The Boeing Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "View.hpp"

#include <QContextMenuEvent>

#include "DockWidget.hpp"

TypeBrowser::View::View(QWidget* aParent)
   : QTreeView(aParent)
   , mParentPtr()
{
}

void TypeBrowser::View::MoveCursor(int aDirection)
{
   QModelIndex idx   = currentIndex();
   bool        moved = false;
   if (aDirection == -2)
   {
      moved      = true;
      idx        = moveCursor(MoveHome, Qt::NoModifier);
      aDirection = 1;
   }

   while (true)
   {
      if (moved && (!idx.isValid() || mParentPtr->IsFilteredItem(idx)))
      {
         setCurrentIndex(idx);
         break;
      }
      QModelIndex newIdx;
      switch (aDirection)
      {
      case -1:
         newIdx = moveCursor(MoveUp, Qt::NoModifier);
         break;
      case 1:
         newIdx = moveCursor(MoveDown, Qt::NoModifier);
         break;
      }
      if (newIdx == idx)
         break;
      moved = true;
      idx   = newIdx;
      setCurrentIndex(idx);
   }
}

void TypeBrowser::View::contextMenuEvent(QContextMenuEvent* event)
{
   mParentPtr->ContextEvent(currentIndex(), event->globalPos());
}
