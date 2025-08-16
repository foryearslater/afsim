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

#include "ColorUtilPlugin.hpp"

#include <QColor>
#include <QColorDialog>
#include <QMenu>

#include "ColorTips.hpp"
#include "ColorUtils.hpp"
#include "Editor.hpp"
#include "EditorManager.hpp"
#include "EditorPrefObject.hpp"
#include "ProjectWorkspace.hpp"
#include "Signals.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"
#include "WsfEditor.hpp"

WKF_PLUGIN_DEFINE_SYMBOLS(ColorUtils::Plugin,
                          "Color Utils",
                          "Provides the capability to construct, edit, and view colors in the Wizard text editor.",
                          "wizard")

ColorUtils::Plugin::Plugin(const QString& aPluginName, const size_t aUniqueId)
   : wizard::Plugin(aPluginName, aUniqueId)
{
   connect(wizSignals, &wizard::Signals::ActiveEditorChanged, this, &Plugin::UpdateColorTips);
   connect(wizSignals, &wizard::Signals::EditorContextMenu, this, &Plugin::EditorContextMenu);
   connect(wizSignals, &wizard::Signals::EditorToolTipEvent, this, &Plugin::EditorToolTipEvent);

   UpdateColorTips();
}

void ColorUtils::Plugin::ColorConstruct(size_t aStartingPos, const QColor& aInitialColor)
{
   QColorDialog colorDialog(wkfEnv.GetMainWindow());
   colorDialog.setOption(QColorDialog::ShowAlphaChannel);
   colorDialog.setCurrentColor(aInitialColor);
   auto result = colorDialog.exec();
   if (result != QColorDialog::Accepted)
   {
      return;
   }
   QColor            newColor = colorDialog.selectedColor();
   std::stringstream newText;
   newText << "Color.Construct(" << newColor.red() << ", " << newColor.green() << ", " << newColor.blue() << ", "
           << newColor.alpha() << ")";

   size_t line, col;
   mCurrentEditorPtr->GetLineColumn(aStartingPos, line, col);
   auto lineText = mCurrentEditorPtr->GetLineText(line);
   auto linePos  = mCurrentEditorPtr->GetSource()->GetSource()->GetLinePosition(line);
   auto endPos   = aStartingPos + 15; // Color.Construct

   // If there are parenthesis next to the Construct method, replace with new color.
   if (lineText[endPos - linePos] == '(')
   {
      auto closingPos = lineText.find(')', endPos - linePos);
      if (closingPos < lineText.size())
      {
         endPos = linePos + closingPos;
      }
      else
      {
         endPos = linePos + lineText.size();
      }
   }
   if (endPos == linePos + lineText.size())
   {
      newText << '\n';
   }
   UtTextDocumentRange replaceRange(mCurrentEditorPtr->GetSource(), aStartingPos, endPos);
   ReplaceRange(replaceRange, newText.str());
   mColorTipsPtr->SetHoverShow(false);
}

void ColorUtils::Plugin::EditorContextMenu(wizard::Editor* aEditorPtr, QMenu& aMenu)
{
   auto editorPtr = dynamic_cast<wizard::WsfEditor*>(aEditorPtr);
   if (!editorPtr)
   {
      return;
   }
   auto   contextPos   = editorPtr->GetContextMenuCharacterPosition();
   size_t constructPos = 0;
   auto   scriptText   = ColorUtils::IdentifierAtPosition(aEditorPtr, contextPos, constructPos);

   if (scriptText == "Construct")
   {
      auto callingObject = ColorUtils::IdentifierAtPosition(aEditorPtr, constructPos - 2, constructPos);
      if (callingObject == "Color")
      {
         auto   colorStr = ColorUtils::GetColorString(aEditorPtr, constructPos + 15);
         QColor initialColor;
         if (ColorUtils::IsValidColor(colorStr))
         {
            initialColor = ColorUtils::GetColorFromString(colorStr);
         }

         connect(aMenu.addAction(QIcon::fromTheme("paintbrush"), "Choose Color..."),
                 &QAction::triggered,
                 [&, constructPos, initialColor] { ColorUtils::Plugin::ColorConstruct(constructPos, initialColor); });
      }
   }
}

void ColorUtils::Plugin::EditorToolTipEvent(wizard::WsfEditor* aEditorPtr, QHelpEvent* aEvent)
{
   if (wkfEnv.GetPreferenceObject<wizard::EditorPrefObject>()->ShowSyntaxTips() && aEditorPtr == mCurrentEditorPtr &&
       mCurrentEditorPtr->hasFocus())
   {
      mColorTipsPtr->HoverShow(aEditorPtr->GetPosition(aEvent->pos()));
   }
}

void ColorUtils::Plugin::ReplaceRange(const UtTextDocumentRange& aRange, const std::string& aText)
{
   wizard::TextSource* sourcePtr = (wizard::TextSource*)aRange.mSource;
   if (sourcePtr != nullptr)
   {
      sourcePtr->Replace(aRange, aText);
   }
}

void ColorUtils::Plugin::UpdateColorTips()
{
   wizard::Editor* currentPtr = wizard::ProjectWorkspace::Instance()->GetEditorManager()->GetCurrentEditor();
   if (currentPtr != nullptr)
   {
      mColorTipsPtr     = new wizard::ColorTips(dynamic_cast<wizard::WsfEditor*>(currentPtr));
      mCurrentEditorPtr = currentPtr;
   }
}
