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

#ifndef ENUMCOMBOBOX_HPP
#define ENUMCOMBOBOX_HPP

#include "LazyComboBox.hpp"
#include "SingleAttribute.hpp"
#include "WsfPProxyBasicTypes.hpp"

namespace PartManager
{
class EnumComboBox : public SingleAttribute
{
public:
   explicit EnumComboBox(const WsfPProxyNode& aAttrNode, const std::string& aAttributeName, QWidget* aParent = nullptr);
   void UpdateFormContent() override;

   void HandleUserSelection(int aIndex);

private:
   const WsfPEnumIntMap& EnumIntMap() const;

   LazyComboBox* mComboBox;
};
} // namespace PartManager

#endif
