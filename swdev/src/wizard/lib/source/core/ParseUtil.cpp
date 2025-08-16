// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2016 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "ParseUtil.hpp"

#include "UtStringUtil.hpp"
#include "UtTextDocument.hpp"
#include "WsfParseNode.hpp"
#include "WsfParseTypeInfoData.hpp"

// Types are stored in nodes in differing ways depending on the grammar.
// The <...:LoadType> and <...:NewType> rules store type info in an auxiliary data structure.
// Nodes marked with cTYPE_REFERENCE_NODE or cTYPE_NAME_NODE indicate they are followed by a LoadType or NewType.
// Nodes marked with cLAZY_TYPE_REFERENCE_NODE are references to types that may or may not exist, with type data
//  encoded in the node type and value.
// static
bool wizard::ParseUtil::FindReferenceType(WsfParseNode* aNodePtr, WsfParseTypePath& aTypeKey, bool& aNested)
{
   bool      foundType   = false;
   const int cTYPE_FLAGS = WsfParseNode::cTYPE_REFERENCE_NODE | WsfParseNode::cTYPE_NAME_NODE;
   if (aNodePtr->GetFlags() & cTYPE_FLAGS)
   {
      WsfParseNode* typeInfoNodePtr = nullptr;
      if (FindTypeNode(aNodePtr, typeInfoNodePtr))
      {
         WsfParseAuxData* auxPtr = typeInfoNodePtr->GetAuxiliaryValue();
         if (auxPtr != nullptr)
         {
            if (auxPtr->mDataType == WsfParseAuxData::cTYPE_DATA)
            {
               WsfParseTypeInfoData* typeInfoDataPtr = (WsfParseTypeInfoData*)auxPtr;
               if (aNodePtr->GetFlags() & WsfParseNode::cTYPE_REFERENCE_NODE)
               {
                  foundType = true;
                  aTypeKey  = typeInfoDataPtr->mLoadKey;
                  aNested   = 0 != (typeInfoDataPtr->mFlags & WsfParseTypeInfoData::cNESTED_LOAD);
               }
               else if (aNodePtr->GetFlags() & WsfParseNode::cTYPE_NAME_NODE)
               {
                  foundType = true;
                  aTypeKey  = typeInfoDataPtr->mSaveKey;
                  aNested   = 0 != (typeInfoDataPtr->mFlags & WsfParseTypeInfoData::cNESTED_SAVE);
               }
            }
         }
      }
   }
   else if (aNodePtr->GetFlags() & WsfParseNode::cLAZY_TYPE_REFERENCE_NODE)
   {
      foundType = true;
      aTypeKey.clear();
      aTypeKey.push_back(UtStringRef(aNodePtr->mType));
      aTypeKey.push_back(UtStringRef(aNodePtr->mValue.Text()));
      aNested = false;
   }
   else if (aNodePtr->GetFlags() & WsfParseNode::cLAZY_TYPENAME_NODE)
   {
      foundType = true;
      aTypeKey.clear();
      aTypeKey.push_back(UtStringRef(aNodePtr->mType));
      aTypeKey.push_back(UtStringRef(aNodePtr->mValue.Text()));
      aNested = false;
   }
   return foundType;
}

bool wizard::ParseUtil::FindTypeNode(WsfParseNode* aNodePtr, WsfParseNode*& aTypeNodePtr)
{
   WsfParseNode* typeInfoNodePtr = aNodePtr;
   while (typeInfoNodePtr != nullptr)
   {
      if (typeInfoNodePtr->GetFlags() & WsfParseNode::cLOAD_TYPE_NODE)
      {
         aTypeNodePtr = typeInfoNodePtr;
         return true;
      }
      typeInfoNodePtr = typeInfoNodePtr->Right();
   }
   return false;
}

std::vector<std::string> wizard::ParseUtil::GetFlagStringList(const WsfParseNode* aNode) noexcept
{
   std::vector<std::string> flags;
   if (aNode)
   {
      if (aNode->GetFlags() & WsfParseNode::cERROR_INCOMPLETE)
      {
         flags.emplace_back("error_incomplete");
      }
      if (aNode->GetFlags() & WsfParseNode::cTYPE_NAME_NODE)
      {
         flags.emplace_back("type_name_node");
      }
      if (aNode->GetFlags() & WsfParseNode::cTYPE_REFERENCE_NODE)
      {
         flags.emplace_back("type_reference_node");
      }
      if (aNode->GetFlags() & WsfParseNode::cNAMED_NODE)
      {
         flags.emplace_back("named_node");
      }
      if (aNode->GetFlags() & WsfParseNode::cLOAD_TYPE_NODE)
      {
         flags.emplace_back("load_type_node");
      }
      if (aNode->GetFlags() & WsfParseNode::cLAZY_TYPE_REFERENCE_NODE)
      {
         flags.emplace_back("lazy_type_reference_node");
      }
      if (aNode->GetFlags() & WsfParseNode::cLAZY_TYPENAME_NODE)
      {
         flags.emplace_back("lazy_typename_node");
      }
      if (aNode->GetFlags() & WsfParseNode::cBLOCK_NODE)
      {
         flags.emplace_back("block_node");
      }
      if (aNode->GetFlags() & WsfParseNode::cSCRIPT_BLOCK)
      {
         flags.emplace_back("script_block");
      }
      if (aNode->GetFlags() & WsfParseNode::cSCRIPT_FUNCTION)
      {
         flags.emplace_back("script_function");
      }
      if (aNode->GetFlags() & WsfParseNode::cSCRIPT_VARIABLES)
      {
         flags.emplace_back("script_variables");
      }
      if (aNode->GetFlags() & WsfParseNode::cSCRIPT_MASK)
      {
         flags.emplace_back("script_mask");
      }
      if (aNode->GetFlags() & WsfParseNode::cDELAY_LOAD)
      {
         flags.emplace_back("delay_load");
      }
      if (aNode->GetFlags() & WsfParseNode::cBLOCK_START)
      {
         flags.emplace_back("block_start");
      }
      if (aNode->GetFlags() & WsfParseNode::cBLOCK_END)
      {
         flags.emplace_back("block_end");
      }
      if (aNode->GetFlags() & WsfParseNode::cPRUNE_NODE)
      {
         flags.emplace_back("prune_node");
      }
   }
   return flags;
}

wizard::ParseUtil::Context::BlockContext wizard::ParseUtil::Context::BlockContext::GetContext(WsfParseNode* aNode) noexcept
{
   // Require the node to be a "block_node".
   if (aNode && (aNode->GetFlags() & WsfParseNode::cBLOCK_NODE))
   {
      // Find the start of the block.
      auto blockStartNode = aNode;
      while (blockStartNode && !(blockStartNode->GetFlags() & WsfParseNode::cBLOCK_START))
      {
         blockStartNode = blockStartNode->Left();
      }
      auto blockStartRange = blockStartNode->SubtreeRange();
      bool hasBlockStart{false};
      if (blockStartRange.Valid())
      {
         auto blockStart = blockStartRange.Text();
         if (!blockStart.empty())
         {
            hasBlockStart = true;
         }
      }
      // Find the end of the block.
      auto blockEndNode = aNode;
      while (blockEndNode && !(blockEndNode->GetFlags() & WsfParseNode::cBLOCK_END))
      {
         blockEndNode = blockEndNode->Right();
      }
      auto blockEndRange = blockEndNode->SubtreeRange();
      bool hasBlockEnd{false};
      if (blockEndRange.Valid())
      {
         auto blockEnd = blockEndRange.Text();
         // For an AFSIM input block, the last line must be the first line prefixed with "end_".
         if (!blockEnd.empty() && blockEnd.find("end_") == 0)
         {
            hasBlockEnd = true;
         }
      }
      if (hasBlockStart && hasBlockEnd)
      {
         // This is an explicit block context.
         return BlockContext{Type::Explicit};
      }
   }
   // We could not determine the block context.
   return BlockContext{};
}

wizard::ParseUtil::Context::MoverContext wizard::ParseUtil::Context::MoverContext::GetContext(WsfParseNode* aNode) noexcept
{
   if (aNode)
   {
      auto blockRange = aNode->SubtreeRange();
      if (blockRange.Valid())
      {
         if (aNode->GetFlags() & WsfParseNode::cBLOCK_START)
         {
            auto blockStart = blockRange.Text();
            // Check if the block is actually a route block.
            // The parse node has an implicit mover context if the route block is not in a mover block.
            if (blockStart == "route")
            {
               // This is an implicit mover context.
               return MoverContext{Type::Implicit};
            }
            else
            {
               // Check if the block is a mover block.
               bool isMover{blockStart == "mover"};
               // Check if add, edit, or delete precedes the block.
               auto prevNode = aNode->Previous();
               if (prevNode)
               {
                  auto addEditDeleteRange = prevNode->SubtreeRange();
                  if (addEditDeleteRange.Valid())
                  {
                     auto addEditDelete = addEditDeleteRange.Text();
                     // The parse node has an explicit mover context if it is a mover block preceded by the keyword add,
                     // edit, or delete. For example,
                     //   add\wmover\wend_mover
                     // or
                     //   edit\wmover\wend_mover
                     // or
                     //   delete\wmover
                     // where \w is any whitespace character.
                     bool isAddEditDelete = addEditDelete == "add" || addEditDelete == "edit" || addEditDelete == "delete";
                     if (isAddEditDelete || // This catches add\wmover\wend_mover or edit\wmover\wend_mover or delete\wmover.
                         isMover)           // This catches mover\wend_mover.
                     {
                        // This is an explicit mover context.
                        return MoverContext{Type::Explicit};
                     }
                  }
               }
            }
         }
         // This routine is to catch WsfParseNodes that are not block_start nodes, which may have a multi-word range.
         // The logic in the following routine is the same as above, so the comments are omitted.
         else
         {
            std::vector<std::string> rangeSplit;
            auto                     rangeSize = UtStringUtil::ParseWhitespace(blockRange.Text(), rangeSplit);
            if (rangeSize)
            {
               // This is one-word range.
               if (rangeSize == 1 && rangeSplit.front() == "route")
               {
                  // This is an implicit mover context.
                  return MoverContext{Type::Implicit};
               }
               // This is at least a two-word range.
               else if (rangeSize > 1)
               {
                  if (rangeSplit[0] == "add" || rangeSplit[0] == "edit" || rangeSplit[0] == "delete" ||
                      rangeSplit[1] == "mover")
                  {
                     return MoverContext{Type::Explicit};
                  }
               }
            }
         }
      }
   }
   // This is not a mover context.
   return MoverContext{};
}
