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

#include "EditModeBanner.hpp"

#include "Signals.hpp"
#include "Undo.hpp"
#include "UtMemory.hpp"

wizard::EditModeBanner::EditModeBanner(bool aBannerIsForTextEditor, Undo* aUndoPtr)
{
   mBannerIsForTextEditor = aBannerIsForTextEditor;
   mUi.setupUi(this);
   connect(mUi.applyButton, &QPushButton::clicked, this, &EditModeBanner::ApplyClick);
   connect(mUi.revertButton, &QPushButton::clicked, this, &EditModeBanner::RevertClick);
   connect(wizSignals, &Signals::EditModeSwitch, this, &EditModeBanner::EditModeSwitch);
   hide();
   mUndoStackPtr = aUndoPtr->GetUndoStack();
   connect(mUndoStackPtr, &QUndoStack::indexChanged, this, &EditModeBanner::UndoIndexChanged);
   mUi.undoButton->setDefaultAction(mUndoStackPtr->createUndoAction(this));
   mUi.redoButton->setDefaultAction(mUndoStackPtr->createRedoAction(this));
}

void wizard::EditModeBanner::UndoIndexChanged(int aIndex)
{
   QString changeText;
   int     i = aIndex - 1;
   for (; i >= 0 && i >= aIndex - 3; --i)
   {
      const QUndoCommand* cmd = mUndoStackPtr->command(i);
      if (cmd)
      {
         changeText += "&bull;";
         changeText += cmd->text();
         changeText += "<BR/>";
      }
   }
   if (i >= 0)
   {
      changeText += "&bull;";
      changeText += QString::number(i + 1) + " more actions...";
   }
   mUi.changeListLabel->setText(changeText);
   if (aIndex == 0)
   {
      // if all actions have been undone, we can go back to text mode
      wizSignals->revertVisualModeChanges();
   }
   else
   {
      // update redo/undo buttons
      //       const QUndoCommand* redoCmd = mUndoStackPtr->command(aIndex);
      //       const QUndoCommand* undoCmd = mUndoStackPtr->command(aIndex - 1);
      //       if (redoCmd) mUi.redoButton->setText(QString("Redo ") + redoCmd->actionText());
      //       else         mUi.redoButton->setText("Redo");
      //       mUi.redoButton->setEnabled(redoCmd != 0);
      //       if (undoCmd) mUi.undoButton->setText(QString("Undo ") + undoCmd->actionText());
      //       else         mUi.undoButton->setText("Undo");
      //       mUi.undoButton->setEnabled(undoCmd != 0);
   }
}

void wizard::EditModeBanner::ShowVisualMode()
{
   show(); // WIZARD_TODO
   mUi.stackedWidget->setCurrentWidget(mUi.visualModePage);
}

void wizard::EditModeBanner::ShowApplying()
{
   show(); // WIZARD_TODO
   mUi.stackedWidget->setCurrentWidget(mUi.applyingChangesPage);
}

void wizard::EditModeBanner::ApplyClick()
{
   wizSignals->applyProxy();
   //   ShowApplying();
}

void wizard::EditModeBanner::RevertClick()
{
   wizSignals->revertVisualModeChanges();
}

void wizard::EditModeBanner::EditModeSwitch(Vi::EditMode aEditMode)
{
   if (mBannerIsForTextEditor)
   {
      if (aEditMode == Vi::cEDIT_MODE_VISUAL)
      {
         ShowVisualMode();
      }
      else if (aEditMode == Vi::cEDIT_MODE_SWITCHING_TO_TEXT)
      {
         ShowApplying();
      }
      else
      {
         hide();
      }
   }
   else
   {
      // logic TBD
   }
}
