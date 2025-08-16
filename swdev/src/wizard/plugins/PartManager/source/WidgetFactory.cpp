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

#include "WidgetFactory.hpp"

#include <QVariant>

#include "ArticulatedPart.hpp"
#include "Platform.hpp"
#include "PlatformPart.hpp"
#include "Sensor.hpp"
#include "Widget.hpp"

//! The name of the widget property where the proxy node type name will be saved.
//! This will allow checking what the form's "type" is.
const char* const PartManager::WidgetFactory::widgetTypeProperty = "WidgetType";

PartManager::WidgetFactory::WidgetFactory()
{
   mCreateUiFuncMap["ArticulatedPart"] = &WidgetFactory::CreateArticulatedPartUi;
   mCreateUiFuncMap["Platform"]        = &WidgetFactory::CreatePlatformUi;
   mCreateUiFuncMap["PlatformPart"]    = &WidgetFactory::CreatePlatformPartUi;
   mCreateUiFuncMap["Sensor"]          = &WidgetFactory::CreateSensorUi;
}

//! The main factory method for creating widgets that represent proxy nodes.
PartManager::Widget* PartManager::WidgetFactory::CreateUi(const WsfPProxyNode& proxyNode, bool isTopLevel, QWidget* parent)
{
   Widget*                ui             = nullptr;
   const UiTypeFuncPair   uiTypeFuncPair = FindCreateUiFunc(proxyNode);
   const QString&         formTypeName   = uiTypeFuncPair.first;
   const CreateUiFuncType createUiFunc   = uiTypeFuncPair.second;

   if (createUiFunc != nullptr)
   {
      ui = (this->*createUiFunc)(proxyNode, parent);
      ui->SetupUi(isTopLevel);

      // Save the form type as a widget property.
      ui->setProperty(widgetTypeProperty, QVariant(formTypeName));
   }

   return ui;
}

//! Returns whether or not the factory has a GUI for representing the given proxy node.
bool PartManager::WidgetFactory::HasUiFor(const WsfPProxyNode& proxyNode)
{
   const UiTypeFuncPair uiTypeFuncPair = FindCreateUiFunc(proxyNode);

   return (uiTypeFuncPair.second != nullptr);
}

//! Returns whether or not the factory has a GUI for representing the given proxy
//! and proxy path.
bool PartManager::WidgetFactory::HasUiFor(WsfPProxy* proxy, const WsfPProxyPath& proxyPath)
{
   return HasUiFor(WsfPProxyNode(proxy, proxyPath));
}

//! Given a list of types (sorted from most specific to least specific), iterate
//! the list and return the first function that matches a type. The returned
//! function will create a GUI widget specialized for that type. (See the
//! createFooUi functions below.)
PartManager::WidgetFactory::UiTypeFuncPair PartManager::WidgetFactory::FindCreateUiFunc(const WsfPProxyNode& proxyNode) const
{
   const std::vector<std::string>& typeNames = proxyNode.GetInheritedTypeNames();
   QString                         formTypeName;
   CreateUiFuncType                createUiFunc = nullptr;

   for (auto&& it : typeNames)
   {
      UiFuncMapType::const_iterator funcIter = mCreateUiFuncMap.find(it);
      if (funcIter != mCreateUiFuncMap.end())
      {
         formTypeName = QString::fromStdString(funcIter->first);
         createUiFunc = funcIter->second;
         break;
      }
   }

   return UiTypeFuncPair(formTypeName, createUiFunc);
}

//! Creates a GUI for representing an articulated part.
PartManager::Widget* PartManager::WidgetFactory::CreateArticulatedPartUi(const WsfPProxyNode& proxyNode, QWidget* parent)
{
   return new ArticulatedPart(proxyNode, parent);
}

//! Creates a GUI for representing a platform.
PartManager::Widget* PartManager::WidgetFactory::CreatePlatformUi(const WsfPProxyNode& proxyNode, QWidget* parent)
{
   return new Platform(proxyNode, parent);
}

//! Creates a GUI for representing a platform part.
PartManager::Widget* PartManager::WidgetFactory::CreatePlatformPartUi(const WsfPProxyNode& proxyNode, QWidget* parent)
{
   return new PlatformPart(proxyNode, parent);
}

//! Creates a GUI for representing a sensor.
PartManager::Widget* PartManager::WidgetFactory::CreateSensorUi(const WsfPProxyNode& proxyNode, QWidget* parent)
{
   return new Sensor(proxyNode, parent);
}
