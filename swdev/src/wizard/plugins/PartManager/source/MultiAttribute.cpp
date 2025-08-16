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

#include "MultiAttribute.hpp"

PartManager::MultiAttribute::MultiAttribute(QWidget* parent /*= nullptr*/)
   : Attribute(parent)
{
}

void PartManager::MultiAttribute::UpdateFormContent()
{
   for (auto&& it : mAttributes)
   {
      it->UpdateFormContent();
   }
}

void PartManager::MultiAttribute::UpdateFormElements()
{
   for (auto&& it : mAttributes)
   {
      it->UpdateFormElements();
   }
}

void PartManager::MultiAttribute::UpdateParentPath(const WsfPProxyPath& oldParentPath, const WsfPProxyPath& newParentPath)
{
   for (auto&& it : mAttributes)
   {
      it->UpdateParentPath(oldParentPath, newParentPath);
   }
}

bool PartManager::MultiAttribute::IsInherited() const
{
   bool inherited = true;

   for (auto&& it : mAttributes)
   {
      if (!it->IsInherited())
      {
         inherited = false;
         break;
      }
   }

   return inherited;
}

void PartManager::MultiAttribute::AddAttribute(Attribute* attr)
{
   mAttributes.push_back(attr);
}
