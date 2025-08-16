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

#include "DataTypes.hpp"

#include "WkfUnitTypes.hpp"

QString WkTrackDetailsDisplay::TrackTableData::GetIFF() const
{
   switch (mIFF)
   {
   case WsfTrack::cIFF_AMBIGUOUS:
      return QStringLiteral("Ambiguous");
   case WsfTrack::cIFF_FOE:
      return QStringLiteral("Foe");
   case WsfTrack::cIFF_FRIEND:
      return QStringLiteral("Friend");
   case WsfTrack::cIFF_NEUTRAL:
      return QStringLiteral("Neutral");
   default:
      return QStringLiteral("unknown");
      break;
   }
}

QString WkTrackDetailsDisplay::TrackTableData::GetPlatformType() const
{
   if (mPlatformType.has_value())
   {
      return QString::fromStdString(mPlatformType.value());
   }
   else
   {
      return QString("unknown");
   }
}

QString WkTrackDetailsDisplay::TrackTableData::GetSide() const
{
   if (mSide.has_value())
   {
      return QString::fromStdString(mSide.value());
   }
   else
   {
      return QString("unknown");
   }
}

QString WkTrackDetailsDisplay::TrackTableData::GetRange() const
{
   if (mRange.has_value())
   {
      wkf::LengthTableWidgetItem label{"Range"};
      label.SetValue(mRange.value());
      return label.text();
   }
   return QString("unknown");
}

QString WkTrackDetailsDisplay::TrackTableData::GetRangeRate() const
{
   if (mRangeRate.has_value())
   {
      wkf::SpeedTableWidgetItem label{"Range Rate"};
      label.SetValue(mRangeRate.value());
      return label.text();
   }
   return QString("unknown");
}

QString WkTrackDetailsDisplay::TrackTableData::GetSpeed() const
{
   if (mSpeed.has_value())
   {
      wkf::SpeedTableWidgetItem label{"Speed"};
      label.SetValue(mSpeed.value());
      return label.text();
   }
   return QString("unknown");
}

QString WkTrackDetailsDisplay::TrackTableData::GetRelativeHeading() const
{
   if (mRelativeHeading.has_value())
   {
      wkf::AngleTableWidgetItem label{"Relative Heading"};
      label.SetValue(mRelativeHeading.value());
      return label.text();
   }
   return QString("unknown");
}

bool WkTrackDetailsDisplay::TrackTableData::operator!=(const TrackTableData& aRhs) const
{
   return this->mIFF != aRhs.mIFF || this->mPlatformType != aRhs.mPlatformType || this->mRange != aRhs.mRange ||
          this->mRangeRate != aRhs.mRangeRate || this->mSide != aRhs.mSide ||
          this->mRelativeHeading != aRhs.mRelativeHeading || this->mSpeed != aRhs.mSpeed;
}
