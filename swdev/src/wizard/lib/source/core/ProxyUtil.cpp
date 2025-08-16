// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2021 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "ProxyUtil.hpp"

// Wizard includes
#include "Editor.hpp"
#include "ParseUtil.hpp"
#include "ProjectWorkspace.hpp"
#include "ProxyWatcher.hpp"

// Util includes
#include "UtTextDocument.hpp"

// WSF includes
#include "WsfPM_Platform.hpp"
#include "WsfPM_Root.hpp"
#include "WsfPM_Zone.hpp"
#include "WsfPProxy.hpp"
#include "WsfPProxyIndex.hpp"
#include "WsfPProxyNode.hpp"
#include "WsfPProxyPath.hpp"

namespace wizard
{
namespace ProxyUtil
{

//! @par Finding the block_start node
//!   This is a note about finding the block_start node from one of the following node types:
//!   * type_name
//!   * type_reference
//!   * load_type
//!   * lazy_type_name
//!   * lazy_type_reference
//!   * block_node
//!   * block_end
//!   Usually, these nodes are to the right of the block_start node when in an EXPLICIT block context.
//!   However, sometimes these nodes are below the block_start node when in an IMPLICIT block context.
//!   Also, sometimes the nodes do not succeed a block_start node.
//!   In that case, this returns the left-most sibling of the node contained in aEntry.
//! @see ParseUtil::Context::BlockContext
WsfParseNode* FindBlockStartNodeFromEntry(const WsfPProxyIndexEntry& aEntry) noexcept
{
   WsfParseNode* parseNode = aEntry.mNodePtr;

   if (parseNode)
   {
      if ((parseNode->GetFlags() & WsfParseNode::cBLOCK_START))
      {
         return parseNode;
      }
      else if ((parseNode->GetFlags() & WsfParseNode::cTYPE_MASK) ||
               (parseNode->GetFlags() & WsfParseNode::cLOAD_TYPE_NODE) ||
               (parseNode->GetFlags() & WsfParseNode::cLAZY_TYPE_MASK) ||
               (parseNode->GetFlags() & WsfParseNode::cBLOCK_NODE) || (parseNode->GetFlags() & WsfParseNode::cBLOCK_END))
      {
         // If there exists a sibling to the left, traverse left until the block_start node
         WsfParseNode* startNode = parseNode->Left();
         if (startNode)
         {
            while (startNode && startNode->Left() && !(startNode->GetFlags() & WsfParseNode::cBLOCK_START))
            {
               startNode = startNode->Left();
            }
            return startNode;
         }
         // If there exists a sibling to the right, traverse down until the block_start node
         startNode = parseNode->Right();
         if (startNode)
         {
            while (startNode && startNode->Down() && !(startNode->GetFlags() & WsfParseNode::cBLOCK_START))
            {
               startNode = startNode->Down();
            }
            return startNode;
         }
      }
   }
   return parseNode;
}

//! @par Finding the block_end node
//!   This is a note about finding the block_end node from one of the following node types:
//!   * type_name
//!   * type_reference
//!   * load_type
//!   * lazy_type_name
//!   * lazy_type_reference
//!   * block_start
//!   * block_end
//!   Usually, these nodes are to the left of the block_end node when in an EXPLICIT block context.
//!   Also, sometimes the nodes do not precede a block_end node.
//!   In that case, this returns the right-most sibling of the node contained in aEntry.
//! @see ParseUtil::Context::BlockContext
WsfParseNode* FindBlockEndNodeFromEntry(const WsfPProxyIndexEntry& aEntry) noexcept
{
   WsfParseNode* parseNode = aEntry.mNodePtr;

   if (parseNode)
   {
      if ((parseNode->GetFlags() & WsfParseNode::cTYPE_MASK) || (parseNode->GetFlags() & WsfParseNode::cLOAD_TYPE_NODE) ||
          (parseNode->GetFlags() & WsfParseNode::cLAZY_TYPE_MASK) ||
          (parseNode->GetFlags() & WsfParseNode::cBLOCK_START) || (parseNode->GetFlags() & WsfParseNode::cBLOCK_NODE))
      {
         // If there exists a sibling to the right, traverse right until the block_end node
         WsfParseNode* startNode = parseNode->Right();
         if (startNode)
         {
            while (startNode && startNode->Right() && !(startNode->GetFlags() & WsfParseNode::cBLOCK_END))
            {
               startNode = startNode->Right();
            }
            return startNode;
         }
      }
   }
   return parseNode;
}

void FindBlockStartLocations(const std::vector<WsfPProxyIndexEntry>& aEntries,
                             std::vector<UtTextDocumentRange>&       aLocations) noexcept
{
   for (const auto& elem : aEntries)
   {
      WsfParseNode* parseNode = FindBlockStartNodeFromEntry(elem);
      // Only add explicit contexts to the location list.
      // (See wizard::ParseUtil for more information)
      switch (ParseUtil::Context::MoverContext::GetContext(parseNode).mType)
      {
      case ParseUtil::Context::Type::Implicit:
         break;
      case ParseUtil::Context::Type::Explicit:
      case ParseUtil::Context::Type::NotApplicable:
      default:
         if (parseNode) // just for safety
         {
            aLocations.emplace_back(parseNode->SubtreeRange());
         }
         break;
      }
   }
}

void FindBlockEndLocations(const std::vector<WsfPProxyIndexEntry>& aEntries,
                           std::vector<UtTextDocumentRange>&       aLocations) noexcept
{
   for (const auto& elem : aEntries)
   {
      WsfParseNode* parseNode = FindBlockEndNodeFromEntry(elem);
      // BlockContext only applies to the start of the block, so omit the check here.
      if (parseNode) // just for safety
      {
         aLocations.emplace_back(parseNode->SubtreeRange());
      }
   }
}

void FindBlockLocations(const std::vector<WsfPProxyIndexEntry>& aEntries, std::vector<UtTextDocumentRange>& aLocations) noexcept
{
   for (const auto& elem : aEntries)
   {
      // BlockContext only applies to the start of the block, so omit the check here.
      WsfParseNode*       blockStartNode = FindBlockStartNodeFromEntry(elem);
      WsfParseNode*       blockEndNode   = FindBlockEndNodeFromEntry(elem);
      UtTextDocumentRange blockRange;

      if (blockStartNode) // just for safety
      {
         UtTextDocumentRange startSubtreeRange = blockStartNode->SubtreeRange();
         if (startSubtreeRange.Valid())
         {
            blockRange = startSubtreeRange;
         }
      }
      if (blockEndNode) // just for safety
      {
         UtTextDocumentRange endSubtreeRange = blockEndNode->SubtreeRange();
         if (endSubtreeRange.Valid())
         {
            blockRange.Extend(endSubtreeRange);
         }
      }
      aLocations.emplace_back(std::move(blockRange));
   }
}

WsfPM_Zone FindZoneNodeByName(const std::string& aZoneName, const std::string& aParentPlatName) noexcept
{
   WsfPM_Root rootNode{ProxyWatcher::GetActiveProxy()};

   // try parent platform
   if (aParentPlatName != "Global Zones" && !aParentPlatName.empty())
   {
      WsfPM_Platform parentPlatNode = rootNode.platforms().Find(aParentPlatName);
      if (parentPlatNode.IsValid())
      {
         WsfPM_Zone zoneNode = parentPlatNode.Zone().Find(aZoneName);
         if (zoneNode.IsValid())
         {
            return zoneNode;
         }
      }
   }

   // try global zones
   return WsfPM_Zone{rootNode.zones().Find(aZoneName)}; // null cascades up
}

void InvalidateInput(const WsfPProxyNode& aProxyNode) noexcept
{
   WsfPProxyIndexNode* index{aProxyNode.FindIndexNode()};
   if (index)
   {
      const std::vector<WsfPProxyIndexEntry>& entries{index->mEntries};
      if (!entries.empty())
      {
         // Get the last entry
         const WsfPProxyIndexEntry& lastEntry = entries.back();
         // Get the start and end WsfParseNodes
         WsfParseNode* startNode{wizard::ProxyUtil::FindBlockStartNodeFromEntry(lastEntry)};
         WsfParseNode* endNode{wizard::ProxyUtil::FindBlockEndNodeFromEntry(lastEntry)};
         if (startNode && endNode)
         {
            // Mark each sub-node as an error
            WsfParseNode* tempNode{startNode};
            while (tempNode != endNode)
            {
               tempNode->mFlags |= WsfParseNode::cERROR_INCOMPLETE;
               tempNode = tempNode->Next();
            }
            tempNode->mFlags |= WsfParseNode::cERROR_INCOMPLETE;
         }
         // Get the editor containing the range
         UtTextDocumentRange range{startNode->SubtreeRange()};
         wizard::Editor*     editor{nullptr};
         if (range.Valid())
         {
            std::string fileName{range.mSource->GetFilePath().GetNormalizedPath()};
            editor = wizard::ProjectWorkspace::Instance()->GotoFile(fileName);
         }
         // Update the syntax highlighting to show the error
         if (editor)
         {
            editor->UpdateStyle();
         }
      }
   }
}

} // namespace ProxyUtil
} // namespace wizard
