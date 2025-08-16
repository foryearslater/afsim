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

#ifndef WIDGETFACTORY_HPP
#define WIDGETFACTORY_HPP

#include <map>
#include <string>
#include <utility>
#include <vector>

#include <QString>

class QWidget;
class WsfPProxy;
class WsfPProxyNode;
class WsfPProxyPath;

namespace PartManager
{
class Widget;

class WidgetFactory
{
public:
   WidgetFactory();

   Widget* CreateUi(const WsfPProxyNode& proxyNode, bool isTopLevel, QWidget* parent = nullptr);

   bool HasUiFor(const WsfPProxyNode& proxyNode);
   bool HasUiFor(WsfPProxy* proxy, const WsfPProxyPath& proxyPath);

   typedef Widget* (WidgetFactory::*CreateUiFuncType)(const WsfPProxyNode&, QWidget*);
   typedef std::pair<const QString, const CreateUiFuncType> UiTypeFuncPair;

   UiTypeFuncPair FindCreateUiFunc(const WsfPProxyNode& proxyNode) const;

   static const char* const widgetTypeProperty;

private:
   typedef std::map<std::string, CreateUiFuncType> UiFuncMapType;

   //! A map of type names to their respective widget factory methods.
   UiFuncMapType mCreateUiFuncMap;

   // Factory methods.
   Widget* CreateArticulatedPartUi(const WsfPProxyNode& proxyNode, QWidget* parent);
   Widget* CreatePlatformUi(const WsfPProxyNode& proxyNode, QWidget* parent);
   Widget* CreatePlatformPartUi(const WsfPProxyNode& proxyNode, QWidget* parent);
   Widget* CreateSensorUi(const WsfPProxyNode& proxyNode, QWidget* parent);
};
} // namespace PartManager

#endif
