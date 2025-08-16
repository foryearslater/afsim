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

#ifndef LINEEDIT_HPP
#define LINEEDIT_HPP

#include "SingleAttribute.hpp"

class QLineEdit;

namespace PartManager
{
class LineEdit : public SingleAttribute
{
public:
   explicit LineEdit(const WsfPProxyNode& aAttrNode, const std::string& aAttributeName, QWidget* aParent = nullptr);
   explicit LineEdit(const WsfPProxyNode& aAttrNode,
                     const std::string&   aAttributeName,
                     QLineEdit*           aLineEdit,
                     QWidget*             aParent = nullptr);

   void       UpdateFormContent() override;
   QLineEdit* GetLineEdit() const;

   virtual void UpdateNodeValue();
   void         HandleEditingFinished();

private:
   void Init();

   QLineEdit* mLineEdit;
};
} // namespace PartManager

#endif
