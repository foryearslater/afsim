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

#include "DragAndDrop.hpp"

#include <QDataStream>
#include <QMimeData>
#include <QStringList>
#include <QUrl>

#include "ParseResults.hpp"
#include "Project.hpp"
#include "ProxyWatcher.hpp"
#include "WsfPM_Root.hpp"
#include "WsfPProxyNode.hpp"

const char* wizard::DragAndDrop::cFILE_PATH_MIME_TYPE       = "application/x-wizard_file_path";
const char* wizard::DragAndDrop::cPROPERTY_SET_MIME_TYPE    = "application/x-wizard_property_set";
const char* wizard::DragAndDrop::cPROXY_NODE_LIST_MIME_TYPE = "application/x-wizard_proxy_path_list";

wizard::DragAndDrop::PropertyList wizard::DragAndDrop::ReadPropertySetMime(const QMimeData* aMimePtr)
{
   QByteArray                bytes = aMimePtr->data(DragAndDrop::cPROPERTY_SET_MIME_TYPE);
   QDataStream               in(bytes);
   DragAndDrop::PropertyList props;
   in >> props;
   return props;
}

QMimeData* wizard::DragAndDrop::CreatePropertySetMime(const PropertyList& aProperties)
{
   QByteArray  bytes;
   QDataStream out(&bytes, QIODevice::WriteOnly);
   out << aProperties;
   QMimeData* mime = new QMimeData;
   mime->setData(DragAndDrop::cPROPERTY_SET_MIME_TYPE, bytes);
   return mime;
}

QMimeData* wizard::DragAndDrop::CreateProxyNodeListMime(const std::vector<WsfPProxyNode>& aProxyNodes)
{
   QString str;
   for (size_t i = 0; i < aProxyNodes.size(); ++i)
   {
      const WsfPProxyNode&     node             = aProxyNodes[i];
      std::vector<std::string> proxyPathStrings = node.GetStablePath().ToStringList(node.GetRoot());
      if (i != 0)
         str += '\n';
      for (size_t j = 0; j < proxyPathStrings.size(); ++j)
      {
         if (j != 0)
            str += ' ';
         str += proxyPathStrings[j].c_str();
      }
   }
   QMimeData* mime = new QMimeData();
   mime->setData(cPROXY_NODE_LIST_MIME_TYPE, str.toLatin1());
   return mime;
}

std::vector<WsfPProxyNode> wizard::DragAndDrop::ReadProxyNodeListMime(const QMimeData* aMimePtr)
{
   std::vector<WsfPProxyNode> nodeList;
   if (!aMimePtr->hasFormat(cPROXY_NODE_LIST_MIME_TYPE) || !Project::Instance() ||
       !Project::Instance()->GetParseResults() || !Project::Instance()->GetParseResults()->ProxyRoot())
      return nodeList;
   WsfPProxyValue rootValue = Project::Instance()->GetParseResults()->ProxyRoot();

   QString     str   = aMimePtr->data(cPROXY_NODE_LIST_MIME_TYPE);
   QStringList lines = str.split('\n');
   for (int i = 0; i < lines.size(); ++i)
   {
      QStringList words = lines[i].split(' ');
      if (!words.isEmpty())
      {
         std::vector<std::string> proxyPathStrings;
         for (int j = 0; j < words.size(); ++j)
         {
            proxyPathStrings.push_back(words[j].toStdString());
         }
         WsfPProxyPath path = WsfPProxyPath::FromStringList(rootValue, proxyPathStrings);
         WsfPProxyNode node(Project::Instance()->GetParseResults()->Proxy(), path);
         if (node)
         {
            nodeList.push_back(node);
         }
      }
   }
   return nodeList;
}

QString wizard::DragAndDrop::ToString(const WsfPProxyNode& aNode)
{
   std::vector<std::string> proxyPathStrings = aNode.GetStablePath().ToStringList(aNode.GetRoot());
   QString                  str;
   for (size_t j = 0; j < proxyPathStrings.size(); ++j)
   {
      if (j != 0)
         str += ' ';
      str += proxyPathStrings[j].c_str();
   }
   return str;
}

namespace
{
// Convert a stable path into standard form.
WsfPProxyPath FromStablePath(WsfPProxyValue aRoot, const WsfPProxyPath& aStablePath)
{
   // Just replace string keys with integer keys
   WsfPProxyPath  standard = aStablePath;
   WsfPProxyValue root     = aRoot;
   for (size_t i = 0; i < standard.size(); ++i)
   {
      WsfPProxyKey& key = standard[i];
      if (root.IsStruct() && key.IsString())
      {
         key.SetIndex(WsfPProxyStructValue(root).GetMemberIndex(key.GetMapKey()));
         key.ClearMapKey();
      }
      root = root.GetAttr(key);
   }
   return standard;
}
} // namespace

WsfPProxyNode wizard::DragAndDrop::ToProxyNode(const QString& aStr)
{
   WsfPProxy* proxyPtr = ProxyWatcher::GetActiveProxy();
   if (!proxyPtr)
   {
      return WsfPProxyNode();
   }
   QStringList   words = aStr.split(' ');
   WsfPProxyPath path;
   for (int i = 0; i < words.size(); ++i)
   {
      path += words[i].toStdString();
   }
   WsfPProxyPath realPath = FromStablePath(WsfPProxyNode::FromProxy(proxyPtr).GetValue(), path);
   return WsfPProxyNode(proxyPtr, realPath);
}
