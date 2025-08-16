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

#include "UnitValueLineEdit.hpp"

#include "UtQtUnitValueEdit.hpp"

PartManager::UnitValueLineEdit::UnitValueLineEdit(const WsfPProxyNode& aAttrNode,
                                                  const std::string&   aAttributeName,
                                                  UtUnits::UnitType    aUnitType,
                                                  QWidget*             aParent /*= nullptr*/)
   : LineEdit(aAttrNode, aAttributeName, new UtQtUnitValueEdit(nullptr, aUnitType), aParent)
   , mUnitValueLineEdit(dynamic_cast<UtQtUnitValueEdit*>(GetLineEdit()))
{
}

void PartManager::UnitValueLineEdit::UpdateNodeValue()
{
   if (mUnitValueLineEdit->IsValid())
   {
      LineEdit::UpdateNodeValue();
   }
}
