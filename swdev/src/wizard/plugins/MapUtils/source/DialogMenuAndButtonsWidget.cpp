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

#include "DialogMenuAndButtonsWidget.hpp"

#include <QStringList>

namespace MapUtils
{
namespace ScenarioTransformation
{

DialogMenuAndButtonsWidget::DialogMenuAndButtonsWidget(QWidget* aParent, Qt::WindowFlags aFlags /* = Qt::WindowFlags() */)
{
   mUi.setupUi(this);
}

void DialogMenuAndButtonsWidget::ConstructMenu(const QStringList& aMenuItemList)
{
   // clear the menu to prepare it for construction
   mUi.mMenu->clear();

   // build the menu from the list of items
   for (const auto& item : aMenuItemList)
   {
      // if the item has no text, create a separator
      if (item.isEmpty())
      {
         mUi.mMenu->insertSeparator(mUi.mMenu->count());
      }
      // otherwise, create a menu entry using the provided text
      else
      {
         mUi.mMenu->addItem(item);
      }
   }
}

void DialogMenuAndButtonsWidget::UseOkayAndCancel()
{
   // prevent unnecessary calls
   if (!mUsingOkayAndCancel)
   {
      mUsingOkayAndCancel = true;
      mUsingApplyAndReset = false;
      // remove widgets from layout
      mUi.mMenuAndButtonsLayout->removeWidget(mUi.mOkay);
      mUi.mMenuAndButtonsLayout->removeWidget(mUi.mCancel);
      mUi.mMenuAndButtonsLayout->removeWidget(mUi.mApply);
      mUi.mMenuAndButtonsLayout->removeWidget(mUi.mReset);
      // hide the "Apply" and "Reset" buttons
      mUi.mApply->setHidden(true);
      mUi.mReset->setHidden(true);
      // show the "Okay" and "Cancel" buttons
      mUi.mOkay->setVisible(true);
      mUi.mCancel->setVisible(true);
      if (mHelpButtonHidden)
      {
         // add widgets to layout, shifted right by one
         mUi.mMenuAndButtonsLayout->addWidget(mUi.mOkay, 1, 4, 1, 1);
         mUi.mMenuAndButtonsLayout->addWidget(mUi.mCancel, 1, 5, 1, 1);
      }
      else
      {
         // add widgets to layout, shifted right by one
         mUi.mMenuAndButtonsLayout->addWidget(mUi.mOkay, 1, 3, 1, 1);
         mUi.mMenuAndButtonsLayout->addWidget(mUi.mCancel, 1, 4, 1, 1);
      }
   }
}

void DialogMenuAndButtonsWidget::UseApplyAndReset()
{
   // prevent unnecessary calls
   if (!mUsingApplyAndReset)
   {
      mUsingOkayAndCancel = false;
      mUsingApplyAndReset = true;
      // remove widgets from layout
      mUi.mMenuAndButtonsLayout->removeWidget(mUi.mOkay);
      mUi.mMenuAndButtonsLayout->removeWidget(mUi.mCancel);
      mUi.mMenuAndButtonsLayout->removeWidget(mUi.mApply);
      mUi.mMenuAndButtonsLayout->removeWidget(mUi.mReset);
      // hide the "Okay" and "Cancel" buttons
      mUi.mOkay->setHidden(true);
      mUi.mCancel->setHidden(true);
      // show the "Apply" and "Reset" buttons
      mUi.mApply->setVisible(true);
      mUi.mReset->setVisible(true);
      if (mHelpButtonHidden)
      {
         // add widgets to layout, shifted right by one
         mUi.mMenuAndButtonsLayout->addWidget(mUi.mApply, 1, 4, 1, 1);
         mUi.mMenuAndButtonsLayout->addWidget(mUi.mReset, 1, 5, 1, 1);
      }
      else
      {
         // add widgets to layout, shifted right by one
         mUi.mMenuAndButtonsLayout->addWidget(mUi.mApply, 1, 3, 1, 1);
         mUi.mMenuAndButtonsLayout->addWidget(mUi.mReset, 1, 4, 1, 1);
      }
   }
}

void DialogMenuAndButtonsWidget::HideMenu()
{
   // prevent unnecessary calls
   if (!mMenuHidden)
   {
      mMenuHidden = true;
      mUi.mMenu->setHidden(true);
   }
}

void DialogMenuAndButtonsWidget::ShowMenu()
{
   // prevent unnecessary calls
   if (mMenuHidden)
   {
      mMenuHidden = false;
      mUi.mMenu->setVisible(true);
   }
}

void DialogMenuAndButtonsWidget::HideHelpButton()
{
   // prevent unnecessary calls
   if (!mHelpButtonHidden)
   {
      mHelpButtonHidden = true;
      // remove widgets from layout
      if (mUsingOkayAndCancel)
      {
         mUi.mMenuAndButtonsLayout->removeWidget(mUi.mOkay);
         mUi.mMenuAndButtonsLayout->removeWidget(mUi.mCancel);
      }
      else if (mUsingApplyAndReset)
      {
         mUi.mMenuAndButtonsLayout->removeWidget(mUi.mApply);
         mUi.mMenuAndButtonsLayout->removeWidget(mUi.mReset);
      }
      mUi.mMenuAndButtonsLayout->removeWidget(mUi.mHelp);
      // hide "Help" button
      mUi.mHelp->setHidden(true);
      // add widgets to layout, shifted right by one
      if (mUsingOkayAndCancel)
      {
         mUi.mMenuAndButtonsLayout->addWidget(mUi.mOkay, 1, 4, 1, 1);
         mUi.mMenuAndButtonsLayout->addWidget(mUi.mCancel, 1, 5, 1, 1);
      }
      else if (mUsingApplyAndReset)
      {
         mUi.mMenuAndButtonsLayout->addWidget(mUi.mApply, 1, 4, 1, 1);
         mUi.mMenuAndButtonsLayout->addWidget(mUi.mReset, 1, 5, 1, 1);
      }
   }
}

void DialogMenuAndButtonsWidget::ShowHelpButton()
{
   // prevent unnecessary calls
   if (mHelpButtonHidden)
   {
      mHelpButtonHidden = true;
      // remove widgets from layout
      if (mUsingOkayAndCancel)
      {
         mUi.mMenuAndButtonsLayout->removeWidget(mUi.mOkay);
         mUi.mMenuAndButtonsLayout->removeWidget(mUi.mCancel);
      }
      else if (mUsingApplyAndReset)
      {
         mUi.mMenuAndButtonsLayout->removeWidget(mUi.mApply);
         mUi.mMenuAndButtonsLayout->removeWidget(mUi.mReset);
      }
      // show "Help" button
      mUi.mHelp->setVisible(true);
      // add widgets to layout, shifted left by one
      if (mUsingOkayAndCancel)
      {
         mUi.mMenuAndButtonsLayout->addWidget(mUi.mOkay, 1, 3, 1, 1);
         mUi.mMenuAndButtonsLayout->addWidget(mUi.mCancel, 1, 4, 1, 1);
      }
      else if (mUsingApplyAndReset)
      {
         mUi.mMenuAndButtonsLayout->addWidget(mUi.mApply, 1, 3, 1, 1);
         mUi.mMenuAndButtonsLayout->addWidget(mUi.mReset, 1, 4, 1, 1);
      }
      mUi.mMenuAndButtonsLayout->addWidget(mUi.mHelp, 1, 5, 1, 1);
   }
}

} // namespace ScenarioTransformation
} // namespace MapUtils
