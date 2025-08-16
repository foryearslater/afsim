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

#ifndef WIZARD_NEW_FILE_PREF_WIDGET_HPP
#define WIZARD_NEW_FILE_PREF_WIDGET_HPP

#include "NewFilePrefObject.hpp"
#include "WkfPrefWidget.hpp"
#include "ui_NewFileMacroHelp.h"
#include "ui_NewFilePrefWidget.h"

namespace wizard
{
class NewFilePrefWidget : public wkf::PrefWidgetT<NewFilePrefObject>
{
   Q_OBJECT
public:
   NewFilePrefWidget(QWidget* aParent = nullptr);
   ~NewFilePrefWidget() noexcept override = default;

   QString GetCategoryHint() const override { return "Editor"; }

   void ReadPreferenceData(const NewFilePrefData& aPrefData) override;
   void WritePreferenceData(NewFilePrefData& aPrefData) override;

private:
   //! Takes the pref data for the current item and presents it to the UI.
   void ReadCurrentItem();
   //! Takes the current state of the UI and saves it to the pref data.
   void SaveCurrentItem();

   //! If the name given is not valid, presents an error the user and returns false.
   //! Otherwise returns true.
   bool ValidateName() const;

   void GoTo(const std::string& aIndex);
   void AddItem();
   void RemoveItem();

   void UpdateComboBox();

   Ui::newFilePrefWidget mUi;
   QDialog               mMacroHelpDialog;
   Ui::MacroHelpDialog   mMacroHelpDialogUi;
   std::string           mCurrentItem;

   NewFilePrefObject mTemporary;
};
} // namespace wizard

#endif
