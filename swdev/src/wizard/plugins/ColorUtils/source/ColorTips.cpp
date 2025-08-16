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

#include "ColorTips.hpp"

#include <iostream>

#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QVBoxLayout>

#include "ColorUtils.hpp"
#include "Editor.hpp"
#include "EditorPrefObject.hpp"
#include "ParseResults.hpp"
#include "Project.hpp"
#include "Signals.hpp"
#include "UtQt.hpp"
#include "WkfEnvironment.hpp"
#include "WkfTeamVisibilityPrefObject.hpp"
#include "WsfEditor.hpp"
#include "WsfParseNode.hpp"

namespace
{
std::map<std::string, std::string> altColorNames{{"darkblue", "dark_blue"},
                                                 {"darkgreen", "dark_green"},
                                                 {"darkpurple", "dark_purple"},
                                                 {"darkred", "dark_red"},
                                                 {"darkyellow", "dark_yellow"},
                                                 {"lightblue", "light_blue"},
                                                 {"lightgreen", "light_green"},
                                                 {"lightpurple", "light_purple"},
                                                 {"lightred", "light_red"},
                                                 {"lightyellow", "light_yellow"}};

std::string GetColorString(const std::string& aString)
{
   auto color = aString;
   UtStringUtil::ToLower(color);

   auto altNameIt = altColorNames.find(color);
   if (altNameIt != altColorNames.end())
   {
      color = altNameIt->second;
   }
   return color;
}
} // namespace

// Popup window note:
//  There is no great portable way to make a popup window that doesn't steal focus from the main window.
//  Instead, this widget is a child of the text editor
wizard::ColorTips::ColorTips(WsfEditor* aEditControlPtr)
   : QWidget(aEditControlPtr, Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint)
   , mEditorPtr(aEditControlPtr)
{
   setAttribute(Qt::WA_ShowWithoutActivating);

   // Create a layout to show the label containing the color
   QVBoxLayout* layoutPtr = new QVBoxLayout();
   layoutPtr->setContentsMargins(0, 0, 0, 0);
   setLayout(layoutPtr);
   mLabel = new QLabel("", this);
   mLabel->setFrameStyle(QFrame::Raised | QFrame::Panel);
   mLabel->setContentsMargins(3, 3, 3, 3);
   layout()->addWidget(mLabel);

   UpdateScenario();
   mHoverMaintainTimer.setInterval(500);
   SetPalette();

   connect(wkfEnv.GetPreferenceObject<EditorPrefObject>(), &EditorPrefObject::Changed, this, &ColorTips::SetPalette);
   connect(wizSignals, &Signals::ActiveEditorChanged, this, &ColorTips::EndHoverShow);
   QObject::connect(&mHoverMaintainTimer, &QTimer::timeout, this, &ColorTips::HoverCheck);
}

void wizard::ColorTips::EndHoverShow()
{
   mHoverShow = false;
   mHoverMaintainTimer.stop();
   hide();
}

wizard::Project* wizard::ColorTips::GetProject()
{
   return mEditorPtr->GetProject();
}

//=====================================================
//! Determine the status of the hover tip and change the visibility and
//! content of the tip as necessary.
//! @param aCharOffset The current position of the cursor
void wizard::ColorTips::HoverShow(size_t aCharOffset)
{
   UpdateScenario();
   mHoverCharacterOffset = aCharOffset;

   if (GetProject() && GetProject()->ParseIsUpToDate())
   {
      // Reduces the amount of times that a color gets reset.
      if (mCommandExtent.Contains(mHoverCharacterOffset) && mHoverShow)
      {
         ShowWindow();
      }
      else if (UpdateTip())
      {
         mHoverShow = true;
         mHoverMaintainTimer.start();
      }
      else
      {
         EndHoverShow();
      }
   }
}

void wizard::ColorTips::HoverCheck()
{
   // Make sure cursor is over the command text range
   size_t pos = mEditorPtr->GetPosition(mEditorPtr->mapFromGlobal(QCursor::pos()));
   if (!mCommandExtent.Contains(pos))
   {
      EndHoverShow();
   }
}

void wizard::ColorTips::paintEvent(QPaintEvent* e)
{
   QWidget::paintEvent(e);
   if (mPreviousSize != mLabel->sizeHint())
   {
      mPreviousSize = mLabel->sizeHint();
      resize(mPreviousSize);
   }
}

void wizard::ColorTips::ParseUpdate(bool aParseUpdate)
{
   if (mHoverShow)
   {
      if (UpdateTip())
      {
         mHoverMaintainTimer.start();
      }
      else
      {
         EndHoverShow();
      }
   }
}

void wizard::ColorTips::ShowWindow()
{
   QPoint pt = mEditorPtr->GetPoint(mHoverCharacterOffset);
   // Move tip box down 3 pixels to prevent it from covering up underscores
   pt.ry() += mEditorPtr->GetLineHeight(0) + 3;

   move(pt);

   if (!isVisible())
   {
      show();
   }
}

bool wizard::ColorTips::UpdateTip()
{
   ParseResults* parseResultsPtr = GetProject()->GetParseResults();
   if (!parseResultsPtr || mHoverCharacterOffset > mEditorPtr->GetSource()->GetSource()->Size())
   {
      return false;
   }
   bool colorSet = false;

   UtTextDocumentRange range(mEditorPtr->GetSource(), mHoverCharacterOffset, mHoverCharacterOffset);
   WsfParseNode*       nodePtr = parseResultsPtr->FindLastParseNode(range);

   // Set color from Color script class
   if (nodePtr && (nodePtr->GetFlags() & WsfParseNode::cSCRIPT_MASK))
   {
      colorSet = SetScriptColor(nodePtr, mHoverCharacterOffset);
   }
   else // Set platform side color
   {
      colorSet = SetSideColor(nodePtr, mHoverCharacterOffset);
   }

   if (colorSet)
   {
      ShowWindow();
   }

   return colorSet;
}

// aEndPos should be the position of the end of "Construct"
void wizard::ColorTips::SetConstructColor(size_t aEndPos)
{
   bool colorSet          = false;
   auto constructColorStr = ColorUtils::GetColorString(mEditorPtr, aEndPos);
   if (!constructColorStr.empty())
   {
      colorSet = ColorUtils::IsValidColor(constructColorStr);
      if (colorSet)
      {
         mColor = ColorUtils::GetColorFromString(constructColorStr);
      }
   }
   if (!colorSet)
   {
      mColor = UtQtColor(wkfEnv.GetPreferenceObject<wkf::TeamVisibilityPrefObject>()->GetTeamUtColor("default"));
   }
   UpdateRange(aEndPos - 9, aEndPos); // 9 = Construct
   SetLabel();
}

void wizard::ColorTips::SetLabel()
{
   QPixmap ico(20, 20);
   ico.fill(mColor);
   mLabel->setFrameStyle(QFrame::Raised | QFrame::Panel);
   mLabel->setPixmap(ico);
}

void wizard::ColorTips::SetPalette()
{
   // Set the palette to the opposite theme for more definition
   auto    currentTheme = QIcon::themeName();
   QString otherTheme;
   if (currentTheme == "Dark")
   {
      otherTheme = "Light";
   }
   else
   {
      otherTheme = "Dark";
   }
   QIcon::setThemeName(otherTheme);

   QPalette pal = palette();
   pal.setBrush(QPalette::All,
                QPalette::Background,
                wkfEnv.GetPreferenceObject<EditorPrefObject>()->EditorColor("editor_syntax_tips"));
   setPalette(pal);
   setAutoFillBackground(true);
   QIcon::setThemeName(currentTheme);
}

bool wizard::ColorTips::SetScriptColor(WsfParseNode* aNodePtr, size_t aOffset)
{
   bool   colorSet       = false;
   size_t methodStartPos = 0;
   auto   scriptText     = ColorUtils::IdentifierAtPosition(mEditorPtr, aOffset, methodStartPos);
   if (!scriptText.empty() && mEditorPtr->GetSource()->GetText()[methodStartPos - 1] == '.')
   {
      size_t callerStartPos = 0;
      auto   callingObject  = ColorUtils::IdentifierAtPosition(mEditorPtr, methodStartPos - 2, callerStartPos);
      if (callingObject == "Color")
      {
         auto color = GetColorString(scriptText);

         // If the color exists but the method, which contains uppercase letters and should not exist as a color,
         // set the color to the team color. This takes care of showing invalid static methods like "Color.red()"
         if (UtColor::ColorExists(color) && !UtColor::ColorExists(scriptText))
         {
            mColor = UtQtColor(wkfEnv.GetPreferenceObject<wkf::TeamVisibilityPrefObject>()->GetTeamUtColor(color));
            UpdateRange(methodStartPos, methodStartPos + color.size());
            colorSet = true;
            SetLabel();
         }
         else if (scriptText == "Construct")
         {
            colorSet = true;
            SetConstructColor(methodStartPos + 9);
         }
      }
   }

   return colorSet;
}

bool wizard::ColorTips::SetSideColor(WsfParseNode* aNodePtr, size_t aOffset)
{
   bool colorSet = false;
   while (aNodePtr)
   {
      WsfParseRule* rulePtr = aNodePtr->GetTopLevelRule();

      // if the node is on a platform
      if (rulePtr && rulePtr->GetRuleDescription() == "<Platform>")
      {
         if (strcmp(aNodePtr->mType, "side") == 0)
         {
            size_t startPos = 0;
            auto   teamName = ColorUtils::IdentifierAtPosition(mEditorPtr, aOffset, startPos, false);

            mColor   = UtQtColor(wkfEnv.GetPreferenceObject<wkf::TeamVisibilityPrefObject>()->GetTeamUtColor(teamName));
            colorSet = true;
            SetLabel();
            UpdateRange(startPos, startPos + teamName.size());
            break;
         }
      }
      aNodePtr = aNodePtr->GetParent();
   }
   return colorSet;
}

void wizard::ColorTips::UpdateRange(size_t aBegin, size_t aEnd)
{
   mCommandExtent.SetRange(aBegin, aEnd);
}

//! Updates the ParseUpdated callback if the scenario has changed.
void wizard::ColorTips::UpdateScenario()
{
   if (GetProject())
   {
      mParseUpdateCallbackPtr = GetProject()->ParseUpdated.Connect(&ColorTips::ParseUpdate, this);
   }
}
