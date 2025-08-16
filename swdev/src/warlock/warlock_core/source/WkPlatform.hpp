// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2017 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef WKPLATFORM_HPP
#define WKPLATFORM_HPP

#include "warlock_core_export.h"

#include <set>

#include "DisEntityId.hpp"
#include "WkfPlatform.hpp"

namespace warlock
{
class WARLOCK_CORE_EXPORT Platform : public wkf::Platform
{
public:
   Platform(const std::string& aName);

   void   SetUpdateTime(double aLastUpdateTime) { mUpdateTime = aLastUpdateTime; }
   double GetUpdateTime() const { return mUpdateTime; }

   void SetExternallyControlled(bool aValue) { mIsExternallyControlled = aValue; }
   bool GetExternallyControlled() const { return mIsExternallyControlled; }

   void SetXIO_Controlled(bool aValue) { mIsXIO_Controlled = aValue; }
   bool GetXIO_Controlled() const { return mIsXIO_Controlled; }

   void        SetDisEntityId(DisEntityId aDisId) noexcept { mDisEntityId = aDisId; }
   DisEntityId GetDisEntityId() const noexcept { return mDisEntityId; }

   void SetSensorNames(const std::set<std::string>& aSensorNames) { mSensorNames = aSensorNames; }
   const std::set<std::string>& GetSensorNames() const { return mSensorNames; }

   void SetWeaponNames(const std::set<std::string>& aWeaponNames) { mWeaponNames = aWeaponNames; }
   const std::set<std::string>& GetWeaponNames() const { return mWeaponNames; }

   void                         SetCommNames(const std::set<std::string>& aCommNames) { mCommNames = aCommNames; }
   const std::set<std::string>& GetCommNames() const { return mCommNames; }

private:
   double mUpdateTime;
   bool   mIsExternallyControlled;
   bool   mIsXIO_Controlled;

   DisEntityId mDisEntityId;

   std::set<std::string> mSensorNames;
   std::set<std::string> mWeaponNames;
   std::set<std::string> mCommNames;
};
} // namespace warlock

VA_DECLARE_OBJECT_TYPE(warlock::Platform)

#endif // WSFWKPLATFORM_HPP
