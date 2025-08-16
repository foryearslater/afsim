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

#include "MapUtilsPlugin.hpp"

#include <QMenu>

#include "ChangeWrapper.hpp"
#include "CreatePlatformDialog.hpp"
#include "EditorPrefObject.hpp"
#include "GhostingInterface.hpp"
#include "Message.hpp"
#include "ParseResults.hpp"
#include "Platform.hpp"
#include "PluginUtil.hpp"
#include "Project.hpp"
#include "ProjectWorkspace.hpp"
#include "ProxyWatcher.hpp"
#include "UtEllipsoidalEarth.hpp"
#include "UtQtMemory.hpp"
#include "VaCamera.hpp"
#include "VaEphemeris.hpp"
#include "VaModelDatabase.hpp"
#include "VaObserver.hpp"
#include "WkfAction.hpp"
#include "WkfMainWindow.hpp"
#include "WkfTeamVisibilityPrefObject.hpp"
#include "WkfViewer.hpp"
#include "WkfVtkEnvironment.hpp"
#include "WsfEditor.hpp"
#include "WsfPProxyRegistry.hpp"

WKF_PLUGIN_DEFINE_SYMBOLS(MapUtils::Plugin,
                          "Map Utilities",
                          "This plug-in provides a set of utilities to use with the Map Display.\n"
                          "The features it provides are:\n"
                          "* wizard specific operations on the map entity context menu.\n"
                          "* wizard specific tools to manipulate scenarios.",
                          "wizard");

namespace MapUtils
{

Plugin* Plugin::mInstancePtr = nullptr;

Plugin* Plugin::Instance()
{
   assert(mInstancePtr != nullptr);
   return mInstancePtr;
}

Plugin::Plugin(const QString& aPluginName, const size_t aUniqueId)
   : wizard::Plugin(aPluginName, aUniqueId)
   , mModelMenu("Icons")
{
   assert(mInstancePtr == nullptr);
   mInstancePtr = this;
   // create the ghostMan
   ghost::Manager::Create();

   // Provide access from Tools menu
   wkf::MainWindow* mainWindowPtr = wkfEnv.GetMainWindow();
   QMenu*           toolsMenuPtr  = mainWindowPtr->GetMenuByName("Tools");

   // create map utilities menu (sub-menu of "Tools")
   QMenu* mapUtilsMenuPtr = ut::qt::make_qt_ptr<QMenu>("Map Utilities", toolsMenuPtr);
   mapUtilsMenuPtr->setIcon(QIcon::fromTheme("globe"));
   // build the map utilities menu
   BuildMapUtilitiesMenu(mapUtilsMenuPtr);
   // add map utilities menu to the tools menu
   toolsMenuPtr->addMenu(mapUtilsMenuPtr);

   // close any ScenarioTransformation dialogs that may still be open
   connect(wizard::ProjectWorkspace::Instance(), &wizard::ProjectWorkspace::ProjectOpened, this, &Plugin::OnProjectOpened);
   // disable the platform tools of this plug-in when project is closed
   // close any ScenarioTransformation dialogs that may still be open
   connect(wizard::ProjectWorkspace::Instance(), &wizard::ProjectWorkspace::ProjectClosed, this, &Plugin::OnProjectClosed);

   connect(wizSignals, &wizard::Signals::EditorContextMenu, this, &Plugin::OnEditorContextMenu);
   connect(wizSignals, &wizard::Signals::RequestNameSuggestions, this, &Plugin::OnRequestNameSuggestions);
   connect(wizSignals, &wizard::Signals::EpochUpdated, this, &Plugin::EpochUpdated);
   // close any ScenarioTransformation dialogs that may still be open
   connect(wizSignals, &wizard::Signals::ProjectStartupFilesChanged, this, &Plugin::OnProjectStartupFilesChanged);

   connect(&wkfEnv, &wkf::Environment::DeleteShortcutTriggered, this, &Plugin::DeleteCurrentlySelectedEntities);
}

Plugin::~Plugin()
{
   assert(mInstancePtr != nullptr);
   mInstancePtr = nullptr;
}

QList<wkf::Action*> Plugin::GetActions() const
{
   return mActions;
}

void Plugin::OnProjectOpened(wizard::Project* /* aSrc */)
{
   ForceCloseScenarioTransformationDialogs();
}

void Plugin::OnProjectClosed(wizard::Project* /* aSrc */)
{
   ForceCloseScenarioTransformationDialogs();
   DisableMapUtilitiesMenuActions();
}

void Plugin::OnProjectStartupFilesChanged(const std::vector<UtPath>& aMainFiles)
{
   ForceCloseScenarioTransformationDialogs();
}

void Plugin::StartRotateScenario(const RotateMenuItem aMode)
{
   // try to create and open the RotateScenario::Dialog
   try
   {
      mRotateDialogPtr =
         ut::qt::make_qt_ptr<RotateScenarioDialog>(wkfEnv.GetMainWindow(), Qt::WindowFlags(), aMode, false, false);
   }
   catch (const Message& aProblem)
   {
      aProblem.Display();
      return;
   }
   DisconnectMapUtilitiesMenuActionAvailabilityUpdate();
   DisableMapUtilitiesMenuActions();
   mRotateDialogPtr->show();
   // connect "dialog closed" signal to EnableMapUtilitiesMenuActions (will enable them)
   connect(mRotateDialogPtr.data(),
           &RotateScenarioDialog::Closed,
           this,
           [=]()
           {
              delete mRotateDialogPtr;
              ConnectMapUtilitiesMenuActionAvailabilityUpdate();
           });
}
void Plugin::StartTranslateScenario(const TranslateMenuItem aMode)
{
   // try to create and open the TranslateScenario::Dialog
   try
   {
      mTranslateDialogPtr =
         ut::qt::make_qt_ptr<TranslateScenarioDialog>(wkfEnv.GetMainWindow(), Qt::WindowFlags(), aMode, false, false);
   }
   catch (const Message& aProblem)
   {
      aProblem.Display();
      return;
   }
   DisconnectMapUtilitiesMenuActionAvailabilityUpdate();
   DisableMapUtilitiesMenuActions();
   mTranslateDialogPtr->show();
   // connect "dialog closed" signal to EnableMapUtilitiesMenuActions (will enable them)
   connect(mTranslateDialogPtr.data(),
           &TranslateScenarioDialog::Closed,
           this,
           [=]()
           {
              delete mTranslateDialogPtr;
              ConnectMapUtilitiesMenuActionAvailabilityUpdate();
           });
}

void Plugin::ConnectMapUtilitiesMenuActionAvailabilityUpdate()
{
   connect(mRotateMenuPtr.data(), &QMenu::aboutToShow, this, &Plugin::UpdateAvailableActions);
   connect(mTranslateMenuPtr.data(), &QMenu::aboutToShow, this, &Plugin::UpdateAvailableActions);
}

void Plugin::DisconnectMapUtilitiesMenuActionAvailabilityUpdate()
{
   disconnect(mRotateMenuPtr.data(), &QMenu::aboutToShow, this, &Plugin::UpdateAvailableActions);
   disconnect(mTranslateMenuPtr.data(), &QMenu::aboutToShow, this, &Plugin::UpdateAvailableActions);
}

void Plugin::DisableMapUtilitiesMenuActions()
{
   // disable all the rotate actions
   for (const auto& elem : mRotateActionList)
   {
      elem.second->setDisabled(true);
   }
   // disable all the translate actions
   for (const auto& elem : mTranslateActionList)
   {
      elem.second->setDisabled(true);
   }
}

void Plugin::EnableMapUtilitiesMenuActions(const AvailableActions aWhichOnes)
{
   switch (aWhichOnes)
   {
   case AvailableActions::cSCENARIO:
      // enable the rotate scenario actions
      for (const auto& elem : mRotateActionList)
      {
         if (elem.first == RotateMenuItem::cROTATE_SCENARIO)
         {
            elem.second->setEnabled(true);
         }
         else
         {
            elem.second->setEnabled(false);
         }
      }
      // enable the translate scenario actions
      for (const auto& elem : mTranslateActionList)
      {
         if (elem.first == TranslateMenuItem::cTRANSLATE_SCENARIO_TO ||
             elem.first == TranslateMenuItem::cTRANSLATE_SCENARIO_BY)
         {
            elem.second->setEnabled(true);
         }
         else
         {
            elem.second->setEnabled(false);
         }
      }
      break;
   case AvailableActions::cSELECTION:
      // enable all the rotate actions
      for (const auto& elem : mRotateActionList)
      {
         elem.second->setEnabled(true);
      }
      // enable all the translate actions
      for (const auto& elem : mTranslateActionList)
      {
         elem.second->setEnabled(true);
      }
   case AvailableActions::cNONE:
      // use default
   default:
      break;
   }
}

void Plugin::ForceCloseScenarioTransformationDialogs()
{
   if (mRotateDialogPtr != nullptr)
   {
      emit ForceClose(mRotateDialogPtr);
      delete mRotateDialogPtr;
   }
   if (mTranslateDialogPtr != nullptr)
   {
      emit ForceClose(mTranslateDialogPtr);
      delete mTranslateDialogPtr;
   }
}

void Plugin::BuildMapUtilitiesMenu(QMenu* aMenuPtr)
{
   // create the rotate menu
   mRotateMenuPtr = ut::qt::make_qt_ptr<QMenu>("Rotate", aMenuPtr);
   mRotateMenuPtr->setIcon(QIcon::fromTheme("rotate"));
   connect(mRotateMenuPtr.data(), &QMenu::aboutToShow, this, &Plugin::UpdateAvailableActions);
   // create the rotate actions
   mRotateActionList.push_back(
      {RotateMenuItem::cROTATE_SCENARIO, ut::qt::make_qt_ptr<wkf::Action>("Scenario...", mRotateMenuPtr)});
   mRotateActionList.push_back(
      {RotateMenuItem::cROTATE_SELECTION, ut::qt::make_qt_ptr<wkf::Action>("Selection...", mRotateMenuPtr)});
   // build the rotate menu
   for (const auto& elem : mRotateActionList)
   {
      // connect each action trigger to the appropriate slot
      connect(elem.second.data(), &wkf::Action::triggered, this, [=]() { StartRotateScenario(elem.first); });
      mRotateMenuPtr->addAction(elem.second);
   }
   // add rotate menu as a submenu of the map utilities menu
   aMenuPtr->addMenu(mRotateMenuPtr);

   // create the translate menu
   mTranslateMenuPtr = ut::qt::make_qt_ptr<QMenu>("Translate", aMenuPtr);
   mTranslateMenuPtr->setIcon(QIcon::fromTheme("translate"));
   connect(mTranslateMenuPtr.data(), &QMenu::aboutToShow, this, &Plugin::UpdateAvailableActions);
   // create the translate actions
   mTranslateActionList.push_back({TranslateMenuItem::cTRANSLATE_SCENARIO_TO,
                                   ut::qt::make_qt_ptr<wkf::Action>("Scenario to...", mTranslateMenuPtr)});
   // mTranslateActionList.push_back({ TranslateMenuItem::cTRANSLATE_SCENARIO_BY,
   // ut::qt::make_qt_ptr<wkf::Action>("Scenario by...", mTranslateMenuPtr) });
   mTranslateActionList.push_back({TranslateMenuItem::cTRANSLATE_SELECTION_TO,
                                   ut::qt::make_qt_ptr<wkf::Action>("Selection to...", mTranslateMenuPtr)});
   // mTranslateActionList.push_back({ TranslateMenuItem::cTRANSLATE_SELECTION_BY,
   // ut::qt::make_qt_ptr<wkf::Action>("Selection by...", mTranslateMenuPtr) }); build the translate menu
   for (const auto& elem : mTranslateActionList)
   {
      // connect each action trigger to the appropriate slot
      connect(elem.second.data(), &wkf::Action::triggered, this, [=]() { StartTranslateScenario(elem.first); });
      mTranslateMenuPtr->addAction(elem.second);
   }
   // add translate menu as a submenu of the map utilities menu
   aMenuPtr->addMenu(mTranslateMenuPtr);
}

void Plugin::UpdateAvailableActions()
{
   DisableMapUtilitiesMenuActions();
   // enable specific actions inside the MapUtils menu based on the state of the scenario
   EnableMapUtilitiesMenuActions(PluginUtil::GetAcquisitionType());
}

void Plugin::DeletePlatformActionHandler(wkf::Entity* aEntityPtr)
{
   wizard::Platform* platform = dynamic_cast<wizard::Platform*>(aEntityPtr);
   if (platform)
   {
      platform->RemovePlatformFromEditor();
   }
}

void Plugin::ClonePlatformActionHandler(wizard::Platform* aPlatformPtr)
{
   UtTextDocumentRange range     = aPlatformPtr->GetPlatform().GetSourceDocumentRange();
   wizard::TextSource* sourcePtr = (wizard::TextSource*)range.mSource;

   if (sourcePtr)
   {
      QString     platformDef(range.Text().c_str());
      QStringList platformDefTokenList = platformDef.split(" ");
      QString     platformName(aPlatformPtr->GetName().c_str());

      int     cloneCount = 1;
      QString cloneName(aPlatformPtr->GetName().c_str());
      cloneName.append("_");
      cloneName.append(QString::number(cloneCount));
      // Check to see if this name is unique..
      while (WsfPM_Root(aPlatformPtr->GetPlatform().GetProxy()).platforms().Find(cloneName.toStdString()).IsValid())
      {
         ++cloneCount;
         cloneName = aPlatformPtr->GetName().c_str();
         cloneName.append("_");
         cloneName.append(QString::number(cloneCount));
      }

      int nameIdx                   = platformDefTokenList.indexOf(platformName);
      platformDefTokenList[nameIdx] = cloneName;
      QString     newCloneStr       = platformDefTokenList.join(" ");
      std::string cloneDef          = "\n";
      cloneDef.append(newCloneStr.toStdString());
      sourcePtr->Append(cloneDef);
      wizEnv.RequestSelectPlatform(cloneName);
   }
}

void Plugin::CreatePlatformActionHandler()
{
   if (wizard::Project::Instance())
   {
      WsfPProxy* projProxy = wizard::Project::Instance()->Proxy();
      if (projProxy != nullptr)
      {
         vespa::VaViewer* viewerPtr = vaEnv.GetStandardViewer();
         if (viewerPtr)
         {
            auto pos = vaEnv.GetCurrentCursorPosition();

            wizard::CreatePlatformDialog* dlg =
               new wizard::CreatePlatformDialog(projProxy, (QWidget*)wkfEnv.GetMainWindow());
            dlg->SetPlatformPosition(pos.GetLat(), pos.GetLon());
            dlg->ChooseFileFromType();
            dlg->show();
         }
      }
   }
}

void Plugin::OnEditorContextMenu(wizard::Editor* aEditorPtr, QMenu& aMenu)
{
   wizard::WsfEditor* editorPtr = dynamic_cast<wizard::WsfEditor*>(aEditorPtr);
   if (!editorPtr)
   {
      return;
   }
   wizard::ParseResults* parseResultsPtr = editorPtr->GetProject()->GetParseResults();
   if (!parseResultsPtr || parseResultsPtr->ProxyRegistry() == nullptr)
   {
      return;
   }
   if (editorPtr->GetContextMenuCharacterPosition() < 0)
   {
      return;
   }
   UtTextDocumentRange range(editorPtr->GetSource(),
                             editorPtr->GetContextMenuCharacterPosition(),
                             editorPtr->GetContextMenuCharacterPosition());
   WsfParseNode*       nodePtr = parseResultsPtr->FindLastParseNode(range);
   if (!nodePtr)
   {
      return;
   }
   while (nodePtr)
   {
      struct ReplaceRangeAction
      {
         UtTextDocumentRange mRange;
         std::string         mText;
         ReplaceRangeAction(UtTextDocumentRange aRange, const std::string& aText)
            : mRange(aRange)
            , mText(aText)
         {
         }
         void operator()()
         {
            wizard::TextSource* sourcePtr = (wizard::TextSource*)mRange.mSource;
            if (sourcePtr != nullptr)
            {
               sourcePtr->Replace(mRange, mText);
            }
         }
      };

      WsfParseRule* rulePtr = nodePtr->GetTopLevelRule();
      // if the node is on a platform
      if (rulePtr && rulePtr->GetRuleDescription() == "<Platform>")
      {
         if (strcmp(nodePtr->mType, "icon") == 0)
         {
            UtTextDocumentRange range = nodePtr->SubtreeRange();
            if (range.Valid())
            {
               if (mModelMenu.isEmpty())
               {
                  PopulateModelMenu();
               }
               aMenu.addMenu(&mModelMenu);
               mModelMenu.disconnect();
               QObject::connect(&mModelMenu,
                                &QMenu::triggered,
                                [=](QAction* aAction) { ReplaceRangeAction(range, aAction->text().toStdString())(); });
               return;
            }
         }
      }
      nodePtr = nodePtr->GetParent();
   }
}

void Plugin::OnRequestNameSuggestions(const std::string& aNameType, std::vector<std::string>& aNameList)
{
   if (aNameType == "icon")
   {
      std::set<std::string> namelist = vaEnv.GetModelDatabase()->GetNameList();
      for (auto&& it : namelist)
      {
         aNameList.emplace_back(it);
      }
   }
   else if (aNameType == "side")
   {
      auto* teamPrefs = wkfEnv.GetPreferenceObject<wkf::TeamVisibilityPrefObject>();

      if (teamPrefs)
      {
         for (auto&& it : teamPrefs->GetTeamVisibilityMap())
         {
            aNameList.emplace_back(it.first.toStdString());
         }
      }
   }
}

void Plugin::EpochUpdated(const UtCalendar& aCalendar)
{
   vespa::VaEphemeris::SetStartTime(aCalendar);
}

void Plugin::BuildEntityContextMenu(QMenu* aMenu, wkf::Entity* aEntityPtr)
{
   if (aEntityPtr->IsA_TypeOf<wizard::Platform>())
   {
      wizard::Platform* platform = dynamic_cast<wizard::Platform*>(aEntityPtr);
      if (platform)
      {
         QString name = QString::fromStdString(aEntityPtr->GetName());

         connect(aMenu->addAction(QString("Select %1").arg(name)),
                 &QAction::triggered,
                 [=]() { wizEnv.RequestSelectPlatform(name); });

         wkf::Action* gotoDef = new wkf::Action(QIcon::fromTheme("find_text"), QString("Find Definition"), aMenu);
         aMenu->addAction(gotoDef);
         connect(gotoDef, &QAction::triggered, [platform]() { platform->GotoDefinition(); });

         wkf::Action* deleteAction = new wkf::Action(QIcon::fromTheme("delete"), QString("Delete %1").arg(name), aMenu);
         aMenu->addAction(deleteAction);
         connect(deleteAction, &QAction::triggered, this, [=]() { DeletePlatformActionHandler(aEntityPtr); });

         wkf::Action* cloneAction = new wkf::Action(QIcon::fromTheme("clone"), QString("Clone %1").arg(name), aMenu);
         aMenu->addAction(cloneAction);
         connect(cloneAction, &QAction::triggered, this, [=]() { ClonePlatformActionHandler(platform); });
      }
   }
}

void Plugin::BuildViewerContextMenu(QMenu* aMenu, vespa::VaViewer* aViewerPtr)
{
   if (aViewerPtr == vaEnv.GetStandardViewer())
   {
      auto AddLocationMenu = [aMenu]()
      {
         auto addMenu = aMenu->findChild<QMenu*>("Add at Location", Qt::FindDirectChildrenOnly);
         if (!addMenu)
         {
            addMenu = aMenu->addMenu("Add at Location");
            addMenu->setObjectName("Add at Location");
         }
         return addMenu;
      };

      if (wizard::Project::Instance() != nullptr)
      {
         auto         addMenu      = AddLocationMenu();
         wkf::Action* createAction = new wkf::Action(QIcon::fromTheme("add"), QString("Platform"), addMenu);
         addMenu->addAction(createAction);
         connect(createAction, &QAction::triggered, this, [=]() { CreatePlatformActionHandler(); });
      }
   }
}

void Plugin::PopulateModelMenu()
{
   std::set<std::string> namelist = vaEnv.GetModelDatabase()->GetNameList();
   QList<QAction*>       al;
   for (auto&& name : namelist)
   {
      const vespa::VaModelDefinition* def = vaEnv.GetModelDatabase()->GetModelDefinition(name);
      if (def)
      {
         QMenu*      menu = &mModelMenu;
         std::string cat  = def->GetCategories();
         if (!cat.empty())
         {
            QStringList catlist = QString(def->GetCategories().c_str()).split('/');
            for (auto& cat : catlist)
            {
               QMenu* child = menu->findChild<QMenu*>(cat, Qt::FindDirectChildrenOnly);
               if (child)
               {
                  menu = child;
               }
               else
               {
                  child = menu->addMenu(cat);
                  child->setObjectName(cat);
                  menu = child;
               }
            }
         }
         menu->addAction(name.c_str());
      }
   }
}

void MapUtils::Plugin::DeleteCurrentlySelectedEntities()
{
   //! @brief a wrapper for deleting data
   //! @note many changes to our documents are about to happen.  The proxy informs these changes by finding the nodes of
   //! values we are about to change and telling us where we can find them in file.  Unfortunately once we make a change
   //! in the file, proxy nodes that point to the same file may be invalidated.  To avoid this, we sort the necessary
   //! changes from the bottom of the file to the top, so that early changes never effect the location of later changes.
   class DeleteWrapper : public wizard::ChangeWrapper
   {
   public:
      DeleteWrapper(wizard::Platform*     aPlatformPtr,
                    wkf::AttachmentRoute* aRoutePtr,
                    unsigned int          aIndex,
                    WsfPM_Waypoint&       aWaypointNode)
         : wizard::ChangeWrapper(aPlatformPtr, aRoutePtr, aWaypointNode)
         , mIndex(aIndex)
      {
      }
      DeleteWrapper(wizard::Editor* aEditor, UtTextDocumentRange aRange)
         : wizard::ChangeWrapper(aEditor, aRange)
      {
      }
      DeleteWrapper(wkf::PointOfInterest* aPoiPtr, WsfPProxyNode aProxyNode)
         : wizard::ChangeWrapper(aPoiPtr, aProxyNode)
      {
      }
      void Remove() const
      {
         if (GetPlatform())
         {
            if (GetRoute())
            {
               GetPlatform()->RemoveWaypointFromEditor(GetRoute(), mIndex);
            }
         }
         else if (GetEditor())
         {
            GetEditor()->Replace(GetRange(), "");
         }
      }

   private:
      unsigned int mIndex = 0;
   };

   std::set<DeleteWrapper> sortedDeletes;
   std::set<unsigned int>  platforms;

   vespa::VaHitSet itemList = vaEnv.GetStandardViewer()->GetSelection();
   for (auto&& item : itemList)
   {
      if (item.IsEntity())
      {
         wizard::Platform*     plat = dynamic_cast<wizard::Platform*>(item.FindEntity());
         wkf::PointOfInterest* poi  = dynamic_cast<wkf::PointOfInterest*>(item.FindEntity());
         if (plat)
         {
            std::vector<std::pair<wizard::Editor*, UtTextDocumentRange>> rangeList = plat->GetEditRanges();
            for (auto it : rangeList)
            {
               sortedDeletes.insert(DeleteWrapper(it.first, it.second));
            }
            platforms.insert(plat->GetUniqueId());
         }
         else if (poi)
         {
            WsfPM_Root proxyRoot{wizard::ProxyWatcher::GetActiveProxy()};
            if (proxyRoot)
            {
               WsfPProxyNode proxyPoi{proxyRoot + "pois" + poi->GetName()};
               if (proxyPoi)
               {
                  sortedDeletes.emplace(poi, proxyPoi);
               }
            }
         }
      }
   }
   // 2nd pass for waypoints (so that we can remove those with a platform entry)
   std::map<std::string, std::vector<UtTextDocumentRange>> routeMap;
   for (auto&& item : itemList)
   {
      if (platforms.find(item.EntityId()) == platforms.end()) // if we aren't deleting the platform anyway
      {
         if (item.IsAttachment())
         {
            wkf::AttachmentRoute* rte = dynamic_cast<wkf::AttachmentRoute*>(item.FindAttachment());
            if (rte != nullptr)
            {
               vespa::VaAttachmentSubId sid = item.GetSubId();
               int                      idx = rte->FindWaypointIndex(sid.mSubId);

               auto wizPlat = dynamic_cast<wizard::Platform*>(&rte->GetParent());
               if (wizPlat != nullptr && wizPlat->GetPlatform().IsValid() && wizPlat->GetPlatform().Mover().IsValid())
               {
                  WsfPM_Route route = wizPlat->GetPlatform().Mover().InitialRoute();
                  if (route.IsValid())
                  {
                     WsfPM_Waypoint wayptNode = route.GetWaypoint(idx);
                     if (wayptNode.IsValid())
                     {
                        sortedDeletes.insert(DeleteWrapper(wizPlat, rte, idx, wayptNode));
                     }
                  }
               }
               // if no platform, this is most likely a global route and needs to be handled as such
               else if (wizPlat == nullptr)
               {
                  QString routeAnchor = QString::fromStdString(rte->GetParent().GetName());
                  if (routeAnchor.endsWith("_anchor"))
                  {
                     WsfPM_Root  proxyRoot = wizard::Project::Instance()->Proxy();
                     std::string routeName = rte->GetName();
                     WsfPM_Route curRoute  = WsfPM_RouteMap(proxyRoot + "routes" + routeName);
                     if (curRoute.IsValid())
                     {
                        WsfPM_Waypoint wayptNode = curRoute.GetWaypoint(idx);
                        std::string    fileName =
                           curRoute.GetSourceDocumentRange().mSource->GetFilePath().GetNormalizedPath();
                        wizard::Editor* editorPtr = wizard::Project::Instance()->GotoFile(fileName);

                        // check to see if this route is
                        routeMap[routeName].push_back(wayptNode.GetSourceDocumentRange());

                        if (routeMap[routeName].size() == curRoute.WaypointCount())
                        {
                           // delete the entire route
                           // first remove all the waypoints in the sorted deletes that fall within the range of this route
                           std::set<DeleteWrapper>::iterator sortedDeleteIter = sortedDeletes.begin();
                           while (sortedDeleteIter != sortedDeletes.end())
                           {
                              bool curPtRemoved = false;
                              for (auto curRouteWayptRange : routeMap[routeName])
                              {
                                 if (sortedDeleteIter->GetRange() == curRouteWayptRange)
                                 {
                                    sortedDeleteIter = sortedDeletes.erase(sortedDeleteIter);
                                    curPtRemoved     = true;
                                    break;
                                 }
                              }
                              if (!curPtRemoved)
                              {
                                 ++sortedDeleteIter;
                              }
                           }
                           // now delete the entire route
                           sortedDeletes.emplace(DeleteWrapper(editorPtr, curRoute.GetSourceDocumentRange()));
                        }
                        else
                        {
                           sortedDeletes.emplace(DeleteWrapper(editorPtr, wayptNode.GetSourceDocumentRange()));
                        }
                     }
                  }
               }
            }
         }
      }
   }

   wizEnv.BeginUndoCapture();
   // enact all the changes
   for (auto&& it : sortedDeletes)
   {
      it.Remove();
   }
   wizEnv.EndUndoCapture();
}

} // namespace MapUtils
