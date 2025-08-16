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

namespace TypeBrowser
{
class DockWidget;

class View : public QTreeView
{
public:
   DockWidget* mParentPtr;
   explicit View(QWidget* aParent);
   void MoveCursor(int aDirection);

protected:
   void contextMenuEvent(QContextMenuEvent* event) override;
   void currentChanged(const QModelIndex& current, const QModelIndex& previous) override
   {
      QTreeView::currentChanged(current, previous);
   }
};
} // namespace TypeBrowser

#endif
