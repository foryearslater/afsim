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
#ifndef PARTMANAGERPLUGIN_HPP
#define PARTMANAGERPLUGIN_HPP

#include "Plugin.hpp"

class QMenu;
class QWidget;
class WsfPProxyNode;

namespace PartManager
{
class Plugin : public wizard::Plugin
{
public:
   Plugin(const QString& aPluginName, const size_t aUniqueId);
   ~Plugin() override = default;

   void           Show();
   static QString GetNodeTypeString(const WsfPProxyNode& aNode);

protected:
private:
   void ActionsForNodeRequested(const WsfPProxyNode& aNode, QMenu* aMenuPtr, QWidget* aParentWidgetPtr);

   struct ManagePlatformParts
   {
      ManagePlatformParts(const WsfPProxyNode& aNode, QWidget* aParentWidget = nullptr);
      void operator()();

      WsfPProxyNode mNode;
      QWidget*      mParentWidgetPtr;
   };
};
} // namespace PartManager
#endif
