// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2018 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef EDITORPREFWIDGET_HPP
#define EDITORPREFWIDGET_HPP

#include <QTextCharFormat>

#include "EditorPrefObject.hpp"
#include "WkfPrefWidget.hpp"
#include "ui_EditorPrefWidget.h"

class QListWidgetItem;

namespace wizard
{
class EditorPrefWidget : public wkf::PrefWidgetT<EditorPrefObject>
{
public:
   EditorPrefWidget(QWidget* parent = nullptr);
   ~EditorPrefWidget() override = default;

   QString GetCategoryHint() const override { return "Editor"; }
   bool    GetCategoryHintPriority() const override { return true; }

private:
   void showEvent(QShowEvent* aEvent) override;
   void ReadPreferenceData(const EditorPrefData& aPrefData) override;
   void WritePreferenceData(EditorPrefData& aPrefData) override;

   void            UpdatePreview();
   QTextCharFormat MakeFormat(const TextStyle& aStyle) const;
   void            UpdateStyle();
   void            StyleClicked(QListWidgetItem* aItem);

   Ui::EditorPrefWidget   mUi;
   std::map<int, QString> mStyleNames;
   bool                   mDarkThemed;
};
} // namespace wizard

#endif
