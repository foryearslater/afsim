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

#ifndef PARENTWIDGET_HPP
#define PARENTWIDGET_HPP

#include "Widget.hpp"

namespace PartManager
{
class ParentWidget : public Widget
{
public:
   explicit ParentWidget(const WsfPProxyNode& proxyNode, QWidget* parent = nullptr);

   void         SetupUi(bool isTopLevel) override;
   virtual void HandleProxyChanged(const wizard::ProxyChange& proxyChange);
   void         ExpandPath(const WsfPProxyPath& path) override;

protected:
   void InitChildren(const WsfPM_ObjectMap& partMap);
   void InitChild(const WsfPProxyNode& proxyNode);

   //! In this function, derived classes must use initChildren(...) and/or
   //! initChild(...) to define which children to represent in the GUI.
   virtual void InitAllChildren() = 0;

private:
   typedef std::map<WsfPProxyPath, Widget*> ChildrenMap;
   typedef std::vector<WsfPProxyPath>       ChildrenPaths;

   void AddChild(const WsfPProxyNode& proxyNode);
   void DeleteChild(const WsfPProxyNode& proxyNode);
   void AddChildWidget(Widget* childWidget, int row);
   void DeleteChildWidget(Widget* childWidget);
   void AddChildAtRow(const WsfPProxyNode& proxyNode, int row);

   ChildrenPaths    mChildrenPaths;
   ChildrenMap      mChildrenMap;
   UtCallbackHolder mProxyCallbacks;
};
} // namespace PartManager

#endif
