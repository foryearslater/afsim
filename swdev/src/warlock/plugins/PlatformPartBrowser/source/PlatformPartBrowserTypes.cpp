// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2018 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "PlatformPartBrowserTypes.hpp"

QVariant WkPlatformPartBrowser::PlatformPart::FindProperty(const PropertyName& aPropertyName) const
{
   auto p = mProperties.find(aPropertyName);
   if (p == mProperties.end())
   {
      return QVariant(); // return invalid QVariant
   }
   else
   {
      return p->second;
   }
}

// return value: update was successful
bool WkPlatformPartBrowser::PlatformPart::UpdateProperty(const PropertyName& aName, const QVariant& aValue)
{
   auto p = mProperties.find(aName);
   if (p != mProperties.end())
   {
      mProperties[aName] = aValue;
      return true;
   }
   return false;
}

bool WkPlatformPartBrowser::PlatformPart::AddProperty(const PropertyName& aName, const QVariant& aValue)
{
   mProperties[aName] = aValue;
   return true;
}
