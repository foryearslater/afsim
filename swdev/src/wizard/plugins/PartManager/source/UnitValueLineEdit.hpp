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

#ifndef UNITVALUELINEEDIT_HPP
#define UNITVALUELINEEDIT_HPP

#include "LineEdit.hpp"
#include "UtQtUnitValueEdit.hpp"

namespace PartManager
{
//! Wraps a UtQtUnitValueEdit line edit widget. These line edits validate their
//! input based on the type of unit specified (see UtUnits::UnitType in the
//! constructor for this class). If an input is malformed, the text color is
//! changed to red to alert the user.
class UnitValueLineEdit : public LineEdit
{
public:
   explicit UnitValueLineEdit(const WsfPProxyNode& aAttrNode,
                              const std::string&   aAttributeName,
                              UtUnits::UnitType    aUnitType,
                              QWidget*             aParent = nullptr);

   template<typename UNIT_VALUE>
   bool GetValue(UNIT_VALUE& aVal)
   {
      return mUnitValueLineEdit->GetValue<UNIT_VALUE>(aVal);
   }

   void UpdateNodeValue() override;

private:
   UtQtUnitValueEdit* mUnitValueLineEdit;
};
} // namespace PartManager

#endif
