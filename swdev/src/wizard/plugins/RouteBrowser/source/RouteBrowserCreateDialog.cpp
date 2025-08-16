// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2021 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************


#include "RouteBrowserCreateDialog.hpp"

#include "ProxyWatcher.hpp"
#include "WkfEnvironment.hpp"
#include "WkfUnitsObject.hpp"
#include "WsfPM_Root.hpp"
#include "map/Platform.hpp"

RouteBrowser::RouteBrowserCreateDialog::RouteBrowserCreateDialog(QWidget* aParent, Qt::WindowFlags aFlags)
   : wkf::RouteBrowserCreateDialog(aParent, aFlags)
{
   connect(&wkfEnv,
           QOverload<const wkf::PlatformList&, const wkf::EntityList&, const vespa::VaAttachmentList&>::of(
              &wkf::Environment::MoveSelectionAndAttachmentComplete),
           this,
           &RouteBrowser::RouteBrowserCreateDialog::MoveSelectionAndAttachmentComplete);
}

void RouteBrowser::RouteBrowserCreateDialog::PopulateDialog()
{
   mRouteNameEdit->setValidator(mValidator);

   mCurrentRoute = wkf::RouteBrowserInterface::RouteInfo();

   mPlatformComboBox->clear();
   mPlatformComboBox->addItem("none");
   for (const auto& p : mPlatforms)
   {
      mPlatformComboBox->addItem(p);
   }

   RouteInfoInit(); // initialize all fields with "0" values
   ClearTree();     // clear the left side of dialog

   // if there is only one platform currently selected go ahead and pre-set the platform combo box to this platform
   // assuming the user wants to create a route for this selected platform: also populate the first waypoint with
   // position if it is defined
   wkf::PlatformList selectedPlatforms = wkfEnv.GetSelectedPlatforms();
   if (selectedPlatforms.size() == 1)
   {
      mPlatformComboBox->setCurrentText(QString::fromStdString(selectedPlatforms[0]->GetName()));
      vespa::VaPosition pos     = selectedPlatforms[0]->GetPosition();
      auto              wizPlat = dynamic_cast<wizard::Platform*>(selectedPlatforms[0]);
      if (wizPlat && !wizPlat->GetPlatform().Position().IsUnset())
      {
         emit CreateRouteStarted(QString::fromStdString(selectedPlatforms[0]->GetName()));

         auto           proxyPtr = wizard::ProxyWatcher::GetActiveProxy();
         WsfPM_Root     rootNode{proxyPtr};
         WsfPM_Platform proxyPlatform = rootNode.platforms() + selectedPlatforms[0]->GetName();

         auto alt = UtLengthValue(proxyPlatform.EffectiveAltitude());
         AppendWaypoint(pos.GetLat(), pos.GetLon(), alt);

         if (proxyPlatform.IsValid())
         {
            if (proxyPlatform.EffectiveAGL() == wsf::proxy::AltitudeReferenceEnum::AGL)
            {
               // this platform is using agl
               mUi.mAltRefEdit->setCurrentIndex(2);
               mCurrentRoute.mWaypoints[mCurrentWaypoint].mAltRef =
                  static_cast<int>(wkf::RouteBrowserInterface::WaypointInfo::AltitudeReference::AGL);
            }
            else
            {
               // In the WsfProxy, values can be either unset or set.  In the unset case, the WsfProxy usually uses a
               // default value. In this case, figure out if the altitude reference is defined explicitly. If it is not
               // defined explicitly, then set the combo box to "Mover Default"
               UtTextDocumentRange altNodeRange = proxyPlatform.AltitudeNode().GetSourceDocumentRange();
               auto                curText      = altNodeRange.Text();
               if (curText.rfind("msl") != std::string::npos)
               {
                  // msl is specifically defined
                  mUi.mAltRefEdit->setCurrentIndex(1);
                  mCurrentRoute.mWaypoints[mCurrentWaypoint].mAltRef =
                     static_cast<int>(wkf::RouteBrowserInterface::WaypointInfo::AltitudeReference::MSL);
               }
               else
               {
                  // set to default mover altitude reference
                  mUi.mAltRefEdit->setCurrentIndex(0);
                  mCurrentRoute.mWaypoints[mCurrentWaypoint].mAltRef =
                     static_cast<int>(wkf::RouteBrowserInterface::WaypointInfo::AltitudeReference::MoverDefault);
               }
            }
         }
      }
   }
   show();
}

void RouteBrowser::RouteBrowserCreateDialog::AppendWaypoint(double aLatitude, double aLongitude, UtLengthValue aAltitude)
{
   int                                      insertIndex = mCurrentRoute.mWaypoints.size();
   wkf::RouteBrowserInterface::WaypointInfo insertWaypoint(insertIndex != 0);

   insertWaypoint.mLat = aLatitude;
   insertWaypoint.mLon = aLongitude;

   if (mCurrentRoute.mWaypoints.empty())
   {
      insertWaypoint.mAltRef = mUi.mAltRefEdit->currentIndex();
      auto unitName          = aAltitude.GetUnitName();
      if (unitName.empty())
      {
         unitName = "meters";
      }
      double alt = aAltitude;
      // This is to catch garbage values due to uninitialized altitude variable.
      if (UtMath::NearlyZero(alt, 1e-6))
      {
         insertWaypoint.mAlt.Set(0.0, unitName);
      }
      else
      {
         insertWaypoint.mAlt.Set(aAltitude.GetAsUnit(), unitName);
      }
   }
   else
   {
      wkf::RouteBrowserInterface::WaypointInfo previousWaypoint = mCurrentRoute.mWaypoints.last();
      insertWaypoint.mAlt                                       = previousWaypoint.mAlt;
      insertWaypoint.mAltRef                                    = previousWaypoint.mAltRef;
   }
   insertWaypoint.mLabel = GetDefaultWaypointLabel(insertIndex);

   mCurrentRoute.mWaypoints.push_back(insertWaypoint);
   mCurrentWaypoint = insertIndex;

   bool successfullyAdded = true;

   // Check if the previous waypoint and this new waypoint have the same latitude/longitude
   if (mCurrentWaypoint > 0)
   {
      successfullyAdded = ResolveCoincidentWaypoints(mCurrentWaypoint, mCurrentWaypoint - 1);
   }

   // Update displays
   PopulateTree(mCurrentRoute, mCurrentWaypoint);
   if (successfullyAdded)
   {
      emit WaypointAdded(insertWaypoint, insertIndex, mCurrentRoute.mPlatformName, mCurrentRoute.mName);
   }

   if (IsCurrentRouteGlobal())
   {
      emit GlobalRouteEdited(mCurrentRoute);
   }
   else
   {
      emit RouteEdited(mCurrentRoute);
   }
}


void RouteBrowser::RouteBrowserCreateDialog::MoveSelectionAndAttachmentComplete(const wkf::PlatformList& aPlatformList,
                                                                                const wkf::EntityList& aNonPlatformList,
                                                                                const vespa::VaAttachmentList& aAttachmentList)
{
   if (isVisible())
   {
      for (auto&& curPlatform : aPlatformList)
      {
         if (curPlatform->GetName() == mCurrentRoute.mPlatformName.toStdString())
         {
            UtLLAPos newlla;
            curPlatform->GetPosition().GetLLA(newlla);

            SelectWaypoint(0);
            SetCurrentWaypoint(0);
            mCurrentRoute.mWaypoints[0].mLat = newlla.GetLat();
            mCurrentRoute.mWaypoints[0].mLon = newlla.GetLon();
            mCurrentRoute.mWaypoints[0].mAlt = newlla.GetAlt();
            mUi.mLatEdit->SetValue(newlla.GetLat());
            mUi.mLonEdit->SetValue(newlla.GetLon());

            WaypointLatEdited(mCurrentRoute.mPlatformName, mCurrentRoute.mName, newlla.GetLat(), 0);
            WaypointLonEdited(mCurrentRoute.mPlatformName, mCurrentRoute.mName, newlla.GetLon(), 0);
            WaypointAltEdited(mCurrentRoute.mPlatformName, mCurrentRoute.mName, newlla.GetAlt(), 0);

            // find the platform's route attachments
            for (auto&& curRoute : curPlatform->FindAttachmentsOfType<wkf::AttachmentRoute>())
            {
               emit TempRouteEdited(mCurrentRoute, curRoute);
            }
         }
      }
      for (auto&& curAttachment : aAttachmentList)
      {
         if (curAttachment->IsA_TypeOf<wkf::AttachmentRoute>())
         {
            if (curAttachment->GetParent().GetName() == "Temporary_Route")
            {
               auto curRoute = dynamic_cast<wkf::AttachmentRoute*>(curAttachment);
               for (size_t idx = 0; idx < curRoute->GetWaypoints().size(); ++idx)
               {
                  if (curRoute->GetWaypoint(idx)->IsSelected())
                  {
                     emit TempRouteEdited(mCurrentRoute, curRoute);
                     break;
                  }
               }
            }
         }
      }
   }
}

void RouteBrowser::RouteBrowserCreateDialog::showEvent(QShowEvent* aEvent)
{
   // lock down all platforms while the route create dialog is open.  Moving them causes too much route rebuilding and
   // leads to bad places.
   vespa::VaEntityList platformList;
   vaEnv.GetEntityListOfType<wizard::Platform>(platformList);
   for (auto&& curPlatform : platformList)
   {
      curPlatform->SetDraggable(false);
   }
}

void RouteBrowser::RouteBrowserCreateDialog::hideEvent(QHideEvent* aEvent)
{
   // unlock all platforms
   vespa::VaEntityList platformList;
   vaEnv.GetEntityListOfType<wizard::Platform>(platformList);
   for (auto&& curPlatform : platformList)
   {
      curPlatform->SetDraggable(true);
   }
}
