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

#include "InputContext.hpp"

#include "Editor.hpp"
#include "ParseResults.hpp"
#include "Project.hpp"
#include "TextSource.hpp"
#include "WsfParseNode.hpp"

void wizard::InputContext::ContextFromCursor(Editor* aEditControlPtr, size_t aCursorLine, size_t aCursorColumn, Context& aContext)
{
   TextSource* sourcePtr = aEditControlPtr->GetSource();
   size_t      lines     = sourcePtr->GetSource()->GetLineCount();

   if (aCursorLine < lines)
   {
      size_t pos = aCursorColumn + sourcePtr->GetSource()->GetLinePosition(aCursorLine);
      GetContext(aEditControlPtr, UtTextDocumentRange(sourcePtr->GetSource(), pos, pos), aContext);
   }
}

// Calculate / guess the 'context' of the auto-complete request.
// The context defines the block of text that needs completed, and the parse
// node representing the containing input block.
void wizard::InputContext::GetContext(Editor* aEditControlPtr, const UtTextDocumentRange& aPosition, Context& aContext)
{
   //
   // WIZARD_TODO:  This code gone through many revisions and needs re-factored / simplified.
   //

   // The general idea here is to start from the cursor and back up until we reach
   // a token which has been interpreted correctly.
   WsfParseNode*&       blockNodePtr   = aContext.mBlockNodePtr;
   UtTextDocumentRange& queryTextRange = aContext.mQueryTextRange;

   size_t      pos                  = aPosition.GetEnd();
   TextSource* sourcePtr            = aEditControlPtr->GetSource();
   aContext.mQueryTextRange.mSource = sourcePtr->GetSource();
   Project*      projectPtr         = aEditControlPtr->GetProject();
   ParseResults* results            = nullptr;
   if (projectPtr)
      results = projectPtr->GetParseResults();
   if (results)
   {
      UtTextDocumentRange rangeUpToPos(aPosition);

      // This prevents selecting the node when the cursor lies just before, like:  |dis_interface
      rangeUpToPos.SetRange(0, rangeUpToPos.GetEnd() - 1);

      WsfParseNode* nodePtr = results->FindLastParseNode(rangeUpToPos);
      if (!nodePtr)
      {
         // If no previous token is found in the entire file, we will just assume the current line is the context
         blockNodePtr = nullptr;
         GetInputContext(aEditControlPtr, nullptr, pos, queryTextRange);
      }
      else
      {
         UtTextDocumentRange insertTouch(aPosition);
         insertTouch.SetBegin(insertTouch.GetBegin() - 1);
         bool nodeIsComplete = 0 == (nodePtr->mFlags & nodePtr->cERROR_INCOMPLETE);
         if (false) // nodeIsComplete && nodePtr->mValue.Intersects(insertTouch))
         {
            // Here, the cursor is touching a token that is being parsed correctly
            queryTextRange = nodePtr->mValue;
            blockNodePtr   = nodePtr;
         }
         else
         {
            WsfParseNode* cmdNodePtr = nullptr;
            // The last node with a value could be just prior to a block start.
            //    i.e.  platform <name> <type> (block-start) ... end_platform
            // Search forward looking for a block until the a node with a value is found.
            if (!nodePtr->mValue.Valid() || nodePtr->mValue.GetEnd() < pos)
            {
               WsfParseNode* nextNodePtr = nodePtr->Next();
               while (nextNodePtr != nullptr)
               {
                  if (nextNodePtr->mFlags & WsfParseNode::cERROR_INCOMPLETE)
                  {
                     break;
                  }
                  if (nextNodePtr->mFlags & WsfParseNode::cBLOCK_NODE)
                  {
                     nodePtr = nextNodePtr;
                  }
                  if (nextNodePtr->mValue.Valid())
                  {
                     break;
                  }
                  nextNodePtr = nextNodePtr->Next();
               }
            }

            if (nodeIsComplete && CommandExtent(nodePtr, cmdNodePtr))
            {
               bool cmdHasError = CommandHasError(cmdNodePtr);
               // We have successfully determined the command being modified
               UtTextRange cmdRange = cmdNodePtr->SubtreeRange(aPosition.mSource);
               if (cmdRange.Intersects(aPosition))
               {
                  // Cursor is inside the command
                  blockNodePtr = cmdNodePtr;
                  queryTextRange.SetRange(cmdRange.GetBegin(), std::min(pos, cmdRange.GetEnd()));
                  return;
               }
               else
               {
                  if (cmdHasError)
                  {
                     blockNodePtr = cmdNodePtr->GetParent();
                     queryTextRange.SetRange(cmdRange.GetBegin(), pos);
                  }
                  else
                  {
                     GetInputContext(aEditControlPtr, nodePtr, pos, queryTextRange);
                     blockNodePtr = FindBlockNode(cmdNodePtr);
                  }
                  return;
               }
            }
            else
            {
               blockNodePtr = FindBlockNode(nodePtr);
               static std::string rootCommands("root-commands");
               bool               hasBlock = blockNodePtr && blockNodePtr->mType != rootCommands;
               if (hasBlock && (blockNodePtr->GetFlags() & WsfParseNode::cERROR_INCOMPLETE))
               {
                  WsfParseNode* previous = blockNodePtr->FarLeft()->PreviousLeaf();
                  blockNodePtr           = nullptr;
                  if (previous)
                  {
                     blockNodePtr = FindBlockNode(previous);
                  }
                  GetInputContext(aEditControlPtr, previous, pos, queryTextRange);
               }
               else
               {
                  for (int i = 0; i < 10 && nodePtr; ++i)
                  {
                     if (nodePtr->GetFlags() & WsfParseNode::cERROR_INCOMPLETE)
                     {
                        nodePtr = nodePtr->PreviousLeaf();
                     }
                  }
                  GetInputContext(aEditControlPtr, nodePtr, pos, queryTextRange);
               }
            }
         }
      }
   }
}

// Computes the input context given a 'previous' node that is just before the input context.
// Limits the context to the current line.
void wizard::InputContext::GetInputContext(Editor*       aEditControlPtr,
                                           WsfParseNode* aPreviousNode,
                                           size_t        aInputPosition,
                                           UtTextRange&  aRange)
{
   size_t prevTokenLine = ut::npos;

   WsfParseNode* previousLeaf = aPreviousNode;
   if (previousLeaf && !previousLeaf->mValue.Valid())
   {
      previousLeaf = previousLeaf->PreviousLeaf();
   }
   UtTextDocument* sourcePtr = aEditControlPtr->GetSource()->GetSource();
   size_t          inputLine = sourcePtr->PositionToLineNumber(aInputPosition);

   if (previousLeaf && previousLeaf->mValue.mSource == sourcePtr)
   {
      prevTokenLine = sourcePtr->PositionToLineNumber(previousLeaf->mValue.GetEnd());
   }
   if (prevTokenLine == inputLine && previousLeaf)
   {
      aRange.SetRange(previousLeaf->mValue.GetEnd() + 1, aInputPosition - 1);
   }
   else
   {
      aRange.SetRange(sourcePtr->GetLinePosition(inputLine), aInputPosition - 1);
   }

   if (aRange.GetEnd() != ut::npos)
   {
      size_t begin{aRange.GetBegin()};
      while (begin < aRange.GetEnd())
      {
         if (!isspace((*sourcePtr)[begin]))
         {
            break;
         }
         begin++;
      }
      aRange.SetBegin(begin);
   }
}

WsfParseNode* wizard::InputContext::FindBlockNode(WsfParseNode* aNodePtr)
{
   WsfParseNode* nodePtr = aNodePtr;
   while (nodePtr->GetParent() != nullptr)
   {
      if (nodePtr->mFlags & WsfParseNode::cBLOCK_NODE)
      {
         break;
      }
      nodePtr = nodePtr->GetParent();
   }
   return nodePtr;
}

// Find the node whose children constitute the entire command in which aNodePtr is a part.
bool wizard::InputContext::CommandExtent(WsfParseNode* aNodePtr, WsfParseNode*& aCommandNodePtr)
{
   // This is kind of an approximation, because commands are not specifically defined in the grammar.
   // Here, we are defining a command to be nodes that are parsed as a block entry, or as a root command.
   // i.e. we freely recurse through rules to get a command, using blocks (or root-commands) as boundaries.
   std::string   rootCmdStr("root-commands");
   WsfParseNode* blockPtr       = aNodePtr;
   WsfParseNode* commandNodePtr = blockPtr ? blockPtr : aNodePtr;
   while (blockPtr && ((blockPtr->mFlags & WsfParseNode::cBLOCK_NODE) == 0))
   {
      commandNodePtr          = blockPtr;
      WsfParseNode* parentPtr = blockPtr->GetParent();
      if (parentPtr && parentPtr->mType == rootCmdStr)
      {
         break;
      }
      blockPtr = parentPtr;
   }
   // There is a slight difference between a root command or a block member.
   // Blocks always recur on a single grammar rule, so returning a block's child is guaranteed to have the entire
   // command. Root commands can be sequences, so to get the whole command, we need the parent.
   if (blockPtr && blockPtr->mType == rootCmdStr)
   {
      commandNodePtr = blockPtr;
   }
   aCommandNodePtr = commandNodePtr;
   return aCommandNodePtr != nullptr;
}

bool wizard::InputContext::CommandHasError(WsfParseNode* aCommandNodePtr)
{
   WsfParseNode* n      = aCommandNodePtr;
   WsfParseNode* endPtr = aCommandNodePtr->Next(false);
   while (n && n != endPtr)
   {
      if (n->mFlags & WsfParseNode::cBLOCK_NODE)
      {
         n = n->Next(false);
      }
      else
      {
         if (n->mFlags & WsfParseNode::cERROR_INCOMPLETE)
            return true;
         n = n->Next();
      }
   }
   return false;
}
