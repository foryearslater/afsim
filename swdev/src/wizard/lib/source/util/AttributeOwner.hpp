// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2003-2013 The Boeing Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#ifndef ATTRIBUTEOWNER_HPP
#define ATTRIBUTEOWNER_HPP
#include "ViExport.hpp"

namespace wizard
{
class AttributeSet;
//! Owns a set of attributes
//! instances of AttributeOwner are bound to their associated attribute sets
//! AttributeStateChanged() is called after the attribute set is modified by an
//! outside source
class VI_EXPORT AttributeOwner
{
public:
   explicit AttributeOwner();
   explicit AttributeOwner(AttributeSet* aAttributes);
   virtual ~AttributeOwner();

   //! The associated attribute set has been modified
   //! Derived classes should implement this method applying all attributes
   virtual void  AttributeStateChanged() {}
   bool          HasAttributes() const { return mAttributes != nullptr; }
   AttributeSet& Attributes() const { return *mAttributes; }

protected:
   void SetAttributes(AttributeSet* aAttributesPtr);

private:
   AttributeSet* mAttributes;
};
} // namespace wizard
#endif
