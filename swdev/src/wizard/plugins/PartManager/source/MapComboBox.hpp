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

#ifndef MAPCOMBOBOX_HPP
#define MAPCOMBOBOX_HPP

#include "SingleAttribute.hpp"
#include "WsfPM_ObjectMap.hpp"

namespace PartManager
{
class LazyComboBox;

class MapComboBox : public SingleAttribute
{
public:
   explicit MapComboBox(const WsfPProxyNode&   aAttrNode,
                        const std::string&     aAttributeName,
                        const WsfPM_ObjectMap& aObjectMap,
                        QWidget*               aParent = nullptr);
   void UpdateFormContent() override;

   void HandleUserSelection(QString aSelection);
   void UpdateComboBox();

private:
   WsfPM_ObjectMap mObjectMap;
   LazyComboBox*   mComboBox;
};
} // namespace PartManager

#endif
