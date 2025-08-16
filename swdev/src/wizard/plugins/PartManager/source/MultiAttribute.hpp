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

#ifndef MULTIATTRIBUTE_HPP
#define MULTIATTRIBUTE_HPP

#include "Attribute.hpp"

namespace PartManager
{
//! This class provides a way to represent multiple attributes of a platform or
//! platform part as a single entity and widget.
class MultiAttribute : public Attribute
{
public:
   explicit MultiAttribute(QWidget* parent = nullptr);

   void UpdateFormContent() override;
   void UpdateFormElements() override;
   void UpdateParentPath(const WsfPProxyPath& oldParentPath, const WsfPProxyPath& newParentPath) override;
   bool IsInherited() const override;

protected:
   void AddAttribute(Attribute* attr);

private:
   AttrList mAttributes;
};
} // namespace PartManager

#endif
