// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2020 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "BM_RuleSets.hpp"

#include "BM_Plugin.hpp"
#include "WkfEnvironment.hpp"
#include "WkfUnitsObject.hpp"

void WkBM::PlatformState::Assign(const PlatformState& aOther) noexcept
{
   mDamageFactor = aOther.mDamageFactor;
   mCurrentFuel  = aOther.mCurrentFuel;
   if (!mInitialFuel)
   {
      mInitialFuel = mCurrentFuel;
   }
}

WkBM::RuleSetDamageFactor::RuleSetDamageFactor(QWidget* aParent, Plugin* aPlugin) noexcept
   : wkf::bm::RuleSet(aParent, Mode::Number, "Damage Factor")
   , mPlugin(aPlugin)
{
}

QVariant WkBM::RuleSetDamageFactor::GetValue(const wkf::Platform* aPlatform) const
{
   if (mPlugin && aPlatform)
   {
      auto it = mPlugin->mPlatformStates.find(aPlatform->GetIndex());
      if (it != mPlugin->mPlatformStates.end())
      {
         return it->second.mDamageFactor;
      }
   }
   return QVariant();
}

void WkBM::RuleSetDamageFactor::SetDefaults()
{
   ClearRules();
   SetDefaultColor(ut::nullopt);

   AddRule()->SetState(QColor(0, 128, 0), "==", 0);
   AddRule()->SetState(QColor(128, 128, 0), "<", 1);
   AddRule()->SetState(QColor(128, 0, 0), "==", 1);
}

WkBM::RuleSetFuel::RuleSetFuel(QWidget* aParent, Plugin* aPlugin) noexcept
   : wkf::bm::ResourceRuleSet(aParent, "Fuel")
   , mPlugin(aPlugin)
{
}

QVariant WkBM::RuleSetFuel::GetValue(const wkf::Platform* aPlatform) const
{
   if (mPlugin && aPlatform)
   {
      auto it = mPlugin->mPlatformStates.find(aPlatform->GetIndex());
      if (it != mPlugin->mPlatformStates.end() && it->second.mCurrentFuel >= 0)
      {
         return it->second.mCurrentFuel;
      }
   }
   return QVariant();
}

ut::optional<double> WkBM::RuleSetFuel::GetInitialValue(const wkf::Platform* aPlatform) const
{
   if (mPlugin && aPlatform)
   {
      auto it = mPlugin->mPlatformStates.find(aPlatform->GetIndex());
      if (it != mPlugin->mPlatformStates.end() && it->second.mInitialFuel >= 0)
      {
         return it->second.mInitialFuel;
      }
   }
   return ut::nullopt;
}

WkBM::RuleSetWeaponCount::RuleSetWeaponCount(QWidget* aParent, Plugin* aPlugin) noexcept
   : wkf::bm::ResourceRuleSet(aParent, "Weapon Count")
   , mPlugin(aPlugin)
{
}

QVariant WkBM::RuleSetWeaponCount::GetValue(const wkf::Platform* aPlatform) const
{
   // TODO: Get the weapon count

   return QVariant();
}

ut::optional<double> WkBM::RuleSetWeaponCount::GetInitialValue(const wkf::Platform* aPlatform) const
{
   // TODO: Get the initial weapon count

   return ut::nullopt;
}

WkBM::AuxDataRuleSet::AuxDataRuleSet(QWidget* aParent, Plugin* aPlugin, QString aKey, wkf::bm::RuleSet::Mode aMode) noexcept
   : wkf::bm::RuleSet(aParent, aMode, wkf::bm::cAUX_DATA_PREFIX + aKey, true)
   , mPlugin(aPlugin)
   , mKey(aKey.toStdString())
{
}

QString WkBM::AuxDataRuleSet::GetDisplayLabel() const
{
   return GetLabel().mid(wkf::bm::cAUX_DATA_PREFIX.size());
}

QVariant WkBM::AuxDataRuleSet::GetValue(const wkf::Platform* aPlatform) const
{
   if (mPlugin && aPlatform)
   {
      auto it = mPlugin->mAuxData.find(aPlatform->GetIndex());
      if (it != mPlugin->mAuxData.end())
      {
         auto jt = it->second.find(mKey);
         if (jt != it->second.end())
         {
            return jt->second;
         }
      }
   }

   return QVariant{};
}

QString WkBM::AuxDataRuleSet::GetDisplayValue(const wkf::Platform* aPlatform) const
{
   auto value = GetValue(aPlatform);
   if (value.isValid())
   {
      if (GetMode() == Mode::Number)
      {
         return QString::number(value.toDouble(), 'f', 4);
      }
      else
      {
         return value.toString();
      }
   }
   return "n/a";
}

void WkBM::AuxDataRuleSet::SetDefaults()
{
   ClearRules();
   SetDefaultColor(ut::nullopt);

   if (GetMode() == Mode::Boolean)
   {
      AddRule()->SetState(QColor(0, 128, 0), QString{}, true);
      AddRule()->SetState(QColor(128, 0, 0), QString{}, false);
   }
}

WkBM::AuxDataResourceRuleSet::AuxDataResourceRuleSet(QWidget* aParent,
                                                     Plugin*  aPlugin,
                                                     QString  aNumerator,
                                                     QString  aDenominator) noexcept
   : wkf::bm::ResourceRuleSet(aParent, wkf::bm::cAUX_DATA_PREFIX + aNumerator + " / " + aDenominator, true)
   , mPlugin(aPlugin)
   , mNumeratorKey(aNumerator.toStdString())
   , mDenominatorKey(aDenominator.toStdString())
{
}

QString WkBM::AuxDataResourceRuleSet::GetDisplayLabel() const
{
   return GetLabel().mid(wkf::bm::cAUX_DATA_PREFIX.size());
}

QVariant WkBM::AuxDataResourceRuleSet::GetValue(const wkf::Platform* aPlatform) const
{
   if (mPlugin && aPlatform)
   {
      auto it = mPlugin->mAuxData.find(aPlatform->GetIndex());
      if (it != mPlugin->mAuxData.end())
      {
         auto jt = it->second.find(mNumeratorKey);
         if (jt != it->second.end())
         {
            return jt->second;
         }
      }
   }

   return QVariant{};
}

ut::optional<double> WkBM::AuxDataResourceRuleSet::GetInitialValue(const wkf::Platform* aPlatform) const
{
   if (mPlugin && aPlatform)
   {
      auto it = mPlugin->mAuxData.find(aPlatform->GetIndex());
      if (it != mPlugin->mAuxData.end())
      {
         auto jt = it->second.find(mDenominatorKey);
         if (jt != it->second.end())
         {
            return jt->second.toDouble();
         }
      }
   }
   return ut::nullopt;
}
