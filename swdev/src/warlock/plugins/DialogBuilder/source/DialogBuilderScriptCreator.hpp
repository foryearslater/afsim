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
#ifndef DialogBuilderScriptCreator_hpp
#define DialogBuilderScriptCreator_hpp

#include <QDialog>

#include "DialogBuilderDialogDefinition.hpp"
#include "ui_DialogBuilderScriptCreator.h"

namespace WkDialogBuilder
{
// This is the dialog that appears when a user is defining a script that will used in the a user defined dialog.
class ScriptCreator : public QDialog
{
public:
   ScriptCreator(QWidget* parent = nullptr);

   bool Execute(ScriptDefinition& aDef);

private:
   void ContextChanged(const QString& aText);

   void GetScript(ScriptDefinition& aDef);

   void CreateAndShowFilterDialog(const int aRow);

   void Populate(const ScriptDefinition& aDef);
   void AddArgument(const warlock::script::Argument& aArgument, const warlock::script::Filter& aFilter);

   void BrowseIconHandler();
   void UpdateScriptArgsTable(int aValue);

   Ui::DialogBuilderScriptCreator mUi;
};
} // namespace WkDialogBuilder
#endif
