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

#ifndef SYNTAXTIPS_HPP
#define SYNTAXTIPS_HPP
#include <QLabel>
#include <QTimer>
#include <QWidget>

#include "UtCallback.hpp"
#include "UtCallbackHolder.hpp"
#include "UtTextDocument.hpp"

class QLabel;

namespace wizard
{
class Project;
class WsfEditor;
class WsfScriptLookup;

//! A popup window displaying the syntax for commands as the user types.
class SyntaxTips : public QWidget
{
public:
   explicit SyntaxTips(WsfEditor* aEditControlPtr);
   ~SyntaxTips() override;
   void ParseUpdate(bool aParseUpdate);

   void  ClearLabels();
   void  HoverShow(size_t aCharOffset);
   void  EndHoverShow();
   void  UserModification();
   void  CursorMove();
   void  Hide();
   QRect GetGeometry();

protected:
   Project*   getProject();
   void       UpdateScriptTips();
   void       CheckSizeChange();
   void       paintEvent(QPaintEvent*) override;
   bool       UpdateSuggestions(size_t aCharacterOffset);
   void       HoverCheck();
   void       UpdateWindow(size_t aCursorOffset);
   void       UpdateScenario();
   void       resizeEvent(QResizeEvent*) override;
   WsfEditor* mEditControlPtr;

   // Values for mEditShow:
   // 0: no edits, no show
   // 1: pending edits, waiting for parse
   // 2: edits, showing
   int    mEditShow;
   bool   mHoverShow;
   size_t mHoverCharacterOffset;

   UtCallbackHolder mCallbacks;

   QLabel* mLabel;
   QSize   mPreviousSize;

   UtTextRange                 mCommandExtent;
   QTimer                      mHoverMaintainTimer;
   QRect                       mPreviousRect;
   std::unique_ptr<UtCallback> mParseUpdateCallbackPtr{nullptr};
   // std::vector<QLabel*>    mLabels;

   WsfScriptLookup* mScriptLookupPtr;
   bool             mNeedScriptUpdateOnCursorMove;
   QTimer           mCursorMoveUpdateTimer;
};
} // namespace wizard
#endif
