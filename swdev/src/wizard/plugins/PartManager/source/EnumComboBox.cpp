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

#include "EnumComboBox.hpp"

#include <QVariant>

#include "LazyComboBox.hpp"
#include "Undo.hpp"

PartManager::EnumComboBox::EnumComboBox(const WsfPProxyNode& aAttrNode,
                                        const std::string&   aAttributeName,
                                        QWidget*             aParent /*= nullptr*/)
   : SingleAttribute(aAttrNode, aAttributeName, aParent)
   , mComboBox(new LazyComboBox(this))
{
   mComboBox->addItem("Undefined");

   for (WsfPEnumIntMap::const_iterator i = EnumIntMap().begin(); i != EnumIntMap().end(); ++i)
   {
      mComboBox->addItem(QString::fromStdString(i->second), QVariant(i->first));
   }

   UpdateFormContent();
   connect(mComboBox,
           static_cast<void (LazyComboBox::*)(int)>(&LazyComboBox::activated),
           this,
           &EnumComboBox::HandleUserSelection);
   SetContentWidget(mComboBox);
}

void PartManager::EnumComboBox::UpdateFormContent()
{
   int enumVal;

   if (ProxyNode().GetValue().GetIntegerValue(enumVal))
   {
      mComboBox->setCurrentIndex(mComboBox->findData(QVariant(enumVal)));
   }
   else
   {
      mComboBox->setCurrentIndex(0);
   }
}

void PartManager::EnumComboBox::HandleUserSelection(int aIndex)
{
   wizard::UndoContext undoCtx("Change attribute value");
   WsfPProxyValue      attrVal = ProxyNode().GetValue();

   ProxyNode().NotifyBeforeValueChanged();

   // Index 0 is reserved for "Undefined", which isn't in the enum. So check >= 1.
   if (aIndex >= 1)
   {
      attrVal.SetIntegerValue(mComboBox->itemData(aIndex).toInt());
   }
   else
   {
      attrVal.SetUnset();
   }

   attrVal.SetInherited(false);

   UpdateFormElements();
}

const WsfPEnumIntMap& PartManager::EnumComboBox::EnumIntMap() const
{
   const WsfPProxyEnumType* enumType = dynamic_cast<const WsfPProxyEnumType*>(ProxyNode().GetValue().GetType());
   assert(enumType);
   return enumType->mIntToString;
}
