// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2019 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef COLORTIPS_HPP
#define COLORTIPS_HPP
#include <QLabel>
#include <QTimer>
#include <QWidget>

#include "UtColor.hpp"
#include "UtTextDocument.hpp"

class WsfParseNode;
namespace wizard
{
class Project;
class WsfEditor;
class WsfScriptLookup;

//! A popup window displaying the syntax for commands as the user types.
class ColorTips : public QWidget
{
public:
   ColorTips() = default;
   explicit ColorTips(WsfEditor* aEditControlPtr);
   ~ColorTips() = default;

   void HoverShow(size_t aOffset);
   void SetHoverShow(bool aHoverShow) { mHoverShow = aHoverShow; }

private:
   void     EndHoverShow();
   Project* GetProject();
   void     HoverCheck();
   void     paintEvent(QPaintEvent*) override;
   void     ParseUpdate(bool aParseUpdate);

   void SetConstructColor(size_t aEndPos);
   void SetLabel();
   void SetPalette();
   bool SetScriptColor(WsfParseNode* aNodePtr, size_t aOffset);
   bool SetSideColor(WsfParseNode* aNodePtr, size_t aOffset);
   void ShowWindow();
   void UpdateRange(size_t aBegin, size_t aEnd);
   void UpdateScenario();
   bool UpdateTip();

   QColor                      mColor;
   UtTextRange                 mCommandExtent;
   WsfEditor*                  mEditorPtr{nullptr};
   size_t                      mHoverCharacterOffset;
   QTimer                      mHoverMaintainTimer;
   bool                        mHoverShow{false};
   QLabel*                     mLabel{nullptr};
   std::unique_ptr<UtCallback> mParseUpdateCallbackPtr{nullptr};
   QSize                       mPreviousSize;
};
} // namespace wizard
#endif
