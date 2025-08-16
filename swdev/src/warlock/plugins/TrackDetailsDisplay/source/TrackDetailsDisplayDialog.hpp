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
#ifndef TRACKDETAILSDISPLAYDIALOG_HPP
#define TRACKDETAILSDISPLAYDIALOG_HPP

#include <QDialog>

#include "DataContainer.hpp"
#include "DataTypes.hpp"
#include "TrackDetailsSimInterface.hpp"
#include "WkfPlatform.hpp"
#include "WkfScenario.hpp"
#include "WkfUnitTypes.hpp"
#include "ui_TrackDetailsDisplay.h"

namespace WkTrackDetailsDisplay
{
class TableItemSortable : public QTableWidgetItem
{
public:
   TableItemSortable(const QString& aItem);
   bool operator<(const QTableWidgetItem& aRhs) const override;

private:
   double mData;
};

class TrackDetailsObserver
{
public:
   virtual void Update(const PlatformsTrackData& aTracksData) = 0;
   virtual void Reset()                                       = 0;
   enum TrackTableColumn
   {
      cTRACK_NUM,
      cTRACK_IFF,
      cPLATFORM_TYPE,
      cSIDE,
      cRANGE,
      cRANGE_RATE,
      cSPEED,
      cRELATIVE_HEADING
   };
};

class TrackSensorDisplay : public TrackDetailsObserver
{
public:
   TrackSensorDisplay(Ui::TrackDetailsDisplay& aDisplay);
   void Update(const PlatformsTrackData& aTracksData) override;
   void Reset() override;

private:
   QLabel*              mNameDisplay;
   QLabel*              mTypeDisplay;
   QLabel*              mModeDisplay;
   QLabel*              mSignalToNoiseDisplay;
   QLabel*              mMultipleContributors;
   QLabel*              mOriginator;
   wkf::LatitudeLabel*  mLatitudeDisplay;
   wkf::LongitudeLabel* mLongitudeDisplay;
   wkf::AltitudeLabel*  mAltitudeDisplay;
   wkf::AngleLabel*     mAzimuthDisplay;
   wkf::AngleLabel*     mAzimuthErrorDisplay;
   wkf::AngleLabel*     mElevationDisplay;
   wkf::AngleLabel*     mElevationErrorDisplay;
   wkf::FrequencyLabel* mFrequencyDisplay;
};

class TrackRelativeGeoDisplay : public TrackDetailsObserver
{
public:
   TrackRelativeGeoDisplay(Ui::TrackDetailsDisplay& aDisplay);
   void Update(const PlatformsTrackData& aTracksData) override;
   void Reset() override;

private:
   QTableWidget*     mTableDisplay;
   QLabel*           mRangeDisplay;
   wkf::LengthLabel* mRangeErrorDisplay;
   QLabel*           mRangeRateDisplay;
   wkf::SpeedLabel*  mRangeRateErrorDisplay;
   wkf::AngleLabel*  mBearingDisplay;
   wkf::AngleLabel*  mBearingErrorDisplay;
   wkf::AngleLabel*  mElevationDisplay;
   wkf::AngleLabel*  mElevationErrorDisplay;
};

class TrackStateDisplay : public TrackDetailsObserver
{
public:
   TrackStateDisplay(Ui::TrackDetailsDisplay& aDisplay);
   void Update(const PlatformsTrackData& aTracksData) override;
   void Reset() override;

private:
   QTableWidget*        mTableDisplay;
   QLabel*              mQuality;
   QLabel*              mUpdataCount;
   wkf::TimeLabel*      mTrackStart;
   wkf::TimeLabel*      mTrackUpdate;
   wkf::LatitudeLabel*  mLatitude;
   wkf::LongitudeLabel* mLongitude;
   wkf::AltitudeLabel*  mAltitude;
   QLabel*              mRmsCovarianceDisplay;
   wkf::LengthLabel*    mSemiAxisForwardDisplay;
   wkf::LengthLabel*    mSemiAxisSideDisplay;
   wkf::LengthLabel*    mSemiAxisUpDisplay;
   QLabel*              mSpeed;
   QLabel*              mRelativeHeading;
};

class TrackManagerDisplay : public TrackDetailsObserver
{
public:
   TrackManagerDisplay(Ui::TrackDetailsDisplay& aDisplay);
   void Update(const PlatformsTrackData& aTracksData) override;
   void Reset() override;

private:
   QLabel* mPrebriefedDisplay;
   QLabel* mCorrelationMethodDisplay;
   QLabel* mFusionMethodDisplay;
   QLabel* mPurgeableDisplay;
   QLabel* mRawTrackCount;
   QLabel* mTrackType;
};

class TrackBasicInfoDisplay : public TrackDetailsObserver
{
public:
   TrackBasicInfoDisplay(Ui::TrackDetailsDisplay& aDisplay);
   void Update(const PlatformsTrackData& aTracksData) override;
   void Reset() override;

private:
   QTableWidget* mTableDisplay;
   QLabel*       mIffDisplay;
   QLabel*       mPlatformTypeDisplay;
   QLabel*       mSideDisplay;
   QLabel*       mSpatialDomainDisplay;
};

class TrackTableDisplay : public TrackDetailsObserver
{
public:
   TrackTableDisplay(QTableWidget* aTableDisplay);
   ~TrackTableDisplay() { mTableDisplay->clear(); }
   void Update(const PlatformsTrackData& aTracksData) override;
   void Reset() override { mTableDisplay->clearContents(); }

private:
   void SetRowOfInterest();
   void UpdateTable();
   bool UpdateAvailable(const PlatformsTrackData& aTracksData);

   int                                             mRow;
   int                                             mColumn;
   int                                             mTablesRowOfInterest;
   std::vector<std::pair<TrackId, TrackTableData>> mTrackDataTable;
   QTableWidget*                                   mTableDisplay;
};

class TrackDetailsObservable
{
public:
   TrackDetailsObservable(DataContainer& aDataContainer);
   void AttachObserver(std::unique_ptr<TrackDetailsObserver> aObserver);
   void AttachTableObserver(std::unique_ptr<TrackDetailsObserver> aTableObserver);
   void UpdateTableObserver();
   void UpdateTrackDetailsObservers();
   void ResetTrackDetailsObservers();
   void ResetTableObserver();

private:
   std::unique_ptr<TrackDetailsObserver>              mTableObserver;
   std::vector<std::unique_ptr<TrackDetailsObserver>> mObserverList;
   DataContainer&                                     mData;
};

class Dialog : public QDialog
{
   Q_OBJECT

public:
   Dialog(SimInterface&   aSimInterface,
          DataContainer&  aDataContainer,
          QWidget*        aParent = Q_NULLPTR,
          Qt::WindowFlags f       = Qt::WindowFlags());

private:
   // allows for tabbing through the QTable
   bool eventFilter(QObject*, QEvent*) override;

   void AddPlatformForSelection(const std::string& aNewPlatform);
   void RemovePlatformFromSelection(const std::string& aPlatformName);
   void UpdatePlatformListing();

   void UpdateOnSimReset(wkf::Scenario* /*unused*/);

   void UpdatePlatformOfInterest(const QString& aPlatformOfIntrest);
   void UpdateTrackOfInterest(const int aTrackOfIntrest);
   void HandlePlatformMapSelect(wkf::Platform* aPlatform);

   void UpdateTrackSelected();
   void UpdateDisplay();
   void ResetTableDisplay();
   void ResetBottomDisplay();

   SimInterface&           mSimInterface;
   QString                 mPlatformOfInterest;
   int                     mTrackOfInterest;
   TrackDetailsObservable  mObservable;
   Ui::TrackDetailsDisplay mUi;

   vespa::VaCallbackHolder mCallBacks;
};
} // namespace WkTrackDetailsDisplay
#endif
