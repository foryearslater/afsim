// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2020 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "DialogButtonsWidget.hpp"

#include <QBoxLayout>
#include <QPushButton>

namespace wizard
{
namespace EventOutput
{

DialogButtonsWidget::DialogButtonsWidget(QWidget* aParent /* = nullptr */)
   : QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Reset | QDialogButtonBox::Help,
                      aParent)
{
   // change the name of the Reset button to Restore Defaults
   button(QDialogButtonBox::Reset)->setText("Restore Defaults");

   // do the necessary connections
   connect(this,
           &DialogButtonsWidget::clicked,
           [this](QAbstractButton* aButton)
           {
              switch (buttonRole(aButton))
              {
              case QDialogButtonBox::AcceptRole:
                 emit OkayClicked();
                 break;
              case QDialogButtonBox::RejectRole:
                 emit CancelClicked();
                 break;
              case QDialogButtonBox::HelpRole:
                 emit HelpClicked();
                 break;
              case QDialogButtonBox::ResetRole:
                 emit RestoreDefaultsClicked();
                 break;
              default:
                 return;
              }
           });
}

void DialogButtonsWidget::HideHelpButton()
{
   // prevent unnecessary calls
   if (!mHelpButtonHidden)
   {
      mHelpButtonHidden = true;
      removeButton(button(QDialogButtonBox::Help));
   }
}

void DialogButtonsWidget::ShowHelpButton()
{
   // prevent unnecessary calls
   if (mHelpButtonHidden)
   {
      mHelpButtonHidden = false;
      addButton(QDialogButtonBox::Help);
   }
}

} // namespace EventOutput
} // namespace wizard
