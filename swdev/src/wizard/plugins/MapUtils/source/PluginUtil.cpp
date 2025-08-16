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

#include "PluginUtil.hpp"

#include "Message.hpp"
#include "VaViewer.hpp"
#include "VaViewerManager.hpp"
#include "WkfScenario.hpp"
#include "WkfVtkEnvironment.hpp"

namespace MapUtils
{
namespace PluginUtil
{
namespace Earth
{

double NormalizeEquatorialDistanceWGS84(const double aDifference)
{
   double resMinus = PluginUtil::WrapAroundNegative(-std::abs(aDifference), -Earth::cEQUATORIAL_CIRCUMFERENCE_WGS84);
   double resPlus  = PluginUtil::WrapAroundPositive(std::abs(aDifference), Earth::cEQUATORIAL_CIRCUMFERENCE_WGS84);
   return UtMath::Sign(aDifference) * std::min(std::abs(resMinus), std::abs(resPlus));
}

double NormalizeMeridionalDistanceWGS84(const double aDifference)
{
   double resMinus = PluginUtil::WrapAroundNegative(-std::abs(aDifference), -Earth::cMERIDIONAL_CIRCUMFERENCE_WGS84);
   double resPlus  = PluginUtil::WrapAroundPositive(std::abs(aDifference), Earth::cMERIDIONAL_CIRCUMFERENCE_WGS84);
   return UtMath::Sign(aDifference) * std::min(std::abs(resMinus), std::abs(resPlus));
}

} // namespace Earth

QPalette GeneratePalette(Qt::GlobalColor aColor)
{
   QPalette pal;
   pal.setColor(QPalette::Text, aColor);
   return pal;
}

vespa::VaEntityList AcquireEntities(const WhatEntities aEntitiesToAcquire)
{
   switch (aEntitiesToAcquire)
   {
   case PluginUtil::WhatEntities::cSCENARIO:
      return PluginUtil::GetEntitiesOfScenario();
   case PluginUtil::WhatEntities::cSELECTION:
      return PluginUtil::GetEntitiesOfSelection();
   case PluginUtil::WhatEntities::cNONE:
      // use default
   default:
      return vespa::VaEntityList();
   }
}

vespa::VaEntityList GetEntitiesOfScenario()
{
   wkf::Scenario*   scenarioPtr = vaEnv.GetStandardScenario();
   vespa::VaViewer* viewerPtr   = vaEnv.GetViewerManager()->CurrentViewer();

   vespa::VaEntityList entityList;
   if (scenarioPtr != nullptr && viewerPtr != nullptr)
   {
      scenarioPtr->GetEntityList(entityList);
   }
   return entityList;
}

vespa::VaEntityList GetEntitiesOfSelection()
{
   wkf::Scenario*   scenarioPtr = vaEnv.GetStandardScenario();
   vespa::VaViewer* viewerPtr   = vaEnv.GetViewerManager()->CurrentViewer();

   vespa::VaEntityList entityList;
   if (scenarioPtr != nullptr && viewerPtr != nullptr)
   {
      scenarioPtr->GetSelectedEntityList(viewerPtr, entityList);
   }
   return entityList;
}

WhatEntities GetAcquisitionType()
{
   wkf::Scenario*   scenarioPtr = vaEnv.GetStandardScenario();
   vespa::VaViewer* viewerPtr   = vaEnv.GetViewerManager()->CurrentViewer();

   vespa::VaEntityList entityList;
   if (scenarioPtr == nullptr || viewerPtr == nullptr)
   {
      return WhatEntities::cNONE;
   }
   scenarioPtr->GetSelectedEntityList(viewerPtr, entityList);
   if (entityList.empty())
   {
      scenarioPtr->GetEntityList(entityList);
      if (entityList.empty())
      {
         return WhatEntities::cNONE;
      }
      return WhatEntities::cSCENARIO;
   }
   return WhatEntities::cSELECTION;
}

double WrapAroundNegative(const double aValue, const double aNormalizationFactor)
{
   double res = aValue;
   while (res <= aNormalizationFactor)
   {
      res -= aNormalizationFactor;
   }
   return res;
}

double WrapAroundPositive(const double aValue, const double aNormalizationFactor)
{
   double res = aValue;
   while (res >= aNormalizationFactor)
   {
      res -= aNormalizationFactor;
   }
   return res;
}

bool AreLatitudesDifferent(const UtLatPos& aPrevious, const UtLatPos& aCurrent)
{
   return AreDifferent<UtLatPos>(aPrevious, aCurrent);
}

bool AreLongitudesDifferent(const UtLonPos& aPrevious, const UtLonPos& aCurrent)
{
   return AreDifferent<UtLonPos>(aPrevious, aCurrent);
}

bool AreAltitudesDifferent(const UtLength& aPrevious, const UtLength& aCurrent)
{
   return AreDifferent<UtLength>(aPrevious, aCurrent);
}

bool AreAltitudesDifferent(const UtLengthValue& aPrevious, const UtLengthValue& aCurrent)
{
   return AreDifferent<UtLengthValue>(aPrevious, aCurrent);
}

bool ArePitchesDifferent(const UtAngleValue& aPrevious, const UtAngleValue& aCurrent)
{
   return AreDifferent<UtAngleValue>(aPrevious, aCurrent);
}

bool AreRollsDifferent(const UtAngleValue& aPrevious, const UtAngleValue& aCurrent)
{
   return AreDifferent<UtAngleValue>(aPrevious, aCurrent);
}

bool AreYawsDifferent(const UtAngleValue& aPrevious, const UtAngleValue& aCurrent)
{
   return AreDifferent<UtAngleValue>(aPrevious, aCurrent);
}

} // namespace PluginUtil
} // namespace MapUtils
