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

#include "RouteBrowserPlugin.hpp"

#include <QAction>
#include <QMenu>

#include "CreateGlobalRouteDialog.hpp"
#include "Editor.hpp"
#include "EditorPrefObject.hpp"
#include "ProjectWorkspace.hpp"
#include "ProxyWatcher.hpp"
#include "RouteBrowserPrefObject.hpp"
#include "TextSource.hpp"
#include "WkfScenario.hpp"
#include "WkfVtkEnvironment.hpp"
#include "route/WkfRouteBrowserDialog.hpp"
#include "route/WkfRouteBrowserDockWidget.hpp"

WKF_PLUGIN_DEFINE_SYMBOLS(RouteBrowser::Plugin, "Route Browser", "Creates and Edits Routes.", "wizard")

RouteBrowser::Plugin::Plugin(const QString& aPluginName, const size_t aUniqueId)
   : wizard::Plugin(aPluginName, aUniqueId)
   , mRouteBrowserInterfacePtr(new RouteBrowserInterface())
{
   mPrefWidgetPtr = new PrefWidget();
   connect(mRouteBrowserInterfacePtr->GetDockWidget()->GetEditDialog(),
           &wkf::RouteBrowserEditDialog::Apply,
           this,
           &RouteBrowser::Plugin::RouteChangesApplied);
   connect(mRouteBrowserInterfacePtr->GetDockWidget()->GetEditDialog(),
           &wkf::RouteBrowserDialog::WaypointAdded,
           this,
           &RouteBrowser::Plugin::RouteWaypointAdded);
   connect(mRouteBrowserInterfacePtr->GetDockWidget()->GetEditDialog(),
           &wkf::RouteBrowserDialog::WaypointRemoved,
           this,
           &RouteBrowser::Plugin::RouteWaypointRemoved);
   connect(mRouteBrowserInterfacePtr->GetDockWidget()->GetCreateDialog(),
           &wkf::RouteBrowserCreateDialog::RouteAssignedToPlatform,
           this,
           &RouteBrowser::Plugin::CreateRoute);
   connect(mRouteBrowserInterfacePtr->GetDockWidget()->GetCreateDialog(),
           &wkf::RouteBrowserCreateDialog::GlobalRouteCreated,
           this,
           &RouteBrowser::Plugin::CreateGlobalRoute);
   connect(mRouteBrowserInterfacePtr,
           &wkf::RouteBrowserInterface::DeleteRouteHandler,
           this,
           &RouteBrowser::Plugin::GlobalRouteDeleted);

   connect(mPrefWidgetPtr->GetPreferenceObject(), &PrefObject::PreferencesChanged, this, &Plugin::PreferencesChanged);
}

QList<wkf::PrefWidget*> RouteBrowser::Plugin::GetPreferencesWidgets() const
{
   QList<wkf::PrefWidget*> prefs;
   prefs << mPrefWidgetPtr;
   return prefs;
}

void RouteBrowser::Plugin::RouteChangesApplied(const RouteBrowserInterface::RouteInfo& aRouteInfo)
{
   // check to see if this is a global route
   if (aRouteInfo.mPlatformName.endsWith("_anchor") || aRouteInfo.mPlatformName.isEmpty())
   {
      EditGlobalRoute(aRouteInfo);
   }
   else if (vaEnv.GetStandardScenario())
   {
      wizard::Platform* entityPtr =
         dynamic_cast<wizard::Platform*>(vaEnv.GetStandardScenario()->FindPlatformByName(aRouteInfo.mPlatformName));
      if (entityPtr)
      {
         const auto& attachmentList = entityPtr->GetAttachments();
         for (const auto& curAttachment : attachmentList)
         {
            auto* route = dynamic_cast<wkf::AttachmentRoute*>(curAttachment.get());
            if (route)
            {
               entityPtr->UpdateRoute(aRouteInfo);
               route->SetDraggable(true);
            }
         }
      }
   }
}

void RouteBrowser::Plugin::BuildAttachmentContextMenu(QMenu* aMenu, vespa::VaAttachment* aAttachmentPtr, unsigned int aSubPartId)
{
   wkf::AttachmentRoute* routePtr = dynamic_cast<wkf::AttachmentRoute*>(aAttachmentPtr);
   if (routePtr && routePtr->GetContextMenuEnabled())
   {
      if (aSubPartId != 0)
      {
         int waypointIndex = 0;
         // loop through the route and find the waypoint that was hit
         for (size_t i = 0; i < routePtr->GetWaypoints().size(); ++i)
         {
            wkf::Waypoint* curWaypt = routePtr->GetWaypoint(i);
            if (aSubPartId == curWaypt->GetId())
            {
               QMenu* routeSub = aMenu->findChild<QMenu*>("Route");
               if (!routeSub)
               {
                  routeSub = aMenu->addMenu(QIcon::fromTheme("route"), "Route");
                  routeSub->setObjectName("Route");
               }
               waypointIndex = i;
               QString  name = "Waypoint " + QString::number(waypointIndex);
               QAction* editWaypointAction =
                  new QAction(QIcon::fromTheme("wrench"), QString("Edit %1").arg(name), routeSub);
               routeSub->addAction(editWaypointAction);
               connect(editWaypointAction,
                       &QAction::triggered,
                       this,
                       [=]() { EditWaypointActionHandler(routePtr, waypointIndex); });
            }
         }
      }
   }
}

void RouteBrowser::Plugin::EditWaypointActionHandler(wkf::AttachmentRoute* aAttachmentPtr, unsigned int aWaypointIndex)
{
   if (aAttachmentPtr->GetWaypoints().size() > aWaypointIndex)
   {
      std::string platName = aAttachmentPtr->GetParent().GetName();
      if (mRouteBrowserInterfacePtr->GetDockWidget()->GetEditDialog()->isVisible())
      {
         mRouteBrowserInterfacePtr->GetDockWidget()->GetEditDialog()->reject();
      }
      mRouteBrowserInterfacePtr->GetDockWidget()->GetEditDialog()->PopulateDialog(
         mRouteBrowserInterfacePtr->GetRouteInfo(platName));
      mRouteBrowserInterfacePtr->GetDockWidget()->GetEditDialog()->SelectWaypoint(aWaypointIndex);
      mRouteBrowserInterfacePtr->GetDockWidget()->GetEditDialog()->show();
   }
}

void RouteBrowser::Plugin::RouteWaypointAdded(const wkf::RouteBrowserInterface::WaypointInfo& aWaypoint,
                                              int                                             aIndex,
                                              const QString&                                  aPlatformName,
                                              const QString&                                  aRouteName)
{
   auto* scenarioPtr = vaEnv.GetStandardScenario();
   if (scenarioPtr)
   {
      wizard::Platform* entityPtr = dynamic_cast<wizard::Platform*>(scenarioPtr->FindPlatformByName(aPlatformName));
      if (entityPtr)
      {
         const auto& attachmentList = entityPtr->GetAttachments();
         for (const auto& curAttachment : attachmentList)
         {
            auto* route = dynamic_cast<wkf::AttachmentRoute*>(curAttachment.get());
            if (route)
            {
               entityPtr->AddQueuedOperation(aIndex, true);
            }
         }
      }
   }
}

void RouteBrowser::Plugin::CreateRoute(const QString& aPlatformName, const RouteBrowserInterface::RouteInfo& aRouteInfo)
{
   auto* scenarioPtr = vaEnv.GetStandardScenario();
   if (scenarioPtr)
   {
      wizard::Platform* entityPtr = dynamic_cast<wizard::Platform*>(scenarioPtr->FindPlatformByName(aPlatformName));
      if (entityPtr && entityPtr->IsRouteable())
      {
         entityPtr->AddNewRouteInText(aRouteInfo);
      }
   }
}

void RouteBrowser::Plugin::CreateGlobalRoute(const RouteBrowserInterface::RouteInfo& aGlobalRoute)
{
   if (wizard::Project::Instance() != nullptr)
   {
      WsfPProxy* projProxy = wizard::Project::Instance()->Proxy();
      if (projProxy != nullptr)
      {
         RouteBrowser::CreateGlobalRouteDialog* dlg =
            new RouteBrowser::CreateGlobalRouteDialog(projProxy, (QWidget*)wkfEnv.GetMainWindow());
         if (!mNewRouteSaveFileName.empty())
         {
            dlg->SelectFile(mNewRouteSaveFileName);
         }
         if (dlg->exec())
         {
            mNewRouteSaveFileName = dlg->GetSelectedFile();
         }
         else
         {
            mRouteBrowserInterfacePtr->GetDockWidget()->RemoveGlobal(aGlobalRoute.mName);
         }
      }

      wizard::TextSource* src = wizard::ProjectWorkspace::Instance()->FindSource(mNewRouteSaveFileName, false, false);
      if (src)
      {
         QString routeDef = GetGlobalRouteText(aGlobalRoute);
         if (src->GetViews().empty())
         {
            src->NewView(wizard::Project::Instance());
         }
         src->Append(routeDef.toStdString());
         mRouteBrowserInterfacePtr->GlobalRouteCreated(aGlobalRoute);
      }
   }
}

void RouteBrowser::Plugin::EditGlobalRoute(const RouteBrowserInterface::RouteInfo& aGlobalRoute)
{
   WsfPM_Root  proxyRoot(wizard::ProxyWatcher::GetActiveProxy());
   WsfPM_Route curRoute = WsfPM_Route(proxyRoot + "routes" + aGlobalRoute.mName.toStdString());
   if (curRoute.IsValid() && (curRoute.WaypointCount() > 0))
   {
      QString routeDef = GetGlobalRouteText(aGlobalRoute);

      UtTextDocumentRange routeTextRange = curRoute.GetSourceDocumentRange();
      std::string         fileName       = curRoute.GetSourceDocumentRange().mSource->GetFilePath().GetNormalizedPath();
      wizard::Editor*     editorPtr      = wizard::Project::Instance()->GotoFile(fileName);
      if (editorPtr != nullptr)
      {
         QTextCursor cursor = editorPtr->textCursor();
         cursor.setPosition(routeTextRange.GetBegin());
         cursor.setPosition(routeTextRange.GetEnd() + 1, QTextCursor::KeepAnchor);
         editorPtr->setTextCursor(cursor);
         cursor.insertText(routeDef);
      }
   }
}

void RouteBrowser::Plugin::RouteWaypointRemoved(int aIndex, const QString& aPlatformName, const QString& aRouteName)
{
   auto* scenarioPtr = vaEnv.GetStandardScenario();
   if (scenarioPtr)
   {
      wizard::Platform* entityPtr = dynamic_cast<wizard::Platform*>(scenarioPtr->FindPlatformByName(aPlatformName));
      if (entityPtr)
      {
         const auto& attachmentList = entityPtr->GetAttachments();
         for (const auto& curAttachment : attachmentList)
         {
            auto* route = dynamic_cast<wkf::AttachmentRoute*>(curAttachment.get());
            if (route)
            {
               entityPtr->AddQueuedOperation(aIndex, false);
            }
         }
      }
   }
}
void RouteBrowser::Plugin::PlatformDeleted(const QString& aPlatformName)
{
   if (mRouteBrowserInterfacePtr->GetDockWidget()->GetEditDialog()->isVisible())
   {
      if (mRouteBrowserInterfacePtr->GetDockWidget()->GetEditDialog()->GetRouteInfo().mPlatformName == aPlatformName)
      {
         mRouteBrowserInterfacePtr->GetDockWidget()->GetEditDialog()->close();
      }
   }
   else if (mRouteBrowserInterfacePtr->GetDockWidget()->GetCreateDialog()->isVisible())
   {
      // force close create dialog no matter what platform is deleted, this will force the dialog to properly add/remove
      // valid platforms to attach new routes to when the dialog gets reopened.
      mRouteBrowserInterfacePtr->GetDockWidget()->GetCreateDialog()->close();
   }
}

void RouteBrowser::Plugin::PreferencesChanged()
{
   mRouteBrowserInterfacePtr->UpdateAllGlobalRoutes();

   auto* dockwidget = mRouteBrowserInterfacePtr->GetDockWidget();
   auto* prefs      = wkfEnv.GetPreferenceObject<RouteBrowser::PrefObject>();

   dockwidget->GetCreateDialog()->SetDefaultRoutePrefix(prefs->GetDefaultRouteName());
   if (prefs->GetUsingDefaultWaypointLabels())
   {
      dockwidget->GetCreateDialog()->SetDefaultWaypointPrefix(prefs->GetDefaultWaypointLabel());
      dockwidget->GetEditDialog()->SetDefaultWaypointPrefix(prefs->GetDefaultWaypointLabel());
   }
   else
   {
      dockwidget->GetCreateDialog()->SetDefaultWaypointPrefix("");
      dockwidget->GetEditDialog()->SetDefaultWaypointPrefix("");
   }
}

void RouteBrowser::Plugin::GlobalRouteDeleted(const QString& aRouteName)
{
   WsfPM_Root  proxyRoot(wizard::ProxyWatcher::GetActiveProxy());
   WsfPM_Route curRoute = WsfPM_Route(proxyRoot + "routes" + aRouteName.toStdString());
   if (curRoute.IsValid() && (curRoute.WaypointCount() > 0))
   {
      UtTextDocumentRange routeTextRange = curRoute.GetSourceDocumentRange();
      std::string         fileName       = curRoute.GetSourceDocumentRange().mSource->GetFilePath().GetNormalizedPath();
      wizard::Editor*     editorPtr      = wizard::Project::Instance()->GotoFile(fileName);
      if (editorPtr != nullptr)
      {
         editorPtr->Replace(routeTextRange, "");
      }
   }
}

QString RouteBrowser::Plugin::GetGlobalRouteText(const RouteBrowserInterface::RouteInfo& aGlobalRoute) const
{
   QString routeDef = "\n";
   routeDef += "route ";
   routeDef += aGlobalRoute.mName;
   routeDef += "\n";

   QString      tabSpacesStr;
   unsigned int tabSpace = wkfEnv.GetPreferenceObject<wizard::EditorPrefObject>()->TabSpace();
   for (unsigned int i = 0; i < tabSpace; i++)
   {
      tabSpacesStr += " ";
   }
   // int curWayptIdx = 0;
   for (auto curWaypt : aGlobalRoute.mWaypoints)
   {
      // check for a label first
      if (curWaypt.mLabel != "")
      {
         // label is set
         QString labelStr = "\n" + tabSpacesStr + "label ";
         labelStr += curWaypt.mLabel;
         routeDef += labelStr;
      }

      // write out the position
      vespa::VaPosition newPos(curWaypt.mLat, curWaypt.mLon, curWaypt.mAlt);
      QString           posStr = "\n" + tabSpacesStr + "position ";

      std::ostringstream ossLat;
      ossLat << newPos.GetLat();
      std::ostringstream ossLon;
      ossLon << newPos.GetLon();
      std::ostringstream ossAlt;
      ossAlt << newPos.GetAlt();

      posStr.append(QString(ossLat.str().c_str()));
      posStr.append(" ");
      posStr.append(QString(ossLon.str().c_str()));
      posStr.append(" altitude ");
      posStr.append(QString(ossAlt.str().c_str()));

      // Append the altitude reference
      if (curWaypt.mAltRef == static_cast<int>(RouteBrowserInterface::WaypointInfo::AltitudeReference::AGL))
      {
         posStr.append(" agl");
      }
      else if (curWaypt.mAltRef == static_cast<int>(RouteBrowserInterface::WaypointInfo::AltitudeReference::MSL))
      {
         posStr.append(" msl");
      }

      routeDef += posStr;

      if (curWaypt.mSpeed != wkf::WaypointValueType::cNOT_SET && curWaypt.mSpeed != wkf::WaypointValueType::cUSE_PREVIOUS &&
          curWaypt.mSpeed != wkf::WaypointValueType::cUSE_DEFAULT)
      {
         // speed is set
         QString speedStr = "\n" + tabSpacesStr + tabSpacesStr + "speed ";
         speedStr += QString::number(curWaypt.mSpeed.GetAsUnit());
         speedStr += " ";
         speedStr += curWaypt.mSpeed.GetUnitName().c_str();
         routeDef += speedStr;
      }
      if (curWaypt.mClimbRate != wkf::WaypointValueType::cNOT_SET &&
          curWaypt.mClimbRate != wkf::WaypointValueType::cUSE_PREVIOUS &&
          curWaypt.mClimbRate != wkf::WaypointValueType::cUSE_DEFAULT)
      {
         // climbe_rate is set
         QString climbRateStr = "\n" + tabSpacesStr + tabSpacesStr + "climb_rate ";
         climbRateStr += QString::number(curWaypt.mClimbRate.GetAsUnit());
         climbRateStr += " ";
         climbRateStr += curWaypt.mClimbRate.GetUnitName().c_str();
         routeDef += climbRateStr;
      }
      if (curWaypt.mLinAccel != wkf::WaypointValueType::cNOT_SET &&
          curWaypt.mLinAccel != wkf::WaypointValueType::cUSE_PREVIOUS &&
          curWaypt.mLinAccel != wkf::WaypointValueType::cUSE_DEFAULT)
      {
         // linear_acceleration is set
         QString linAccelStr = "\n" + tabSpacesStr + tabSpacesStr + "linear_acceleration ";
         linAccelStr += QString::number(curWaypt.mLinAccel.GetAsUnit());
         linAccelStr += " ";
         linAccelStr += curWaypt.mLinAccel.GetUnitName().c_str();
         routeDef += linAccelStr;
      }
      // radial_acceleration/Bank Angle (bank_angle_limit)/ Maximum G's (turn_g_limit)
      if (curWaypt.mRadAccel != wkf::WaypointValueType::cNOT_SET &&
          curWaypt.mRadAccel != wkf::WaypointValueType::cUSE_PREVIOUS &&
          curWaypt.mRadAccel != wkf::WaypointValueType::cUSE_DEFAULT)
      {
         // radial_acceleration is set
         QString radAccelStr = "\n" + tabSpacesStr + tabSpacesStr + "radial_acceleration ";
         radAccelStr += QString::number(curWaypt.mRadAccel.GetAsUnit());
         radAccelStr += " ";
         radAccelStr += curWaypt.mRadAccel.GetUnitName().c_str();
         routeDef += radAccelStr;
      }

      // Turn Direction (turn left, turn right, turn_to_heading)
      if (curWaypt.mTurnDir != 0)
      {
         // turn direction is set
         QString turnDirStr = "\n" + tabSpacesStr + tabSpacesStr;
         if (curWaypt.mTurnDir == -1)
         {
            turnDirStr += "turn left";
         }
         else if (curWaypt.mTurnDir == 1)
         {
            turnDirStr += "turn right";
         }
         routeDef += turnDirStr;
      }

      // Goto needs to be the last attribute added to avoid errors
      if (curWaypt.mGoTo != "")
      {
         // goto is set
         QString gotoStr = "\n" + tabSpacesStr + tabSpacesStr + "goto ";
         gotoStr += curWaypt.mGoTo;
         routeDef += gotoStr;
      }
   }
   routeDef += "\nend_route\n";

   return routeDef;
}
