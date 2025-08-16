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

#include "NetworkLogFilterRules.hpp"

#include "NetworkLogModel.hpp"
#include "UtMemory.hpp"
#include "WkfEnvironment.hpp"
#include "WkfScenario.hpp"
#include "WkfVtkEnvironment.hpp"

static unsigned GetPlatformIndex(const QString& aModelData, bool& aValid)
{
   aValid = false;
   return 0;
}

bool NetworkLog::PlatformRule::TestValue(const QVariant& aModelData, const QString& aFilterData) const
{
   bool                            valid;
   wkf::PlatformGrouping           group{GetGroupingType(), aFilterData.toStdString()};
   std::set<wkf::PlatformGrouping> cat = wkfEnv.GetGroupings(::GetPlatformIndex(aModelData.toString(), valid));
   return valid && (cat.find(group) != cat.end());
}

bool NetworkLog::PlatformRule::Predicate(const QAbstractItemModel* aModel, int aColumn) const
{
   if (aModel != nullptr)
   {
      for (int i = 0; i < aModel->rowCount(); i++)
      {
         auto data = aModel->index(i, aColumn).data(Model::cPLATFORM_INDEX_ROLE);
         if (!data.isValid())
         {
            return false;
         }
      }
      return true;
   }
   return false;
}

// Side

QString NetworkLog::PlatformRuleSide::Name() const
{
   return "is on side";
}

wkf::PlatformGrouping::TypeEnum NetworkLog::PlatformRuleSide::GetGroupingType() const
{
   return wkf::PlatformGrouping::eSide;
}

// Type

QString NetworkLog::PlatformRuleType::Name() const
{
   return "is of type";
}

wkf::PlatformGrouping::TypeEnum NetworkLog::PlatformRuleType::GetGroupingType() const
{
   return wkf::PlatformGrouping::eType;
}

// Category

QString NetworkLog::PlatformRuleCategory::Name() const
{
   return "is of category";
}

wkf::PlatformGrouping::TypeEnum NetworkLog::PlatformRuleCategory::GetGroupingType() const
{
   return wkf::PlatformGrouping::eCategory;
}

// Time Before

QString NetworkLog::TimeBeforeRule::Name() const
{
   return "is before";
}

bool NetworkLog::TimeBeforeRule::TestValue(const QVariant& aModelData, const QString& aFilterData) const
{
   return aModelData.toTime() < QTime::fromString(aFilterData, Qt::ISODateWithMs);
}

bool NetworkLog::TimeBeforeRule::Predicate(const QAbstractItemModel* aModel, int aColumn) const
{
   return aColumn == Model::cTIME_COLUMN;
}

// Time After

QString NetworkLog::TimeAfterRule::Name() const
{
   return "is after";
}

bool NetworkLog::TimeAfterRule::TestValue(const QVariant& aModelData, const QString& aFilterData) const
{
   return aModelData.toTime() > QTime::fromString(aFilterData, Qt::ISODateWithMs);
}

bool NetworkLog::TimeAfterRule::Predicate(const QAbstractItemModel* aModel, int aColumn) const
{
   return aColumn == Model::cTIME_COLUMN;
}
