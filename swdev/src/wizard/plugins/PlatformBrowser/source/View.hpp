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

#ifndef VIEW_HPP
#define VIEW_HPP

#include <QTreeView>

namespace PlatformBrowser
{

class DockWidget;
class Item;

//! Platform Browser view.
class View : public QTreeView
{
   Q_OBJECT

public:
   explicit View(QWidget* aParent);
   void MoveCursor(int aDirection);

   void FilterUpdateLater();

   DockWidget* mParentPtr;
   QRegExp     mSearchRegex;
   bool        mNeedsFilterUpdate;
   int         mFilteredPlatforms;

protected slots:

   void FilterUpdate();

protected:
   bool FilterUpdateP(Item* aItemPtr, bool aParentCanHide);
   bool ItemMatchesFilter(Item* aItemPtr);

   void contextMenuEvent(QContextMenuEvent* event) override;
   void currentChanged(const QModelIndex& current, const QModelIndex& previous) override
   {
      QTreeView::currentChanged(current, previous);
   }
};

} // namespace PlatformBrowser

#endif
