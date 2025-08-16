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

#include "PlatformBrowserInterface.hpp"

#include <QMenu>
#include <QMessageBox>

#include "PlatformBrowserSimCommands.hpp"
#include "UtMemory.hpp"
#include "WkPermissions.hpp"
#include "WkPlatform.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"
#include "WkfObserver.hpp"
#include "WkfTeamVisibilityPrefObject.hpp"
#include "WkfVtkEnvironment.hpp"
#include "platform_browser/WkfPlatformBrowserDockWidget.hpp"
#include "platform_browser/WkfPlatformBrowserInterface.hpp"

WkPlatformBrowser::Interface::Interface(SimInterface* aSimInterface)
   : wkf::PlatformBrowserInterface(wkfEnv.GetMainWindow())
{
   mSimInterface = aSimInterface;
   // When the scenario is removed, clear the Platform Browser
   mCallbacks.Add(wkf::Observer::StandardScenarioRemoved.Connect(&Interface::ScenarioRemovedCB, this));
}

void WkPlatformBrowser::Interface::Update()
{
   // There are no events to process
   // mSimInterface->ProcessEvents();
   mDockWidget->UpdateInfo();
}

void WkPlatformBrowser::Interface::DeletePlatform(const QString& aPlatformName)
{
   if (!warlock::HasPermissionToCreateOrRemovePlatforms())
   {
      QMessageBox::warning(nullptr, "Platform not deleted", "User does not have permissions to delete a platform.");
      return;
   }

   warlock::Platform* platform = dynamic_cast<warlock::Platform*>(vaEnv.FindEntity(aPlatformName.toStdString()));
   if (platform && !HasPermissionToControlPlatform(*platform))
   {
      QMessageBox::warning(nullptr,
                           "Platform not deleted",
                           "User does not have permissions to modify platform " +
                              QString::fromStdString(platform->GetName()) + ".");
      return;
   }

   bool ableToDelete = true;
   if (platform != nullptr)
   {
      // Unable to delete an external entity that is not able to be controlled over XIO
      ableToDelete = !(platform->GetExternallyControlled() && !platform->GetXIO_Controlled());
   }

   if (ableToDelete)
   {
      int response = QMessageBox::warning(nullptr,
                                          "Delete Platform",
                                          QString("Are you sure you want to delete %1?").arg(aPlatformName),
                                          QMessageBox::No | QMessageBox::Yes,
                                          QMessageBox::No);
      if (response == QMessageBox::Yes)
      {
         QList<std::string> list;
         list.push_back(aPlatformName.toStdString());
         mSimInterface->AddSimCommand(ut::make_unique<DeletePlatformsCommand>(list));
      }
   }
   else
   {
      QMessageBox::information(nullptr,
                               "Delete Platform",
                               QString("Unable to delete the platform %1 because it is a DIS entity.").arg(aPlatformName));
   }
}

void WkPlatformBrowser::Interface::DeleteCurrentlySelectedPlatforms()
{
   if (!warlock::HasPermissionToCreateOrRemovePlatforms())
   {
      QMessageBox::warning(nullptr, "Platforms not deleted", "User does not have permissions to delete a platform.");
      return;
   }

   QStringList disPlatformList;
   QStringList permissionPlatformList;
   QStringList itemList = mDockWidget->GetSelectedPlatforms();
   for (const QString& item : itemList)
   {
      warlock::Platform* platform = dynamic_cast<warlock::Platform*>(vaEnv.FindEntity(item.toStdString()));
      if (platform != nullptr)
      {
         // Unable to delete an external entity that is not able to be controlled over XIO
         if (platform->GetExternallyControlled() && !platform->GetXIO_Controlled())
         {
            disPlatformList.push_back(item);
         }
         if (!HasPermissionToControlPlatform(*platform))
         {
            permissionPlatformList.push_back(QString::fromStdString(platform->GetName()));
         }
      }
   }

   // If there are no dis platforms in the list of entities to delete then we can delete the entities.
   if (disPlatformList.empty() && permissionPlatformList.empty())
   {
      if (!itemList.empty())
      {
         QList<std::string> platformList;
         QString            message("Are you sure you want to delete the following platform(s)?");

         for (const QString& i : itemList)
         {
            message = message + "\n" + i;
            platformList.push_back(i.toStdString());
         }

         int response =
            QMessageBox::warning(nullptr, "Delete Platform", message, QMessageBox::No | QMessageBox::Yes, QMessageBox::No);
         if (response == QMessageBox::Yes)
         {
            mSimInterface->AddSimCommand(ut::make_unique<DeletePlatformsCommand>(platformList));
         }
      }
   }
   else if (!disPlatformList.empty())
   {
      QString message = "Unable to delete the platforms, because the following platforms are DIS entities:";
      for (QString& name : disPlatformList)
      {
         message = message + "\n" + name;
      }
      QMessageBox::information(nullptr, "Delete Platform", message);
   }
   else
   {
      QString message =
         "Unable to delete the platforms, because the user does not have permission to modify the following platforms:";
      for (QString& name : permissionPlatformList)
      {
         message = message + "\n" + name;
      }
      QMessageBox::information(nullptr, "Delete Platform", message);
   }
}

void WkPlatformBrowser::Interface::ScenarioRemovedCB(wkf::Scenario* aScenarioPtr)
{
   GetDockWidget()->Reset();
}
