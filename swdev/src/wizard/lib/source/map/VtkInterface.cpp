// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2018 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "VtkInterface.hpp"

#include <string>
#include <unordered_set>

#include <QApplication>
#include <QDesktopWidget>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMenu>
#include <QMimeData>
#include <QTextBlock>
#include <QTextDocumentFragment>

#include "ChangeWrapper.hpp"
#include "CreatePlatformDialog.hpp"
#include "DragAndDrop.hpp"
#include "Editor.hpp"
#include "EditorHelpers.hpp"
#include "EditorPrefObject.hpp"
#include "Environment.hpp"
#include "ParseResults.hpp"
#include "PasteContext.hpp"
#include "Platform.hpp"
#include "Project.hpp"
#include "ProjectWorkspace.hpp"
#include "ProxyWatcher.hpp"
#include "Signals.hpp"
#include "UtEarth.hpp"
#include "UtEllipsoidalEarth.hpp"
#include "VaCamera.hpp"
#include "VaUtils.hpp"
#include "VaWidget.hpp"
#include "WkfEnvironment.hpp"
#include "WkfGeneralPrefObject.hpp"
#include "WkfMainWindow.hpp"
#include "WkfObserver.hpp"
#include "WkfScenario.hpp"
#include "WkfUnitsObject.hpp"
#include "WkfViewer.hpp"
#include "WkfVtkEnvironment.hpp"
#include "WsfPathComputer.hpp"
#include "WsfPathList.hpp"
#include "WsfPathState.hpp"
#include "Zone.hpp"
#include "ZoneSet.hpp"
#include "route/WkfAttachmentRoute.hpp"
#include "route/WkfRouteBrowserCreateDialog.hpp"
#include "route/WkfRouteBrowserInterface.hpp"
#include "zone/WkfAttachmentZone.hpp"
#include "zone/WkfAttachmentZonePolygon.hpp"
#include "zone/WkfZonePoint.hpp"

const std::string wizard::VtkInterface::cGLOBAL_ZONES_PREFIX = "Global_Zones_";

wizard::VtkInterface::VtkInterface()
   : mProxyRoot(nullptr)
   , mNextId(0)
{
   mProxyAvailableCallback += wizard::ProxyWatcher::ProxyAvailable.Connect(&VtkInterface::OnProxyAvailable, this);
   mConnection =
      QObject::connect(wizSignals, &Signals::ActionsForNodeRequested, this, &VtkInterface::ActionsForNodeRequested);

   vespa::VaWidget* widget = vaEnv.GetStandardWidget();
   if (nullptr != widget)
   {
      connect(widget, &vespa::VaWidget::DragEnter, this, &VtkInterface::DragEnter);
      connect(widget, &vespa::VaWidget::DragDrop, this, &VtkInterface::DragDrop);
   }
   else
   {
      mVtkCallbacks.Add(wkf::Observer::StandardWidgetChanged.Connect(&VtkInterface::StandardWidgetChangedCB, this));
   }
   vaEnv.EnablePlatformsDraggable(true);
}

wizard::VtkInterface::~VtkInterface()
{
   QObject::disconnect(mConnection);
}

void wizard::VtkInterface::OnProxyAvailable(WsfPProxy* aProxyPtr)
{
   if (mProxyRoot.GetProxy() == aProxyPtr)
   {
      return;
   }

   if (!vaEnv.GetStandardScenario())
   {
      wkf::Scenario* scenario = (wkf::Scenario*)vaEnv.NewScenario("wkf");
      vaEnv.SetStandardScenario(scenario);
      connect(&wkfEnv,
              &wkf::Environment::MoveSelectionAndAttachmentComplete,
              this,
              &VtkInterface::MoveSelectionAndAttachmentComplete);
   }

   mProxyRoot = aProxyPtr;
   if (aProxyPtr != nullptr)
   {
      auto pmap = mProxyRoot.platforms();
      for (auto&& it = pmap.begin(); it != pmap.end(); ++it)
      {
         auto platform = pmap.AtT(it);
         PlatformAdded(platform);
      }
      // now that all platforms have been added, add their zones
      for (auto&& it = pmap.begin(); it != pmap.end(); ++it)
      {
         auto                       platform = pmap.AtT(it);
         auto                       zoneMap  = platform.Zone();
         std::vector<WsfPProxyNode> platformZones;
         zoneMap.GetAllValues(platformZones);
         for (const auto& zoneIter : platformZones)
         {
            auto zoneNode = static_cast<WsfPM_Zone>(zoneIter);
            UpdateZone(zoneNode);
         }
      }
      auto zoneMap = mProxyRoot.zones();
      for (auto&& it = zoneMap.begin(); it != zoneMap.end(); ++it)
      {
         auto zone = zoneMap.AtT(it);
         ZoneAdded(zone);
      }

      mProxyCallbacks.Add(
         WatchProxy(mProxyRoot.platforms().GetPath()).AnyUpdate.Connect(&VtkInterface::OnPlatformListChange, this));
      mProxyCallbacks.Add(
         WatchProxy(mProxyRoot.scenario().GetPath()).ChildUpdate.Connect(&VtkInterface::OnScenarioChange, this));
      mProxyCallbacks.Add(WatchProxy(mProxyRoot.zones().GetPath()).AnyUpdate.Connect(&VtkInterface::OnZoneListChange, this));


      wkf::Viewer* viewerPtr = vaEnv.GetStandardViewer();
      if (viewerPtr)
      {
         if (!viewerPtr->DataExists("CenterOnStart") || viewerPtr->GetData("CenterOnStart").ToBool())
         {
            viewerPtr->CenterCamera(false);
         }
      }

      WsfPM_Scenario scenario = mProxyRoot.scenario();
      UpdateClassificationBannerText(scenario.GetClassificationLabel().ToString(), scenario.GetClassificationColor());
   }
   else
   {
      mProxyCallbacks.Clear();
      ClearData();
   }
}

void wizard::VtkInterface::RequestSelectPlatform(const QString& aPlatformName)
{
   wkf::Scenario* scenario = vaEnv.GetStandardScenario();
   if (scenario)
   {
      wkf::Platform* entityPtr = scenario->FindPlatformByName(aPlatformName);
      if (entityPtr)
      {
         wkf::Viewer* viewer = vaEnv.GetStandardViewer();
         if (viewer)
         {
            viewer->ClearSelection();
         }
         wkfEnv.SetPlatformsSelected({entityPtr});
         mPlatformSelectionRequest.reset();
         return;
      }
   }
   // if we didn't find it, place a request, assuming it is in the process of being created
   mPlatformSelectionRequest = aPlatformName;
}

void wizard::VtkInterface::SetMoveMode(wizard::VtkInterface::MoveMode aMoveMode) noexcept
{
   mMoveMode = aMoveMode;
}

wizard::VtkInterface::MoveMode wizard::VtkInterface::GetMoveMode() const noexcept
{
   return mMoveMode;
}

void wizard::VtkInterface::PlatformAdded(WsfPM_Platform& aPlatform)
{
   unsigned int      id        = AssignId(aPlatform.GetPathString());
   wizard::Platform* entityPtr = dynamic_cast<wizard::Platform*>(vaEnv.GetStandardScenario()->FindPlatformByIndex(id));
   if (!entityPtr)
   {
      entityPtr        = new wizard::Platform(aPlatform.GetName(), id, aPlatform);
      mPlatformMap[id] = entityPtr;
   }
   else
   {
      wkf::Observer::PlatformPropertiesChanged(entityPtr);
   }

   // add any zones that might have been added along with this platform
   auto                       zoneMap = aPlatform.Zone();
   std::vector<WsfPProxyNode> platformZones;
   zoneMap.GetAllValues(platformZones);
   for (const auto& zoneIter : platformZones)
   {
      auto zoneNode = static_cast<WsfPM_Zone>(zoneIter);
      ZoneAdded(zoneNode);
   }

   if (mPlatformSelectionRequest.has_value())
   {
      if (mPlatformSelectionRequest->toStdString() == entityPtr->GetName())
      {
         wkf::Viewer* viewer = vaEnv.GetStandardViewer();
         if (viewer)
         {
            viewer->ClearSelection();
         }
         wkfEnv.SetPlatformsSelected({entityPtr});
         mPlatformSelectionRequest.reset();
      }
   }
}


void wizard::VtkInterface::RemoveZone(WsfPM_Zone& aZone)
{
   std::string zonePlatName = aZone.ParentPlatform();
   auto        zoneName     = aZone.GetName();

   for (auto zoneIter = mZoneSetList.begin(); zoneIter != mZoneSetList.end();)
   {
      auto&       curZone     = *zoneIter;
      std::string curZoneName = curZone->GetZoneSetName();
      std::string curPlatName = curZone->GetParentPlatformName();

      // If the zones match, remove it from mZoneList.
      if (zonePlatName == curPlatName && zoneName == curZoneName)
      {
         curZone->RemoveZoneSet();
         zoneIter = mZoneSetList.erase(zoneIter);
      }
      else
      {
         ++zoneIter;
      }
   }

   // Update all zones attempting to reference this zone
   for (auto& zone : aZone.GetReferencingZones())
   {
      UpdateZone(zone);
   }

   // update all zone sets using this zone
   for (auto& zoneSet : aZone.GetUsingZoneSets())
   {
      UpdateZone(zoneSet);
   }
}

bool wizard::VtkInterface::ZoneExists(WsfPM_Zone& aZone) const
{
   bool foundZone = false;

   auto zoneName = aZone.GetName();

   std::string zonePlatformName = aZone.ParentPlatform();

   for (auto& curZone : mZoneSetList)
   {
      auto curZoneName = curZone->GetZoneSetName();
      auto curPlatName = curZone->GetParentPlatformName();

      if ((curZoneName == zoneName) && ((curPlatName == zonePlatformName) || (curPlatName == zonePlatformName + zoneName)))
      {
         foundZone = true;
         break;
      }
   }
   return foundZone;
}

void wizard::VtkInterface::UpdateZone(WsfPM_Zone& aZone)
{
   auto addZone = aZone;
   RemoveZone(aZone);
   ZoneAdded(addZone);

   // Update all zones attempting to reference this zone
   auto zoneDef = WsfPM_ZoneDefinition(aZone);
   for (auto& zone : zoneDef.GetReferencingZones())
   {
      UpdateZone(zone);
   }
}

void wizard::VtkInterface::ZoneAdded(WsfPM_Zone& aZone)
{
   // If zone already exists, do not add it.
   for (auto& zone : mZoneSetList)
   {
      if (zone->GetZoneSetNode() == aZone)
      {
         return;
      }
   }

   std::string zoneName = aZone.GetName();

   auto zoneSetPtr = ut::make_unique<ZoneSet>(aZone);
   mZoneSetList.push_back(std::move(zoneSetPtr));

   // Update all zones attempting to reference this zone
   for (WsfPM_Zone& zone : aZone.GetReferencingZones())
   {
      UpdateZone(zone);
   }

   // update all zone sets attempting to use this zone
   for (WsfPM_ZoneSet& zoneSet : aZone.GetUsingZoneSets())
   {
      UpdateZone(zoneSet);
   }
}

void wizard::VtkInterface::PlatformRemoved(const WsfPM_Platform& aPlatform)
{
   for (auto zoneIter = mZoneSetList.begin(); zoneIter != mZoneSetList.end();)
   {
      auto& curZone = *zoneIter;
      if (curZone->GetParentPlatformName() == aPlatform.GetName())
      {
         curZone->RemoveZoneSet();
         zoneIter = mZoneSetList.erase(zoneIter);
      }
      else
      {
         ++zoneIter;
      }
   }

   unsigned int index = LookupId(aPlatform.GetPathString());
   if (index != 0)
   {
      mIdToNameMap.erase(index);
      mNameToIdMap.erase(aPlatform.GetName());
      vaEnv.GetStandardScenario()->RemoveEntity(mPlatformMap[index]);
      delete mPlatformMap[index];
      mPlatformMap.erase(index);
   }
}

void wizard::VtkInterface::PlatformUpdated(WsfPM_Platform& aPlatform)
{
   // if a platform with zones has moved, move its zones along with it
   auto                       zoneMap = aPlatform.Zone();
   std::vector<WsfPProxyNode> platformZones;
   zoneMap.GetAllValues(platformZones);
   for (const auto& zoneIter : platformZones)
   {
      auto zoneNode = static_cast<WsfPM_Zone>(zoneIter);
      UpdateZone(zoneNode);
   }

   // look for zone sets that use this platform as a reference platform
   std::vector<WsfPM_Zone> refZones;
   for (auto& curZoneSet : mZoneSetList)
   {
      auto zonePlatformName = curZoneSet->GetParentPlatformName();
      if (zonePlatformName == aPlatform.GetName())
      {
         // this is a zone that resides on this platform and has already been updated
         continue;
      }

      std::string platName = aPlatform.GetName();
      if (curZoneSet->SubzoneReferencesPlatform(platName))
      {
         refZones.emplace_back(curZoneSet->GetZoneSetNode());
      }
   }
   for (auto& curZone : refZones)
   {
      UpdateZone(curZone);
   }
}

void wizard::VtkInterface::OnPlatformListChange(const ProxyChange& aProxyChange)
{
   WsfPM_Root proxyRoot(ProxyWatcher::GetActiveProxy());

   WsfPM_Platform platform    = static_cast<WsfPM_Platform>(aProxyChange.changed());
   WsfPProxyNode  currentNode = aProxyChange.changed();
   auto           types       = currentNode.GetInheritedTypeNames();
   while (types.empty() && currentNode.IsValid())
   {
      currentNode = currentNode.GetParent();
      types       = currentNode.GetInheritedTypeNames();
   }
   if (std::find(types.begin(), types.end(), "ZoneDefinition") != types.end() ||
       std::find(types.begin(), types.end(), "ZoneSet") != types.end())
   {
      OnZoneListChange(aProxyChange);
      return;
   }

   std::vector<std::string> pathStringList =
      aProxyChange.changed().GetPath().ToStringList(aProxyChange.changed().GetRoot());
   // Return early if a non-platform is being updated, or a component is being added as its own platform
   if (pathStringList[0] != "platform" || (aProxyChange.reason() == ProxyChange::cADDED && pathStringList.size() != 2))
   {
      return;
   }

   switch (aProxyChange.reason())
   {
   case ProxyChange::cADDED:
      PlatformAdded(platform);
      UpdateReferencePlatformZones(platform.GetName());
      break;
   case ProxyChange::cREMOVED:

      if (pathStringList.size() >= 2)
      {
         size_t index =
            pathStringList.size() - 2; // - 2 because the path list is in ["type", "name"] format and we want the type
         if (pathStringList[index] == "zone")
         {
            OnZoneListChange(aProxyChange);
         }
         else if (pathStringList[index] == "platform")
         {
            PlatformRemoved(platform);
         }
         UpdateReferencePlatformZones(platform.GetName());
      }
      break;
   case ProxyChange::cRENAMED:
      //      PlatformRenamed(platform, aProxyChange.oldName());
      break;
   case ProxyChange::cUPDATED:
      platform = static_cast<WsfPM_Platform>(proxyRoot.platforms() + aProxyChange.changed().GetPath()[1]);
      PlatformUpdated(platform);
      break;
   default:
      break;
   }
}

void wizard::VtkInterface::OnZoneListChange(const ProxyChange& aProxyChange)
{
   auto       node  = aProxyChange.changed();
   auto       root  = node.Root();
   auto       types = node.GetInheritedTypeNames();
   WsfPM_Zone zone(aProxyChange.changed());
   while (types.empty() && node.IsValid())
   {
      node  = node.GetParent();
      types = node.GetInheritedTypeNames();
      if (node == root)
      {
         return;
      }
   }
   if (std::find(types.begin(), types.end(), "ZoneDefinition") != types.end() ||
       std::find(types.begin(), types.end(), "ZoneSet") != types.end())
   {
      zone = static_cast<WsfPM_Zone>(node);
   }
   switch (aProxyChange.reason())
   {
   case ProxyChange::cADDED:
      if (ZoneExists(zone))
      {
         // zone was found in mZoneList, we are attempting to add a duplicate zone
         // so, we call UpdateZone, rather than ZoneAdded, which will remove the zone,
         // and the re-add the zone with updated configuration
         UpdateZone(zone);
      }
      else
      {
         // zone was not found in mZoneList, so add it
         ZoneAdded(zone);
      }
      break;
   case ProxyChange::cREMOVED:
      RemoveZone(zone);
      break;
   case ProxyChange::cUPDATED:
      UpdateZone(zone);
      break;
   default:
      break;
   }
}

void wizard::VtkInterface::OnScenarioChange(const ProxyChange& aProxyChange)
{
   WsfPM_Scenario scenario(aProxyChange.changed().GetParent());
   UpdateClassificationBannerText(scenario.GetClassificationLabel().ToString(), scenario.GetClassificationColor());
}

unsigned int wizard::VtkInterface::AssignId(const std::string& aPath)
{
   auto retval          = ++mNextId;
   mIdToNameMap[retval] = aPath;
   mNameToIdMap[aPath]  = retval;
   return retval;
}

unsigned int wizard::VtkInterface::LookupId(const std::string& aPath) const
{
   auto it = mNameToIdMap.find(aPath);
   if (it != mNameToIdMap.end())
   {
      return it->second;
   }
   return 0;
}

void wizard::VtkInterface::ClearData()
{
   for (auto&& it : mPlatformMap)
   {
      vaEnv.GetStandardScenario()->RemoveEntity(it.second);
      delete it.second;
   }
   vaEnv.ResetStandardScenario();
   mNextId = 0;
   mNameToIdMap.clear();
   mIdToNameMap.clear();
   mPlatformMap.clear();
   mZoneSetList.clear();
}

void wizard::VtkInterface::DragEnter(QDragEnterEvent* aEvent)
{
   if (aEvent->mimeData()->hasFormat(DragAndDrop::cPROPERTY_SET_MIME_TYPE))
   {
      if (!ProxyWatcher::GetActiveProxy())
         return;
      DragAndDrop::PropertyList properties = DragAndDrop::ReadPropertySetMime(aEvent->mimeData());
      if (properties.empty())
         return;
      if (properties.size() != 1)
         return;
      DragAndDrop::PropertyMap pMap = properties[0];
      if (!pMap.contains("proxy-node"))
         return;
      WsfPProxyNode node = DragAndDrop::ToProxyNode(pMap["proxy-node"].toString());
      WsfPM_Root    proxyRoot(ProxyWatcher::GetActiveProxy());
      if (node.GetParent() == proxyRoot.platformTypes())
      {
         aEvent->accept();
      }
   }
   else if (aEvent->mimeData()->hasUrls())
   {
      QList<QUrl> files = aEvent->mimeData()->urls();
      if (!files.empty())
      {
         QString localFile = files[0].toLocalFile();
         if (!localFile.isEmpty())
         {
            aEvent->accept();
         }
      }
   }
}

void wizard::VtkInterface::DragDrop(QDropEvent* aEvent)
{
   if (aEvent->mimeData()->hasFormat(DragAndDrop::cPROPERTY_SET_MIME_TYPE))
   {
      auto viewerPtr = vaEnv.GetStandardViewer();
      if (nullptr != viewerPtr)
      {
         double lat;
         double lon;
         double alt;
         int    highDpiFactor = QApplication::desktop()->devicePixelRatio();
         viewerPtr->GetCamera()->ConvertScreenToLLA(aEvent->pos().x() * highDpiFactor,
                                                    viewerPtr->GetHeight() - aEvent->pos().y() * highDpiFactor,
                                                    lat,
                                                    lon,
                                                    alt);
         PasteMime(aEvent->mimeData(), lat, lon);
      }
   }
   else if (aEvent->mimeData()->hasUrls())
   {
      QList<QUrl> files = aEvent->mimeData()->urls();
      if (!Project::Instance())
      {
         QStringList fileList;
         for (const QUrl& url : files)
         {
            fileList << url.toLocalFile();
         }
         Environment::GetInstance().LoadScenario(fileList);
      }
      // Load scenario only seems to open one file, so open the others if necessary
      if (Project::Instance())
      {
         for (const auto& fileIt : files)
         {
            QString localFile = fileIt.toLocalFile();
            if (!localFile.isEmpty())
            {
               Project::Instance()->OpenFile(localFile.toStdString());
            }
         }
      }
   }
}

bool wizard::VtkInterface::PasteMime(const QMimeData* aMimeData, double aLat, double aLon)
{
   if (!ProxyWatcher::GetActiveProxy())
      return false;
   DragAndDrop::PropertyList properties = DragAndDrop::ReadPropertySetMime(aMimeData);
   if (properties.empty())
      return false;
   if (properties.size() != 1)
      return false;
   DragAndDrop::PropertyMap pMap = properties[0];
   if (!pMap.contains("proxy-node"))
      return false;
   WsfPProxyNode node = DragAndDrop::ToProxyNode(pMap["proxy-node"].toString());
   WsfPM_Root    proxyRoot(ProxyWatcher::GetActiveProxy());
   if (node.GetParent() == proxyRoot.platformTypes())
   {
      CreatePlatformDialog* dlg = new CreatePlatformDialog(node.GetProxy(), wkfEnv.GetMainWindow());
      dlg->SetPlatformPosition(aLat, aLon);
      dlg->SelectPlatformType(node.GetName());
      dlg->ChooseFileFromType();
      dlg->show();
      return true;
   }
   return false;
}

void wizard::VtkInterface::MoveSelectionAndAttachmentComplete(const wkf::PlatformList&       aPlatformList,
                                                              const wkf::EntityList&         aNonPlatformList,
                                                              const vespa::VaAttachmentList& aAttachmentList)
{
   // many changes to our documents are about to happen.  The proxy informs these changes by finding the nodes of values
   // we are about to change and telling us where we can find them in file.  Unfortunately once we make a change in the
   // file, proxy nodes that point to the same file may be invalidated.  To avoid this, we sort the necessary changes
   // from the bottom of the file to the top, so that early changes never effect the location of later changes.
   class MoveWrapper : public ChangeWrapper
   {
   public:
      // Constructs a MoveWrapper
      // @param aPlatformPtr is the wizard::Platform modified on the Map
      // @param aProxyNode is the proxy property to modify
      MoveWrapper(wizard::Platform* aPlatformPtr, WsfPProxyNode aProxyNode)
         : ChangeWrapper(aPlatformPtr, std::move(aProxyNode))
      {
      }

      //! Constructs a MoveWrapper
      //! @param aPlatformPtr is the wizard::Platform modified on the Map
      //! @param aProxyNode is the proxy property to modify
      //! @param aCommand is the name of the attribute to add to the text
      //! @param aValue is the value of the attribute to add to the text
      MoveWrapper(wizard::Platform* aPlatformPtr, WsfPProxyNode aProxyNode, const QString& aCommand, const QString& aValue)
         : ChangeWrapper(aPlatformPtr, std::move(aProxyNode))
         , mCommand(aCommand)
         , mValue(aValue)
      {
      }

      //! Constructs a MoveWrapper
      //! @param aPlatformPtr is the wizard::Platform containing the attachment
      //! @param aWaypointPtr is the vespa::VaAttachment modified on the Map
      //! @param aProxyNode is the proxy property to modify
      //! @param aCommand is the name of the attribute to add to the text
      //! @param aValue is the value of the attribute to add to the text
      MoveWrapper(wizard::Platform*    aPlatformPtr,
                  vespa::VaAttachment* aAttachmentPtr,
                  WsfPProxyNode        aProxyNode,
                  const QString&       aCommand,
                  const QString&       aValue)
         : ChangeWrapper(aPlatformPtr, aAttachmentPtr, std::move(aProxyNode))
         , mCommand(aCommand)
         , mValue(aValue)
      {
      }

      //! Constructs a MoveWrapper
      //! @param aPlatformPtr is the wizard::Platform containing the zone
      //! @param aZonePointPtr is the wkf::ZonePoint modified on the Map
      //! @param aProxyNode is the proxy property to modify
      //! @param aCommand is the name of the attribute to add to the text
      //! @param aValue is the value of the attribute to add to the text
      MoveWrapper(wizard::Platform* aPlatformPtr,
                  wkf::ZonePoint*   aZonePointPtr,
                  WsfPProxyNode     aProxyNode,
                  const QString&    aCommand,
                  const QString&    aValue)
         : ChangeWrapper(aPlatformPtr, aZonePointPtr, std::move(aProxyNode))
         , mCommand(aCommand)
         , mValue(aValue)
      {
      }

      //! @param aPointOfInterestPtr is the wkf::PointOfInterest modified on the Map
      //! @param aProxyNode is the proxy property to modify
      //! @param aCommand is the name of the attribute to add to the text
      //! @param aValue is the value of the attribute to add to the text
      MoveWrapper(wkf::PointOfInterest* aPointOfInterestPtr,
                  WsfPProxyNode         aProxyNode,
                  const QString&        aCommand,
                  const QString&        aValue)
         : ChangeWrapper(aPointOfInterestPtr, std::move(aProxyNode))
         , mCommand(aCommand)
         , mValue(aValue)
      {
      }

      // @warning DO NOT REMOVE UNTIL AFSIM-1050 IS RESOLVED!
      using ChangeWrapper::ChangeWrapper;

      // Wraps the zone definition's attributes
      // @param aMoves is the return-value-parameter referencing the comprehensive set of moves to perform
      // @param aPlatformPtr is the platform on which the zone is "attached" (a valid platform if a local zone; nullptr
      // if a global zone)
      // @param aProxyZone is the proxy zone object referencing the text to modify
      // @param aVtkZone is the visual zone object modified on the Map
      // @note "implementation detail": zones have separate anchor entities than the platforms to which they are attached
      static void WrapZoneDefinition(std::multiset<MoveWrapper>& aMoves,
                                     wizard::Platform*           aPlatformPtr,
                                     WsfPM_ZoneDefinition        aProxyZone,
                                     wkf::AttachmentZone*        aVtkZone)
      {
         // Determine the type of zone definition
         switch (aProxyZone.GetDefinitionType())
         {
         case WsfPM_ZoneDefinition::cPOLYGONAL:
         {
            auto vtkZonePolygon = static_cast<wkf::AttachmentZonePolygon*>(aVtkZone);
            // Cast to wkf::AttachmentZonePolygon to get access to the wkf::VaZonePoints
            if (vtkZonePolygon)
            {
               // Get the WsfPM_ZonePoint list
               WsfPM_ZonePointList proxyPoints{aProxyZone.PointList()};
               // Get the wkf::ZonePoint point list
               std::list<wkf::ZonePoint*> vtkPoints{vtkZonePolygon->GetZonePoints()};
               // Handle the relative zone position and heading attributes
               if (!(aProxyZone.UseLatLon() || aProxyZone.UseMGRS()))
               {
                  // Wrap the zone's position node
                  WsfPProxyNode proxyPosition{aProxyZone.PositionNode()};
                  if (proxyPosition && proxyPosition.GetLastDocumentRange().Valid())
                  {
                     aMoves.emplace(aPlatformPtr, aVtkZone, std::move(proxyPosition));
                  }
                  // There is no position defined on the zone, create a special MoveWrapper to add it
                  else
                  {
                     // Create a special MoveWrapper that will add the "position" attribute to the zone
                     const vespa::VaPosition& pos{aVtkZone->GetParent().GetPosition()};
                     aMoves.emplace(aPlatformPtr,
                                    aVtkZone,
                                    std::move(aProxyZone),
                                    "position",
                                    wizard::EditorHelpers::GetPositionString(pos));
                  }
                  auto moveMode = wizEnv.GetVtkMoveMode();
                  if (moveMode == MoveMode::RotateScenario || moveMode == MoveMode::RotateSelection)
                  {
                     // Wrap the zone's heading node
                     WsfPProxyNode proxyHeading{aProxyZone.HeadingNode()};
                     if (proxyHeading && proxyHeading.GetLastDocumentRange().Valid())
                     {
                        aMoves.emplace(aPlatformPtr, aVtkZone, std::move(proxyHeading));
                     }
                     // There is no heading defined on the zone, create a special MoveWrapper to add it
                     else
                     {
                        // Create a special MoveWrapper that will add the "position" attribute to the zone
                        UtAngleValue heading{aVtkZone->GetParent().GetHeading()};
                        aMoves.emplace(aPlatformPtr,
                                       aVtkZone,
                                       std::move(aProxyZone),
                                       "heading",
                                       wizard::EditorHelpers::GetUnitaryString<wkf::ValueType::cANGLE>(heading));
                     }
                  }
               }
               // Wrap the zone points
               auto vtkPointIter = vtkPoints.begin();
               // Loop through vtkPoints and proxyPoints in parallel
               for (size_t index = 0; index < proxyPoints.size() && vtkPointIter != vtkPoints.end(); ++index, ++vtkPointIter)
               {
                  WsfPM_ZonePoint proxyPoint{proxyPoints[index]};
                  wkf::ZonePoint* vtkPoint{*vtkPointIter};
                  if (proxyPoint && vtkPoint)
                  {
                     aMoves.emplace(aPlatformPtr, vtkPoint, std::move(proxyPoint));
                  }
               }
            }
            break;
         }
         case WsfPM_ZoneDefinition::cCIRCULAR:
         case WsfPM_ZoneDefinition::cELLIPTICAL:
         case WsfPM_ZoneDefinition::cSPHERICAL:
         case WsfPM_ZoneDefinition::cTYPE_NOT_SET:
         {
            // Wrap the zone's position node
            WsfPProxyNode proxyPosition{aProxyZone.PositionNode()};
            if (proxyPosition && proxyPosition.GetLastDocumentRange().Valid())
            {
               aMoves.emplace(aPlatformPtr, aVtkZone, std::move(proxyPosition));
            }
            // There is no position defined on the zone, create a special MoveWrapper to add it
            else
            {
               // Create a special MoveWrapper that will add the "position" attribute to the zone
               const vespa::VaPosition& pos{aVtkZone->GetParent().GetPosition()};
               aMoves.emplace(aPlatformPtr,
                              aVtkZone,
                              std::move(aProxyZone),
                              "position",
                              wizard::EditorHelpers::GetPositionString(pos));
            }
            auto moveMode = wizEnv.GetVtkMoveMode();
            if (moveMode == MoveMode::RotateScenario || moveMode == MoveMode::RotateSelection)
            {
               // Wrap the zone's heading node
               WsfPProxyNode proxyHeading{aProxyZone.HeadingNode()};
               if (proxyHeading && proxyHeading.GetLastDocumentRange().Valid())
               {
                  aMoves.emplace(aPlatformPtr, aVtkZone, std::move(proxyHeading));
               }
               // There is no heading defined on the zone, create a special MoveWrapper to add it
               else
               {
                  // Create a special MoveWrapper that will add the "heading" attribute to the zone
                  UtAngleValue heading{aVtkZone->GetParent().GetHeading()};
                  aMoves.emplace(aPlatformPtr,
                                 aVtkZone,
                                 std::move(aProxyZone),
                                 "heading",
                                 wizard::EditorHelpers::GetUnitaryString<wkf::ValueType::cANGLE>(heading));
               }
            }
            break;
         }
         default:
            break;
         }
      }

      void Move() const
      {
         // Handle platform or local attachment property
         if (GetPlatform())
         {
            // Edit local route waypoint
            if (GetWaypoint() && GetProxyNode().GetAttrName() == "position")
            {
               EditWaypointPosition();
            }
            // Handle local zone
            else if (GetZone())
            {
               // Handle local, relative zone position
               if (GetProxyNode().GetAttrName() == "position")
               {
                  EditZonePosition();
               }
               // Handle local, relative zone heading
               else if (GetProxyNode().GetAttrName() == "heading")
               {
                  EditZoneHeading();
               }
               // Handle non-existent zone attribute
               else
               {
                  wizard::EditorHelpers::AddNewAttributeInText(GetProxyNode(), mCommand, mValue);
               }
            }
            // Handle local zone point position
            else if (GetZonePoint() && GetProxyNode().IsA_StructTypeOf("ZonePoint"))
            {
               EditZonePointPosition();
            }
            // Handle platform position
            else if (GetProxyNode().GetAttrName() == "position")
            {
               EditPlatformPosition();
            }
            // Handle platform heading
            else if (GetProxyNode().GetAttrName() == "heading")
            {
               EditPlatformHeading();
            }
            // Handle non-existent attribute
            else
            {
               wizard::EditorHelpers::AddNewAttributeInText(GetProxyNode(), mCommand, mValue);
            }
         }
         // Edit global route waypoint
         else if (GetWaypoint() && GetProxyNode().GetAttrName() == "position")
         {
            EditWaypointPosition();
         }
         // Handle global zone
         else if (GetZone())
         {
            // Handle global, relative zone position
            if (GetProxyNode().GetAttrName() == "position")
            {
               EditZonePosition();
            }
            // Handle global, relative zone heading
            else if (GetProxyNode().GetAttrName() == "heading")
            {
               EditZoneHeading();
            }
            // Handle non-existent zone attribute
            else
            {
               wizard::EditorHelpers::AddNewAttributeInText(GetProxyNode(), mCommand, mValue);
            }
         }
         // Handle global zone point position
         else if (GetZonePoint() && GetProxyNode().IsA_StructTypeOf("ZonePoint"))
         {
            EditZonePointPosition();
         }
         // Handle point-of-interest
         else if (GetPointOfInterest() && GetProxyNode().GetAttrName() == "position")
         {
            EditPointOfInterestPosition();
         }
         // Handle non-existent attribute
         else
         {
            wizard::EditorHelpers::AddNewAttributeInText(GetProxyNode(), mCommand, mValue);
         }
      }

   private:
      // Edit the platform position
      void EditPlatformPosition() const noexcept
      {
         GetPlatform()->Unwrap();
         QTextCursor positionCursor{GetEditor()->textCursor()};
         positionCursor.setPosition(GetRange().GetBegin());
         positionCursor.setPosition(GetRange().GetEnd() + 1, QTextCursor::KeepAnchor);
         QString     positionText{positionCursor.selection().toPlainText()};
         QStringList positionSplit{Editor::Split(positionText)};
         bool        edited{false};
         if (!positionSplit.isEmpty())
         {
            // Get the position from the platform
            vespa::VaPosition pos{GetPlatform()->GetPosition()};

            // Read the command "position"
            int commandIndex{positionSplit.indexOf("position")};
            if (commandIndex >= 0)
            {
               edited = wizard::EditorHelpers::ReplacePositionText(positionSplit, commandIndex, pos);
            }
            // Read the command "mgrs_coordinate"
            commandIndex = positionSplit.indexOf("mgrs_coordinate");
            if (!edited && commandIndex >= 0)
            {
               edited = wizard::EditorHelpers::ReplaceMGRS_Text(positionSplit, commandIndex, pos);
            }
         }
         // Replace the old position text with the new position text
         if (edited)
         {
            wizard::EditorHelpers::InsertTextAndResetCursor(GetEditor(), positionCursor, positionSplit.join(""));
         }
      }

      // Edit the platform position
      void EditPlatformHeading() const noexcept
      {
         // Go to the location of the heading
         QTextCursor headingCursor{GetEditor()->textCursor()};
         headingCursor.setPosition(GetRange().GetBegin());
         headingCursor.setPosition(GetRange().GetEnd() + 1, QTextCursor::KeepAnchor);
         // Extract the heading text
         QString     headingText{headingCursor.selection().toPlainText()};
         QStringList headingSplit{Editor::Split(headingText)};
         bool        edited{false};
         if (!headingSplit.isEmpty())
         {
            // Read the command "heading"
            int commandIndex{headingSplit.indexOf("heading")};
            if (commandIndex >= 0)
            {
               UtAngleValue heading{GetPlatform()->GetHeading()};
               edited = wizard::EditorHelpers::ReplaceUnitaryText(headingSplit, commandIndex, heading);
            }
         }
         // Replace the old heading text with the new heading text
         if (edited)
         {
            wizard::EditorHelpers::InsertTextAndResetCursor(GetEditor(), headingCursor, headingSplit.join(""));
         }
      }

      // Edit the point-of-interest position
      void EditPointOfInterestPosition() const noexcept
      {
         QTextCursor positionCursor{GetEditor()->textCursor()};
         positionCursor.setPosition(GetRange().GetBegin());
         positionCursor.setPosition(GetRange().GetEnd() + 1, QTextCursor::KeepAnchor);
         QString     positionText{positionCursor.selection().toPlainText()};
         QStringList positionSplit{Editor::Split(positionText)};
         bool        edited{false};
         if (!positionSplit.isEmpty())
         {
            // Get the position from the point-of-interest
            vespa::VaPosition pos{GetPointOfInterest()->GetPosition()};

            // Read the command "position"
            int commandIndex{positionSplit.indexOf("position")};
            if (commandIndex >= 0)
            {
               edited = wizard::EditorHelpers::ReplacePositionText(positionSplit, commandIndex, pos);
            }
         }
         // Replace the old position text with the new position text
         if (edited)
         {
            wizard::EditorHelpers::InsertTextAndResetCursor(GetEditor(), positionCursor, positionSplit.join(""));
         }
      }

      // Routine to edit a relative zone's position
      void EditZonePosition() const noexcept
      {
         // Go to the location of the position
         QTextCursor positionCursor{GetEditor()->textCursor()};
         positionCursor.setPosition(GetRange().GetBegin());
         positionCursor.setPosition(GetRange().GetEnd() + 1, QTextCursor::KeepAnchor);
         // Extract the position text
         QString     positionText{positionCursor.selection().toPlainText()};
         QStringList positionSplit{Editor::Split(positionText)};
         bool        edited{false};
         if (!positionSplit.isEmpty())
         {
            // Get the entity position
            const vespa::VaPosition& pos{GetZone()->GetParent().GetPosition()};

            // Read the command "position"
            int commandIndex{positionSplit.indexOf("position")};
            if (commandIndex >= 0)
            {
               edited = wizard::EditorHelpers::ReplacePositionText(positionSplit, commandIndex, pos);
            }
         }
         // Replace the old position text with the new position text
         if (edited)
         {
            wizard::EditorHelpers::InsertTextAndResetCursor(GetEditor(), positionCursor, positionSplit.join(""));
         }
      }

      // Edit the waypoint position
      void EditWaypointPosition() const noexcept
      {
         QTextCursor positionCursor{GetEditor()->textCursor()};
         positionCursor.setPosition(GetRange().GetBegin());
         positionCursor.setPosition(GetRange().GetEnd() + 1, QTextCursor::KeepAnchor);
         // Extract the position text
         QString     positionText{positionCursor.selection().toPlainText()};
         QStringList positionSplit{Editor::Split(positionText)};
         bool        edited{false};
         if (!positionSplit.isEmpty())
         {
            // Get the position from the waypoint
            vespa::VaPosition pos{*GetWaypoint()};

            // Read the command "position"
            int commandIndex{positionSplit.indexOf("position")};
            if (commandIndex >= 0)
            {
               edited = wizard::EditorHelpers::ReplacePositionText(positionSplit, commandIndex, pos);
            }
            // TODO:  add support for MGRS, polar, and offset zone points (see AFSIM-1130 and AFSIM-1157)
         }
         // Replace the old position text with the new position text
         if (edited)
         {
            wizard::EditorHelpers::InsertTextAndResetCursor(GetEditor(), positionCursor, positionSplit.join(""));
         }
      }

      // Routine to edit a relative zone's heading
      void EditZoneHeading() const noexcept
      {
         // Go to the location of the heading
         QTextCursor headingCursor{GetEditor()->textCursor()};
         headingCursor.setPosition(GetRange().GetBegin());
         headingCursor.setPosition(GetRange().GetEnd() + 1, QTextCursor::KeepAnchor);
         // Extract the heading text
         QString     headingText{headingCursor.selection().toPlainText()};
         QStringList headingSplit{Editor::Split(headingText)};
         bool        edited{false};
         if (!headingSplit.isEmpty())
         {
            // Read the command "heading"
            int commandIndex{headingSplit.indexOf("heading")};
            if (commandIndex >= 0)
            {
               // Get the entity heading
               UtAngleValue heading{GetZone()->GetParent().GetHeading()};
               edited = wizard::EditorHelpers::ReplaceUnitaryText(headingSplit, commandIndex, heading);
            }
         }
         // Replace the old heading text with the new heading text
         if (edited)
         {
            wizard::EditorHelpers::InsertTextAndResetCursor(GetEditor(), headingCursor, headingSplit.join(""));
         }
      }

      // Routine to edit a zone point's position
      void EditZonePointPosition() const noexcept
      {
         auto getProxyZone = [this]()
         { return static_cast<WsfPM_ZoneDefinition>(GetProxyNode().GetParent().GetParent()); };
         QTextCursor pointCursor{GetEditor()->textCursor()};
         pointCursor.setPosition(GetRange().GetBegin());
         pointCursor.setPosition(GetRange().GetEnd() + 1, QTextCursor::KeepAnchor);
         QString     pointText{pointCursor.selection().toPlainText()};
         QStringList pointSplit{Editor::Split(pointText)};
         bool        edited{false};
         if (!pointSplit.isEmpty())
         {
            // Get the zone point position
            const vespa::VaPosition& pos{GetZonePoint()->GetPosition()};

            // Read the command "point"
            int commandIndex{pointSplit.indexOf("point")};
            if (commandIndex >= 0)
            {
               if (getProxyZone().UseLatLon())
               {
                  edited = wizard::EditorHelpers::ReplacePositionText(pointSplit, commandIndex, pos);
               }
               // TODO:  add support for MGRS, polar, and offset zone points (see AFSIM-1130 and AFSIM-1131)
            }
         }
         // Replace the old position text with the new position text
         if (edited)
         {
            wizard::EditorHelpers::InsertTextAndResetCursor(GetEditor(), pointCursor, pointSplit.join(""));
         }
      }

      // @name Command and Value strings
      // @note These only need to be used when the attribute does not exist in the text
      //{
      QString mCommand;
      QString mValue;
      //}
   };

   std::multiset<MoveWrapper> sortedMoves;

   // Iterate through the aPlatformList
   for (auto* plat : aPlatformList)
   {
      auto wizPlat = dynamic_cast<wizard::Platform*>(plat);
      // The wkf::Platform is actually a wizard::Platform
      if (wizPlat)
      {
         WsfPM_Platform proxyPlatform{wizPlat->GetPlatform()};
         if (proxyPlatform)
         {
            if (wizPlat->PlatformHasRoute())
            {
               WsfPM_Route proxyRoute{wizPlat->GetProxyRoute()};
               // Get all the routes on the platform
               auto wkfRoutes = wizPlat->FindAttachmentsOfType<wkf::AttachmentRoute>();
               for (auto* route : wkfRoutes)
               {
                  // Make sure the route is in the selected attachments
                  bool found{std::find(aAttachmentList.begin(), aAttachmentList.end(), route) != aAttachmentList.end()};
                  if (proxyRoute && route)
                  {
                     wkf::Waypoint* firstWkfWaypoint{route->GetWaypoint(0)};
                     WsfPM_Waypoint firstProxyWaypoint{proxyRoute.GetWaypoint(0)};
                     if (firstWkfWaypoint && firstProxyWaypoint)
                     {
                        // Sync the platform and the waypoint position
                        // The first waypoint moved, so sync the platform position with the first waypoint position
                        if (found && firstWkfWaypoint->IsSelected())
                        {
                           wizPlat->SetPosition(*firstWkfWaypoint);
                        }
                        // The platform moved, so sync the first waypoint position with the platform position
                        else
                        {
                           vespa::VaPosition platformPosition{wizPlat->GetPosition()};
                           route->UpdateWaypoint(0,
                                                 platformPosition.GetLat(),
                                                 platformPosition.GetLon(),
                                                 firstWkfWaypoint->GetAlt(),
                                                 firstWkfWaypoint->GetAlt(),
                                                 firstWkfWaypoint->GetLabel());
                        }

                        // Wrap the platform's position node
                        auto proxyPosition = firstProxyWaypoint.PositionNode();
                        if (proxyPosition && proxyPosition.GetSourceDocumentRange().Valid())
                        {
                           wizPlat->Wrap();
                           sortedMoves.emplace(wizPlat, std::move(proxyPosition));
                        }
                        // There is no position defined on the platform, create a special MoveWrapper to add it
                        else
                        {
                           // Create a special MoveWrapper that will add the "position" attribute to the platform
                           sortedMoves.emplace(wizPlat,
                                               proxyPlatform,
                                               "position",
                                               wizard::EditorHelpers::GetPositionString(wizPlat->GetPosition()));
                        }
                     }
                  }
               }
            }
            else
            {
               // Wrap the platform's position node
               auto proxyPosition = proxyPlatform.PositionNode();
               if (proxyPosition && proxyPosition.GetSourceDocumentRange().Valid())
               {
                  sortedMoves.emplace(wizPlat, std::move(proxyPosition));
               }
               // There is no position defined on the platform, create a special MoveWrapper to add it
               else
               {
                  // Create a special MoveWrapper that will add the "position" attribute to the platform
                  sortedMoves.emplace(wizPlat,
                                      proxyPlatform,
                                      "position",
                                      wizard::EditorHelpers::GetPositionString(wizPlat->GetPosition()));
               }
            }
            if (mMoveMode == MoveMode::RotateScenario || mMoveMode == MoveMode::RotateSelection)
            {
               // Wrap the platform's heading node
               auto proxyHeading = proxyPlatform.HeadingNode();
               if (proxyHeading && proxyHeading.GetSourceDocumentRange().Valid())
               {
                  sortedMoves.emplace(wizPlat, std::move(proxyHeading));
               }
               // There is no heading defined on the platform, create a special MoveWrapper to add it
               else
               {
                  // Create a special MoveWrapper that will add the "heading" attribute to the platform
                  UtAngleValue heading{wizPlat->GetHeading()};
                  sortedMoves.emplace(wizPlat,
                                      proxyPlatform,
                                      "heading",
                                      wizard::EditorHelpers::GetUnitaryString<wkf::ValueType::cANGLE>(heading));
               }
            }
         }
      }
   }

   for (auto&& poiIt : aNonPlatformList)
   {
      auto poi = dynamic_cast<wkf::PointOfInterest*>(poiIt);
      // The vespa::VaEntity is actually a wkf::PointOfInterest
      if (poi)
      {
         WsfPM_Root proxyRoot{ProxyWatcher::GetActiveProxy()};
         if (proxyRoot)
         {
            WsfPProxyNode proxyPoi{proxyRoot + "pois" + poi->GetName()};
            if (proxyPoi)
            {
               // Wrap the point-of-interest's position node
               WsfPProxyNode proxyPosition{proxyPoi + "position"};
               if (proxyPosition && proxyPosition.GetSourceDocumentRange().Valid())
               {
                  sortedMoves.emplace(poi, proxyPosition);
               }
               // There is no position defined on the point-of-interest, create a special MoveWrapper to add it
               else
               {
                  // Create a special MoveWrapper that will add the "position" attribute to the point-of-interest
                  sortedMoves.emplace(poi, proxyPoi, "position", wizard::EditorHelpers::GetPositionString(poi->GetPosition()));
               }
            }
         }
      }
   }

   std::unordered_set<std::string> processedZones;

   // Iterate through the aAttachmentList
   for (auto&& att : aAttachmentList)
   {
      auto wkfRoute = dynamic_cast<wkf::AttachmentRoute*>(att);
      // The vespa::VaAttachment is actually a wkf::AttachmentRoute
      if (wkfRoute)
      {
         // Get the parent entity
         vespa::VaEntity* curEntity = &wkfRoute->GetParent();
         auto*            wizPlat   = dynamic_cast<wizard::Platform*>(curEntity);
         WsfPM_Route      proxyRoute;
         size_t           waypointIndex{0};
         // Check to see if this is a global route
         // If the parent is actually a vespa::VaEntity suffixed with "_anchor", then it is a global route.
         if (wkfRoute->IsGlobalRoute())
         {
            WsfPM_Root proxyRoot{ProxyWatcher::GetActiveProxy()};
            if (proxyRoot)
            {
               // Get the proxy route from the global route map
               proxyRoute = proxyRoot.routes().FindT(wkfRoute->GetName());
            }
         }
         // Check to see if this is a local route
         // If the parent is actually a wizard::Platform, then it is a local route.
         else if (wizPlat && wizPlat->PlatformHasRoute())
         {
            auto wkfRoutes = wizPlat->FindAttachmentsOfType<wkf::AttachmentRoute>();
            // Make sure the selected route is on the platform
            bool found{std::find(wkfRoutes.begin(), wkfRoutes.end(), wkfRoute) != wkfRoutes.end()};
            if (found)
            {
               // Get the proxy route from the platform
               proxyRoute = wizPlat->GetProxyRoute();
               if (wizPlat->IsWrapped())
               {
                  // Avoid the first waypoint if the platform is wrapped
                  // This avoids text corruption due to a double-edit
                  waypointIndex = 1;
               }
            }
         }
         // Wrap all of the selected waypoints
         if (proxyRoute)
         {
            size_t waypointCount{proxyRoute.WaypointCount()};
            for (; waypointIndex < waypointCount; ++waypointIndex)
            {
               wkf::Waypoint* wkfWaypoint{wkfRoute->GetWaypoint(ut::safe_cast<unsigned int>(waypointIndex))};
               WsfPM_Waypoint proxyWaypoint{proxyRoute.GetWaypoint(waypointIndex)};
               if (wkfWaypoint && proxyWaypoint)
               {
                  WsfPProxyNode proxyPosition{proxyWaypoint.PositionNode()};
                  if (proxyPosition && proxyPosition.GetSourceDocumentRange().Valid())
                  {
                     // Wrap the waypoint's position node
                     sortedMoves.emplace(wizPlat, wkfWaypoint, std::move(proxyPosition));
                  }
                  // TODO: Handle turn and offset waypoints (see AFSIM-1130 and AFSIM-1157).
               }
            }
         }
      }
      auto vtkZone = dynamic_cast<wkf::AttachmentZone*>(att);
      // The vespa::VaAttachment is actually a wkf::AttachmentZone
      if (vtkZone)
      {
         // Find the wizard::Zone in this VtkInterface
         wizard::Zone* wizZone{nullptr};
         for (const auto& zoneSetPtr : mZoneSetList)
         {
            wizZone = zoneSetPtr->GetZoneByAttachment(vtkZone);
            if (wizZone != nullptr)
            {
               break;
            }
         }

         if (wizZone)
         {
            // zones are sometimes used in multiple places (e.g., included onto zone sets with `use_zone`)
            // this prevents duplicate changes from being made to zone definitions
            std::string zoneIdentifier = wizZone->GetDefinitionParentPlatformName() + "_" + wizZone->GetDefinitionName();
            if (processedZones.count(zoneIdentifier) == 0)
            {
               processedZones.insert(zoneIdentifier);

               // Find the platform from the wkf::VtkEnvironment
               auto* zonePlatform = vaEnv.FindEntity<wizard::Platform>(wizZone->GetParentPlatformName());

               // Wrap the zone definition
               MoveWrapper::WrapZoneDefinition(sortedMoves, zonePlatform, wizZone->GetZoneProxyNode(), vtkZone);
            }
         }
      }
   }

   // Disable dragging while the positions are being updated.
   vaEnv.EnablePlatformsDraggable(false);

   wizEnv.BeginUndoCapture();
   // enact all the changes
   for (auto& it : sortedMoves)
   {
      it.Move();
   }
   wizEnv.EndUndoCapture();

   vaEnv.EnablePlatformsDraggable(true);
}

void wizard::VtkInterface::UpdateClassificationBannerText(const std::string& aText, const UtColor& aColor)
{
   auto* generalPrefs = wkfEnv.GetPreferenceObject<wkf::GeneralPrefObject>();
   if (generalPrefs)
   {
      generalPrefs->SetOverlayBannerText(aText, aColor);
   }
}

void wizard::VtkInterface::StandardWidgetChangedCB(vespa::VaWidget* aWidgetPtr)
{
   if (nullptr != aWidgetPtr)
   {
      connect(aWidgetPtr, &vespa::VaWidget::DragEnter, this, &VtkInterface::DragEnter);
      connect(aWidgetPtr, &vespa::VaWidget::DragDrop, this, &VtkInterface::DragDrop);
   }
}

void wizard::VtkInterface::ActionsForNodeRequested(const WsfPProxyNode& aNode, QMenu* aMenuPtr, QWidget* aParentWidgetPtr)
{
   if (aNode.IsOfType("Platform"))
   {
      WsfPM_Platform plat(aNode);
      std::string    name = "platform." + plat.GetName();
      auto           it   = mNameToIdMap.find(name);
      if (it != mNameToIdMap.end())
      {
         auto jt = mPlatformMap.find(it->second);
         if (jt != mPlatformMap.end())
         {
            if (jt->second)
            {
               QMenu* subMenu = aMenuPtr->addMenu("Map");
               wkfEnv.BuildEntityContextMenu(subMenu, jt->second);
               subMenu->setIcon(QIcon::fromTheme("globe"));
            }
         }
      }
   }
}

void wizard::VtkInterface::UpdateReferencePlatformZones(const std::string& aPlatformName)
{
   std::vector<WsfPM_Zone> updateZones;

   auto getSetZonesLambda = [&, aPlatformName](WsfPM_Zone aZone)
   {
      // loop through the zones
      auto zoneSet                = static_cast<WsfPM_ZoneSet>(aZone);
      auto embeddedInclusionZones = zoneSet.EmbeddedInclusionZones();
      auto embeddedExclusionZones = zoneSet.EmbeddedExclusionZones();

      for (size_t i = 0; i < embeddedInclusionZones.size(); ++i)
      {
         auto embeddedInclusionZoneNode = static_cast<WsfPM_Zone>(embeddedInclusionZones[i]);
         if (embeddedInclusionZoneNode.IsDefinition())
         {
            auto zoneDef = static_cast<WsfPM_ZoneDefinition>(embeddedInclusionZoneNode);
            if (zoneDef.ReferencePlatform() == aPlatformName)
            {
               updateZones.emplace_back(aZone + "embeddedInclusionZones");
               break;
            }
         }
      }
      for (size_t i = 0; i < embeddedExclusionZones.size(); ++i)
      {
         auto embeddedExclusionZoneNode = static_cast<WsfPM_Zone>(embeddedExclusionZones[i]);
         if (embeddedExclusionZoneNode.IsDefinition())
         {
            auto zoneDef = static_cast<WsfPM_ZoneDefinition>(embeddedExclusionZoneNode);
            if (zoneDef.ReferencePlatform() == aPlatformName)
            {
               updateZones.emplace_back(aZone + "embeddedExclusionZones");
               break;
            }
         }
      }
   };

   auto pmap = mProxyRoot.platforms();
   for (auto&& it = pmap.begin(); it != pmap.end(); ++it)
   {
      auto curPlatform = pmap.AtT(it);
      if (curPlatform.GetName() == aPlatformName)
      {
         continue;
      }

      auto                       zoneMap = curPlatform.Zone();
      std::vector<WsfPProxyNode> platformZones;
      zoneMap.GetAllValues(platformZones);
      for (const auto& zoneIter : platformZones)
      {
         auto zoneNode = static_cast<WsfPM_Zone>(zoneIter);
         if (zoneNode.IsDefinition())
         {
            auto zoneDef = static_cast<WsfPM_ZoneDefinition>(zoneNode);
            if (zoneDef.ReferencePlatform() == aPlatformName)
            {
               updateZones.emplace_back(zoneNode);
            }
         }
         else if (zoneNode.IsZoneSet())
         {
            getSetZonesLambda(zoneNode);
         }
      }
   }

   // Now go through global zones
   auto                       zoneMap = mProxyRoot.zones();
   std::vector<WsfPProxyNode> globalZones;
   zoneMap.GetAllValues(globalZones);
   for (const auto& zoneIter : globalZones)
   {
      auto zoneNode = static_cast<WsfPM_Zone>(zoneIter);
      if (zoneNode.IsDefinition())
      {
         auto zoneDef = static_cast<WsfPM_ZoneDefinition>(zoneNode);
         if (zoneDef.ReferencePlatform() == aPlatformName)
         {
            updateZones.emplace_back(zoneNode);
         }
      }
      else if (zoneNode.IsZoneSet())
      {
         getSetZonesLambda(zoneNode);
      }
   }

   // update all the zones
   for (auto& curZone : updateZones)
   {
      WsfPProxyNode modifiedNode(mProxyRoot.GetProxy(), curZone.GetPath());
      wizard::ProxyWatcher::Instance()->HandleProxyModified(modifiedNode);
   }
}
