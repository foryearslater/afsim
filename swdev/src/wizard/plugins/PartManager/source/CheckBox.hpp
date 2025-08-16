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

#ifndef CHECKBOX_HPP
#define CHECKBOX_HPP

#include "SingleAttribute.hpp"

class QCheckBox;

//! Implements a check box widget to represent a Boolean attribute. For example,
//! this could represent the "on" attribute of a sensor.
//!
//! This class also handles Integer attributes (and therefore enumerations) - but
//! obviously you should be sure that you only care about the integer values 0
//! and 1 (and/or are representing a two-value enumeration, e.g., MSL and AGL,
//! which uses values 0 and 1).
namespace PartManager
{
class CheckBox : public SingleAttribute
{
public:
   explicit CheckBox(const WsfPProxyNode& aAttrNode, const std::string& aAttributeName, QWidget* aParent = nullptr);

   void         UpdateFormContent() override;
   void         SetText(const QString& aText);
   virtual void CheckBoxClicked(bool aChecked);

private:
   bool BoolAttrValue() const;
   void SetBoolAttrValue(bool aValue);

   QCheckBox* mCheckBox;
};
} // namespace PartManager

#endif
