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
#include "AttributeOwner.hpp"

#include "AttributeSet.hpp"

//! Construct without an attribute set
//! Should be avoided, but allows assignment of attributes after construction
wizard::AttributeOwner::AttributeOwner()
   : mAttributes(nullptr)
{
}

wizard::AttributeOwner::~AttributeOwner()
{
   if (mAttributes)
      mAttributes->UnbindOwner(this);
}

wizard::AttributeOwner::AttributeOwner(AttributeSet* aAttributes)
   : mAttributes(aAttributes)
{
   if (mAttributes)
      mAttributes->BindOwner(this);
}

void wizard::AttributeOwner::SetAttributes(AttributeSet* aAttributesPtr)
{
   if (mAttributes != aAttributesPtr)
   {
      if (mAttributes)
      {
         mAttributes->UnbindOwner(this);
      }
      mAttributes = aAttributesPtr;
      if (mAttributes)
      {
         mAttributes->BindOwner(this);
      }
   }
}
