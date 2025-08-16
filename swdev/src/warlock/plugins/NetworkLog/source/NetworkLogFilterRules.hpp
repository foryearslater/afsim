// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2021 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#ifndef NETWORK_LOG_FILTER_RULES_HPP
#define NETWORK_LOG_FILTER_RULES_HPP

#include "WkfPlatformGrouping.hpp"
#include "filter_widget/WkfFilterObject.hpp"

namespace NetworkLog
{
struct PlatformRule : WkfFilter::Rule
{
   bool TestValue(const QVariant& aModelData, const QString& aFilterData) const final;
   bool Predicate(const QAbstractItemModel* aModel, int aColumn) const final;

   virtual wkf::PlatformGrouping::TypeEnum GetGroupingType() const = 0;
};

struct PlatformRuleSide final : PlatformRule
{
   QString                         Name() const override;
   wkf::PlatformGrouping::TypeEnum GetGroupingType() const override;
};

struct PlatformRuleType final : PlatformRule
{
   QString                         Name() const override;
   wkf::PlatformGrouping::TypeEnum GetGroupingType() const override;
};

struct PlatformRuleCategory final : PlatformRule
{
   QString                         Name() const override;
   wkf::PlatformGrouping::TypeEnum GetGroupingType() const override;
};

struct TimeBeforeRule final : WkfFilter::Rule
{
   QString Name() const override;
   bool    TestValue(const QVariant& aModelData, const QString& aFilterData) const override;
   bool    Predicate(const QAbstractItemModel* aModel, int aColumn) const override;
};

struct TimeAfterRule final : WkfFilter::Rule
{
   QString Name() const override;
   bool    TestValue(const QVariant& aModelData, const QString& aFilterData) const override;
   bool    Predicate(const QAbstractItemModel* aModel, int aColumn) const override;
};
} // namespace NetworkLog

#endif
