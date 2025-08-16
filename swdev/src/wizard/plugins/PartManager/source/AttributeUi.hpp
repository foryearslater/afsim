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

#ifndef ATTRIBUTEUI_HPP
#define ATTRIBUTEUI_HPP

#include "ui_Attribute.h"

class QPushButton;
class QWidget;

namespace PartManager
{
class AttributeUi
{
public:
   explicit AttributeUi();
   explicit AttributeUi(QWidget* contentWidget);
   AttributeUi(const AttributeUi&) = delete; // Copy
   virtual ~AttributeUi()          = default;

   QPushButton* ResetButton() const;
   QWidget*     ContentWidget() const;
   QWidget*     Widget() const;

   void SetupUi(QWidget* widget);
   void SetContentWidget(QWidget* contentWidget);

private:
   Ui::Attribute mUi;

   // The entire attribute widget.
   QWidget* mWidget;

   // The main content/input widget of this attribute, e.g., ViPpmCheckBox
   // would pass in a QCheckBox* as its content widget. This is the widget
   // the user will interact with to explicitly set/change the value of the
   // attribute.
   QWidget* mContentWidget;
};
} // namespace PartManager

#endif
