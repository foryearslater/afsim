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

#include "WsfStyler.hpp"

#include <algorithm>
#include <stack>

#include <QPlainTextEdit>

#include "ParseResults.hpp"
#include "TextSource.hpp"
#include "UtCast.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"
#include "WsfParseNode.hpp"
#include "WsfParseRule.hpp"
#include "WsfParseSourceInclude.hpp"
#include "WsfScriptData.hpp"

void wizard::WsfStyler::StyleRequest::GetRangeToStyle(size_t& styleNeedStart, size_t& styleNeededEnd) const
{
   styleNeedStart = mFirstPosition;
   styleNeededEnd = mLastPosition;
}

//! Update the syntax highlighting for the text editor
void wizard::WsfStyler::UpdateStyle(const StyleRequest& aRequest)
{
   // Updating the style of the editor is a bit different than the standard scintilla application.
   // Since parsing requires context, we can't just parse the lines as needed.  Instead, we wait
   // until the parsing is complete and then update the editor -- only styling the current screen's text.
   //
   // Don't bother updating the highlighting if the editor is hidden
   QPlainTextEdit* cPtr = aRequest.mControl;
   if (!aRequest.mStyleIfHidden && !cPtr->isVisible())
   {
      return;
   }
   size_t editorOffset = aRequest.mPositionOffset;

   ParseResults& results = *aRequest.mResultsPtr;

   UtTextDocument* parseSourcePtr = aRequest.mSourcePtr->GetSource();

   // The first pass will skip scripts as to maintain any existing styles.
   // keep the location of any scripts:
   std::vector<UtTextRange> scriptRanges;

   size_t styleNeedStart, styleNeededEnd;
   aRequest.GetRangeToStyle(styleNeedStart, styleNeededEnd);

   if (styleNeededEnd > styleNeedStart)
   {
      bool incompleteFound = false;
      // Include commands can scatter the contents of a file across the parse tree.
      // Instead of walking the entire parse tree, we will only walk through blocks that contain chunks of this file.
      for (size_t i = 0; i < results.mFileTransitionNodes.size(); ++i)
      {
         WsfParseNode* nodePtr = results.mFileTransitionNodes[i];
         if (nodePtr->mValue.mSource == parseSourcePtr)
         {
            WsfParseNode* previousNodePtr = nullptr;
            // Walk all nodes in the block, coloring text in the block
            while (nodePtr != nullptr)
            {
               if (nodePtr->mFlags & WsfParseNode::cERROR_INCOMPLETE)
               {
                  incompleteFound = true;
               }
               else if (incompleteFound && nodePtr->GetParent() != previousNodePtr)
               {
                  incompleteFound = false;
               }
               if (nodePtr->mValue.mSource == parseSourcePtr)
               {
                  size_t begin = std::max(nodePtr->mValue.GetBegin() - editorOffset, styleNeedStart);
                  size_t end   = std::min(nodePtr->mValue.GetEnd() + 1 - editorOffset, styleNeededEnd + 1);
                  if (begin < end)
                  {
                     if (nodePtr->GetFlags() & WsfParseNode::cSCRIPT_MASK)
                     {
                        scriptRanges.push_back(UtTextRange(nodePtr->mValue));
                     }
                     else
                     {
                        auto typeId = GetTextTypeFromNode(nodePtr);
                        if (incompleteFound)
                        {
                           typeId          = TextType::Unmatched_Block;
                           incompleteFound = false;
                        }
                        aRequest.mStylerInterface->SetTextFormat(begin, end - begin, mTextFormats[typeId]);
                     }
                  }
               }
               else if (nodePtr->mValue.mSource != nullptr)
               {
                  break;
               }
               previousNodePtr = nodePtr;
               nodePtr         = nodePtr->Next();
            }
         }
      }

      WsfParseSourceInclude* sourceDataPtr = results.FindSourceData(parseSourcePtr);
      if (sourceDataPtr != nullptr)
      {
         for (auto&& e : sourceDataPtr->mEntries)
         {
            size_t begin = std::max(e.mLocation.GetBegin() - editorOffset, styleNeedStart);
            size_t end   = std::min(e.mLocation.GetEnd() + 1 - editorOffset, styleNeededEnd);
            if (begin < end)
            {
               auto styleId        = TextType::Comment;
               bool isIncludeError = false;
               if (e.mType == WsfParseSourceInclude::cINCLUDE_COMMAND)
               {
                  styleId = TextType::Include;
                  if (e.mTargetSource == nullptr)
                  {
                     isIncludeError = true;
                  }
               }
               else if (e.mType == WsfParseSourceInclude::cPREPROCESSOR_DEFINE ||
                        e.mType == WsfParseSourceInclude::cPREPROCESSOR_VARIABLE)
               {
                  styleId = TextType::Preprocessor;
               }
               aRequest.mStylerInterface->SetTextFormat(begin, end - begin, mTextFormats[styleId]);
               if (isIncludeError)
               {
                  aRequest.mStylerInterface->MergeTextFormat(begin, end - begin, mErrorUnderlineFmt);
               }
            }
         }
      }
   }
   if (aRequest.mShowNotifications)
   {
      ParseResults::FileNotificationList* notificationsPtr = results.GetFileNotifications(aRequest.mSourcePtr);
      if (notificationsPtr)
      {
         for (auto&& note : *notificationsPtr)
         {
            if (0 != (note.mNotificationType & (ParseResults::cERROR_BIT | ParseResults::cWARNING_BIT)))
            {
               size_t begin = std::max(note.mRange.GetBegin(), styleNeedStart);
               size_t end   = std::min(note.mRange.GetEnd() + 1, styleNeededEnd);
               if (begin < end)
               {
                  if (note.mNotificationType & ParseResults::cERROR_BIT)
                  {
                     aRequest.mStylerInterface->MergeTextFormat(begin, end - begin, mErrorUnderlineFmt);
                  }
                  else if (note.mNotificationType & ParseResults::cWARNING_BIT)
                  {
                     aRequest.mStylerInterface->MergeTextFormat(begin, end - begin, mWarningUnderlineFmt);
                  }
               }
            }
         }
      }
   }
   UpdateScriptStyle(aRequest);
}

void wizard::WsfStyler::UpdateScriptStyle(const StyleRequest& aRequest)
{
   size_t editorOffset = aRequest.mPositionOffset;

   ParseResults& results = *aRequest.mResultsPtr;

   WsfScriptData* scriptDataPtr = results.mScriptDataPtr;
   if (!scriptDataPtr)
   {
      return;
   }
   size_t styleNeedStart, styleNeededEnd;
   aRequest.GetRangeToStyle(styleNeedStart, styleNeededEnd);
   UtTextRange styleNeededRange(styleNeedStart + editorOffset, styleNeededEnd - 1 + editorOffset);
   if (styleNeededEnd > styleNeedStart)
   {
      UtTextDocumentLocation                     top(aRequest.mSourcePtr, 0);
      WsfScriptData::LocationMap::const_iterator iter = scriptDataPtr->GetLocationMap().lower_bound(top);
      while (iter != scriptDataPtr->GetLocationMap().end() && iter->first.mSource == top.mSource)
      {
         WsfScriptInfo* infoPtr = iter->second;
         if (infoPtr->mRange.Intersects(styleNeededRange))
         {
            UpdateScriptStyle(aRequest, iter->second);
         }
         ++iter;
      }
   }
}

void wizard::WsfStyler::UpdateScriptStyle(const StyleRequest& aRequest, WsfScriptInfo* aScriptPtr)
{
   size_t editorOffset = aRequest.mPositionOffset;
   size_t styleStart   = std::max(size_t{0}, aScriptPtr->mRange.GetBegin() - editorOffset);
   size_t styleEnd     = aScriptPtr->mRange.GetEnd() - editorOffset;
   if (styleEnd > styleStart)
   {
      size_t styleSize = styleEnd - styleStart + 1;
      for (auto&& tok : aScriptPtr->mTokens)
      {
         size_t b = std::max(tok.mRange.GetBegin() - editorOffset - styleStart, size_t{0});
         size_t e = std::min(tok.mRange.GetEnd() - editorOffset - styleStart, styleSize);
         if (e > b)
         {
            auto styleId = TextType::Script_None;
            switch (tok.mTokenType)
            {
            case WsfScriptToken::cIDENTIFIER:
               styleId = TextType::Script_Type;
               break;
            case WsfScriptToken::cKEYWORD:
               styleId = TextType::Script_Keyword;
               break;
            case WsfScriptToken::cOPERATOR:
               styleId = TextType::Script_Operator;
               break;
            case WsfScriptToken::cSTRING:
               styleId = TextType::Script_Quoted;
               break;
            case WsfScriptToken::cNUMBER:
               styleId = TextType::Script_Number;
               break;
            case WsfScriptToken::cCOMMENT:
               styleId = TextType::Script_Comment;
               break;
            case WsfScriptToken::cSTATIC_METHOD:
               styleId = TextType::Script_Static_Method;
               break;
            case WsfScriptToken::cMETHOD:
               styleId = TextType::Script_Method;
               break;
            case WsfScriptToken::cLOCAL_METHOD:
               styleId = TextType::Script_Local_Method;
               break;
            case WsfScriptToken::cLOCAL_VARIABLE:
               styleId = TextType::Script_Local_Variable;
               break;
            case WsfScriptToken::cPARAMETER:
               styleId = TextType::Script_Parameter;
               break;
            }
            aRequest.mStylerInterface->SetTextFormat(tok.mRange.GetBegin() - editorOffset,
                                                     tok.mRange.GetEnd() - tok.mRange.GetBegin(),
                                                     mTextFormats[styleId]);
         }
      }
      // Qt-Bug?   This works, but it adds items to the undo stack
      //  For now, just disabling the script background color
      //       QTextBlockFormat scriptBg;
      //       scriptBg.setBackground(mScriptBg);
      //       scriptBg.setProperty(QTextFormat::FullWidthSelection, true);
      //       QTextCursor cur = aRequest.mControl->textCursor();
      //       cur.setPosition(aScriptPtr->mRange.mBegin - editorOffset);
      //       cur.setPosition(aScriptPtr->mRange.mEnd - editorOffset, QTextCursor::KeepAnchor);
      //       cur.movePosition(QTextCursor::PreviousWord, QTextCursor::KeepAnchor);   // don't count whitespace
      //       cur.movePosition(QTextCursor::PreviousWord, QTextCursor::KeepAnchor);   // don't count whitespace
      //       cur.mergeBlockFormat(scriptBg);
   }
   if (aRequest.mShowScriptErrors)
   {
      const size_t    cMAX_ERRORS_STYLED = 100;
      QTextCharFormat errorFmt;
      errorFmt.setUnderlineColor(QColor(255, 0, 0));
      errorFmt.setUnderlineStyle(QTextCharFormat::WaveUnderline);
      // errorFmt.setFontUnderline(true);
      for (size_t i = 0; i < aScriptPtr->mErrors.size() && i < cMAX_ERRORS_STYLED; ++i)
      {
         size_t errPos = aScriptPtr->mErrors[i].first + aScriptPtr->mRange.GetBegin();
         size_t begin  = std::max(errPos, styleStart);
         size_t end    = std::min(errPos + aScriptPtr->mErrors[i].second, styleEnd);
         if (begin < end)
         {
            aRequest.mStylerInterface->MergeTextFormat(begin - editorOffset, end - begin, errorFmt);
         }
      }
   }
}

wizard::TextType wizard::WsfStyler::GetTextTypeFromNode(const WsfParseNode* aNodePtr)
{
   const int typeNodeFlags = WsfParseNode::cLAZY_TYPE_REFERENCE_NODE | WsfParseNode::cTYPE_REFERENCE_NODE;
   const int nameNodeFlags = WsfParseNode::cTYPE_NAME_NODE | WsfParseNode::cNAMED_NODE | WsfParseNode::cLAZY_TYPENAME_NODE;
   const int blockKeywordFlags = WsfParseNode::cBLOCK_START | WsfParseNode::cBLOCK_END;
   // We can't cache these because the same node types are used:
   if (aNodePtr->mFlags & blockKeywordFlags)
   {
      return TextType::Block_Keyword;
   }
   else if (aNodePtr->mFlags & typeNodeFlags)
   {
      return TextType::Type;
   }
   else if (aNodePtr->mFlags & nameNodeFlags)
   {
      return TextType::Name;
   }

   auto textType = TextType::None;
   // Most styles should be cacheable from the node-type.  This will provide a quick lookup for style.
   // In the future we may check other criteria first.
   auto iter = mNodeTypeToTextType.find(aNodePtr->mType);
   if (iter != mNodeTypeToTextType.end())
   {
      textType = iter->second;
   }
   else
   {
      std::string typeStr;
      if (aNodePtr->mType != nullptr)
      {
         typeStr = aNodePtr->mType;
      }
      if (typeStr == "ignore")
      {
         textType = TextType::None;
      }
      else if (typeStr == "int" || typeStr == "real" || typeStr == "string")
      {
         textType = TextType::User_Input;
      }
      else
      {
         textType = TextType::Keyword;
      }
      mNodeTypeToTextType[aNodePtr->mType] = textType;
   }

   // Change cSTYLE_KEYWORD to cSTYLE_COMMAND if the keyword is the start of a command
   if (textType == TextType::Keyword)
   {
      // Some commands are particularly deep, such as implicit mode commands.
      // Limit search to 20 nodes, it should catch most commands
      int                 counter = 20;
      const WsfParseNode* prev    = aNodePtr->Previous();
      while (prev && counter)
      {
         if (prev->mValue.Valid())
            break;
         if (prev->GetParent() && (prev->GetParent()->GetFlags() & WsfParseNode::cBLOCK_NODE))
         {
            textType = TextType::Command;
            break;
         }
         prev = prev->Previous();
         --counter;
      }
   }
   return textType;
}

void wizard::WsfStyler::ApplyEditorStyleSettings(QPlainTextEdit* aEditorPtr, int aFontSizeOverride /*= 0*/)
{
   // For readability
   EditorPrefObject* settings = wkfEnv.GetPreferenceObject<EditorPrefObject>();

   // Font
   int fontSize = (aFontSizeOverride > 0) ? aFontSizeOverride : settings->FontSize();
   aEditorPtr->setFont(QFont(settings->FontType(), fontSize));

   // Tab size
   aEditorPtr->setTabStopWidth(settings->TabSpace());
}


namespace
{
size_t GetLine(UtTextDocument* aDocPtr, WsfParseNode* aNode)
{
   WsfParseNode* tokenNode = aNode;
   if (tokenNode->mValue.mSource != aDocPtr)
   {
      tokenNode = aNode->PreviousLeaf();
   }
   if (tokenNode && tokenNode->mValue.mSource == aDocPtr)
   {
      return aDocPtr->PositionToLineNumber(tokenNode->mValue.GetEnd());
   }
   return ut::npos;
}
} // namespace
std::vector<std::pair<int, int>> wizard::WsfStyler::FoldDocument(ParseResults* aResultsPtr, TextSource* aSourcePtr)
{
   std::vector<std::pair<int, int>> foldRanges;
   if (!aResultsPtr)
      return foldRanges;
   ParseResults&             results        = *aResultsPtr;
   UtTextDocument*           parseSourcePtr = aSourcePtr->GetSource();
   std::stack<WsfParseNode*> blockStack;
   for (WsfParseNode* nodePtr : results.mFileTransitionNodes)
   {
      if ((nodePtr != nullptr) && (nodePtr->mValue.mSource == parseSourcePtr))
      {
         while (nodePtr != nullptr)
         {
            if (nodePtr->mValue.mSource != nullptr && nodePtr->mValue.mSource != parseSourcePtr)
               break;
            if (nodePtr->GetFlags() & WsfParseNode::cBLOCK_NODE)
            {
               size_t line = GetLine(parseSourcePtr, nodePtr);
               if (line != ut::npos)
               {
                  WsfParseNode* nextPtr = nodePtr->Next(false);
                  if (nextPtr)
                  {
                     size_t finalLine = GetLine(parseSourcePtr, nextPtr);
                     if (finalLine != ut::npos && finalLine > line)
                     {
                        foldRanges.emplace_back(ut::cast_to_int(line), ut::cast_to_int(finalLine));
                     }
                  }
               }
            }
            nodePtr = nodePtr->Next();
         }
      }
   }
   AddCommentsToFolding(foldRanges, aResultsPtr, aSourcePtr);
   return foldRanges;
}

void wizard::WsfStyler::AddCommentsToFolding(std::vector<std::pair<int, int>>& aFoldRanges,
                                             ParseResults*                     aResultsPtr,
                                             TextSource*                       aSourcePtr)
{
   ParseResults&             results        = *aResultsPtr;
   UtTextDocument*           parseSourcePtr = aSourcePtr->GetSource();
   std::stack<WsfParseNode*> blockStack;
   WsfParseSourceInclude*    sourceDataPtr = results.FindSourceData(parseSourcePtr);
   // don't run if there is no source found
   if (sourceDataPtr != nullptr)
   {
      std::vector<size_t> lineComments;
      // checks source for line and block comment tags, and deals with them accordingly
      for (auto&& e : sourceDataPtr->mEntries)
      {
         // checks or moves folding for block comments based on position
         // then inserts if the block comment is more than one line
         if (e.mType == WsfParseSourceInclude::cBLOCK_COMMENT)
         {
            size_t startLine = parseSourcePtr->PositionToLineNumber(e.mLocation.GetBegin());
            size_t endLine   = parseSourcePtr->PositionToLineNumber(e.mLocation.GetEnd());
            IsOverlaping(aFoldRanges, startLine, endLine);
            if (endLine > startLine)
            {
               aFoldRanges.emplace_back(ut::cast_to_int(startLine), ut::cast_to_int(endLine));
            }
         }
         // adds line comment to vector for later processing
         if (e.mType == WsfParseSourceInclude::cLINE_COMMENT)
         {
            size_t line2 = parseSourcePtr->PositionToLineNumber(e.mLocation.GetBegin());
            lineComments.push_back(line2);
         }
      }
      size_t lineBlockStart = ut::npos;
      size_t lineBlockEnd   = ut::npos;
      size_t lineHold{0};
      // main purpose of loop is to check for viable line comment groups
      for (auto&& currentLine : lineComments)
      {
         if (lineBlockStart == ut::npos || lineHold != currentLine)
         {
            // will not run on initial pass
            if (lineBlockStart != ut::npos && lineBlockEnd != ut::npos)
            {
               bool covered = IsOverlaping(aFoldRanges, lineBlockStart, lineBlockEnd);
               if (!covered)
               {
                  aFoldRanges.emplace_back(ut::cast_to_int(lineBlockStart), ut::cast_to_int(lineBlockEnd));
               }
               lineBlockEnd = ut::npos;
            }
            lineBlockStart = currentLine;
            lineHold       = currentLine + 1;
         }
         else
         {
            lineBlockEnd = currentLine;
            lineHold++;
         }
      }
      if (lineBlockEnd != ut::npos)
      {
         if (!IsOverlaping(aFoldRanges, lineBlockStart, lineBlockEnd))
         {
            aFoldRanges.emplace_back(ut::cast_to_int(lineBlockStart), ut::cast_to_int(lineBlockEnd));
         }
      }
   }
   std::sort(aFoldRanges.begin(), aFoldRanges.end());
}

bool wizard::WsfStyler::IsOverlaping(std::vector<std::pair<int, int>>& aFoldRanges,
                                     size_t&                           aLineBlockStart,
                                     size_t&                           aLineBlockEnd)
{
   for (auto&& iter : aFoldRanges)
   {
      if ((ut::cast_to_size_t(iter.first) <= aLineBlockStart) && (aLineBlockStart <= ut::cast_to_size_t(iter.second)))
      {
         size_t shortened = ut::cast_to_size_t(iter.second) + 1;
         // will move folding for comment blocks one space below already existing folding block if able
         if (aLineBlockEnd > shortened)
         {
            aLineBlockStart = shortened;
         }
         else
         {
            return true;
         }
      }
      else if ((ut::cast_to_size_t(iter.first) <= aLineBlockEnd) && (aLineBlockEnd <= ut::cast_to_size_t(iter.second)))
      {
         size_t shortened = ut::cast_to_size_t(iter.first) - 1;
         // will move folding for comment blocks one space above already existing folding block if able
         if (aLineBlockStart < shortened)
         {
            aLineBlockEnd = shortened;
         }
         else
         {
            return true;
         }
      }
      else if (aLineBlockStart <= ut::cast_to_size_t(iter.first) && (ut::cast_to_size_t(iter.second) <= aLineBlockEnd))
      {
         return true;
      }
   }
   return false;
}

void wizard::WsfStyler::LoadSettings()
{
   for (auto&& style : wkfEnv.GetPreferenceObject<EditorPrefObject>()->GetStyles())
   {
      QTextCharFormat fmt;

      // Change foreground font color only
      fmt.setForeground(style.second.mColor);
      if (style.second.mBold)
      {
         fmt.setFontWeight(QFont::Bold);
      }
      fmt.setFontItalic(style.second.mItalic);
      fmt.setFontUnderline(style.second.mUnderline);

      mTextFormats[style.first] = fmt;
   }
}

wizard::WsfStyler::WsfStyler()
{
   mErrorUnderlineFmt.setUnderlineColor(QColor(255, 0, 0));
   mErrorUnderlineFmt.setUnderlineStyle(QTextCharFormat::WaveUnderline);

   mWarningUnderlineFmt.setUnderlineColor(QColor(255, 201, 14));
   mWarningUnderlineFmt.setUnderlineStyle(QTextCharFormat::WaveUnderline);

   QObject::connect(wkfEnv.GetPreferenceObject<EditorPrefObject>(),
                    &EditorPrefObject::Changed,
                    [this]() { LoadSettings(); });
   LoadSettings();
}
