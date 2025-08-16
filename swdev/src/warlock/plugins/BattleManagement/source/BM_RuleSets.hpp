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

#ifndef BM_RULE_SETS_HPP
#define BM_RULE_SETS_HPP

#include "statistics/WkfBM_ResourceRuleSet.hpp"
#include "statistics/WkfBM_RuleSet.hpp"

namespace WkBM
{
class Plugin;

struct PlatformState
{
   double               mDamageFactor = 0;
   double               mCurrentFuel  = -1;
   ut::optional<double> mInitialFuel;

   void Assign(const PlatformState& aOther) noexcept;
};

class RuleSetDamageFactor final : public wkf::bm::RuleSet
{
public:
   RuleSetDamageFactor(QWidget* aParent, Plugin* aPlugin) noexcept;
   QVariant GetValue(const wkf::Platform* aPlatform) const override;
   void     SetDefaults() override;

private:
   Plugin* mPlugin;
};

class RuleSetFuel final : public wkf::bm::ResourceRuleSet
{
public:
   RuleSetFuel(QWidget* aParent, Plugin* aPlugin) noexcept;
   QVariant             GetValue(const wkf::Platform* aPlatform) const override;
   ut::optional<double> GetInitialValue(const wkf::Platform* aPlatform) const override;

private:
   Plugin* mPlugin;
};

class RuleSetWeaponCount final : public wkf::bm::ResourceRuleSet
{
public:
   RuleSetWeaponCount(QWidget* aParent, Plugin* aPlugin) noexcept;
   QVariant             GetValue(const wkf::Platform* aPlatform) const override;
   ut::optional<double> GetInitialValue(const wkf::Platform* aPlatform) const override;

private:
   Plugin* mPlugin;
};

class AuxDataRuleSet final : public wkf::bm::RuleSet
{
public:
   AuxDataRuleSet(QWidget* aParent, Plugin* aPlugin, QString aKey, wkf::bm::RuleSet::Mode aMode) noexcept;

   QString  GetDisplayLabel() const override;
   QVariant GetValue(const wkf::Platform* aPlatform) const override;
   QString  GetDisplayValue(const wkf::Platform* aPlatform) const override;
   void     SetDefaults() override;

private:
   Plugin*     mPlugin;
   std::string mKey;
};

class AuxDataResourceRuleSet final : public wkf::bm::ResourceRuleSet
{
public:
   AuxDataResourceRuleSet(QWidget* aParent, Plugin* aPlugin, QString aNumerator, QString aDenominator) noexcept;

   QString              GetDisplayLabel() const override;
   QVariant             GetValue(const wkf::Platform* aPlatform) const override;
   ut::optional<double> GetInitialValue(const wkf::Platform* aPlatform) const override;

private:
   Plugin*     mPlugin;
   std::string mNumeratorKey;
   std::string mDenominatorKey;
};
} // namespace WkBM

#endif
