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

#include "MapComboBox.hpp"

#include "LazyComboBox.hpp"
#include "Undo.hpp"
#include "Util.hpp"

PartManager::MapComboBox::MapComboBox(const WsfPProxyNode&   aAttrNode,
                                      const std::string&     aAttributeName,
                                      const WsfPM_ObjectMap& aObjectMap,
                                      QWidget*               aParent /*= nullptr*/)
   : SingleAttribute(aAttrNode, aAttributeName, aParent)
   , mObjectMap(aObjectMap)
   , mComboBox(new LazyComboBox(this))
{
   UpdateFormContent();
   mComboBox->setEditable(true);
   mComboBox->setValidator(new QRegExpValidator(wizard::Util::wsfValidInputRegexp, this));

   connect(mComboBox, &LazyComboBox::OnBeforePopup, this, &MapComboBox::UpdateComboBox);
   connect(mComboBox,
           static_cast<void (LazyComboBox::*)(const QString&)>(&LazyComboBox::activated),
           this,
           &MapComboBox::HandleUserSelection);

   SetContentWidget(mComboBox);
}

void PartManager::MapComboBox::UpdateFormContent()
{
   UpdateComboBox();
}

void PartManager::MapComboBox::HandleUserSelection(QString aSelection)
{
   WsfPProxyValue attrVal = ProxyNode().GetValue();

   if (!aSelection.isEmpty())
   {
      wizard::UndoContext undoCtx("Change attribute value");
      ProxyNode().NotifyBeforeValueChanged();
      attrVal.SetValue(aSelection.toStdString());
      attrVal.SetInherited(false);
      AttributeValueUpdated(aSelection);
      UpdateFormElements();
   }
}

void PartManager::MapComboBox::UpdateComboBox()
{
   const std::string& curNodeVal      = ProxyNode().GetValue().ValueToString();
   bool               foundCurNodeVal = false;

   std::vector<WsfPProxyNode> nodes;
   mObjectMap.GetValues(nodes);

   mComboBox->clear();
   for (size_t i = 0; i < nodes.size(); i++)
   {
      const WsfPProxyNode& node = nodes[i];
      mComboBox->addItem(QString::fromStdString(node.GetName()));
      if (!curNodeVal.empty() && node.GetName() == curNodeVal)
      {
         mComboBox->setCurrentIndex(ut::cast_to_int(i));
         foundCurNodeVal = true;
      }
   }
   if (!foundCurNodeVal)
   {
      mComboBox->insertItem(0, QString::fromStdString(curNodeVal));
      mComboBox->setCurrentIndex(0);
   }
}
