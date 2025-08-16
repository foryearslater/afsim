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
#ifndef WKPLUGINTRACKS_HPP
#define WKPLUGINTRACKS_HPP

#include <QList>
#include <QMap>
#include <QTreeWidgetItem>

#include "TracksData.hpp"
#include "TracksSimInterface.hpp"
#include "VaCallbackHolder.hpp"
#include "WkPlugin.hpp"
#include "WkfScenario.hpp"
#include "WkfUnitTypes.hpp"

namespace vespa
{
class VaAttachment;
class VaEntity;
} // namespace vespa

namespace wkf
{
class Platform;
class Track;
class TrackDisplayInterface;
} // namespace wkf

namespace WkTracks
{
class Plugin : public warlock::PluginT<SimInterface>
{
public:
   Plugin(const QString& aPluginName, const size_t aUniqueId);

   std::unique_ptr<warlock::PlotUpdater> GetPlotUpdater(const QString& aIdentifier,
                                                        const QString& aY,
                                                        const unsigned aContext = 0) const override;

private:
   void GuiUpdate() override;
   void UpdatePlatformData();
   void SetTrackPrefs();

   void TrackInitiatedHandler(const TrackContainer::TrackData& aTrackData);
   void TrackDroppedHandler(const wkf::TrackId& aTrackId);
   void TrackUpdatedHandler(const TrackContainer::TrackData& aTrackData);

   QList<QTreeWidgetItem*> GetPlatformData(const std::string& aPlatformName) override;
   void                    PlatformDataItemSelected(QTreeWidgetItem* aItem) override;
   void                    PlatformDataItemUnSelected() override;

   void ResetTracks();
   void TrackVisibilityChangedCB();
   void PlatformSelectionChanged(const wkf::PlatformList& aSelectedPlatforms,
                                 const wkf::PlatformList& aUnselectedPlatforms);

   void EntityDeletedCB(vespa::VaEntity* aEntityPtr);

   void        CreateTrackIfVisible(const TrackContainer::TrackData& aTrackData);
   void        CreateTracksIfVisible(const TrackContainer& aTrackData);
   static void CreateTrack(wkf::Scenario* aScenario, const TrackContainer::TrackData& aTrackData);
   static void SetTrackState(wkf::Track* aTrack, const TrackContainer::TrackData& aData);

   void CreateTrackWidgets(int aTrackId, QTreeWidgetItem* aParentItem);
   void PopulateTrackWidgets(int aTrackId, TrackContainer::TrackData aTrackData);

   static void UpdateTrackLabel(wkf::Track* aTrack, bool aVisible, bool aShowColor);

   void RemoveTrackHighlight();

   vespa::VaCallbackHolder mCallbacks;

   TrackContainer mTrackData;

   std::string                               mPlatformOfInterest;
   QList<QTreeWidgetItem*>                   mTopLevelWidgets;
   std::map<int, wkf::PlatformDataTreeItem*> mTrackParentItems;

   wkf::Track*                       mTempHighlightTrack;
   std::vector<vespa::VaAttachment*> mTempHighlightAttachments;

   wkf::TrackDisplayInterface* mTrackDisplayInterface;

   class TrackItems
   {
   public:
      TrackItems(QTreeWidgetItem* aParentItem, int aTrackId);
      TrackItems(const TrackItems& aRhs) = default;
      TrackItems& operator=(const TrackItems&) = delete;

      void SetUserDataIdString(QString aPlatformName, int aTrackId);

      wkf::PlatformDataTreeItem*    mPositionValid;
      wkf::LatitudeTreeWidgetItem*  mLatitude;
      wkf::LongitudeTreeWidgetItem* mLongitude;
      wkf::AltitudeTreeWidgetItem*  mAltitude;
      wkf::PlatformDataTreeItem*    mVelocityValid;
      wkf::SpeedTreeWidgetItem*     mSpeed;
      wkf::AngleTreeWidgetItem*     mHeading;

      wkf::PlatformDataTreeItem* mRangeValid;
      wkf::LengthTreeWidgetItem* mRange;
      wkf::PlatformDataTreeItem* mBearingValid;
      wkf::AngleTreeWidgetItem*  mBearing;
      wkf::PlatformDataTreeItem* mElevationValid;
      wkf::AngleTreeWidgetItem*  mElevation;

      wkf::PlatformDataTreeItem*   mSide;
      wkf::PlatformDataTreeItem*   mType;
      wkf::PlatformDataTreeItem*   mSpatialDomain;
      wkf::UnitlessTreeWidgetItem* mQuality;
   };
   std::map<int, TrackItems> mTrackChildItems;
};
} // namespace WkTracks
#endif
