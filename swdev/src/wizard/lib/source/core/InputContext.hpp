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

#ifndef INPUTCONTEXT_HPP
#define INPUTCONTEXT_HPP

#include "UtTextDocument.hpp"
class WsfParseNode;

namespace wizard
{
class Editor;

//! InputContext contains functions that compute the 'context' from a position in the input file.
class InputContext
{
public:
   struct Context
   {
      WsfParseNode*       mBlockNodePtr;
      UtTextDocumentRange mQueryTextRange;
   };
   static void ContextFromCursor(Editor* aEditControlPtr, size_t aCursorLine, size_t aCursorColumn, Context& aContext);

   static void GetContext(Editor* aEditControlPtr, const UtTextDocumentRange& aPosition, Context& aContext);

   static void GetInputContext(Editor* aEditControlPtr, WsfParseNode* aPreviousNode, size_t aInputPosition, UtTextRange& aRange);

   static WsfParseNode* FindBlockNode(WsfParseNode* aNodePtr);
   static bool          CommandExtent(WsfParseNode* aNodePtr, WsfParseNode*& aCommandNodePtr);
   static bool          CommandHasError(WsfParseNode* aCommandNodePtr);
};
} // namespace wizard
#endif
