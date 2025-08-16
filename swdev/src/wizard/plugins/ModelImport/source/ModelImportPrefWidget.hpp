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
#ifndef MODELIMPORT_PREFWIDGET_HPP
#define MODELIMPORT_PREFWIDGET_HPP

#include "ModelImportPrefObject.hpp"
#include "WkfPrefWidget.hpp"
#include "ui_ModelImportPrefWidget.h"

namespace ModelImport
{
//! The widget shown in the Preferences menu.
class PrefWidget : public wkf::PrefWidgetT<ModelImport::PrefObject>
{
   Q_OBJECT

public:
   PrefWidget(QWidget* aParent, Qt::WindowFlags aFlag = Qt::WindowFlags());

   //! Updates the widget to reflect the preference data.
   //! @param aPrefData The data that the widget is being updated to.
   virtual void ReadPreferenceData(const PrefDataType& aPrefData) override;


   //! Updates preference data to reflect the state of the widget.
   //! @param aPrefData The data to fill with the widget state.
   virtual void WritePreferenceData(PrefDataType& aPrefData) override;

signals:
   void UpdateWithOverwrite();
   void UpdateWithMerge();
   void ClearData();
   void ReloadJson();

private:
   Ui::Form mUI;

   //! Creates a dialog for file selection.
   //! Sets the Search Path field.
   void BrowseForDirectory();

   //! Creates a dialog to ask about how to handle existing file.
   void PromptForUpdate();
};
} // namespace ModelImport

#endif
