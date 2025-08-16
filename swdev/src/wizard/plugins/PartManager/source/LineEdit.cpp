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

#include "LineEdit.hpp"

#include <QLineEdit>
#include <QRegExpValidator>

#include "Undo.hpp"
#include "Util.hpp"

PartManager::LineEdit::LineEdit(const WsfPProxyNode& aAttrNode,
                                const std::string&   aAttributeName,
                                QWidget*             aParent /*= nullptr*/)
   : SingleAttribute(aAttrNode, aAttributeName, aParent)
   , mLineEdit(new QLineEdit(this))
{
   mLineEdit->setValidator(new QRegExpValidator(wizard::Util::wsfValidInputRegexp, this));
   Init();
}

//! Set our line edit text equal to the text value of our attribute.
void PartManager::LineEdit::UpdateFormContent()
{
   const std::string attrValue = ProxyNode().GetValue().ValueToString();
   mLineEdit->setText(QString::fromStdString(attrValue));
}

QLineEdit* PartManager::LineEdit::GetLineEdit() const
{
   return mLineEdit;
}

//! Update our proxy node value when the user changes the line edit text.
//! Currently this sends out a proxy modified every time the user types a key.
void PartManager::LineEdit::UpdateNodeValue()
{
   WsfPProxyValue    proxyVal = ProxyNode().GetValue().Copy();
   const std::string text     = mLineEdit->text().toStdString();

   if (!text.empty() && (ProxyNode().IsInherited() || mLineEdit->isModified()))
   {
      wizard::UndoContext undoCtx("Change attribute value");
      AttributeValueUpdated(mLineEdit->text());
      UpdateFormElements();
   }
   else if (text.empty() && !ProxyNode().IsInherited())
   {
      ResetAttrValue();
   }
   else
   {
      UpdateFormContent();
   }
}

void PartManager::LineEdit::HandleEditingFinished()
{
   if (mLineEdit->isModified())
   {
      UpdateNodeValue();
   }
}

void PartManager::LineEdit::Init()
{
   UpdateFormContent();

   connect(mLineEdit, &QLineEdit::returnPressed, this, &LineEdit::UpdateNodeValue);
   connect(mLineEdit, &QLineEdit::editingFinished, this, &LineEdit::HandleEditingFinished);

   SetContentWidget(mLineEdit);
}

PartManager::LineEdit::LineEdit(const WsfPProxyNode& aAttrNode,
                                const std::string&   aAttributeName,
                                QLineEdit*           aLineEdit,
                                QWidget*             aParent /*= nullptr*/)
   : SingleAttribute(aAttrNode, aAttributeName, aParent)
   , mLineEdit(aLineEdit)
{
   mLineEdit->setParent(this);
   Init();
}
