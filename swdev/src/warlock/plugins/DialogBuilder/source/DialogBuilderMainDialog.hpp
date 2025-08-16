// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2017 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#ifndef DialogBuilderMainDialog_hpp
#define DialogBuilderMainDialog_hpp

#include <QDialog>
#include <QGridLayout>
#include <QPoint>
#include <QPushButton>

#include "DialogBuilderDialogDefinition.hpp"
#include "ui_DialogBuilderMainDialog.h"

namespace WkDialogBuilder
{
// This class is the primary Dialog that appears when a user is editing or creating a custom dialog.
class MainDialog : public QDialog
{
public:
   MainDialog(QWidget* parent = nullptr);

   // Provide a list of names that would be conflicts.  This way the PrefWidget does not need to do the check after this
   //  dialog completed but instead the user can be warned prior to accepting the dialog completed signal.
   void SetBannedNames(const QStringList& aNames) { mBannedNames = aNames; }

   bool Execute(DialogDef& aDef);

private:
   void ButtonContextMenuRequested(QPoint pos);
   void HandleButtonClicked();
   void GetDialog(DialogDef& aDef);
   void Populate(const DialogDef& aDef);
   void PopulateScrollArea(int aRows, int aCols);
   void PopulateScriptButton(QPushButton* aButton, const ScriptDefinition& aDef);

   int GetMinRows();
   int GetMinColumns();

   void SwapScriptLocations(ScriptDefinition& aFirst, ScriptDefinition& aSecond);
   void MoveScriptLocation(ScriptDefinition& aDefinition, int aRow, int aCol);

   void SwapButtonLocations(int aRow1, int aCol1, int aRow2, int aCol2);

   int GetScriptIndex(int aRow, int aCol);

   bool eventFilter(QObject* watched, QEvent* event) override;
   bool VerifyButtonValidity(QString& aErrorMsg);

   // names that are already being used by other dialogs, and therefore not allowed.
   QStringList       mBannedNames;
   ScriptDefinitions mScripts;

   QList<QPushButton*> mScrollAreaButtons;
   QGridLayout*        mScrollAreaLayout;

   bool    mScriptToMoveValid;
   QString mScriptToMoveName;
   int     mScriptToMoveRow;
   int     mScriptToMoveColumn;

   QPoint mDragStartPos;

   Ui::DialogBuilderMainDialog mUi;
};
} // namespace WkDialogBuilder
#endif
