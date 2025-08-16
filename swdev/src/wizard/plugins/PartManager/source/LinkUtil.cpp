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

#include "LinkUtil.hpp"

#include <QStringList>

#include "WidgetFactory.hpp"
#include "WsfPProxyNode.hpp"

std::map<std::string, std::string> PartManager::LinkUtil::msDeclarationMap = {{"comms", "comm"},
                                                                              {"commType", "comm"},
                                                                              {"mover", "mover"},
                                                                              {"moverType", "mover"},
                                                                              {"platform", "platform"},
                                                                              {"platformType", "platform"},
                                                                              {"processors", "processor"},
                                                                              {"processorType", "processor"},
                                                                              {"sensors", "sensor"},
                                                                              {"sensorType", "sensor"},
                                                                              {"weapons", "weapon"},
                                                                              {"weaponType", "weapon"}};

//! Given a URL created with MakeUrlFromPath(...), return the original path
//! used to create the URL.
WsfPProxyPath PartManager::LinkUtil::MakePathFromUrl(const WsfPProxyValue& proxyRoot, const QString& url)
{
   const QStringList        splitUrl = url.split(' ');
   const std::list<QString> qtList   = splitUrl.toStdList();
   std::vector<std::string> stdList;

   for (auto&& it : qtList)
   {
      stdList.push_back(it.toStdString());
   }

   WsfPProxyPath proxyPath = WsfPProxyPath::FromStringList(proxyRoot, stdList);

   return proxyPath;
}

//! Creates an HTML label for the given proxy node. The label will contain an
//! HTML link to the type of the node.
QString PartManager::LinkUtil::MakeWidgetLabel(const WsfPProxyNode& aProxyNode)
{
   const WsfPProxyValue     proxyRoot   = aProxyNode.GetRoot();
   const WsfPProxyPath&     proxyPath   = aProxyNode.GetPath();
   std::vector<std::string> proxyPathLs = proxyPath.ToStringList(proxyRoot);

   if (!aProxyNode.GetName().empty())
   {
      proxyPathLs.pop_back();
   }

   const std::string declarationText = msDeclarationMap[proxyPathLs.back()];
   const std::string nameText        = aProxyNode.GetName();
   const std::string typeNameHtml    = MakeTypeNameHtml(aProxyNode);

   const std::string widgetName = declarationText + " " + nameText + (!nameText.empty() ? " " : "") + typeNameHtml;

   return QString::fromStdString(widgetName);
}

//! Returns the type name of the given proxy node as an HTML link to the proxy
//! path of its type.
std::string PartManager::LinkUtil::MakeTypeNameHtml(const WsfPProxyNode& proxyNode)
{
   std::string         typeNameHtml;
   const WsfPProxyPath inheritedPath = proxyNode.GetInheritedPath();

   if (!inheritedPath.Empty())
   {
      WsfPProxy* proxy     = proxyNode.GetProxy();
      const bool typeHasUi = WidgetFactory().HasUiFor(proxy, inheritedPath);

      typeNameHtml = proxyNode.GetTypeName();

      if (typeHasUi)
      {
         const std::string url = MakeUrlFromPath(proxy->mRoot, inheritedPath);
         typeNameHtml          = "<a href=\"" + url + "\">" + typeNameHtml + "</a>";
      }
   }

   return typeNameHtml;
}

//! Convert a path into a string, where each element in the path is separated
//! by a space character.
std::string PartManager::LinkUtil::MakeUrlFromPath(const WsfPProxyValue& proxyRoot, const WsfPProxyPath& proxyPath)
{
   const std::vector<std::string> pathLs = proxyPath.ToStringList(proxyRoot);
   std::string                    url;

   for (size_t i = 0; i < pathLs.size(); i++)
   {
      url += pathLs[i];
      if ((i + 1) < pathLs.size())
      {
         url += ' ';
      }
   }

   return url;
}