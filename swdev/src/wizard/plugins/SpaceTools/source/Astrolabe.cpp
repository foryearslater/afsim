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

#include "Astrolabe.hpp"

#include <algorithm>
#include <sstream>
#include <string>

#include <QMenu>
#include <QMessageBox>
#include <QString>

#include "AstrolabeUI.hpp"
#include "InputReader.hpp"
#include "InputWriter.hpp"
#include "MissionVerificationResults.hpp"
#include "MissionVerifier.hpp"
#include "OrbitalMissionModel.hpp"
#include "Project.hpp"
#include "ProjectWorkspace.hpp"
#include "ProxyChange.hpp"
#include "ProxyWatcher.hpp"
#include "Signals.hpp"
#include "SpaceToolsScenario.hpp"
#include "UtMemory.hpp"
#include "WkfEntity.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"
#include "WsfPM_Root.hpp"
#include "WsfPProxyNode.hpp"

extern void Register_wsf_space(WsfApplication& aApplication);

SpaceTools::Astrolabe::Astrolabe()
   : mConfig{false}
   , mAstrolabeUI_Ptr{nullptr}
   , mProjectPtr{nullptr}
   , mProxyModified{}
{
   if (mConfig.IsValid())
   {
      wkf::MainWindow* mainWindowPtr = wkfEnv.GetMainWindow();

      mAstrolabeUI_Ptr = new AstrolabeUI{mConfig, mainWindowPtr};
      mainWindowPtr->addDockWidget(Qt::RightDockWidgetArea, mAstrolabeUI_Ptr);
      mAstrolabeUI_Ptr->hide();

      connect(mAstrolabeUI_Ptr, &AstrolabeUI::PlatformRequested, this, &Astrolabe::LoadPlatform);
      connect(wizSignals, &wizard::Signals::ActionsForNodeRequested, this, &Astrolabe::ActionsForNodeRequested);
      connect(mAstrolabeUI_Ptr, &AstrolabeUI::OutputRequested, this, &Astrolabe::ProduceOutput);
      connect(mAstrolabeUI_Ptr, &AstrolabeUI::VerificationRequested, this, &Astrolabe::Verify);
      connect(mAstrolabeUI_Ptr, &AstrolabeUI::visibilityChanged, this, &Astrolabe::DockVisibilityChanged);
   }
}

void SpaceTools::Astrolabe::BuildEntityContextMenu(QMenu* aMenuPtr, wkf::Entity* aEntityPtr)
{
   if (aEntityPtr->IsA_TypeOf<wkf::Platform>())
   {
      auto platformName = QString::fromStdString(aEntityPtr->GetName());
      if (IsSpacePlatform(platformName))
      {
         QAction* actionPtr = new QAction(QString("Open with Astrolabe"), aMenuPtr);
         actionPtr->setIcon(QIcon::fromTheme("satellite"));
         aMenuPtr->addAction(actionPtr);
         connect(actionPtr, &QAction::triggered, [this, platformName]() { ShowToolForPlatform(platformName); });
      }
   }
}

void SpaceTools::Astrolabe::OnProjectOpened(wizard::Project* aProjectPtr)
{
   mProjectPtr = aProjectPtr;
}

void SpaceTools::Astrolabe::OnProjectClosed(wizard::Project* aProjectPtr)
{
   mProjectPtr = nullptr;
   if (mAstrolabeUI_Ptr != nullptr)
   {
      mAstrolabeUI_Ptr->close();
   }
}

void SpaceTools::Astrolabe::OnFullParseComplete(wizard::Project* aProjectPtr)
{
   PopulateSpacePlatforms();
}

//! Show the Astrolabe tool with the given platform's sequence loaded.
//!
//! If there was trouble loading the Astrolabe configuration file, then this
//! will instead provide a message box indicating the error that was encountered.
void SpaceTools::Astrolabe::ShowToolForPlatform(const QString& aPlatformName)
{
   if (mAstrolabeUI_Ptr != nullptr)
   {
      mAstrolabeUI_Ptr->show();
      mAstrolabeUI_Ptr->activateWindow();
      LoadPlatform(aPlatformName);
   }
   else
   {
      int result =
         QMessageBox::warning(nullptr,
                              QString("Astrolabe Error"),
                              QString("Error in Astrolabe configuration file. Would you like to see more information?"),
                              QMessageBox::Yes | QMessageBox::No);
      if (result == QMessageBox::Yes)
      {
         QMessageBox::information(nullptr,
                                  QString("Astrolabe Error"),
                                  QString::fromStdString(mConfig.GetStatusMessage()),
                                  QMessageBox::Ok);
      }
   }
}

//! Load the data for the given platform into the UI.
void SpaceTools::Astrolabe::LoadPlatform(const QString& aPlatformName)
{
   Scenario    scenario{};
   InputReader reader{scenario.GetScenario()};
   auto        modelPtr = reader.GetPlatformMissionModel(aPlatformName.toStdString());
   if (modelPtr != nullptr)
   {
      mAstrolabeUI_Ptr->SelectPlatform(aPlatformName);
      mAstrolabeUI_Ptr->SetOrbitalMissionModel(modelPtr.release());
   }
   else
   {
      mAstrolabeUI_Ptr->SelectPlatform(QString{});
      mAstrolabeUI_Ptr->SetOrbitalMissionModel(nullptr);
      if (!aPlatformName.isEmpty())
      {
         mAstrolabeUI_Ptr->SetErrorMessage(QString{"Unable to load mission sequence for platform '"} + aPlatformName +
                                           QString{"'."});
      }
   }
}

//! Start watching the proxy to detect when space platforms are added or removed.
//!
//! When the tool is shown, changes to the proxy are observed so that the list of
//! platforms in the UI are kept up to date with the input stack.
void SpaceTools::Astrolabe::StartWatchingProxy()
{
   WsfPProxy*        proxyPtr = wizard::ProxyWatcher::GetActiveProxy();
   WsfPM_Root        root(proxyPtr);
   WsfPM_PlatformMap platforms = root.platforms();
   mProxyModified.Add(wizard::WatchProxy(platforms.GetPath()).ChildUpdate.Connect(&Astrolabe::OnProxyModified, this));
}

//! Stop watching the proxy for changes in the set of space-domain platforms.
//!
//! When the tool is not shown, there is no reason to keep the list of space-platforms
//! up to date. So the observation is ended.
void SpaceTools::Astrolabe::StopWatchingProxy()
{
   mProxyModified.Clear();
}

//! Produce the mission sequence commands into the input stack.
void SpaceTools::Astrolabe::ProduceOutput()
{
   InputWriter writer{*mProjectPtr};
   writer.ProduceOutput(mAstrolabeUI_Ptr->GetSelectedPlatform().toStdString(), mAstrolabeUI_Ptr->GetMission());
   mAstrolabeUI_Ptr->SetStatusMessage(QString{"Mission sequence written out for platform '"} +
                                      mAstrolabeUI_Ptr->GetSelectedPlatform() + QString{"'."});
}

//! Verify the mission sequence.
void SpaceTools::Astrolabe::Verify()
{
   try
   {
      MissionVerifier                          verifier{};
      wsfg::spaceg::MissionVerificationResults results =
         verifier.Verify(mAstrolabeUI_Ptr->GetSelectedPlatform().toStdString(),
                         mAstrolabeUI_Ptr->GetMission()->GetMissionData());
      mAstrolabeUI_Ptr->SetMissionVerificationResults(results);
   }
   catch (std::runtime_error& err)
   {
      mAstrolabeUI_Ptr->SetErrorMessage(QString{"Error during verification."});
      QMessageBox::critical(nullptr, QString("Verification Error"), QString::fromStdString(err.what()), QMessageBox::Ok);
   }
}

void SpaceTools::Astrolabe::DockVisibilityChanged(bool aIsVisble)
{
   if (aIsVisble)
   {
      PopulateSpacePlatforms();
      StartWatchingProxy();
   }
   else
   {
      StopWatchingProxy();
   }
}

//! Add Astrolabe to context menu for platforms with space movers.
void SpaceTools::Astrolabe::ActionsForNodeRequested(const WsfPProxyNode& aNode, QMenu* aMenuPtr, QWidget* aParentWidgetPtr)
{
   WsfPM_Root rootNode{aNode.GetProxy()};
   if (!rootNode.platforms().FindT(aNode.GetName()).IsNull())
   {
      WsfPM_Platform platNode{aNode};
      if (platNode.Mover().IsOfType("WSF_SPACE_MOVER_BASE"))
      {
         QString  platformName = QString::fromStdString(aNode.GetName());
         QAction* actionPtr    = aMenuPtr->addAction(QIcon::fromTheme("satellite"), QString{"Open with Astrolabe"});
         QObject::connect(actionPtr, &QAction::triggered, [this, platformName]() { ShowToolForPlatform(platformName); });
      }
   }
}

void SpaceTools::Astrolabe::OnProxyModified(const wizard::ProxyChange& aProxyChange)
{
   // NOTE: There are some transformations of the proxy that are not captured below.
   // For example, if a platform is modified by changing the type of mover into a
   // space domain mover then this will not add the platform to the Astrolabe UI.

   auto reason = aProxyChange.reason();
   if (reason == wizard::ProxyChange::Reason::cADDED)
   {
      WsfPM_Platform platNode{aProxyChange.changed()};
      if (platNode.IsValid() && InputReader::IsSpacePlatform(platNode))
      {
         mAstrolabeUI_Ptr->PlatformAdded(QString::fromStdString(platNode.GetName()));
      }
   }
   else if (reason == wizard::ProxyChange::Reason::cREMOVED)
   {
      mAstrolabeUI_Ptr->PlatformRemoved(QString::fromStdString(aProxyChange.changed().GetName()));
   }
}

void SpaceTools::Astrolabe::PopulateSpacePlatforms()
{
   QStringList spacePlatforms = InputReader::GetAllSpacePlatforms();
   mAstrolabeUI_Ptr->PopulateSpacePlatforms(spacePlatforms);
}

bool SpaceTools::Astrolabe::IsSpacePlatform(const QString& aPlatformName) const
{
   return InputReader::IsSpacePlatform(aPlatformName.toStdString());
}
