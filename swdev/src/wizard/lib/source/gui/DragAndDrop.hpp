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

#ifndef DRAGANDDROP_HPP
#define DRAGANDDROP_HPP
#include <vector>

#include <QMap>
#include <QString>
#include <QVariant>

#include "ViExport.hpp"

class QMimeData;
class ViFileItem;
class WsfPProxyNode;

namespace wizard
{
//! Routines for handling conversion to/from QMimeData for the Wizard
class VI_EXPORT DragAndDrop
{
public:
   // A QMimeData objects can not be aggregated, which is problematic because
   // copy/paste/drag/drop operations typically allow multiselect.  This leads to implementing
   // these operations at the top-level, instead of the item level.
   // To avoid this issue, any non-text mime data should use PropertyList to store mime data.
   //
   // currently used property keys:
   //    "proxy-node"    - A string with a space delimited 'stable' proxy path pointing to a proxy node
   //    "command-chain" - A command chain name associated with the item
   //

   typedef QMap<QString, QVariant> PropertyMap;
   typedef QList<PropertyMap>      PropertyList;
   static const char*              cFILE_PATH_MIME_TYPE;
   static const char*              cPROPERTY_SET_MIME_TYPE;
   static const char*              cPROXY_NODE_LIST_MIME_TYPE;

   static QMimeData*                 CreatePropertySetMime(const PropertyList& aProperties);
   static PropertyList               ReadPropertySetMime(const QMimeData* aMimePtr);
   static QMimeData*                 CreateProxyNodeListMime(const std::vector<WsfPProxyNode>& aProxyNodes);
   static QString                    ToString(const WsfPProxyNode& aNode);
   static std::vector<WsfPProxyNode> ReadProxyNodeListMime(const QMimeData* aMimePtr);
   static WsfPProxyNode              ToProxyNode(const QString& aStr);

   // ViWorkspaceItem* GetWorkspaceItem();
   // void SetWorkspaceItem(ViWorkspaceItem* aItemPtr);
   // int AddWorkspaceItem(ViWorkspaceItem* aItemPtr);
};
} // namespace wizard
#endif
