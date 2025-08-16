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

#ifndef BROWSER_HPP
#define BROWSER_HPP

#include <QDialog>

#include "WsfPProxyNode.hpp"
#include "ui_Browser.h"

namespace wizard
{
class ProxyChange;
}

namespace PartManager
{
class Widget;

class Browser : public QDialog
{
public:
   Browser(const WsfPProxyNode& aProxyNode, QWidget* aParent = nullptr);
   ~Browser() override = default;

private:
   void OpenLink(const QString& aLink);
   void UpdateHeader(int aIndex);
   void NavigateBack();
   void NavigateFoward();

   Widget*                     CreateWidget(const WsfPProxyNode& aProxyNode);
   void                        AddWidget(Widget* aWidgetPtr);
   void                        AddWidgetAtIndex(int aIndex, Widget* aWidgetPtr);
   void                        UpdateLabel(int aIndex);
   Widget*                     GetWidget(int aStackIndex);
   void                        DeleteCallbacksForIndex(int aIndex);
   void                        DeleteWidgetAtIndex(int aIndex);
   void                        WatchChild(const Widget* aWidgetPtr);
   std::unique_ptr<UtCallback> WatchPath(const WsfPProxyPath& aPath);
   void                        HandleRenamed(const wizard::ProxyChange& aProxyChange);
   void                        MoveCallbacks(const Widget* aFromWidgetPtr, const Widget* aToWidgetPtr);
   bool                        IsInherited(int aIndex);

   Ui::Browser mUi;
   using CallbackPair = std::pair<std::unique_ptr<UtCallback>, std::unique_ptr<UtCallback>>;
   using CallbackMap  = std::map<const Widget*, CallbackPair>;
   CallbackMap mProxyCallbacks;
};
} // namespace PartManager

#endif
