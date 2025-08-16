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
#include "TracksPlugin.hpp"

#include "TrackUpdaters.hpp"
#include "TracksSimCommands.hpp"
#include "UtEntity.hpp"
#include "UtMath.hpp"
#include "UtMemory.hpp"
#include "VaAttachment.hpp"
#include "VaAttachmentModel.hpp"
#include "VaObserver.hpp"
#include "WkfAttachmentLabel.hpp"
#include "WkfDefines.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMilStdIconPrefObject.hpp"
#include "WkfObserver.hpp"
#include "WkfPlatform.hpp"
#include "WkfScenario.hpp"
#include "WkfTeamVisibilityPrefObject.hpp"
#include "WkfTrack.hpp"
#include "WkfViewer.hpp"
#include "WkfVtkEnvironment.hpp"
#include "misc/WkfTrackDisplayInterface.hpp"
#include "visibility/WkfTrackVisibilityPrefObject.hpp"

static const QString TRACK_SEPARATOR = " Track:";
static const int     TRACK_ID_ROLE   = Qt::UserRole;

enum eDataType
{
   ePOSITION_VALID,
   eLATITUDE,
   eLONGITUDE,
   eALTITUDE,
   eVELOCITY_VALID,
   eSPEED,
   eMACH,
   eHEADING,
   eRANGE_VALID,
   eRANGE,
   eBEARING_VALID,
   eBEARING,
   eELEVATION_VALID,
   eELEVATION,
   eSIDE,
   eTYPE,
   eSPATIAL_DOMAIN,
   eQUALITY
};

static const std::map<eDataType, QString> sDataTypeToStringMap = {{ePOSITION_VALID, "Position Valid"},
                                                                  {eLATITUDE, "Latitude"},
                                                                  {eLONGITUDE, "Longitude"},
                                                                  {eALTITUDE, "Altitude"},
                                                                  {eVELOCITY_VALID, "Velocity Valid"},
                                                                  {eSPEED, "Speed"},
                                                                  {eMACH, "Mach"},
                                                                  {eHEADING, "Heading"},
                                                                  {eRANGE_VALID, "Range Valid"},
                                                                  {eRANGE, "Range"},
                                                                  {eBEARING_VALID, "Bearing Valid"},
                                                                  {eBEARING, "Bearing"},
                                                                  {eELEVATION_VALID, "Elevation Valid"},
                                                                  {eELEVATION, "Elevation"},
                                                                  {eSIDE, "Side"},
                                                                  {eTYPE, "Type"},
                                                                  {eSPATIAL_DOMAIN, "Spatial Domain"},
                                                                  {eQUALITY, "Quality"}};

WKF_PLUGIN_DEFINE_SYMBOLS(
   WkTracks::Plugin,
   "Tracks",
   "The Tracks plugin gets track information from the simulation and displays icons on the main map display. It also "
   "populates the master track list section on the Platform Details dialog.",
   "warlock")

WkTracks::Plugin::Plugin(const QString& aPluginName, const size_t aUniqueId)
   : warlock::PluginT<SimInterface>(aPluginName, aUniqueId)
   , mTempHighlightTrack(nullptr)
   , mTrackDisplayInterface(new wkf::TrackDisplayInterface(this))
{
   connect(&wkfEnv, &wkf::Environment::PlatformVisibilityChanged, this, &Plugin::ResetTracks);
   connect(wkfEnv.GetPreferenceObject<wkf::TrackVisibilityPrefObject>(),
           &wkf::TrackVisibilityPrefObject::TrackVisibilityChanged,
           this,
           &Plugin::TrackVisibilityChangedCB);
   connect(&wkfEnv, &wkf::Environment::PlatformSelectionChanged, this, &Plugin::PlatformSelectionChanged);

   connect(&mTrackData, &TrackContainer::TrackInitiated, this, &Plugin::TrackInitiatedHandler);
   connect(&mTrackData, &TrackContainer::TrackDropped, this, &Plugin::TrackDroppedHandler);
   connect(&mTrackData, &TrackContainer::TrackUpdated, this, &Plugin::TrackUpdatedHandler);

   mCallbacks.Add(vespa::VaObserver::EntityDeleted.Connect(&Plugin::EntityDeletedCB, this));

   wkf::PlatformDataTreeItem* trackParent = new wkf::PlatformDataTreeItem();
   trackParent->setText(0, "Master Track List");
   mTopLevelWidgets.push_back(trackParent);

   SetTrackPrefs();
}

std::unique_ptr<warlock::PlotUpdater> WkTracks::Plugin::GetPlotUpdater(const QString& aIdentifier,
                                                                       const QString& aY,
                                                                       const unsigned aContext) const
{
   QStringList list = aIdentifier.split(TRACK_SEPARATOR);
   if (list.size() == 2)
   {
      std::string platformName = list.at(0).toStdString();
      int         trackId      = list.at(1).toInt();

      // Assume the X value is simulation time
      if (aContext == 0)
      {
         if (aY == sDataTypeToStringMap.at(eLATITUDE))
         {
            return ut::make_unique<LatitudeUpdater>(platformName, trackId, 1);
         }
         if (aY == sDataTypeToStringMap.at(eLONGITUDE))
         {
            return ut::make_unique<LongitudeUpdater>(platformName, trackId, 1);
         }
         if (aY == sDataTypeToStringMap.at(eALTITUDE))
         {
            return ut::make_unique<AltitudeUpdater>(platformName, trackId, 1);
         }
         if (aY == sDataTypeToStringMap.at(eSPEED))
         {
            return ut::make_unique<SpeedUpdater>(platformName, trackId, 1);
         }
         if (aY == sDataTypeToStringMap.at(eMACH))
         {
            return ut::make_unique<MachUpdater>(platformName, trackId, 1);
         }
         if (aY == sDataTypeToStringMap.at(eHEADING))
         {
            return ut::make_unique<HeadingUpdater>(platformName, trackId, 1);
         }
         if (aY == sDataTypeToStringMap.at(eRANGE))
         {
            return ut::make_unique<RangeUpdater>(platformName, trackId, 1);
         }
         if (aY == sDataTypeToStringMap.at(eBEARING))
         {
            return ut::make_unique<BearingUpdater>(platformName, trackId, 1);
         }
         if (aY == sDataTypeToStringMap.at(eELEVATION))
         {
            return ut::make_unique<ElevationUpdater>(platformName, trackId, 1);
         }
      }
   }
   return nullptr;
}

void WkTracks::Plugin::GuiUpdate()
{
   mInterfacePtr->ProcessEvents(mTrackData);
   UpdatePlatformData();
}

void WkTracks::Plugin::TrackInitiatedHandler(const TrackContainer::TrackData& aTrackData)
{
   CreateTrackIfVisible(aTrackData);
}

void WkTracks::Plugin::TrackDroppedHandler(const wkf::TrackId& aTrackId)
{
   wkf::Scenario* scenario = vaEnv.GetStandardScenario();
   if (scenario)
   {
      scenario->DeleteTrack(aTrackId);
   }
}

void WkTracks::Plugin::TrackUpdatedHandler(const TrackContainer::TrackData& aTrackData)
{
   wkf::Scenario* scenario = vaEnv.GetStandardScenario();
   if (scenario)
   {
      wkf::Track* track = scenario->FindTrack(aTrackData.mTrackId);
      if (track)
      {
         SetTrackState(track, aTrackData);
      }
      else
      {
         // This is here because there is no concept of "track initiated" for xio tracks.
         // So, if we are told to update a track that doesn't exist yet, pass on
         // to TrackInitiatedHandler
         TrackInitiatedHandler(aTrackData);
      }
   }
}

void WkTracks::Plugin::UpdatePlatformData()
{
   auto trackData = mTrackData.GetTrackData(mPlatformOfInterest);

   // First remove any tracks that may have been dropped since last update
   std::set<int> indicesToRemove;
   for (auto& track : mTrackParentItems)
   {
      // if the track no longer exist
      if (trackData.count(track.first) == 0)
      {
         indicesToRemove.insert(track.first);
      }
   }

   for (int index : indicesToRemove)
   {
      delete mTrackParentItems[index];
      mTrackParentItems.erase(index);
      mTrackChildItems.erase(index);
   }

   // Second add any tracks that have been created since last update
   std::set<int> indicesToAdd;
   for (auto& data : trackData)
   {
      if (mTrackParentItems.count(data.first) == 0)
      {
         indicesToAdd.insert(data.first);
      }
   }
   for (int index : indicesToAdd)
   {
      CreateTrackWidgets(index, mTopLevelWidgets.at(0));
      mTrackChildItems.at(index).SetUserDataIdString(QString::fromStdString(mPlatformOfInterest), index);
   }

   // Third update all track data
   for (auto& data : trackData)
   {
      PopulateTrackWidgets(data.first, data.second);
   }
}

// Let SimInterface know how to filter the track requests when it gets them
void WkTracks::Plugin::SetTrackPrefs()
{
   auto* trackPrefs = wkfEnv.GetPreferenceObject<wkf::TrackVisibilityPrefObject>();
   auto* teamPrefs  = wkfEnv.GetPreferenceObject<wkf::TeamVisibilityPrefObject>();

   if (trackPrefs && teamPrefs)
   {
      mInterfacePtr->SetTrackPrefs(trackPrefs->GetPreferences(), teamPrefs->GetTeamVisibilityMap());
   }
}

QList<QTreeWidgetItem*> WkTracks::Plugin::GetPlatformData(const std::string& aPlatformName)
{
   // Store the new platform to be shown
   mPlatformOfInterest = aPlatformName;
   // Remove track highlighting relating to the previously selected platform
   RemoveTrackHighlight();

   // Remove the data display which for the old platform's master track list
   QTreeWidgetItem* masterTrackItem = mTopLevelWidgets.at(0);
   for (auto& trackWidget : mTrackParentItems)
   {
      masterTrackItem->removeChild(trackWidget.second);
   }
   mTrackParentItems.clear();
   mTrackChildItems.clear();

   // Get data for the new platform, and populate the tree widget
   auto trackData = mTrackData.GetTrackData(mPlatformOfInterest);
   for (auto& data : trackData)
   {
      int trackId = data.first;
      CreateTrackWidgets(trackId, masterTrackItem);
      mTrackChildItems.at(trackId).SetUserDataIdString(QString::fromStdString(mPlatformOfInterest), trackId);
      PopulateTrackWidgets(trackId, data.second);
   }

   return mTopLevelWidgets;
}

void WkTracks::Plugin::PlatformDataItemSelected(QTreeWidgetItem* aItem)
{
   wkf::Scenario* scenario = vaEnv.GetStandardScenario();
   if (scenario)
   {
      int         trackId = aItem->data(0, TRACK_ID_ROLE).toInt();
      wkf::Track* track   = scenario->FindTrack(wkf::TrackId(mPlatformOfInterest, trackId));
      if (track)
      {
         mTempHighlightAttachments.clear();
         track->FindAttachments<vespa::VaAttachmentModel>(mTempHighlightAttachments);
         for (auto* attachment : mTempHighlightAttachments)
         {
            UtColor color(.5, 1, .5, 0.7f);
            attachment->Highlight(vaEnv.GetStandardViewer(), 0, true, color);
            mTempHighlightTrack = track;
         }
      }
   }
}

void WkTracks::Plugin::PlatformDataItemUnSelected()
{
   RemoveTrackHighlight();
}

// Delete tracks and recreate them
void WkTracks::Plugin::ResetTracks()
{
   wkf::Scenario* scenario = vaEnv.GetStandardScenario();
   if (scenario)
   {
      // Remove all the existing tracks
      scenario->DeleteAllTracks();
      // Pass in list of all tracks
      CreateTracksIfVisible(mTrackData);
   }
}

void WkTracks::Plugin::TrackVisibilityChangedCB()
{
   // Let SimInterface know how to filter the track requests when it gets them
   SetTrackPrefs();

   // We only want to request tracks if the user wants to see remote tracks.
   // Once the user clicks on a remote platform, the tracks will be requested
   // in PlatformSelectionChanged
   auto* trackPrefs = wkfEnv.GetPreferenceObject<wkf::TrackVisibilityPrefObject>();
   auto  trackVis   = trackPrefs->GetTrackVisibility();
   if (trackPrefs->GetShowRemoteTracks())
   {
      if (trackVis == wkf::tracks::Visibility::cSELECTED_TEAM)
      {
         mInterfacePtr->AddTeamTracksRequestCommand();
      }
      else if (trackVis == wkf::tracks::Visibility::cALL_VISIBLE_TEAMS)
      {
         mInterfacePtr->AddAllTracksRequestCommand();
      }
   }

   // When track visibility changes, redraw all tracks in the scenario
   ResetTracks();
}

void WkTracks::Plugin::PlatformSelectionChanged(const wkf::PlatformList& aSelectedPlatforms,
                                                const wkf::PlatformList& aUnselectedPlatforms)
{
   // When the platform selection changes,
   // add/remove track requests (for remote platforms)
   for (const auto& p : aSelectedPlatforms)
   {
      mInterfacePtr->AddPlatformTrackRequestCommand(p->GetIndex());
   }
   for (const auto& p : aUnselectedPlatforms)
   {
      mInterfacePtr->RemovePlatformTrackRequestCommand(p->GetIndex());
   }

   // When the platform selection changes and we are displaying local tracks,
   // redraw all tracks in the scenario.
   if (wkfEnv.GetPreferenceObject<wkf::TrackVisibilityPrefObject>()->GetTrackVisibility() ==
       wkf::tracks::Visibility::cLOCAL_ONLY)
   {
      ResetTracks();
   }
}

void WkTracks::Plugin::EntityDeletedCB(vespa::VaEntity* aEntityPtr)
{
   wkf::Track* track = dynamic_cast<wkf::Track*>(aEntityPtr);
   if (track && mTempHighlightTrack == track)
   {
      RemoveTrackHighlight();
   }
}

void WkTracks::Plugin::CreateTrackIfVisible(const TrackContainer::TrackData& aTrackData)
{
   wkf::Scenario* scenario = vaEnv.GetStandardScenario();
   if (scenario)
   {
      const auto* trackPrefs = wkfEnv.GetPreferenceObject<wkf::TrackVisibilityPrefObject>();
      auto        trackVis   = trackPrefs->GetTrackVisibility();

      if (trackVis == wkf::tracks::Visibility::cLOCAL_ONLY)
      {
         wkf::Platform* platform =
            dynamic_cast<wkf::Platform*>(scenario->FindEntity(aTrackData.mTrackId.GetPlatformName()));
         if (wkfEnv.IsPlatformSelected(aTrackData.mTrackId.GetPlatformName()) && // If the platform is selected
             platform &&                         // and the platform exists (This is done second for optimization)
             wkfEnv.IsPlatformVisible(platform)) // and the platform is visible
         {
            CreateTrack(scenario, aTrackData);
         }
      }
      else if (trackVis == wkf::tracks::Visibility::cSELECTED_TEAM)
      {
         wkf::Platform* platform =
            dynamic_cast<wkf::Platform*>(scenario->FindEntity(aTrackData.mTrackId.GetPlatformName()));
         if (platform &&                           // If platform exists
             wkfEnv.IsPlatformVisible(platform) && // and the platform owning the tracks is visible
             platform->GetSide() ==
                trackPrefs->GetSelectedTeamForTracks().toStdString()) // and the platform is on the selected side
         {
            CreateTrack(scenario, aTrackData);
         }
      }
      else if (trackVis == wkf::tracks::Visibility::cALL_VISIBLE_TEAMS)
      {
         wkf::Platform* platform =
            dynamic_cast<wkf::Platform*>(scenario->FindEntity(aTrackData.mTrackId.GetPlatformName()));
         if (platform &&                         // If platform exists
             wkfEnv.IsPlatformVisible(platform)) // and the platform owning the tracks is visible
         {
            CreateTrack(scenario, aTrackData);
         }
      }
   }
}

void WkTracks::Plugin::CreateTracksIfVisible(const TrackContainer& aTrackData)
{
   wkf::Scenario* scenario = vaEnv.GetStandardScenario();
   if (scenario)
   {
      auto* trackPrefs = wkfEnv.GetPreferenceObject<wkf::TrackVisibilityPrefObject>();
      auto  trackVis   = trackPrefs->GetTrackVisibility();

      // create filtered list of tracks based on preferences.
      TrackContainer::TrackDataList filteredTracks;
      if (trackVis == wkf::tracks::Visibility::cLOCAL_ONLY)
      {
         for (const auto& p : wkfEnv.GetSelectedPlatforms())
         {
            const std::string& name = p->GetName();

            wkf::Platform* platform = dynamic_cast<wkf::Platform*>(scenario->FindEntity(name));
            if (platform &&                         // If platform exists
                wkfEnv.IsPlatformVisible(platform)) // and the platform owning the tracks is visible
            {
               auto platformTracks = aTrackData.GetTrackData(name);

               for (auto& track : platformTracks)
               {
                  filteredTracks[track.second.mTrackId] = std::move(track.second);
               }
            }
         }
      }
      else if (trackVis == wkf::tracks::Visibility::cSELECTED_TEAM)
      {
         std::string selectedTeam = trackPrefs->GetSelectedTeamForTracks().toStdString();
         for (const auto& data : aTrackData.GetAllTrackData())
         {
            wkf::Platform* platform = dynamic_cast<wkf::Platform*>(scenario->FindEntity(data.first.GetPlatformName()));
            if (platform &&                            // If platform exists
                platform->GetSide() == selectedTeam && // and the platform on the selected team
                wkfEnv.IsPlatformVisible(platform))    // and the platform owning the tracks is visible
            {
               filteredTracks[data.first] = data.second;
            }
         }
      }
      else if (trackVis == wkf::tracks::Visibility::cALL_VISIBLE_TEAMS)
      {
         for (const auto& data : aTrackData.GetAllTrackData())
         {
            wkf::Platform* platform = dynamic_cast<wkf::Platform*>(scenario->FindEntity(data.first.GetPlatformName()));
            if (platform &&                         // If platform exists
                wkfEnv.IsPlatformVisible(platform)) // and the platform owning the tracks is visible
            {
               filteredTracks[data.first] = data.second;
            }
         }
      }

      for (const auto& track : filteredTracks)
      {
         CreateTrack(scenario, track.second);
      }
   }
}

void WkTracks::Plugin::CreateTrack(wkf::Scenario* aScenario, const TrackContainer::TrackData& aTrackData)
{
   auto showRemote = wkfEnv.GetPreferenceObject<wkf::TrackVisibilityPrefObject>()->GetShowRemoteTracks();

   // don't create the track if the platform is remote,
   // and the user doesn't want to show remote tracks
   if (!(aTrackData.mRemote && !showRemote))
   {
      if (!aScenario->FindTrack(aTrackData.mTrackId))
      {
         wkf::Track* track = new wkf::Track(aTrackData.mTrackId);
         SetTrackState(track, aTrackData);
         aScenario->AddTrack(track);
         wkf::Observer::TrackAdded(track);
      }
   }
}

void WkTracks::Plugin::UpdateTrackLabel(wkf::Track* aTrack, bool aVisible, bool aShowColor)
{
   wkf::AttachmentLabel* labelPtr = dynamic_cast<wkf::AttachmentLabel*>(aTrack->FindAttachment("label"));
   // if label attachment does not exist, create it
   if (!labelPtr)
   {
      try
      {
         labelPtr =
            vespa::VaEnvironment::CreateAttachment<wkf::AttachmentLabel>("label", *aTrack, vaEnv.GetStandardViewer(), true);
      }
      catch (const vespa::UnknownAttachmentTypeError& e)
      {
         ut::log::error() << e.what() << "in WkTracks::Plugin::UpdateTrackLabel()";
         return;
      }
      // Fortify still warns even if an exception is thrown.
      if (!labelPtr)
      {
         return;
      }
   }
   if (labelPtr)
   {
      if (aVisible)
      {
         auto* trackPref = wkfEnv.GetPreferenceObject<wkf::TrackVisibilityPrefObject>();

         auto   side     = aTrack->GetSide();
         auto*  teamPref = wkfEnv.GetPreferenceObject<wkf::TeamVisibilityPrefObject>();
         QColor color;

         if (aShowColor && !side.empty())
         {
            color = teamPref->GetTeamColor(QString::fromStdString(side)); // set the label color as the track team color
         }
         else
         {
            color =
               teamPref->GetTeamColor(wkf::TeamVisibilityPrefData::cDEFAULT_TEAM); // set the label color as default team
         }
         labelPtr->Setup(QString::fromStdString(aTrack->GetName()), color, trackPref->GetTrackFont());
      }

      labelPtr->SetStateVisibility(aVisible);
   }
}

void WkTracks::Plugin::SetTrackState(wkf::Track* aTrack, const TrackContainer::TrackData& aData)
{
   UtEntity entity;
   entity.SetLocationWCS(aData.mPositionWCS);
   entity.SetVelocityWCS(aData.mVelocityWCS);

   const bool useHeading =
      aData.mVelocityValid && wkfEnv.GetPreferenceObject<wkf::TrackVisibilityPrefObject>()->GetShowDirection();
   if (useHeading)
   {
      double vel[3];
      entity.GetVelocityNED(vel);
      const double heading_rad = atan2(vel[1], vel[0]);
      entity.SetOrientationNED(heading_rad, 0, 0);
   }
   else
   {
      entity.SetOrientationNED(0, 0, 0);
   }

   double psi, theta, phi;
   entity.GetOrientationWCS(psi, theta, phi);

   aTrack->SetPositionValid(aData.mLocationValid);
   aTrack->SetVelocityValid(aData.mVelocityValid);
   aTrack->SetPositionOrientation(aData.mPositionWCS, psi, theta, phi);
   aTrack->SetVelocityWCS(aData.mVelocityWCS);
   aTrack->SetSide(aData.mSide);
   aTrack->SetTrackType(aData.mType);
   aTrack->SetTargetPlatform(aData.mTargetPlatform);
   aTrack->SetSpatialDomain(aData.mSpatialDomain);
   aTrack->SetIcon(aData.mMilStdIcon);

   const bool showLabel = wkfEnv.GetPreferenceObject<wkf::TrackVisibilityPrefObject>()->GetShowLabel();
   const bool showColor = wkfEnv.GetPreferenceObject<wkf::TrackVisibilityPrefObject>()->GetShowColor();
   UpdateTrackLabel(aTrack, showLabel, showColor);
}

void WkTracks::Plugin::CreateTrackWidgets(int aTrackId, QTreeWidgetItem* aParentItem)
{
   mTrackParentItems[aTrackId] = new wkf::PlatformDataTreeItem(aParentItem, {"Track " + QString::number(aTrackId)});
   mTrackParentItems[aTrackId]->SetUnhideable();
   mTrackParentItems[aTrackId]->setData(0, TRACK_ID_ROLE, aTrackId);
   mTrackChildItems.emplace(aTrackId, TrackItems(mTrackParentItems[aTrackId], aTrackId));
}

void WkTracks::Plugin::PopulateTrackWidgets(int aTrackId, TrackContainer::TrackData aTrackData)
{
   UtEntity entity;
   entity.SetLocationWCS(aTrackData.mPositionWCS);
   entity.SetVelocityWCS(aTrackData.mVelocityWCS);
   double vel[3] = {0, 0, 0};
   entity.GetVelocityNED(vel);

   double heading_rad = atan2(vel[1], vel[0]);
   // For display purposes put the valid heading range in [0, 2*PI]
   heading_rad = UtMath::NormalizeAngle0_TwoPi(heading_rad);

   double lat, lon, alt_m;
   entity.GetLocationLLA(lat, lon, alt_m);
   double speed_mps = entity.GetSpeed();

   mTrackChildItems.at(aTrackId).mPositionValid->SetHidden(aTrackData.mLocationValid);
   mTrackChildItems.at(aTrackId).mPositionValid->setText(1, aTrackData.mLocationValid ? "true" : "false");
   mTrackChildItems.at(aTrackId).mLatitude->SetHidden(!aTrackData.mLocationValid);
   mTrackChildItems.at(aTrackId).mLatitude->SetValue(lat);
   mTrackChildItems.at(aTrackId).mLongitude->SetHidden(!aTrackData.mLocationValid);
   mTrackChildItems.at(aTrackId).mLongitude->SetValue(lon);
   mTrackChildItems.at(aTrackId).mAltitude->SetHidden(!aTrackData.mLocationValid);
   mTrackChildItems.at(aTrackId).mAltitude->SetValue(alt_m);

   mTrackChildItems.at(aTrackId).mVelocityValid->SetHidden(aTrackData.mVelocityValid);
   mTrackChildItems.at(aTrackId).mVelocityValid->setText(1, aTrackData.mVelocityValid ? "true" : "false");
   mTrackChildItems.at(aTrackId).mSpeed->SetHidden(!aTrackData.mVelocityValid);
   mTrackChildItems.at(aTrackId).mSpeed->SetValue(speed_mps);
   mTrackChildItems.at(aTrackId).mHeading->SetHidden(!aTrackData.mVelocityValid);
   mTrackChildItems.at(aTrackId).mHeading->SetValue(heading_rad);

   mTrackChildItems.at(aTrackId).mRangeValid->SetHidden(aTrackData.mRangeValid);
   mTrackChildItems.at(aTrackId).mRangeValid->setText(1, aTrackData.mRangeValid ? "true" : "false");
   mTrackChildItems.at(aTrackId).mRange->SetHidden(!aTrackData.mRangeValid);
   mTrackChildItems.at(aTrackId).mRange->SetValue(aTrackData.mRange_m);

   mTrackChildItems.at(aTrackId).mBearingValid->setHidden(aTrackData.mBearingValid);
   mTrackChildItems.at(aTrackId).mBearingValid->setText(1, aTrackData.mBearingValid ? "true" : "false");
   mTrackChildItems.at(aTrackId).mBearing->SetHidden(!aTrackData.mBearingValid);
   mTrackChildItems.at(aTrackId).mBearing->SetValue(aTrackData.mBearing_rad);

   mTrackChildItems.at(aTrackId).mElevationValid->setHidden(aTrackData.mElevationValid);
   mTrackChildItems.at(aTrackId).mElevationValid->setText(1, aTrackData.mElevationValid ? "true" : "false");
   mTrackChildItems.at(aTrackId).mElevation->SetHidden(!aTrackData.mElevationValid);
   mTrackChildItems.at(aTrackId).mElevation->SetValue(aTrackData.mElevation_rad);

   QString side = QString::fromStdString(aTrackData.mSide);
   if (side.isEmpty())
   {
      side = "Unknown";
   }
   mTrackChildItems.at(aTrackId).mSide->setText(1, side);
   QString type = QString::fromStdString(aTrackData.mType);
   if (type.isEmpty())
   {
      type = "Unknown";
   }
   mTrackChildItems.at(aTrackId).mType->setText(1, type);
   mTrackChildItems.at(aTrackId).mSpatialDomain->setText(1,
                                                         QString::fromStdString(
                                                            wkf::SpatialDomainToString(aTrackData.mSpatialDomain)));
   mTrackChildItems.at(aTrackId).mQuality->SetValue(aTrackData.mQuality);
}

void WkTracks::Plugin::RemoveTrackHighlight()
{
   for (auto* attachment : mTempHighlightAttachments)
   {
      UtColor color(.5, 1, .5, 0.7f);
      attachment->Highlight(vaEnv.GetStandardViewer(), 0, false, color);
      mTempHighlightTrack = nullptr;
   }
   mTempHighlightAttachments.clear();
}

WkTracks::Plugin::TrackItems::TrackItems(QTreeWidgetItem* aParentItem, int aTrackId)
{
   mPositionValid  = new wkf::PlatformDataTreeItem(aParentItem, {sDataTypeToStringMap.at(ePOSITION_VALID)});
   mLatitude       = new wkf::LatitudeTreeWidgetItem(aParentItem, {sDataTypeToStringMap.at(eLATITUDE)});
   mLongitude      = new wkf::LongitudeTreeWidgetItem(aParentItem, {sDataTypeToStringMap.at(eLONGITUDE)});
   mAltitude       = new wkf::AltitudeTreeWidgetItem(aParentItem, {sDataTypeToStringMap.at(eALTITUDE)});
   mVelocityValid  = new wkf::PlatformDataTreeItem(aParentItem, {sDataTypeToStringMap.at(eVELOCITY_VALID)});
   mSpeed          = new wkf::SpeedTreeWidgetItem(aParentItem, {sDataTypeToStringMap.at(eSPEED)});
   mHeading        = new wkf::AngleTreeWidgetItem(aParentItem, {sDataTypeToStringMap.at(eHEADING)});
   mRangeValid     = new wkf::PlatformDataTreeItem(aParentItem, {sDataTypeToStringMap.at(eRANGE_VALID)});
   mRange          = new wkf::LengthTreeWidgetItem(aParentItem, {sDataTypeToStringMap.at(eRANGE)});
   mBearingValid   = new wkf::PlatformDataTreeItem(aParentItem, {sDataTypeToStringMap.at(eBEARING_VALID)});
   mBearing        = new wkf::AngleTreeWidgetItem(aParentItem, {sDataTypeToStringMap.at(eBEARING)});
   mElevationValid = new wkf::PlatformDataTreeItem(aParentItem, {sDataTypeToStringMap.at(eELEVATION_VALID)});
   mElevation      = new wkf::AngleTreeWidgetItem(aParentItem, {sDataTypeToStringMap.at(eELEVATION)});
   mSide           = new wkf::PlatformDataTreeItem(aParentItem, {sDataTypeToStringMap.at(eSIDE)});
   mType           = new wkf::PlatformDataTreeItem(aParentItem, {sDataTypeToStringMap.at(eTYPE)});
   mSpatialDomain  = new wkf::PlatformDataTreeItem(aParentItem, {sDataTypeToStringMap.at(eSPATIAL_DOMAIN)});
   mQuality        = new wkf::UnitlessTreeWidgetItem(aParentItem, {sDataTypeToStringMap.at(eQUALITY)});

   std::array<wkf::PlatformDataTreeItem*, 17> items = {mPositionValid,
                                                       mLatitude,
                                                       mLongitude,
                                                       mAltitude,
                                                       mVelocityValid,
                                                       mSpeed,
                                                       mHeading,
                                                       mRangeValid,
                                                       mRange,
                                                       mBearingValid,
                                                       mBearing,
                                                       mElevationValid,
                                                       mElevation,
                                                       mSide,
                                                       mType,
                                                       mSpatialDomain,
                                                       mQuality};

   for (auto* item : items)
   {
      item->setData(0, TRACK_ID_ROLE, aTrackId);
      item->SetUnhideable();
   }
}

void WkTracks::Plugin::TrackItems::SetUserDataIdString(QString aPlatformName, int aTrackId)
{
   QString id = aPlatformName + TRACK_SEPARATOR + QString::number(aTrackId);

   std::array<wkf::PlatformDataTreeItem*, 17> items = {mPositionValid,
                                                       mLatitude,
                                                       mLongitude,
                                                       mAltitude,
                                                       mVelocityValid,
                                                       mSpeed,
                                                       mHeading,
                                                       mRangeValid,
                                                       mRange,
                                                       mBearingValid,
                                                       mBearing,
                                                       mElevationValid,
                                                       mElevation,
                                                       mSide,
                                                       mType,
                                                       mSpatialDomain,
                                                       mQuality};

   for (auto* item : items)
   {
      item->setData(0, wkf::ID_ROLE, id);
   }
}
