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

#ifndef TOOLPREFWIDGET_HPP
#define TOOLPREFWIDGET_HPP

#include "ToolPrefObject.hpp"
#include "WkfPrefWidget.hpp"
#include "ui_ToolLauncherPrefWidget.h"

class QLineEdit;
class QListWidgetItem;

namespace wizard
{
class ToolPrefWidget : public wkf::PrefWidgetT<ToolPrefObject>
{
public:
   ToolPrefWidget(QWidget* parent = nullptr);
   ~ToolPrefWidget() override = default;

private:
   void    showEvent(QShowEvent* aEvent) override;
   void    ReadPreferenceData(const ToolPrefData& aPrefData) override;
   void    WritePreferenceData(ToolPrefData& aPrefData) override;
   void    Add();
   void    Remove();
   void    ToolClicked(QListWidgetItem* aItem);
   QString FindAvailableAlias(const QString& input);
   void    DataChanged();
   void    PopulateCME_Tools(const QString& aPath);
   void    CommandLineVariableMenuRequested();
   void    WorkingPathVariableMenuRequested();
   void    BuildVariableMenu(QLineEdit* aLineEdit);

   Ui::ToolLauncherPrefWidget mUi;
};
} // namespace wizard

#endif
