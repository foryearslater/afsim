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

#include "TrackDetailsDisplayDialog.hpp"

#include <QModelIndexList>
#include <QStringListModel>

#include "WkfObserver.hpp"

WkTrackDetailsDisplay::TableItemSortable::TableItemSortable(const QString& aItem)
   : QTableWidgetItem(aItem)
{
   QRegExp rx("[a-zA-Z ]");
   QString number = aItem;
   number.remove(rx);
   bool ok;
   mData = number.toDouble(&ok);
   if (!ok)
   {
      mData = 0.0;
   }
}

bool WkTrackDetailsDisplay::TableItemSortable::operator<(const QTableWidgetItem& aRhs) const
{
   const TableItemSortable* item(dynamic_cast<const TableItemSortable*>(&aRhs));
   if (item)
   {
      return this->mData < item->mData;
   }
   return false;
}

WkTrackDetailsDisplay::TrackSensorDisplay::TrackSensorDisplay(Ui::TrackDetailsDisplay& aDisplay)
   : mNameDisplay(aDisplay.sensorNameDataLabel)
   , mTypeDisplay(aDisplay.sensorTypeDataLabel)
   , mModeDisplay(aDisplay.sensorModeDataLabel)
   , mSignalToNoiseDisplay(aDisplay.signalToNoiseDataLabel)
   , mMultipleContributors(aDisplay.multipleContributorsDataLabel)
   , mOriginator(aDisplay.originatorDataLabel)
   , mLatitudeDisplay(aDisplay.sensorLatDataLabel)
   , mLongitudeDisplay(aDisplay.sensorLonDataLabel)
   , mAltitudeDisplay(aDisplay.sensorAltitudeDatalabel)
   , mAzimuthDisplay(aDisplay.sensorAzimuthDataLabel)
   , mAzimuthErrorDisplay(aDisplay.sensorAzimuthErrorDataLabel)
   , mElevationDisplay(aDisplay.sensorElevationDataLabel)
   , mElevationErrorDisplay(aDisplay.sensorElevationErrorDataLabel)
   , mFrequencyDisplay(aDisplay.frequencyDataLabel)
{
}

void WkTrackDetailsDisplay::TrackSensorDisplay::Update(const PlatformsTrackData& aTracksData)
{
   QString text;

   mMultipleContributors->setText(QVariant(aTracksData.mTrackData.mMultipleContributors).toString());

   text = aTracksData.mTrackData.mSensorName.c_str();
   text.isEmpty() ? mNameDisplay->setText("unknown") : mNameDisplay->setText(text);

   text = aTracksData.mTrackData.mSensorType.c_str();
   text.isEmpty() ? mTypeDisplay->setText("unknown") : mTypeDisplay->setText(text);

   text = aTracksData.mTrackData.mSensorMode.c_str();
   text.isEmpty() ? mModeDisplay->setText("unknown") : mModeDisplay->setText(text);

   mLatitudeDisplay->SetValue(aTracksData.mTrackData.mSensorLLA.GetLat());
   mLongitudeDisplay->SetValue(aTracksData.mTrackData.mSensorLLA.GetLon());
   mAltitudeDisplay->SetValue(aTracksData.mTrackData.mSensorLLA.GetAlt());

   mAzimuthDisplay->SetValue(aTracksData.mTrackData.mSensorAzimuth);
   mAzimuthErrorDisplay->SetValue(aTracksData.mTrackData.mSensorAzimuthError);

   if (aTracksData.mTrackData.mElevation.has_value())
   {
      mElevationDisplay->SetValue(*aTracksData.mTrackData.mElevation);
      mElevationErrorDisplay->SetValue(aTracksData.mTrackData.mElevationError);
   }
   else
   {
      mElevationDisplay->setText("unknown");
      mElevationErrorDisplay->setText("unknown");
   }

   if (aTracksData.mTrackData.mSignalToNoise.has_value())
   {
      mSignalToNoiseDisplay->setText(QString::number(*aTracksData.mTrackData.mSignalToNoise));
   }
   else
   {
      mSignalToNoiseDisplay->setText("unknown");
   }

   if (aTracksData.mTrackData.mFrequency.has_value())
   {
      mFrequencyDisplay->SetValue(*aTracksData.mTrackData.mFrequency);
   }
   else
   {
      mFrequencyDisplay->setText("unknown");
   }

   if (aTracksData.mTrackData.mOriginatorName == mNameDisplay->text().toStdString())
   {
      mOriginator->setText("true");
   }
   else if (!aTracksData.mTrackData.mOriginatorName.empty())
   {
      mOriginator->setText(aTracksData.mTrackData.mOriginatorName.c_str());
   }
}

void WkTrackDetailsDisplay::TrackSensorDisplay::Reset()
{
   mNameDisplay->clear();
   mTypeDisplay->clear();
   mModeDisplay->clear();
   mLatitudeDisplay->clear();
   mLongitudeDisplay->clear();
   mAltitudeDisplay->clear();
   mAzimuthDisplay->clear();
   mAzimuthDisplay->clear();
   mAzimuthErrorDisplay->clear();
   mElevationDisplay->clear();
   mElevationErrorDisplay->clear();
   mSignalToNoiseDisplay->clear();
   mMultipleContributors->clear();
   mFrequencyDisplay->clear();
   mOriginator->clear();
}

WkTrackDetailsDisplay::TrackRelativeGeoDisplay::TrackRelativeGeoDisplay(Ui::TrackDetailsDisplay& aDisplay)
   : mTableDisplay(aDisplay.trackTableWidget)
   , mRangeDisplay(aDisplay.rangeDataLabel)
   , mRangeErrorDisplay(aDisplay.rangeErrorDataLabel)
   , mRangeRateDisplay(aDisplay.rangeRateDataLabel)
   , mRangeRateErrorDisplay(aDisplay.rangeRateErrorDataLabel)
   , mBearingDisplay(aDisplay.bearingDataLabel)
   , mBearingErrorDisplay(aDisplay.bearingErrorDataLabel)
   , mElevationDisplay(aDisplay.elevationDataLabel)
   , mElevationErrorDisplay(aDisplay.elevationErrorDataLabel)
{
}

void WkTrackDetailsDisplay::TrackRelativeGeoDisplay::Update(const PlatformsTrackData& aTracksData)
{
   QItemSelectionModel* selectedTableItems = mTableDisplay->selectionModel();
   // Grabbing data from the table to be mocked in the bottom corresponding display
   QModelIndexList itemList = selectedTableItems->selectedRows();
   QModelIndex     index    = itemList.at(0);
   int             rowKey   = index.row();

   mRangeDisplay->setText(mTableDisplay->item(rowKey, cRANGE)->text());
   (mRangeDisplay->text() != "unknown") ? mRangeErrorDisplay->SetValue(aTracksData.mTrackData.mRangeError) :
                                          mRangeErrorDisplay->setText("unknown");

   mRangeRateDisplay->setText(mTableDisplay->item(rowKey, TrackTableColumn::cRANGE_RATE)->text());
   mRangeRateDisplay->text() != "unknown" ? mRangeRateErrorDisplay->SetValue(aTracksData.mTrackData.mRangeRateError) :
                                            mRangeRateErrorDisplay->setText("unknown");

   if (aTracksData.mTrackData.mBearing.has_value())
   {
      mBearingDisplay->SetValue(*aTracksData.mTrackData.mBearing);
      mBearingErrorDisplay->SetValue(aTracksData.mTrackData.mBearingError);
   }
   else
   {
      mBearingDisplay->setText("unknown");
      mBearingErrorDisplay->setText("unknown");
   }

   if (aTracksData.mTrackData.mElevation.has_value())
   {
      mElevationDisplay->SetValue(*aTracksData.mTrackData.mElevation);
      mElevationErrorDisplay->SetValue(aTracksData.mTrackData.mElevationError);
   }
   else
   {
      mElevationDisplay->setText("unknown");
      mElevationErrorDisplay->setText("unknown");
   }
}

void WkTrackDetailsDisplay::TrackRelativeGeoDisplay::Reset()
{
   mRangeDisplay->clear();
   mRangeErrorDisplay->clear();
   mRangeRateDisplay->clear();
   mRangeRateErrorDisplay->clear();
   mBearingDisplay->clear();
   mBearingErrorDisplay->clear();
   mElevationDisplay->clear();
   mElevationErrorDisplay->clear();
}

WkTrackDetailsDisplay::TrackStateDisplay::TrackStateDisplay(Ui::TrackDetailsDisplay& aDisplay)
   : mTableDisplay(aDisplay.trackTableWidget)
   , mQuality(aDisplay.qualityDataLabel)
   , mUpdataCount(aDisplay.updateCountDataLabel)
   , mTrackStart(aDisplay.trackStartDataLabel)
   , mTrackUpdate(aDisplay.trackUpdateDataLabel)
   , mLatitude(aDisplay.latDataLabel)
   , mLongitude(aDisplay.lonDataLabel)
   , mAltitude(aDisplay.altDataLabel)
   , mRmsCovarianceDisplay(aDisplay.rmsValueDataLabel)
   , mSemiAxisForwardDisplay(aDisplay.semiAxisForwardDataLabel)
   , mSemiAxisSideDisplay(aDisplay.semiAxisSideDataLabel)
   , mSemiAxisUpDisplay(aDisplay.semiAxisUpDataLabel)
   , mSpeed(aDisplay.speedDataLabel)
   , mRelativeHeading(aDisplay.relativeHeadingDataLabel)
{
}

void WkTrackDetailsDisplay::TrackStateDisplay::Update(const PlatformsTrackData& aTracksData)
{
   QItemSelectionModel* selectedTableItems = mTableDisplay->selectionModel();
   // Grabbing data from the table to be mocked in the bottom corresponding display
   QModelIndexList itemList = selectedTableItems->selectedRows();
   QModelIndex     index    = itemList.at(0);
   int             rowKey   = index.row();

   mQuality->setText(QString::number(aTracksData.mTrackData.mQuality));

   mUpdataCount->setText(QString::number(aTracksData.mTrackData.mUpdateCount));

   mTrackUpdate->SetValue(aTracksData.mTrackData.mTrackUpdate);

   // might include later
   // mTrackAge->SetValue(aTracksData.mTrackData.mTrackAge);

   mTrackStart->SetValue(aTracksData.mTrackData.mTrackStart);

   if (aTracksData.mTrackData.mLLA.has_value())
   {
      mLatitude->SetValue(aTracksData.mTrackData.mLLA->GetLat());
      mLongitude->SetValue(aTracksData.mTrackData.mLLA->GetLon());
      mAltitude->SetValue(aTracksData.mTrackData.mLLA->GetAlt());
   }
   else
   {
      mLatitude->setText("unknown");
      mLongitude->setText("unknown");
      mAltitude->setText("unknown");
   }

   mSpeed->setText(mTableDisplay->item(rowKey, cSPEED)->text());

   mRelativeHeading->setText(mTableDisplay->item(rowKey, cRELATIVE_HEADING)->text());

   if (aTracksData.mTrackData.mRmsCovariance.has_value())
   {
      mSemiAxisForwardDisplay->SetValue(aTracksData.mTrackData.mSemiAxisForward);
      mSemiAxisSideDisplay->SetValue(aTracksData.mTrackData.mSemiAxisSide);
      mSemiAxisUpDisplay->SetValue(aTracksData.mTrackData.mSemiAxisUp);
      mRmsCovarianceDisplay->setText(QString::number(*aTracksData.mTrackData.mRmsCovariance));
   }
   else
   {
      mSemiAxisForwardDisplay->setText("unknown");
      mSemiAxisSideDisplay->setText("unknown");
      mSemiAxisUpDisplay->setText("unknown");
      mRmsCovarianceDisplay->setText("unknown");
   }
}

void WkTrackDetailsDisplay::TrackStateDisplay::Reset()
{
   mQuality->clear();
   mUpdataCount->clear();
   mTrackStart->clear();
   mTrackUpdate->clear();
   mLatitude->clear();
   mLongitude->clear();
   mAltitude->clear();
   mRmsCovarianceDisplay->clear();
   mSemiAxisForwardDisplay->clear();
   mSemiAxisSideDisplay->clear();
   mSemiAxisUpDisplay->clear();
   mSpeed->clear();
   mRelativeHeading->clear();
}


WkTrackDetailsDisplay::TrackManagerDisplay::TrackManagerDisplay(Ui::TrackDetailsDisplay& aDisplay)
   : mPrebriefedDisplay(aDisplay.prebriefedDataLabel)
   , mCorrelationMethodDisplay(aDisplay.correlationMethodDataLabel)
   , mFusionMethodDisplay(aDisplay.fusionMethodDataLabel)
   , mPurgeableDisplay(aDisplay.purgeableDataLabel)
   , mRawTrackCount(aDisplay.rawTrackCountDataLabel)
   , mTrackType(aDisplay.trackTypeDataLabel)
{
}

void WkTrackDetailsDisplay::TrackManagerDisplay::Update(const PlatformsTrackData& aTracksData)
{
   mPrebriefedDisplay->setText(QVariant(aTracksData.mTrackData.mPrebriefed).toString());

   mCorrelationMethodDisplay->setText(aTracksData.mTrackData.mCorrelationMethod.c_str());

   mFusionMethodDisplay->setText(aTracksData.mTrackData.mFusionMethod.c_str());

   mPurgeableDisplay->setText(QVariant(aTracksData.mTrackData.mTrackPurgeable).toString());

   mRawTrackCount->setText(QString::number(aTracksData.mTrackData.mRawTrackCount));

   mTrackType->setText(aTracksData.mTrackData.mTrackType.c_str());
}

void WkTrackDetailsDisplay::TrackManagerDisplay::Reset()
{
   mPrebriefedDisplay->clear();
   mCorrelationMethodDisplay->clear();
   mFusionMethodDisplay->clear();
   mPurgeableDisplay->clear();
   mRawTrackCount->clear();
   mTrackType->clear();
}

WkTrackDetailsDisplay::TrackBasicInfoDisplay::TrackBasicInfoDisplay(Ui::TrackDetailsDisplay& aDisplay)
   : mTableDisplay(aDisplay.trackTableWidget)
   , mIffDisplay(aDisplay.iff_DataLabel)
   , mPlatformTypeDisplay(aDisplay.platformTypeDataLabel)
   , mSideDisplay(aDisplay.sideDataLabel)
   , mSpatialDomainDisplay(aDisplay.spatialDomainDataLabel)
{
}

void WkTrackDetailsDisplay::TrackBasicInfoDisplay::Update(const PlatformsTrackData& aTracksData)
{
   QItemSelectionModel* selectedTableItems = mTableDisplay->selectionModel();
   // Grabbing data from the table to be mocked in the bottom corresponding display
   QModelIndexList itemList = selectedTableItems->selectedRows();
   QModelIndex     index    = itemList.at(0);
   int             rowKey   = index.row();

   mIffDisplay->setText(mTableDisplay->item(rowKey, cTRACK_IFF)->text());

   mPlatformTypeDisplay->setText(mTableDisplay->item(rowKey, cPLATFORM_TYPE)->text());

   mSideDisplay->setText(mTableDisplay->item(rowKey, TrackTableColumn::cSIDE)->text());

   mSpatialDomainDisplay->setText(aTracksData.mTrackData.mSpatialDomain.c_str());
}

void WkTrackDetailsDisplay::TrackBasicInfoDisplay::Reset()
{
   mIffDisplay->clear();
   mPlatformTypeDisplay->clear();
   mSideDisplay->clear();
   mSpatialDomainDisplay->clear();
}

WkTrackDetailsDisplay::TrackTableDisplay::TrackTableDisplay(QTableWidget* aTableDisplay)
   : mRow(0)
   , mColumn(0)
   , mTablesRowOfInterest(-1)
   , mTableDisplay(aTableDisplay)
{
   // make table items un-editable
   mTableDisplay->setEditTriggers(QAbstractItemView::NoEditTriggers);
   mTableDisplay->setTabKeyNavigation(true);

   // resizes table based on dialogs size
   mTableDisplay->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

   // set table selection to row and disable drag and select
   mTableDisplay->setSelectionBehavior(QAbstractItemView::SelectRows);
   mTableDisplay->setSelectionMode(QAbstractItemView::SingleSelection);
}

void WkTrackDetailsDisplay::TrackTableDisplay::Update(const PlatformsTrackData& aTracksData)
{
   SetRowOfInterest();
   if (UpdateAvailable(aTracksData))
   {
      mTrackDataTable = aTracksData.mTableData;
      mTableDisplay->setSortingEnabled(false);
      int currentRowSize = static_cast<int>(mTrackDataTable.size());
      mTableDisplay->clearContents();
      mTableDisplay->setRowCount(currentRowSize);
      UpdateTable();
      mTableDisplay->setSortingEnabled(true);
   }
   if (mTablesRowOfInterest >= 0 && mTablesRowOfInterest <= mTableDisplay->rowCount())
   {
      mTableDisplay->selectRow(mTablesRowOfInterest);
   }
   else
   {
      mTableDisplay->clearSelection();
   }
}

void WkTrackDetailsDisplay::TrackTableDisplay::SetRowOfInterest()
{
   if (mTableDisplay->selectionModel()->hasSelection())
   {
      mTablesRowOfInterest = mTableDisplay->currentRow();
      mTableDisplay->clearSelection();
   }
   else
   {
      mTablesRowOfInterest = -1;
   }
}

void WkTrackDetailsDisplay::TrackTableDisplay::UpdateTable()
{
   // QTableWidge takes ownership of the pointers
   for (auto& data : mTrackDataTable)
   {
      TrackId        trackId   = data.first;
      TrackTableData trackData = data.second;
      mTableDisplay->setItem(mRow, mColumn++, new TableItemSortable(QStringLiteral("Track %1").arg(trackId.mId)));
      mTableDisplay->setItem(mRow, mColumn++, new TableItemSortable(trackData.GetIFF()));
      mTableDisplay->setItem(mRow, mColumn++, new TableItemSortable(trackData.GetPlatformType()));
      mTableDisplay->setItem(mRow, mColumn++, new TableItemSortable(trackData.GetSide()));
      mTableDisplay->setItem(mRow, mColumn++, new TableItemSortable(trackData.GetRange()));
      mTableDisplay->setItem(mRow, mColumn++, new TableItemSortable(trackData.GetRangeRate()));
      mTableDisplay->setItem(mRow, mColumn++, new TableItemSortable(trackData.GetSpeed()));
      mTableDisplay->setItem(mRow, mColumn++, new TableItemSortable(trackData.GetRelativeHeading()));
      mRow++;
      mColumn = 0;
   }
   mRow = 0;
}

bool WkTrackDetailsDisplay::TrackTableDisplay::UpdateAvailable(const PlatformsTrackData& aTracksData)
{
   std::vector<std::pair<TrackId, TrackTableData>> potentialUpdate = aTracksData.mTableData;
   if (mTrackDataTable.size() != potentialUpdate.size())
   {
      return true;
   }

   auto it_m1  = mTrackDataTable.begin();
   auto end_m1 = mTrackDataTable.end();
   auto it_m2  = potentialUpdate.begin();
   auto end_m2 = potentialUpdate.end();

   while (it_m1 != end_m1 && it_m2 != end_m2)
   {
      if ((it_m1->first != it_m2->first) || (it_m1->second != it_m2->second))
      {
         return true;
      }
      ++it_m1;
      ++it_m2;
   }

   return false;
}

void WkTrackDetailsDisplay::TrackDetailsObservable::AttachObserver(std::unique_ptr<TrackDetailsObserver> aObserver)
{
   mObserverList.emplace_back(std::move(aObserver));
}

void WkTrackDetailsDisplay::TrackDetailsObservable::AttachTableObserver(std::unique_ptr<TrackDetailsObserver> aTableObserver)
{
   mTableObserver = std::move(aTableObserver);
}

void WkTrackDetailsDisplay::TrackDetailsObservable::UpdateTableObserver()
{
   mTableObserver->Update(mData.GetData());
}

WkTrackDetailsDisplay::TrackDetailsObservable::TrackDetailsObservable(DataContainer& aDataContainer)
   : mData(aDataContainer)
{
}

void WkTrackDetailsDisplay::TrackDetailsObservable::UpdateTrackDetailsObservers()
{
   for (auto& observer : mObserverList)
   {
      observer->Update(mData.GetData());
   }
}

void WkTrackDetailsDisplay::TrackDetailsObservable::ResetTrackDetailsObservers()
{
   for (auto& observer : mObserverList)
   {
      observer->Reset();
   }
}

void WkTrackDetailsDisplay::TrackDetailsObservable::ResetTableObserver()
{
   mTableObserver->Reset();
}

WkTrackDetailsDisplay::Dialog::Dialog(SimInterface&   aSimInterface,
                                      DataContainer&  aDataContainer,
                                      QWidget*        aParent,
                                      Qt::WindowFlags f)
   : QDialog(aParent, f)
   , mSimInterface(aSimInterface)
   , mTrackOfInterest(-1)
   , mObservable(aDataContainer)
{
   mUi.setupUi(this);

   // Attach Ui components to the Observable Data for more manageable updating.
   mObservable.AttachTableObserver(ut::make_unique<TrackTableDisplay>(mUi.trackTableWidget));
   mObservable.AttachObserver(ut::make_unique<TrackBasicInfoDisplay>(mUi));
   mObservable.AttachObserver(ut::make_unique<TrackManagerDisplay>(mUi));
   mObservable.AttachObserver(ut::make_unique<TrackStateDisplay>(mUi));
   mObservable.AttachObserver(ut::make_unique<TrackRelativeGeoDisplay>(mUi));
   mObservable.AttachObserver(ut::make_unique<TrackSensorDisplay>(mUi));

   // These connections will maintain the visible platforms for the user to select from
   connect(&aDataContainer, &DataContainer::PlatformAdded, this, &Dialog::AddPlatformForSelection);
   connect(&aDataContainer, &DataContainer::PlatformRemoved, this, &Dialog::RemovePlatformFromSelection);
   connect(&wkfEnv, &wkf::Environment::PlatformVisibilityChanged, this, &Dialog::UpdatePlatformListing);

   // update Sim with users latest selection of platform or track of interest
   connect(mUi.platformComboBox,
           QOverload<const QString&>::of(&QComboBox::currentTextChanged),
           this,
           &Dialog::UpdatePlatformOfInterest);
   connect(mUi.trackTableWidget, &QTableWidget::cellChanged, [this](int row, int column) { UpdateTrackOfInterest(row); });
   connect(mUi.trackTableWidget,
           &QTableWidget::currentCellChanged,
           [this](int row, int column) { UpdateTrackOfInterest(row); });
   connect(mUi.platformSelectButton, &wkf::PlatformSelector::Selected, this, &Dialog::HandlePlatformMapSelect);

   // Update Display with the latest data extracted from the Sim
   connect(&aDataContainer, &DataContainer::DataUpdated, this, &Dialog::UpdateDisplay);
   connect(wkfEnv.GetPreferenceObject<wkf::UnitsObject>(), &wkf::UnitsObject::PreferencesChanged, this, &Dialog::UpdateDisplay);

   // changing display from Platform X removed back to Data display
   connect(mUi.pushButton,
           &QAbstractButton::clicked,
           [this]()
           {
              mUi.platformComboBox->removeItem(mUi.platformComboBox->findText(mPlatformOfInterest));
              mUi.dataDisplayFrame->setCurrentIndex(0);
           });

   // Dealing with sim restart
   mCallBacks += wkf::Observer::StandardScenarioRemoved.Connect(&Dialog::UpdateOnSimReset, this);

   // Allows user to tab through the QTable
   mUi.trackTableWidget->installEventFilter(this);
}

bool WkTrackDetailsDisplay::Dialog::eventFilter(QObject* aObject, QEvent* aEvent)
{
   if (aObject == mUi.trackTableWidget && aEvent->type() == QEvent::KeyPress &&
       static_cast<QKeyEvent*>(aEvent)->key() == Qt::Key_Tab)
   {
      // Handle the tab press here
      if (mUi.trackTableWidget->selectionModel()->hasSelection())
      {
         if (mUi.trackTableWidget->currentRow() != mUi.trackTableWidget->rowCount() - 1)
         {
            mUi.trackTableWidget->selectRow(mUi.trackTableWidget->currentRow() + 1);
            return true;
         }
         else
         {
            mUi.trackTableWidget->selectRow(0);
            return true;
         }
         return false;
      }
      else
      {
         if (mUi.trackTableWidget->rowCount() > 0)
         {
            mUi.trackTableWidget->selectRow(0);
            return true;
         }
         return false;
      }
   }
   return false;
}

void WkTrackDetailsDisplay::Dialog::AddPlatformForSelection(const std::string& aNewPlatform)
{
   wkf::Scenario* scenario = vaEnv.GetStandardScenario();
   if (scenario)
   {
      wkf::Platform* platform = scenario->FindPlatformByName(aNewPlatform);
      if (wkfEnv.IsPlatformVisible(platform))
      {
         mUi.platformComboBox->addItem(aNewPlatform.c_str());
      }
   }
}

void WkTrackDetailsDisplay::Dialog::RemovePlatformFromSelection(const std::string& aPlatformName)
{
   QString platform{QString::fromStdString(aPlatformName)};
   int     index = mUi.platformComboBox->findText(platform, Qt::MatchExactly);

   if (mUi.platformComboBox->currentText() == platform)
   {
      mUi.platformRemovedlabel->setText("Platform " + platform + " has been removed from the simulation");
      mUi.dataDisplayFrame->setCurrentIndex(1);
   }
   else if (index >= 0 && mUi.platformComboBox->currentText() != platform)
   {
      mUi.platformComboBox->removeItem(index);
   }
}

void WkTrackDetailsDisplay::Dialog::UpdatePlatformListing()
{
   mUi.platformComboBox->clear();
   mUi.trackTableWidget->clearContents();
   wkf::Scenario* scenario = vaEnv.GetStandardScenario();
   if (scenario)
   {
      for (const auto& indexPlatformPair : scenario->GetIndexPlatformMap())
      {
         wkf::Platform* platform = indexPlatformPair.second;
         if (wkfEnv.IsPlatformVisible(platform))
         {
            mUi.platformComboBox->addItem(QString::fromStdString(platform->GetName()));
         }
      }
      UpdatePlatformOfInterest(mUi.platformComboBox->currentText());
   }
}

void WkTrackDetailsDisplay::Dialog::UpdateOnSimReset(wkf::Scenario*)
{
   mUi.pushButton->setEnabled(false);
   mUi.trackTableWidget->clearContents();
   UpdatePlatformListing();
   mUi.dataDisplayFrame->setCurrentIndex(0);
   mUi.pushButton->setEnabled(true);
}

void WkTrackDetailsDisplay::Dialog::UpdatePlatformOfInterest(const QString& aPlatformOfInterest)
{
   if (mPlatformOfInterest != aPlatformOfInterest)
   {
      mPlatformOfInterest = aPlatformOfInterest;
      mUi.trackTableWidget->clearContents();
      UpdateTrackOfInterest(-1);
      mSimInterface.SetTrackOfInterest(mTrackOfInterest, mPlatformOfInterest.toStdString());
      mObservable.UpdateTableObserver();
      ResetBottomDisplay();
   }
}

void WkTrackDetailsDisplay::Dialog::UpdateTrackOfInterest(const int aTrackOfInterest)
{
   if (mTrackOfInterest != aTrackOfInterest)
   {
      mTrackOfInterest = aTrackOfInterest;
      mSimInterface.SetTrackOfInterest(mTrackOfInterest, mPlatformOfInterest.toStdString());
   }
}

void WkTrackDetailsDisplay::Dialog::HandlePlatformMapSelect(wkf::Platform* aPlatform)
{
   mUi.platformComboBox->setCurrentIndex(mUi.platformComboBox->findText(aPlatform->GetName().c_str()));
}

void WkTrackDetailsDisplay::Dialog::UpdateTrackSelected()
{
   if (mUi.trackTableWidget->selectionModel()->hasSelection())
   {
      QModelIndexList indexList = mUi.trackTableWidget->selectionModel()->selectedRows();
      if (indexList.count() == 1)
      {
         int               rowKey = indexList.at(0).row();
         QTableWidgetItem* item   = mUi.trackTableWidget->item(rowKey, 0);
         if (item)
         {
            mUi.trackInfoGroupBox->setTitle(item->text());
         }
      }
   }
   else
   {
      ResetBottomDisplay();
   }
}

void WkTrackDetailsDisplay::Dialog::UpdateDisplay()
{
   mObservable.UpdateTableObserver();
   UpdateTrackSelected();
   if (mUi.trackTableWidget->selectionModel()->hasSelection())
   {
      mObservable.UpdateTrackDetailsObservers();
   }
   else
   {
      mObservable.ResetTrackDetailsObservers();
   }
}

void WkTrackDetailsDisplay::Dialog::ResetTableDisplay()
{
   mObservable.ResetTableObserver();
}

void WkTrackDetailsDisplay::Dialog::ResetBottomDisplay()
{
   mUi.trackInfoGroupBox->setTitle(QString("Track Info"));
   mObservable.ResetTrackDetailsObservers();
}
