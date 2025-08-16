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

#include "AttributeUi.hpp"

PartManager::AttributeUi::AttributeUi()
   : mWidget(nullptr)
   , mContentWidget(nullptr)
{
}

//! @returns The attribute's reset button.
QPushButton* PartManager::AttributeUi::ResetButton() const
{
   return mUi.resetButton;
}

//! @returns The attribute's content widget.
QWidget* PartManager::AttributeUi::ContentWidget() const
{
   return mContentWidget;
}

//! @returns The entire attribute widget (parent widget of the reset button and
//! content widget).
QWidget* PartManager::AttributeUi::Widget() const
{
   return mWidget;
}

//! Sets the main widget and configures it.
void PartManager::AttributeUi::SetupUi(QWidget* widget)
{
   mWidget = widget;
   mUi.setupUi(mWidget);
   mUi.resetButton->setIcon(QIcon::fromTheme("restart"));
}

// Set the main content widget of this attribute. For instance, a ViPpmCheckBox
// would pass in a QCheckBox* as its content widget.
void PartManager::AttributeUi::SetContentWidget(QWidget* contentWidget)
{
   // Ensure there's ever only one added content widget.
   if (mContentWidget)
   {
      mUi.layout->removeWidget(mContentWidget);
   }
   mUi.layout->removeWidget(mUi.resetButton);

   mUi.layout->addWidget(contentWidget);
   mUi.layout->addWidget(mUi.resetButton);

   mContentWidget = contentWidget;
}

PartManager::AttributeUi::AttributeUi(QWidget* contentWidget)
   : mWidget(new QWidget)
   , mContentWidget(contentWidget)
{
   mUi.setupUi(mWidget);
   SetContentWidget(mContentWidget);
}
