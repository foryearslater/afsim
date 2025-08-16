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

#include "ZoneEditorPlugin.hpp"

#include <QMenu>

#include "FindReferencesResultDialog.hpp"
#include "ParseResults.hpp"
#include "ProjectWorkspace.hpp"
#include "ProxyMerge.hpp"
#include "ProxyUtil.hpp"
#include "ProxyWatcher.hpp"
#include "UtEllipsoidalEarth.hpp"
#include "UtMath.hpp"
#include "UtMemory.hpp"
#include "UtQtMemory.hpp"
#include "VtkInterface.hpp"
#include "WkfAction.hpp"
#include "WkfMainWindow.hpp"
#include "WkfVtkEnvironment.hpp"
#include "WsfExe.hpp"
#include "zone/WkfAttachmentZoneSphere.hpp"

namespace
{
std::string GetPlatformName(const WsfPM_Zone& aZone)
{
   std::string platformName = "";
   if (aZone.GetPath().size() > 2)
   {
      platformName = aZone.GetParent().GetParent().GetName();
   }
   if (platformName.empty())
   {
      platformName = "Global Zones";
   }
   return platformName;
}
} // namespace

WKF_PLUGIN_DEFINE_SYMBOLS(ZoneEditor::Plugin, "Zone Editor", "Provides an interface for creating/editing zones.", "wizard");

ZoneEditor::Plugin::Plugin(const QString& aName, const size_t aUniqueId)
   : wizard::Plugin(aName, aUniqueId)
   , mDialogPtr(nullptr)
   , mPrefWidgetPtr(ut::qt::make_qt_ptr<wkf::ZoneBrowserPrefWidget>())
{
   mDockWidgetPtr = ut::qt::make_qt_ptr<DockWidget>(mDataContainer, wkfEnv.GetMainWindow());
   wkfEnv.GetMainWindow()->addDockWidget(Qt::RightDockWidgetArea, mDockWidgetPtr);
   mDockWidgetPtr->hide();

   connect(wizard::ProjectWorkspace::Instance(),
           &wizard::ProjectWorkspace::ProjectClosed,
           [&](wizard::Project* aProjectPtr)
           {
              mDataContainer.ClearData();
              mDockWidgetPtr->ClearZoneProxies();
           });

   // The Simulation Manager plugin will trigger this signal whenever the selected executable
   // changes in Preferences. This also removes all entities from the Map Display, so we should
   // clear out the data container; the zones will be added back when the proxy becomes available again.
   connect(wizSignals, &wizard::Signals::WsfExeChanged, [&](wizard::WsfExe* aExePtr) { mDataContainer.ClearData(); });

   connect(wizSignals,
           &wizard::Signals::ProjectStartupFilesChanged,
           [&](std::vector<UtPath>)
           {
              mDataContainer.ClearData();
              mDockWidgetPtr->ClearZoneProxies();
           });

   // Overwrite existing colors when preferences change.
   connect(mPrefWidgetPtr->GetPreferenceObject(),
           &wkf::ZoneBrowserPrefObject::ColorChoiceChanged,
           [&](wkf::ColorOption aColorChoice)
           {
              if (wizard::ProxyWatcher::GetActiveProxy() == nullptr)
              {
                 if (mDialogPtr != nullptr)
                 {
                    if (mDialogPtr->isVisible())
                    {
                       mDialogPtr->close();
                    }
                 }
              }
              else
              {
                 ApplyColors(aColorChoice);
              }
           });
   connect(mPrefWidgetPtr->GetPreferenceObject(),
           &wkf::ZoneBrowserPrefObject::DefaultLineColorChanged,
           [&](const QColor& aColor)
           {
              if (wizard::ProxyWatcher::GetActiveProxy() == nullptr)
              {
                 if (mDialogPtr != nullptr)
                 {
                    if (mDialogPtr->isVisible())
                    {
                       mDialogPtr->close();
                    }
                 }
              }
              else
              {
                 ApplyColors(mPrefWidgetPtr->GetPreferenceObject()->GetColorChoice());
              }
           });
   connect(mPrefWidgetPtr->GetPreferenceObject(),
           &wkf::ZoneBrowserPrefObject::DefaultFillColorChanged,
           [&](const QColor& aColor)
           {
              if (wizard::ProxyWatcher::GetActiveProxy() == nullptr)
              {
                 if (mDialogPtr != nullptr)
                 {
                    if (mDialogPtr->isVisible())
                    {
                       mDialogPtr->close();
                    }
                 }
              }
              else
              {
                 ApplyColors(mPrefWidgetPtr->GetPreferenceObject()->GetColorChoice());
              }
           });

   mCallbacks += wizard::ProxyWatcher::ProxyAvailable.Connect(&Plugin::OnProxyAvailable, this);
}

ZoneEditor::Plugin::~Plugin()
{
   if (mPrefWidgetPtr)
   {
      delete mPrefWidgetPtr.data();
   }
}

// Creates the Add Zone option on the Map View.
void ZoneEditor::Plugin::BuildViewerContextMenu(QMenu* aMenuPtr, vespa::VaViewer* aViewerPtr)
{
   if (wizard::ProjectWorkspace::Instance()->GetProject() != nullptr)
   {
      auto addMenu = aMenuPtr->findChild<QMenu*>("Add at Location");
      if (!addMenu)
      {
         addMenu = aMenuPtr->addMenu("Add at Location");
         addMenu->setObjectName("Add at Location");
      }
      vespa::VaPosition p        = vaEnv.GetCurrentCursorPosition();
      wkf::Action* createZonePtr = ut::qt::make_qt_ptr<wkf::Action>(QIcon::fromTheme("zone"), QString("Zone"), addMenu);
      addMenu->addAction(createZonePtr);

      connect(createZonePtr, &QAction::triggered, [this, p]() { CreateDialog(p); });
   }
}

// Creates a go to zone definition on the Map View
void ZoneEditor::Plugin::BuildAttachmentContextMenu(QMenu* aMenu, vespa::VaAttachment* aAttachmentPtr, unsigned int aSubPartId)
{
   wkf::AttachmentZone* zonePtr = dynamic_cast<wkf::AttachmentZone*>(aAttachmentPtr);
   if (zonePtr && zonePtr->GetContextMenuEnabled())
   {
      wkf::Action* gotoDefPtr =
         ut::qt::make_qt_ptr<wkf::Action>(QIcon::fromTheme("find_text"), QString("Go to Zone Definition"), aMenu);
      aMenu->addAction(gotoDefPtr);
      connect(gotoDefPtr,
              &QAction::triggered,
              [&, zonePtr]()
              {
                 std::string zoneName     = zonePtr->GetName();
                 std::string platformName = zonePtr->GetParent().GetName(); // returns the entity name
                 std::size_t pos          = platformName.find('_');
                 platformName             = platformName.substr(0, pos); // getting zone set name alone

                 auto node = wizard::ProxyUtil::FindZoneNodeByName(zoneName, platformName);

                 if (wizard::Project::Instance() && wizard::Project::Instance()->TinyParseResults())
                 {
                    wizard::ParseResultsTiny* resultsPtr = wizard::Project::Instance()->TinyParseResults();
                    WsfPProxyIndex*           indexPtr   = resultsPtr->ProxyIndex();

                    if (indexPtr != nullptr)
                    {
                       WsfPProxyPath                    endPath = (node).GetPath();
                       std::vector<UtTextDocumentRange> ranges;
                       resultsPtr->FindModificationLocationsExtended(endPath, ranges);
                       wizard::FindReferencesResultDialog::GoToLocationList(wizard::Project::Instance(), ranges);
                    }
                 }
              });
   }
}

QList<wkf::PrefWidget*> ZoneEditor::Plugin::GetPreferencesWidgets() const
{
   QList<wkf::PrefWidget*> prefs;
   prefs << mPrefWidgetPtr;
   return prefs;
}

void ZoneEditor::Plugin::ApplyColors(wkf::ColorOption aColorChoice)
{
   if (aColorChoice == wkf::ColorOption::cRANDOM)
   {
      mDockWidgetPtr->ApplyRandomColorPreferences();
   }
   else if (aColorChoice == wkf::ColorOption::cSINGLESELECT)
   {
      mDockWidgetPtr->ApplySetColorPreferences(mPrefWidgetPtr->GetPreferenceObject()->GetDefaultLineColor(),
                                               mPrefWidgetPtr->GetPreferenceObject()->GetDefaultFillColor());
   }
}

// Creates the Zone Dialog and initializes the latitude and longitude values with the given position.
void ZoneEditor::Plugin::CreateDialog(vespa::VaPosition aInitialPosition)
{
   mDialogPtr = ut::qt::make_qt_ptr<CreateZoneDialog>(mDataContainer,
                                                      *wizard::ProxyWatcher::GetActiveProxy(),
                                                      aInitialPosition,
                                                      wkfEnv.GetMainWindow());
   mDialogPtr->show();
   mDockWidgetPtr->show();
}

// When the proxy becomes available, populate the data container with all the zones currently
// in the scenario, which also populates the zone browser.
void ZoneEditor::Plugin::OnProxyAvailable(WsfPProxy* aProxyPtr)
{
   if (aProxyPtr != nullptr)
   {
      mProxyCallbacks +=
         wizard::WatchProxy(WsfPM_Root(aProxyPtr).zones().GetPath()).AnyUpdate.Connect(&Plugin::HandleZoneProxyChange, this);
      mProxyCallbacks += wizard::WatchProxy(WsfPM_Root(aProxyPtr).platforms().GetPath())
                            .AnyUpdate.Connect(&Plugin::HandlePlatformProxyChange, this);
      PopulateDataContainer(aProxyPtr);
   }
   else
   {
      mProxyCallbacks.Clear();
      mDockWidgetPtr->ClearZoneProxies();
   }
}

// Whenever the proxy zone map changes, add/remove zones based on the change.
void ZoneEditor::Plugin::HandleZoneProxyChange(const wizard::ProxyChange& aChange)
{
   auto node  = aChange.changed();
   auto root  = node.Root();
   auto types = node.GetInheritedTypeNames();
   auto zone  = static_cast<WsfPM_Zone>(node);
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
   switch (aChange.reason())
   {
   case wizard::ProxyChange::Reason::cADDED:
   {
      AddZoneSet(zone, true);
      break;
   }
   case wizard::ProxyChange::Reason::cREMOVED:
   {
      RemoveZone(zone);
      break;
   }
   case wizard::ProxyChange::Reason::cUPDATED:
   {
      UpdateZone(zone);
      break;
   }
   default:
      break;
   }
   ApplyColors(mPrefWidgetPtr->GetPreferenceObject()->GetColorChoice());
}

// Same as above, but with the proxy platform map.
void ZoneEditor::Plugin::HandlePlatformProxyChange(const wizard::ProxyChange& aChange)
{
   WsfPM_Platform platform = static_cast<WsfPM_Platform>(aChange.changed());
   WsfPM_Root     proxyRoot(wizard::ProxyWatcher::GetActiveProxy());
   auto           types = aChange.changed().GetInheritedTypeNames();
   switch (aChange.reason())
   {
   case wizard::ProxyChange::Reason::cADDED:
   {
      if (aChange.changed().IsOfType("WSF_PLATFORM"))
      {
         // The added change was a platform, so find each of its
         // zones and add them to the data container.
         std::vector<WsfPProxyNode> zones;
         platform.Zone().GetAllValues(zones);
         for (const auto& zone : zones)
         {
            // Don't show these automatically; they won't be drawn
            // since this platform was created just now.
            AddZoneSet(static_cast<WsfPM_Zone>(zone), false);
         }
      }
      else if (std::find(types.begin(), types.end(), "ZoneDefinition") != types.end() ||
               std::find(types.begin(), types.end(), "ZoneSet") != types.end())
      {
         // The added change was a zone definition, so add it to the data container.
         AddZoneSet(static_cast<WsfPM_Zone>(aChange.changed()), true);
      }
      break;
   }
   case wizard::ProxyChange::Reason::cREMOVED:
   {
      // When removing something from a platform, GetParent() can't be used to determine
      // the type of the removed node as it has already been removed. However, the proxy path hasn't
      // updated yet, so we can use that to determine what was removed.
      std::vector<std::string> pathStringList = aChange.changed().GetPath().ToStringList(aChange.changed().GetRoot());

      if (pathStringList.size() >= 2)
      {
         size_t index =
            pathStringList.size() - 2; // - 2 because the path list is in ["type", "name"] format and we want the type
         if (pathStringList[index] == "zone")
         {
            RemoveZone(static_cast<WsfPM_Zone>(aChange.changed()));
         }
         else if (pathStringList[index] == "platform")
         {
            mDataContainer.RemovePlatform(platform.GetName());
            RemovePlatformReferenceZones(platform.GetName());
         }
      }
      break;
   }
   case wizard::ProxyChange::Reason::cUPDATED:
   {
      auto curNode = aChange.changed();
      auto type    = curNode.GetTypeName();
      while (type.empty() && curNode.IsValid())
      {
         curNode = curNode.GetParent();
         type    = curNode.GetTypeName();
      }

      if (aChange.changed().IsOfType("WSF_PLATFORM"))
      {
         std::vector<WsfPProxyNode> zones;
         // platform
         platform = static_cast<WsfPM_Platform>(platform.GetParent());
         platform.Zone().GetAllValues(zones);
         for (const auto& zone : zones)
         {
            // Don't show these automatically; they won't be drawn
            // since this platform was created just now.
            UpdateZone(static_cast<WsfPM_Zone>(zone));
         }

         // check any and all zones that use this platform as a reference platform
         auto pmap = proxyRoot.platforms();
         for (auto&& it = pmap.begin(); it != pmap.end(); ++it)
         {
            auto curPlatform = pmap.AtT(it);
            if (curPlatform.GetName() == platform.GetName())
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
                  if (zoneDef.ReferencePlatform() == platform.GetName())
                  {
                     UpdateZone(static_cast<WsfPM_Zone>(zoneNode));
                  }
               }
               else if (zoneNode.IsZoneSet())
               {
                  UpdatePlatformZoneSet(zoneNode, platform);
               }
            }
         }

         // Now go through global zones
         auto                       zoneMap = proxyRoot.zones();
         std::vector<WsfPProxyNode> globalZones;
         zoneMap.GetAllValues(globalZones);
         for (const auto& zoneIter : globalZones)
         {
            auto zoneNode = static_cast<WsfPM_Zone>(zoneIter);
            if (zoneNode.IsDefinition())
            {
               auto zoneDef = static_cast<WsfPM_ZoneDefinition>(zoneNode);
               if (zoneDef.ReferencePlatform() == platform.GetName())
               {
                  UpdateZone(static_cast<WsfPM_Zone>(zoneNode));
               }
            }
            else if (zoneNode.IsZoneSet())
            {
               UpdatePlatformZoneSet(zoneNode, platform);
            }
         }
      }
      else if (std::find(types.begin(), types.end(), "ZoneDefinition") != types.end() ||
               std::find(types.begin(), types.end(), "ZoneSet") != types.end())
      {
         UpdateZone(static_cast<WsfPM_Zone>(curNode));
      }
      else
      {
         platform = static_cast<WsfPM_Platform>(proxyRoot.platforms() + aChange.changed().GetPath()[1]);
         std::vector<WsfPProxyNode> zones;
         // platform
         platform.Zone().GetAllValues(zones);
         for (const auto& zone : zones)
         {
            // Don't show these automatically; they won't be drawn
            // since this platform was created just now.
            UpdateZone(static_cast<WsfPM_Zone>(zone));
         }

         // check any and all zones that use this platform as a reference platform
         auto pmap = proxyRoot.platforms();
         for (auto&& it = pmap.begin(); it != pmap.end(); ++it)
         {
            auto curPlatform = pmap.AtT(it);
            if (curPlatform.GetName() == platform.GetName())
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
                  if (zoneDef.ReferencePlatform() == platform.GetName())
                  {
                     UpdateZone(static_cast<WsfPM_Zone>(zoneNode));
                  }
               }
               else if (zoneNode.IsZoneSet())
               {
                  UpdatePlatformZoneSet(zoneNode, platform);
               }
            }
         }

         // Now go through global zones
         auto                       zoneMap = proxyRoot.zones();
         std::vector<WsfPProxyNode> globalZones;
         zoneMap.GetAllValues(globalZones);
         for (const auto& zoneIter : globalZones)
         {
            auto zoneNode = static_cast<WsfPM_Zone>(zoneIter);
            if (zoneNode.IsDefinition())
            {
               auto zoneDef = static_cast<WsfPM_ZoneDefinition>(zoneNode);
               if (zoneDef.ReferencePlatform() == platform.GetName())
               {
                  UpdateZone(static_cast<WsfPM_Zone>(zoneNode));
               }
            }
            else if (zoneNode.IsZoneSet())
            {
               UpdatePlatformZoneSet(zoneNode, platform);
            }
         }
      }
      break;
   }
   default:
      break;
   }
   ApplyColors(mPrefWidgetPtr->GetPreferenceObject()->GetColorChoice());
}

void ZoneEditor::Plugin::UpdatePlatformZoneSet(const WsfPM_Zone& aZone, const WsfPM_Platform& aPlatform)
{
   // loop through the zones
   auto zoneSet                = static_cast<WsfPM_ZoneSet>(aZone);
   auto inclusionZones         = zoneSet.InclusionZones();
   auto exclusionZones         = zoneSet.ExclusionZones();
   auto embeddedInclusionZones = zoneSet.EmbeddedInclusionZones();
   auto embeddedExclusionZones = zoneSet.EmbeddedExclusionZones();

   for (size_t i = 0; i < inclusionZones.size(); ++i)
   {
      auto inclusionZoneNode = static_cast<WsfPM_Zone>(inclusionZones[i]);
      if (inclusionZoneNode.IsDefinition())
      {
         auto zoneDef = static_cast<WsfPM_ZoneDefinition>(inclusionZoneNode);
         if (zoneDef.ReferencePlatform() == aPlatform.GetName())
         {
            UpdateZone(aZone);
            break;
         }
      }
   }
   for (size_t i = 0; i < exclusionZones.size(); ++i)
   {
      auto exclusionZoneNode = static_cast<WsfPM_Zone>(exclusionZones[i]);
      if (exclusionZoneNode.IsDefinition())
      {
         auto zoneDef = static_cast<WsfPM_ZoneDefinition>(exclusionZoneNode);
         if (zoneDef.ReferencePlatform() == aPlatform.GetName())
         {
            UpdateZone(aZone);
            break;
         }
      }
   }
   for (size_t i = 0; i < embeddedInclusionZones.size(); ++i)
   {
      auto embeddedInclusionZoneNode = static_cast<WsfPM_Zone>(embeddedInclusionZones[i]);
      if (embeddedInclusionZoneNode.IsDefinition())
      {
         auto zoneDef = static_cast<WsfPM_ZoneDefinition>(embeddedInclusionZoneNode);
         if (zoneDef.ReferencePlatform() == aPlatform.GetName())
         {
            UpdateZone(aZone);
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
         if (zoneDef.ReferencePlatform() == aPlatform.GetName())
         {
            UpdateZone(aZone);
            break;
         }
      }
   }
}

wkf::ZoneSetData ZoneEditor::Plugin::CreateZoneSet(const WsfPM_Zone& aZone)
{
   wkf::ZoneSetData zoneSetData(aZone.GetName(), aZone.ParentPlatform());
   return CreateZoneSet(aZone, zoneSetData);
}

wkf::ZoneSetData ZoneEditor::Plugin::CreateZoneSet(const WsfPM_Zone& aZone, wkf::ZoneSetData& aZoneSetData)
{
   if (aZone.IsZoneSet())
   {
      WsfPM_ZoneSet            zoneSet{aZone};
      WsfPM_List               inclusionZones         = zoneSet.InclusionZones();
      WsfPM_List               exclusionZones         = zoneSet.ExclusionZones();
      WsfPM_ZoneDefinitionList embeddedInclusionZones = zoneSet.EmbeddedInclusionZones();
      WsfPM_ZoneDefinitionList embeddedExclusionZones = zoneSet.EmbeddedExclusionZones();

      for (size_t i = 0; i < inclusionZones.size(); ++i)
      {
         std::string zoneName = inclusionZones[i].GetValue().ValueToString();
         WsfPM_Zone  zoneNode = wizard::ProxyUtil::FindZoneNodeByName(zoneName, aZoneSetData.GetPlatformName());
         if (zoneNode.IsDefinition())
         {
            WsfPM_ZoneDefinition zoneDefNode{zoneNode};
            if (!zoneDefNode.ReferencePlatform().empty() && !zoneDefNode.IsReferencePlatformValid())
            {
               // this zone has a reference entity that doesn't exist, don't add it
               continue;
            }
         }
         CreateZoneSet(zoneNode, aZoneSetData);
      }

      for (size_t i = 0; i < exclusionZones.size(); ++i)
      {
         std::string zoneName = exclusionZones[i].GetValue().ValueToString();
         // look up zone name
         WsfPM_Zone zoneNode = wizard::ProxyUtil::FindZoneNodeByName(zoneName, aZoneSetData.GetPlatformName());
         if (zoneNode.IsDefinition())
         {
            WsfPM_ZoneDefinition zoneDefNode{zoneNode};
            if (!zoneDefNode.ReferencePlatform().empty() && !zoneDefNode.IsReferencePlatformValid())
            {
               // this zone has a reference entity that doesn't exist, don't add it
               continue;
            }
         }
         CreateZoneSet(zoneNode, aZoneSetData);
      }

      for (size_t i = 0; i < embeddedInclusionZones.size(); ++i)
      {
         WsfPM_Zone           embedZone = embeddedInclusionZones[i];
         WsfPM_ZoneDefinition zoneDef{embedZone};
         auto                 refPlat = zoneDef.ReferencePlatform();
         if (!refPlat.empty() && !zoneDef.IsReferencePlatformValid())
         {
            // this zone has a reference entity that doesn't exist, don't add it
            continue;
         }
         else
         {
            AddZone(embedZone, aZoneSetData);
         }
      }

      for (size_t i = 0; i < embeddedExclusionZones.size(); ++i)
      {
         WsfPM_Zone           embedZone = embeddedExclusionZones[i];
         WsfPM_ZoneDefinition zoneDef{embedZone};
         auto                 refPlat = zoneDef.ReferencePlatform();
         if (!refPlat.empty() && !zoneDef.IsReferencePlatformValid())
         {
            // this zone has a reference entity that doesn't exist, don't add it
            continue;
         }
         else
         {
            AddZone(embedZone, aZoneSetData);
         }
      }
   }
   else // if (aZone.IsDefinition())
   {
      AddZone(aZone, aZoneSetData);
   }

   aZoneSetData.SetFillColor(aZone.GetFillColor());
   aZoneSetData.SetLineColor(aZone.GetLineColor());

   return aZoneSetData;
}

// Adds a zone set to the data container, gathering appropriate data based on the type of zone.
void ZoneEditor::Plugin::AddZoneSet(const WsfPM_Zone& aZone, bool aShow)
{
   wkf::ZoneSetData zoneSetData{CreateZoneSet(aZone)};

   if (zoneSetData.GetZones().size() > 0)
   {
      mDataContainer.AddZoneSet(zoneSetData, aShow);
      mDockWidgetPtr->AddZoneProxy(aZone);
   }
}

void ZoneEditor::Plugin::AddZone(const WsfPM_ZoneDefinition& aZone, wkf::ZoneSetData& aZoneSet)
{
   wkf::CommonZoneVariables commonVars = ExtractCommonVars(aZone);

   aZoneSet.SetFillColor(aZone.GetFillColor());
   aZoneSet.SetLineColor(aZone.GetLineColor());

   WsfPM_ZoneDefinition::DefinitionType zoneType = aZone.GetDefinitionType();
   if (zoneType == WsfPM_ZoneDefinition::cTYPE_NOT_SET)
   {
      // if the type is not set due to lack of name other circumstance attempt to figure out what the zonetype is based
      // on other inputs just as core does...
      if (!aZone.Points().empty())
      {
         // points were listed, set the type to polygonal
         zoneType = WsfPM_ZoneDefinition::cPOLYGONAL;
      }
      else if (!aZone.LateralAxis().IsUnset() || !aZone.LongitudinalAxis().IsUnset())
      {
         // lateral or longitudinal axes are set
         zoneType = WsfPM_ZoneDefinition::cELLIPTICAL;
      }
   }
   if (zoneType == WsfPM_ZoneDefinition::cCIRCULAR)
   {
      aZoneSet.AddCircularData(commonVars, ExtractCircularVars(aZone));
   }
   else if (zoneType == WsfPM_ZoneDefinition::cELLIPTICAL)
   {
      aZoneSet.AddEllipticalData(commonVars, ExtractEllipticalVars(aZone));
   }
   else if (zoneType == WsfPM_ZoneDefinition::cSPHERICAL)
   {
      aZoneSet.AddSphericalData(commonVars, ExtractSphericalVars(aZone));
   }
   else if (zoneType == WsfPM_ZoneDefinition::cPOLYGONAL)
   {
      wkf::PolygonalZoneVariables polyVars = ExtractPolygonalVars(aZone);
      if (polyVars.mUseLatLon)
      {
         // If this zone uses specific points, its location is defined even though it's on a platform.
         // Setting this to true means a separate anchor entity will be created for this zone
         // in the dock widget's GetAnchorEntityData function.
         commonVars.mLocationDefined       = true;
         commonVars.mReferencePlatformName = "";
      }
      aZoneSet.AddPolygonalData(commonVars, polyVars);
   }

   auto zoneDef = WsfPM_ZoneDefinition(aZone);
   if (zoneDef.IsValid())
   {
      // Update all zones attempting to reference this zone
      for (auto zone : zoneDef.GetReferencingZones())
      {
         UpdateZone(zone);
      }
   }
}

void ZoneEditor::Plugin::UpdateZone(const WsfPM_Zone& aZone)
{
   wkf::ZoneSetData zoneSet{CreateZoneSet(aZone)};
   if (!zoneSet.GetZones().empty() && !mDockWidgetPtr->ZoneProxyFound(aZone))
   {
      // need to add zone to proxy if it isn't there.
      // This will happen if an empty zoneset is created
      mDockWidgetPtr->AddZoneProxy(aZone);
   }
   mDataContainer.UpdateData(zoneSet);

   auto zoneDef = WsfPM_ZoneDefinition(aZone);
   if (zoneDef.IsValid())
   {
      for (auto& zone : zoneDef.GetReferencingZones())
      {
         UpdateZone(zone);
      }
   }
}

// Removes a zone from the data container.
void ZoneEditor::Plugin::RemoveZone(const WsfPM_Zone& aZone)
{
   auto platformName = GetPlatformName(aZone);
   auto name         = aZone.GetName();
   auto zoneDef      = WsfPM_ZoneDefinition(aZone);
   mDataContainer.RemoveData(aZone.GetName(), platformName);

   // Update all zones attempting to reference this zone
   for (auto& zone : zoneDef.GetReferencingZones())
   {
      UpdateZone(zone);
   }
   mDockWidgetPtr->RemoveZoneProxy(aZone);
}

// Takes a zone and returns the variables that are common amongst all zone types.
wkf::CommonZoneVariables ZoneEditor::Plugin::ExtractCommonVars(const WsfPM_Zone& aZone)
{
   WsfPM_ZoneDefinition definition{aZone};

   wkf::CommonZoneVariables commonVars;

   std::string platformName    = definition.ParentPlatform();
   commonVars.mLocationDefined = (platformName == "Global Zones");

   // Set a source zone only if values are directly inherited from it.
   if (aZone.GetSourceZone().GetName() != aZone.GetName())
   {
      commonVars.mSourceZoneName = aZone.GetSourceZone().GetName();
   }
   commonVars.mReferencePlatformName = definition.ReferencePlatform();
   commonVars.mHeading               = definition.Heading().GetValue();

   WsfProxy::Length minAlt    = definition.MinimumAltitude();
   WsfProxy::Length maxAlt    = definition.MaximumAltitude();
   commonVars.mMinAltitudeSet = !minAlt.IsUnset();
   commonVars.mMaxAltitudeSet = !maxAlt.IsUnset();
   commonVars.mMinAltitude    = (commonVars.mMinAltitudeSet ? static_cast<double>(minAlt.GetValue()) : 0.0);
   commonVars.mMaxAltitude =
      (commonVars.mMaxAltitudeSet ? static_cast<double>(maxAlt.GetValue()) : commonVars.mMinAltitude);

   auto pos                 = definition.Position();
   commonVars.mReferenceLat = pos.IsUnset() ? 0 : static_cast<double>(pos.GetLatitude());
   commonVars.mReferenceLon = pos.IsUnset() ? 0 : static_cast<double>(pos.GetLongitude());

   commonVars.mAuxData = definition.AuxData();

   return commonVars;
}

wkf::CircularZoneVariables ZoneEditor::Plugin::ExtractCircularVars(const WsfPM_ZoneDefinition& aZone)
{
   wkf::CircularZoneVariables circularVars;

   auto maxRadius  = aZone.MaximumRadius();
   auto startAngle = aZone.StartAngle();
   auto stopAngle  = aZone.StopAngle();

   circularVars.mRadius     = (maxRadius.IsUnset() ? 0 : static_cast<double>(maxRadius.GetValue()));
   circularVars.mStartAngle = (startAngle.IsUnset() ? 0 : static_cast<double>(startAngle.GetValue()));
   circularVars.mStopAngle  = (stopAngle.IsUnset() ? UtMath::cTWO_PI : static_cast<double>(stopAngle.GetValue()));

   return circularVars;
}

wkf::EllipticalZoneVariables ZoneEditor::Plugin::ExtractEllipticalVars(const WsfPM_ZoneDefinition& aZone)
{
   wkf::EllipticalZoneVariables ellipticalVars;

   auto latAxis    = aZone.LateralAxis();
   auto lonAxis    = aZone.LongitudinalAxis();
   auto startAngle = aZone.StartAngle();
   auto stopAngle  = aZone.StopAngle();

   ellipticalVars.mLatAxis    = (latAxis.IsUnset() ? 0 : static_cast<double>(latAxis.GetValue()));
   ellipticalVars.mLonAxis    = (lonAxis.IsUnset() ? 0 : static_cast<double>(lonAxis.GetValue()));
   ellipticalVars.mStartAngle = (startAngle.IsUnset() ? 0 : static_cast<double>(startAngle.GetValue()));
   ellipticalVars.mStopAngle  = (stopAngle.IsUnset() ? UtMath::cTWO_PI : static_cast<double>(stopAngle.GetValue()));

   return ellipticalVars;
}

wkf::SphericalZoneVariables ZoneEditor::Plugin::ExtractSphericalVars(const WsfPM_ZoneDefinition& aZone)
{
   wkf::SphericalZoneVariables sphericalVars;

   auto maxRadius  = aZone.MaximumRadius();
   auto minRadius  = aZone.MinimumRadius();
   auto startAngle = aZone.StartAngle();
   auto stopAngle  = aZone.StopAngle();

   sphericalVars.mRadius     = (maxRadius.IsUnset() ? 0 : static_cast<double>(maxRadius.GetValue()));
   sphericalVars.mMinRadius  = (minRadius.IsUnset() ? 0 : static_cast<double>(minRadius.GetValue()));
   sphericalVars.mStartAngle = (startAngle.IsUnset() ? 0 : static_cast<double>(startAngle.GetValue()));
   sphericalVars.mStopAngle  = (stopAngle.IsUnset() ? UtMath::cTWO_PI : static_cast<double>(stopAngle.GetValue()));

   return sphericalVars;
}


wkf::PolygonalZoneVariables ZoneEditor::Plugin::ExtractPolygonalVars(const WsfPM_ZoneDefinition& aZone)
{
   wkf::PolygonalZoneVariables polygonalVars;
   polygonalVars.mUseLatLon = aZone.IsAbsolute();

   std::vector<WsfPM_Zone::Point> pointsVec = aZone.Points();
   for (const auto& point : pointsVec)
   {
      if ((aZone.UseLatLon() || aZone.UseMGRS()) && !aZone.ReferenceZone().empty())
      {
         polygonalVars.mPoints.emplace_back(wkf::ZonePointStruct{-point[0], point[1]});
      }
      else
      {
         polygonalVars.mPoints.emplace_back(wkf::ZonePointStruct{point[0], point[1]});
      }
   }

   return polygonalVars;
}

void ZoneEditor::Plugin::PopulateDataContainer(WsfPProxy* aProxyPtr)
{
   if (aProxyPtr != nullptr)
   {
      mDataContainer.ClearData();

      // This map will only contain the globally-defined zones.
      WsfPM_ZoneMap              zoneMap = WsfPM_Root(aProxyPtr).zones();
      std::vector<WsfPProxyNode> zones;
      zoneMap.GetAllValues(zones);
      for (const auto& zoneIter : zones)
      {
         WsfPM_Zone zone = static_cast<WsfPM_Zone>(zoneIter);
         AddZoneSet(zone);
      }

      // Grab all the zones that are attached to platforms.
      WsfPM_PlatformMap          platformMap = WsfPM_Root(aProxyPtr).platforms();
      std::vector<WsfPProxyNode> platforms;
      platformMap.GetAllValues(platforms);
      for (const auto& platformIter : platforms)
      {
         WsfPM_ZoneMap              platformZoneMap = static_cast<WsfPM_Platform>(platformIter).Zone();
         std::vector<WsfPProxyNode> platformZones;
         platformZoneMap.GetAllValues(platformZones);
         for (const auto& zoneIter : platformZones)
         {
            WsfPM_Zone zone = static_cast<WsfPM_Zone>(zoneIter);
            AddZoneSet(zone);
         }
      }

      ApplyColors(mPrefWidgetPtr->GetPreferenceObject()->GetColorChoice());
   }
}


void ZoneEditor::Plugin::RemovePlatformReferenceZones(const std::string& aPlatformName)
{
   WsfPM_Root proxyRoot(wizard::ProxyWatcher::GetActiveProxy());
   auto       pmap = proxyRoot.platforms();
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
               RemoveZone(zoneDef);
            }
         }
      }
   }

   // now remove any global zones associated with this platform
   auto                       zoneMap = proxyRoot.zones();
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
            RemoveZone(zoneDef);
         }
      }
   }
}


void ZoneEditor::Plugin::RemovePlatformReferenceZoneSetZone(const WsfPM_Zone& aZone, const std::string& aPlatformName)
{
   // loop through the zones
   auto zoneSet                = static_cast<WsfPM_ZoneSet>(aZone);
   auto inclusionZones         = zoneSet.InclusionZones();
   auto exclusionZones         = zoneSet.ExclusionZones();
   auto embeddedInclusionZones = zoneSet.EmbeddedInclusionZones();
   auto embeddedExclusionZones = zoneSet.EmbeddedExclusionZones();

   for (size_t i = 0; i < inclusionZones.size(); ++i)
   {
      auto inclusionZoneNode = static_cast<WsfPM_Zone>(inclusionZones[i]);
      if (inclusionZoneNode.IsDefinition())
      {
         auto zoneDef = static_cast<WsfPM_ZoneDefinition>(inclusionZoneNode);
         if (zoneDef.ReferencePlatform() == aPlatformName)
         {
            RemoveZone(aZone);
         }
      }
   }
   for (size_t i = 0; i < exclusionZones.size(); ++i)
   {
      auto exclusionZoneNode = static_cast<WsfPM_Zone>(exclusionZones[i]);
      if (exclusionZoneNode.IsDefinition())
      {
         auto zoneDef = static_cast<WsfPM_ZoneDefinition>(exclusionZoneNode);
         if (zoneDef.ReferencePlatform() == aPlatformName)
         {
            RemoveZone(aZone);
         }
      }
   }
   for (size_t i = 0; i < embeddedInclusionZones.size(); ++i)
   {
      auto embeddedInclusionZoneNode = static_cast<WsfPM_Zone>(embeddedInclusionZones[i]);
      if (embeddedInclusionZoneNode.IsDefinition())
      {
         auto zoneDef = static_cast<WsfPM_ZoneDefinition>(embeddedInclusionZoneNode);
         if (zoneDef.ReferencePlatform() == aPlatformName)
         {
            RemoveZone(aZone);
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
            RemoveZone(aZone);
         }
      }
   }
}
