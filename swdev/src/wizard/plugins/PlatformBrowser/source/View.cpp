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
#include "Item.hpp"
#include "Model.hpp"

PlatformBrowser::View::View(QWidget* aParent)
   : QTreeView(aParent)
   , mParentPtr(nullptr)
   , mNeedsFilterUpdate(false)
   , mFilteredPlatforms(0)
{
}

void PlatformBrowser::View::contextMenuEvent(QContextMenuEvent* event)
{
   mParentPtr->ContextEvent(currentIndex(), event->globalPos());
}


void PlatformBrowser::View::FilterUpdate()
{
   if (mNeedsFilterUpdate)
   {
      mNeedsFilterUpdate = false;
      Model* modelPtr    = static_cast<Model*>(model());
      mFilteredPlatforms = 0;
      FilterUpdateP(modelPtr->GetRootItem(), true);
   }
}

bool PlatformBrowser::View::ItemMatchesFilter(Item* aItemPtr)
{
   if (!aItemPtr)
   {
      return true;
   }
   if (aItemPtr->GetAbstractItemParent() == 0)
   {
      return false;
   } // never filter the root

   return mSearchRegex.isEmpty() || (-1 != mSearchRegex.indexIn(aItemPtr->mName.c_str()));
}

bool PlatformBrowser::View::FilterUpdateP(Item* aItemPtr, bool aParentCanHide)
{
   if (!aItemPtr)
   {
      return true;
   }

   bool thisCanHide        = aParentCanHide && !ItemMatchesFilter(aItemPtr);
   bool someChildrenHiding = false;
   bool childrenShowing    = false;
   for (int i = 0; i < aItemPtr->GetAbstractItemCount(); ++i)
   {
      bool childCanHide = FilterUpdateP(static_cast<Item*>(aItemPtr->GetAbstractItem(i)), thisCanHide);
      childrenShowing |= !childCanHide;
      someChildrenHiding |= childCanHide;
   }
   thisCanHide &= !childrenShowing;

   Model*      modelPtr = static_cast<Model*>(model());
   QModelIndex idx      = modelPtr->GetRowIndex(*aItemPtr);
   setRowHidden(idx.row(), idx.parent(), thisCanHide);

   // If some but not all children are shown, go through and make all children show
   //    (two passes are needed)
   if (!thisCanHide && someChildrenHiding && aItemPtr->GetNodeType() == Item::NodeType::cPLATFORM)
   {
      for (int i = 0; i < aItemPtr->GetAbstractItemCount(); ++i)
      {
         FilterUpdateP(static_cast<Item*>(aItemPtr->GetAbstractItem(i)), false);
      }
   }

   if (thisCanHide && (modelPtr->GetRootItem() == aItemPtr))
   {
      ++mFilteredPlatforms;
   }

   return thisCanHide;
}

void PlatformBrowser::View::FilterUpdateLater()
{
   if (!mNeedsFilterUpdate)
   {
      mNeedsFilterUpdate = true;
      QTimer::singleShot(100, this, SLOT(FilterUpdate()));
   }
}
