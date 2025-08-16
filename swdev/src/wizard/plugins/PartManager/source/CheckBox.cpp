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

#include "CheckBox.hpp"

#include <QCheckBox>

#include "Undo.hpp"

PartManager::CheckBox::CheckBox(const WsfPProxyNode& aAttrNode,
                                const std::string&   aAttributeName,
                                QWidget*             aParent /*= nullptr*/)
   : SingleAttribute(aAttrNode, aAttributeName, aParent)
   , mCheckBox(new QCheckBox(this))
{
   UpdateFormContent();

   connect(mCheckBox, &QCheckBox::clicked, this, &CheckBox::CheckBoxClicked);

   SetContentWidget(mCheckBox);
}

//! Set our check box state according to the Boolean value of our attribute.
void PartManager::CheckBox::UpdateFormContent()
{
   mCheckBox->setChecked(BoolAttrValue());
}

void PartManager::CheckBox::SetText(const QString& aText)
{
   mCheckBox->setText(aText);
}

//! Set our attribute proxy value every time the user manipulates the check box.
void PartManager::CheckBox::CheckBoxClicked(bool aChecked)
{
   wizard::UndoContext undoCtx("Change attribute value");
   ProxyNode().NotifyBeforeValueChanged();
   SetBoolAttrValue(aChecked);
   ProxyNode().GetValue().SetInherited(false);
   AttributeValueUpdated(aChecked);
   UpdateFormElements();
}

//! Gets the bool value of the bool or int attribute from its proxy node.
bool PartManager::CheckBox::BoolAttrValue() const
{
   WsfPProxyValue attrVal     = ProxyNode().GetValue();
   bool           boolAttrVal = false;

   if (attrVal.IsBoolValue())
   {
      boolAttrVal = attrVal.GetBoolValue();
   }
   else
   {
      int intAttrVal;
      if (attrVal.GetIntegerValue(intAttrVal))
      {
         boolAttrVal = (bool)intAttrVal;
      }
   }

   return boolAttrVal;
}

//! Sets the bool value of the bool or int attribute on its proxy node.
void PartManager::CheckBox::SetBoolAttrValue(bool aValue)
{
   WsfPProxyValue attrVal = ProxyNode().GetValue();

   if (attrVal.IsBoolValue())
   {
      attrVal.SetBoolValue(aValue);
   }
   else
   {
      attrVal.SetIntegerValue((int)aValue);
   }
}
