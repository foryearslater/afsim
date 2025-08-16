// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2019 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "AnnotationSimEvents.hpp"

#include "WkfPointOfInterest.hpp"

namespace
{
QColor UtColorToQColor(const UtColor& aColor)
{
   float rgb[3];
   aColor.Get(rgb[0], rgb[1], rgb[2]);
   QColor newColor;
   newColor.setRgbF(rgb[0], rgb[1], rgb[2]);
   return newColor;
}

wkf::AttachmentRangeRing::Properties GetRangeRingProperties(const AnnotationData::RangeRingData& aData)
{
   wkf::AttachmentRangeRing::Properties properties;
   properties.mName                 = QString::fromStdString(aData.mRingName);
   properties.mBaseRadius           = aData.mCenterRadius;
   properties.mAdditionalRings      = aData.mAdditionalRings;
   properties.mAdditionalRingRadius = aData.mAdditionalRadii;
   properties.mCenterAngle          = aData.mCenterAngle;
   properties.mWidth                = aData.mAngleWidth;
   properties.mRadialRate           = aData.mRadialRate;
   properties.mRingColor            = UtColorToQColor(aData.mRingColor);
   properties.mRadialColor          = UtColorToQColor(aData.mRadialColor);
   properties.mRingLineWidth        = aData.mRingWidth;
   properties.mRadialLineWidth      = aData.mRadialWidth;
   properties.mShowRangeLabels      = aData.mShowRangeLabels;
   properties.mShowAngleLabels      = aData.mShowAngleLabels;
   properties.mRangeLabelColor      = UtColorToQColor(aData.mRangeLabelColor);
   properties.mAngleLabelColor      = UtColorToQColor(aData.mAngleLabelColor);
   properties.mAlignWithPlatform    = aData.mAlignPlatform;
   properties.mExportable           = false;
   return properties;
}
} // namespace

void SimStartingEvent::Process(Annotation::AnnotationDisplayInterface* aDisplayInterface)
{
   for (auto& poiIt : mAnnotationInfo.mPois)
   {
      if (!poiIt.mIsBullseye)
      {
         aDisplayInterface->AddDetailedPOI<wkf::PointOfInterest>("pointOfInterest",
                                                                 poiIt.mLat,
                                                                 poiIt.mLon,
                                                                 poiIt.mAlt,
                                                                 poiIt.mPoiName,
                                                                 false);
      }
      else
      {
         aDisplayInterface->AddDetailedPOI<wkf::Bullseye>("bullseye", poiIt.mLat, poiIt.mLon, poiIt.mAlt, poiIt.mPoiName, false);
      }
   }
   for (const auto& decorationIt : mAnnotationInfo.mDecorations)
   {
      QString text      = "";
      QString imageDesc = "";
      if (decorationIt.mType == AnnotationData::AnnotationType::cTEXT)
      {
         text = QString::fromStdString(decorationIt.mInfo);
      }
      else if (decorationIt.mType == AnnotationData::AnnotationType::cPRESET)
      {
         imageDesc = aDisplayInterface->DecorationPresetToPath(decorationIt.mInfo);
      }
      else // AnnotationData::AnnotationType::cIMAGE
      {
         imageDesc = QString::fromStdString(decorationIt.mInfo);
      }

      wkf::Entity* entity = dynamic_cast<wkf::Entity*>(vaEnv.FindEntity(decorationIt.mEntityName));
      if (entity)
      {
         aDisplayInterface->AddDecoration(*entity, text, imageDesc);
         auto* dec = entity->FindFirstAttachmentOfType<Annotation::AnnotationDecorator>();
         if (dec)
         {
            dec->SetExportable(false);
         }
      }
      else
      {
         aDisplayInterface->AddDeferredDecoration(QString::fromStdString(decorationIt.mEntityName),
                                                  std::pair<QString, QString>(text, imageDesc));
      }
   }
   // Since Pois are processed first and are not dependent on platforms being added, there will be no deferred range
   // rings besides platforms
   for (const auto& rangeRingIt : mAnnotationInfo.mRangeRings)
   {
      wkf::AttachmentRangeRing::Properties properties = GetRangeRingProperties(rangeRingIt);
      wkf::Platform*        platform = dynamic_cast<wkf::Platform*>(vaEnv.FindEntity(rangeRingIt.mReferenceName));
      wkf::PointOfInterest* poi = dynamic_cast<wkf::PointOfInterest*>(vaEnv.FindEntity(rangeRingIt.mReferenceName));
      if (platform)
      {
         aDisplayInterface->AddRangeRing(*platform, GetRangeRingProperties(rangeRingIt));
      }
      else if (poi)
      {
         aDisplayInterface->AddRangeRing(poi, GetRangeRingProperties(rangeRingIt));
      }
      else
      {
         aDisplayInterface->AddDeferredRangeRing(QString::fromStdString(rangeRingIt.mReferenceName),
                                                 GetRangeRingProperties(rangeRingIt));
      }
   }
}
