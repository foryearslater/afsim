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
#ifndef DialogBuilderPrefWidget_hpp
#define DialogBuilderPrefWidget_hpp

#include "DialogBuilderMainDialog.hpp"
#include "DialogBuilderPrefObject.hpp"
#include "WkfPrefWidget.hpp"
#include "ui_DialogBuilderPrefWidget.h"

namespace WkDialogBuilder
{
class PrefWidget : public wkf::PrefWidgetT<PrefObject>
{
public:
   PrefWidget();
   ~PrefWidget() override = default;

private:
   void AddAction();
   void DeleteAction();
   void EditAction();

   void PopulateWidgets();
   void ReadPreferenceData(const PrefData& aPrefData) override;
   void WritePreferenceData(PrefData& aPrefData) override;
   void RestoreDefaults() override;

   void SetBannedNames(MainDialog& dialog, QString aExcludedName = "");

   QMap<QString, DialogDef> mDialogMap;

   Ui::DialogBuilderPrefWidget mUi;
};
} // namespace WkDialogBuilder
#endif
