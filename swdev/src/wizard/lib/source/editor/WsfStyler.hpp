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

#ifndef WSFSTYLER_HPP
#define WSFSTYLER_HPP

#include <unordered_map>

#include <QColor>
#include <QTextCharFormat>

#include "EditorPrefObject.hpp"
#include "TextStyler.hpp"
#include "UtCallbackHolder.hpp"
#include "UtEnumUtil.hpp"
#include "ViExport.hpp"

class WsfParseNode;
class QPlainTextEdit;

namespace wizard
{
class ParseResults;
class TextSource;
class WsfScriptInfo;

//! syntax-highlights a WSF file.
class VI_EXPORT WsfStyler
{
public:
   static int rgb(int r, int g, int b) { return r | (g << 8) | (b << 16); }
   static int rgb(const QColor& aColor) { return rgb(aColor.red(), aColor.green(), aColor.blue()); }
   enum
   {
      cPLATFORM_MARKER                  = 0,
      cCOMM_MARKER                      = 1,
      cMOVER_MARKER                     = 2,
      cPROCESSOR_MARKER                 = 3,
      cSENSOR_MARKER                    = 4,
      cWEAPON_MARKER                    = 5,
      cFUEL_MARKER                      = 6,
      cPARSEERROR_MARKER                = 7,
      cREFERENCEERROR_MARKER            = 8,
      cREFERENCEWARNING_MARKER          = 9,
      cSCRIPT_MARKER                    = 10,
      cCURRENT_LINE_MARKER              = 11,
      cDEBUG_BREAKPOINT_MARKER          = 12,
      cDEBUG_BREAKPOINT_INACTIVE_MARKER = 13,
      cDEBUG_CURRENT_LINE_MARKER        = 14,
      cDEBUG_BREAKPOINT_DISABLED_MARKER = 15
   };
   enum
   {
      cPLATFORM_MARKERBIT         = 1 << cPLATFORM_MARKER,
      cCOMM_MARKERBIT             = 1 << cCOMM_MARKER,
      cMOVER_MARKERBIT            = 1 << cMOVER_MARKER,
      cPROCESSOR_MARKERBIT        = 1 << cPROCESSOR_MARKER,
      cSENSOR_MARKERBIT           = 1 << cSENSOR_MARKER,
      cWEAPON_MARKERBIT           = 1 << cWEAPON_MARKER,
      cFUEL_MARKERBIT             = 1 << cFUEL_MARKER,
      cPARSEERROR_MARKERBIT       = 1 << cPARSEERROR_MARKER,
      cREFERENCEERROR_MARKERBIT   = 1 << cREFERENCEERROR_MARKER,
      cREFERENCEWARNING_MARKERBIT = 1 << cREFERENCEWARNING_MARKER,
      cBREAKPOINT_MARKERMASK      = (1 << cDEBUG_BREAKPOINT_INACTIVE_MARKER) | (1 << cDEBUG_BREAKPOINT_MARKER) |
                               (1 << cDEBUG_BREAKPOINT_DISABLED_MARKER)
   };

   enum
   {
      cERROR_INDICATOR_ID        = 0,
      cERROR_NOTIFICATION_ID     = 1,
      cHIGHLIGHT_NOTIFICATION_ID = 2
   };
   enum
   {
      cMARGIN_INDEX_ICON = 1,
      cMARGIN_INDEX_FOLD = 2
   };
   struct StyleRequest
   {
      StyleRequest()
         : mControl(nullptr)
         , mResultsPtr(nullptr)
         , mSourcePtr(nullptr)
         , mStylerInterface(nullptr)
         , mFirstPosition(0)
         , mLastPosition(0)
         , mShowNotifications(true)
         , mShowScriptErrors(true)
         , mStyleIfHidden(false)
         , mPositionOffset(0)
      {
      }

      void GetRangeToStyle(size_t& styleNeedStart, size_t& styleNeededEnd) const;

      QPlainTextEdit* mControl;
      ParseResults*   mResultsPtr;
      TextSource*     mSourcePtr;
      TextStyler*     mStylerInterface;
      size_t          mFirstPosition;
      size_t          mLastPosition;

      bool   mShowNotifications;
      bool   mShowScriptErrors;
      bool   mStyleIfHidden;
      size_t mPositionOffset;
   };
   WsfStyler();
   void LoadSettings();
   void UpdateStyle(const StyleRequest& aRequest);
   void UpdateScriptStyle(const StyleRequest& aRequest);
   void UpdateScriptStyle(const StyleRequest& aRequest, WsfScriptInfo* aScriptPtr);

   std::vector<std::pair<int, int>> FoldDocument(ParseResults* aResultsPtr, TextSource* aSourcePtr);

   static void ApplyEditorStyleSettings(QPlainTextEdit* aEditorPtr, int aFontSizeOverride = 0);

protected:
   TextType GetTextTypeFromNode(const WsfParseNode* aNodePtr);

   std::unordered_map<const char*, TextType>                             mNodeTypeToTextType;
   std::unordered_map<TextType, QTextCharFormat, ut::EnumHash<TextType>> mTextFormats;
   QTextCharFormat                                                       mErrorUnderlineFmt;
   QTextCharFormat                                                       mWarningUnderlineFmt;

private:
   void AddCommentsToFolding(std::vector<std::pair<int, int>>& aFoldRanges, ParseResults* aResultsPtr, TextSource* aSourcePtr);
   bool IsOverlaping(std::vector<std::pair<int, int>>& aFoldRanges, size_t& aLineBlockStart, size_t& aLineBlockEnd);
};
} // namespace wizard
#endif
