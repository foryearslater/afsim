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

#ifndef LINKUTIL_HPP
#define LINKUTIL_HPP

#include "WsfPProxyPath.hpp"

class QString;
class WsfPProxyNode;
class WsfPProxyValue;

namespace PartManager
{
class LinkUtil
{
public:
   static WsfPProxyPath MakePathFromUrl(const WsfPProxyValue& proxyRoot, const QString& url);
   static QString       MakeWidgetLabel(const WsfPProxyNode& aProxyNode);
   static std::string   MakeTypeNameHtml(const WsfPProxyNode& proxyNode);
   static std::string   MakeUrlFromPath(const WsfPProxyValue& proxyRoot, const WsfPProxyPath& proxyPath);

   static std::map<std::string, std::string> msDeclarationMap;
};
} // namespace PartManager

#endif
