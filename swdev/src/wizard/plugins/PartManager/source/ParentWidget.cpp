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

#include "ParentWidget.hpp"

#include <QFormLayout>

#include "ProxyChange.hpp"
#include "ProxyWatcher.hpp"
#include "WidgetFactory.hpp"

PartManager::ParentWidget::ParentWidget(const WsfPProxyNode& proxyNode, QWidget* parent /*= nullptr*/)
   : Widget(proxyNode, parent)
{
}

//! Initializes the user interface and children widgets. This will include
//! connecting to the correct proxy signals for maintaining the children nodes.
void PartManager::ParentWidget::SetupUi(bool isTopLevel)
{
   Widget::SetupUi(isTopLevel);
   InitAllChildren();
}

//! Handles additions, deletions, type changes, and renames of children nodes.
void PartManager::ParentWidget::HandleProxyChanged(const wizard::ProxyChange& proxyChange)
{
   const WsfPProxyNode& changedNode = proxyChange.changed();

   switch (proxyChange.reason())
   {
   case wizard::ProxyChange::cADDED:
      AddChild(changedNode);
      break;

   case wizard::ProxyChange::cREMOVED:
      DeleteChild(changedNode);
      break;

      // Type changes.
   case wizard::ProxyChange::cUPDATED:
      DeleteChild(changedNode);
      AddChild(changedNode);
      break;

   case wizard::ProxyChange::cRENAMED:
   default:
      break;
   }
}

//! This function takes a path and expands its children according to the path,
//! highlighting the expanded widgets along the way.
void PartManager::ParentWidget::ExpandPath(const WsfPProxyPath& path)
{
   Widget::ExpandPath(path);
   for (auto&& it : mChildrenPaths)
   {
      if (it.IsPrefixOf(path))
      {
         Widget* childWidget = mChildrenMap[it];
         childWidget->SetContentVisible(true);
         childWidget->ExpandPath(path);
         break;
      }
   }
}

//! Iterates an object map, adding a widget for each element in the map. We
//! also connect to signals that pertain to the map and its children.
void PartManager::ParentWidget::InitChildren(const WsfPM_ObjectMap& partMap)
{
   if (partMap.GetObjectMap() == nullptr)
      return;
   for (WsfPM_ObjectMap::iterator i = partMap.begin(); i != partMap.end(); ++i)
   {
      const WsfPProxyNode proxyNode = partMap.At(i);
      AddChild(proxyNode);
   }

   mProxyCallbacks += wizard::WatchProxy(partMap.GetPath()).ChildUpdate.Connect(&ParentWidget::HandleProxyChanged, this);
}

//! Adds a map element and connects to signals that pertain to its map. This is
//! only for special things like Mover and Fuel. A platform, for example, only
//! has one mover, but it is kept in a map like everything else for uniformity.
void PartManager::ParentWidget::InitChild(const WsfPProxyNode& proxyNode)
{
   if (proxyNode.IsValid())
   {
      AddChild(proxyNode);
   }

   mProxyCallbacks +=
      wizard::WatchProxy(proxyNode.GetParent().GetPath()).ChildUpdate.Connect(&ParentWidget::HandleProxyChanged, this);
}

//! Adds a new child widget to represent the given child proxy node.
void PartManager::ParentWidget::AddChild(const WsfPProxyNode& proxyNode)
{
   const WsfPProxyPath& newChildPath = proxyNode.GetPath();
   size_t               insertRow    = 0;

   while ((insertRow < mChildrenPaths.size()) && (newChildPath >= mChildrenPaths[insertRow]))
   {
      insertRow++;
   }

   AddChildAtRow(proxyNode, static_cast<int>(insertRow));
}

//! Deletes the child widget pertaining to the given child proxy node.
void PartManager::ParentWidget::DeleteChild(const WsfPProxyNode& proxyNode)
{
   const WsfPProxyPath& deletedPath = proxyNode.GetPath();

   for (size_t i = 0; i < mChildrenPaths.size(); i++)
   {
      if (mChildrenPaths[i] == deletedPath)
      {
         Widget* childWidget = mChildrenMap[deletedPath];
         DeleteChildWidget(childWidget);
         mChildrenMap.erase(deletedPath);
         mChildrenPaths.erase(mChildrenPaths.begin() + i);
         break;
      }
   }
}

//! Adds a given child widget to the form layout at the specified row.
void PartManager::ParentWidget::AddChildWidget(Widget* childWidget, int row)
{
   ContentLayout()->insertRow(static_cast<int>(NumOfAttributes() + row), childWidget);
}

//! Removes the given child widget from the form layout and deletes it.
void PartManager::ParentWidget::DeleteChildWidget(Widget* childWidget)
{
   ContentLayout()->removeWidget(childWidget);
   childWidget->deleteLater();
}

//! Creates a widget (with the factory) for a given proxy node and adds it to
//! the form layout at the specified row.
void PartManager::ParentWidget::AddChildAtRow(const WsfPProxyNode& proxyNode, int row)
{
   const WsfPProxyPath& newChildPath   = proxyNode.GetPath();
   Widget*              newChildWidget = WidgetFactory().CreateUi(proxyNode, false, this);

   if (newChildWidget)
   {
      mChildrenPaths.insert(mChildrenPaths.begin() + row, newChildPath);
      mChildrenMap[newChildPath] = newChildWidget;
      AddChildWidget(newChildWidget, row);
      connect(newChildWidget, &Widget::LinkActivated, this, &ParentWidget::PropagateLinkActivated);
   }
}
