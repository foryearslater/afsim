// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2018 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "PartManagerPlugin.hpp"

#include <QMenu>

#include "Browser.hpp"
#include "Signals.hpp"

WKF_PLUGIN_DEFINE_SYMBOLS(PartManager::Plugin, "Part Manager", "Provides a UI to platform parts", "wizard")

PartManager::Plugin::Plugin(const QString& aPluginName, const size_t aUniqueId)
   : wizard::Plugin(aPluginName, aUniqueId)
{
   QObject::connect(wizSignals, &wizard::Signals::ActionsForNodeRequested, this, &Plugin::ActionsForNodeRequested);
}

void PartManager::Plugin::ActionsForNodeRequested(const WsfPProxyNode& aNode, QMenu* aMenuPtr, QWidget* aParentWidgetPtr)
{
   if (aNode.IsOfType("Platform"))
   {
      QObject::connect(aMenuPtr->addAction("Manage Platform Parts..."),
                       &QAction::triggered,
                       ManagePlatformParts(aNode, aParentWidgetPtr));
   }
}

PartManager::Plugin::ManagePlatformParts::ManagePlatformParts(const WsfPProxyNode& aNode,
                                                              QWidget*             aParentWidget /*= nullptr*/)
   : mNode(aNode)
   , mParentWidgetPtr(aParentWidget)
{
}

void PartManager::Plugin::ManagePlatformParts::operator()()
{
   Browser* browser = new Browser(mNode, mParentWidgetPtr);
   browser->show();
}


QString PartManager::Plugin::GetNodeTypeString(const WsfPProxyNode& aNode)
{
   QString typeStr = "";
   if (aNode.IsValid())
   {
      if (aNode.IsOfType("Platform"))
      {
         typeStr = "platform";
      }
      else if (aNode.IsOfType("Processor"))
      {
         typeStr = "processor";
      }
      else if (aNode.IsOfType("Mover"))
      {
         typeStr = "mover";
      }
      else if (aNode.IsOfType("Fuel"))
      {
         typeStr = "fuel";
      }
      else if (aNode.IsOfType("Sensor"))
      {
         typeStr = "sensor";
      }
      else if (aNode.IsOfType("Comm"))
      {
         typeStr = "comm";
      }
      else if (aNode.IsOfType("Weapon"))
      {
         typeStr = "weapon";
      }
   }
   return typeStr;
}
