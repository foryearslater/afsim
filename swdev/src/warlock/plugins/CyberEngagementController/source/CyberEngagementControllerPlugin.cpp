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

#include "CyberEngagementControllerPlugin.hpp"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <UtMemory.hpp>

#include "CyberEngagementControllerPlugin.hpp"
#include "CyberEngagementControllerSimCommand.hpp"
#include "WkPermissions.hpp"
#include "WkfEnvironment.hpp"
#include "WkfVtkEnvironment.hpp"

WKF_PLUGIN_DEFINE_SYMBOLS(WkCyberEngagementController::Plugin,
                          "Cyber Engagement Controller",
                          "The Cyber Engagement Controller plugin provides the operator the ability to initiate cyber "
                          "attacks and scans through the platform-context menu.",
                          "warlock")

WkCyberEngagementController::Plugin::Plugin(const QString& aPluginName, const size_t aUniqueId)
   : warlock::PluginT<SimInterface>(aPluginName, aUniqueId)
   , mCyberEngagementControllerData()
{
}

void WkCyberEngagementController::Plugin::GuiUpdate()
{
   mInterfacePtr->ProcessEvents(mCyberEngagementControllerData);
}

void WkCyberEngagementController::Plugin::BuildEntityContextMenu(QMenu* aMenu, wkf::Entity* aEntityPtr)
{
   std::vector<std::string> attackTypes = mCyberEngagementControllerData.GetTypes();
   // Only show menus if the entity is a wkfPlatform and there are attack types to show
   if (aEntityPtr->IsA_TypeOf<wkf::Platform>() && !attackTypes.empty() && !wkfEnv.GetSelectedPlatforms().empty())
   {
      QMenu* cyberEffectsMenu = new QMenu("Initiate Cyber Engagement", aMenu);
      QMenu* attackMenu       = new QMenu("Attack", aMenu);
      QMenu* scanMenu         = new QMenu("Scan", aMenu);
      aMenu->addMenu(cyberEffectsMenu);
      cyberEffectsMenu->addMenu(attackMenu);
      cyberEffectsMenu->addMenu(scanMenu);

      for (const auto& type : attackTypes)
      {
         // Add options to attack menu
         QAction* attackAction = new QAction(QString::fromStdString(type), attackMenu);
         connect(attackAction,
                 &QAction::triggered,
                 this,
                 [=]()
                 {
                    for (const auto& platform : wkfEnv.GetSelectedPlatforms())
                    {
                       if (!warlock::HasPermissionToControlPlatform(*platform))
                       {
                          QMessageBox::warning(nullptr,
                                               "Attack Not Executed",
                                               "User does not have permissions to modify platform " +
                                                  QString::fromStdString(platform->GetName()) + ".");
                       }
                       else
                       {
                          mInterfacePtr->AddSimCommand(
                             ut::make_unique<CyberEngagementControllerCommand>(type,
                                                                               platform->GetName(),
                                                                               aEntityPtr->GetName()));
                       }
                    }
                 });
         attackMenu->addAction(attackAction);

         // Add options to scan menu
         QAction* scanAction = new QAction(QString::fromStdString(type), scanMenu);
         connect(scanAction,
                 &QAction::triggered,
                 this,
                 [=]()
                 {
                    for (const auto& platform : wkfEnv.GetSelectedPlatforms())
                    {
                       if (!warlock::HasPermissionToControlPlatform(*platform))
                       {
                          QMessageBox::warning(nullptr,
                                               "Scan Not Executed",
                                               "User does not have permissions to modify platform " +
                                                  QString::fromStdString(platform->GetName()) + ".");
                       }
                       else
                       {
                          mInterfacePtr->AddSimCommand(
                             ut::make_unique<CyberScanCommand>(type, platform->GetName(), aEntityPtr->GetName()));
                       }
                    }
                 });
         scanMenu->addAction(scanAction);
      } // end for loop
   }    // end platform check && !attackType.empty
}
