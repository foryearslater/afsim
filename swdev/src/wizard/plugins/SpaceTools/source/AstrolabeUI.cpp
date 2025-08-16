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

#include "AstrolabeUI.hpp"

#include <QAbstractButton>
#include <QMenu>
#include <QMessageBox>

#include "AstrolabeConfig.hpp"
#include "AstrolabeMissionView.hpp"
#include "Environment.hpp"
#include "MissionVerificationResults.hpp"
#include "OrbitalMissionModel.hpp"
#include "OrbitalModelToSequence.hpp"
#include "Platform.hpp"
#include "PlatformSelector.hpp"
#include "VaEnvironment.hpp"
#include "WkfScenario.hpp"
#include "WkfVtkEnvironment.hpp"
#include "WsfStandardApplication.hpp"
#include "orbit/WkfAttachmentOrbit.hpp"

SpaceTools::AstrolabeUI::AstrolabeUI(const wsfg::spaceg::AstrolabeConfig& aConfig, QWidget* aParentPtr, Qt::WindowFlags aFlags)
   : AstrolabeDockWidgetBase(aConfig, aParentPtr, aFlags)
   , mReloadButtonPtr{new QToolButton{this}}
   , mScenarioPtr{nullptr}
{
   QAction* reloadActionPtr = new QAction{QIcon::fromTheme("undo"), QString("Reload Sequence From Input"), this};
   mReloadButtonPtr->setDefaultAction(reloadActionPtr);
   connect(reloadActionPtr, &QAction::triggered, this, &AstrolabeUI::HandleReload);
   mUI.mSelectorLayout->addWidget(mReloadButtonPtr);

   mUI.mDiscardButton->setVisible(false);
   mUI.mCancelButton->setVisible(false);

   if (aConfig.IsValid())
   {
      connect(this, &AstrolabeDockWidgetBase::PlatformSelected, this, &AstrolabeUI::OnPlatformSelected);

      mScenarioPtr = ut::make_unique<WsfScenario>(wizEnv.GetWsfStandardApplication());
      connect(&wizEnv,
              &wizard::Environment::WsfStandardApplicationPendingDestruction,
              [this]() { mScenarioPtr.reset(nullptr); });
   }
}

//! Populate the platform selector with the given list of space-domain platforms.
void SpaceTools::AstrolabeUI::PopulateSpacePlatforms(const QStringList& aPlatformList)
{
   mUI.mPlatformSelector->SetPlatformList(aPlatformList);
}

//! Return the current mission sequence.
const wsfg::spaceg::OrbitalMissionModel* SpaceTools::AstrolabeUI::GetMission() const
{
   return mCurrentModelPtr.get();
}

//! Select the platform with the given name.
void SpaceTools::AstrolabeUI::SelectPlatform(const QString& aPlatformName)
{
   mPreviewManager.ClearOrbitPreviews();
   mUI.mPlatformSelector->SelectPlatform(aPlatformName);
   if (!aPlatformName.isEmpty())
   {
      SetStatusMessage(QString{"Mission sequence for platform '"} + aPlatformName + QString{"' loaded."});
   }
}

//! Set the current orbital mission model.
void SpaceTools::AstrolabeUI::SetOrbitalMissionModel(wsfg::spaceg::OrbitalMissionModel* aModelPtr)
{
   mUI.mMissionView->SetModel(aModelPtr);
   mUI.mDetailsPtr->SetModel(aModelPtr);
   mCurrentModelPtr.reset(aModelPtr);
   UpdateActionButtons();
}

//! Add the given platform to the list of platforms that may be selected.
void SpaceTools::AstrolabeUI::PlatformAdded(const QString& aAddedPlatformName)
{
   mUI.mPlatformSelector->PlatformAdded(aAddedPlatformName);
}

//! Remove the given platform from the list of platforms that may be selected.
//!
//! If the user is currently modifying the platform in question, they are notified
//! that the platform was removed, and Astrolabe clears the currently selected
//! platform.
void SpaceTools::AstrolabeUI::PlatformRemoved(const QString& aRemovedPlatformName)
{
   mUI.mPlatformSelector->PlatformRemoved(aRemovedPlatformName);
   if (GetSelectedPlatform() == aRemovedPlatformName)
   {
      if (isVisible())
      {
         QString message = QString{"The platform '"} + aRemovedPlatformName + QString{"' has been renamed or removed."};
         SetWarningMessage(message);
         QMessageBox::information(this, QString{"Platform Removed"}, message, QMessageBox::Ok);
      }
      emit PlatformRequested(QString{});
   }
}

void SpaceTools::AstrolabeUI::InitialVerifyAction()
{
   if (mCurrentModelPtr)
   {
      wsfg::spaceg::OrbitalModelToSequence transformer{*mScenarioPtr.get()};
      auto                                 sequencePtr = transformer.Transform(mCurrentModelPtr->GetMissionData());
      FinishVerify(std::move(sequencePtr));
   }
}

//! Verify the mission sequence.
//!
//! Note that this method is called from the template method on the base class.
void SpaceTools::AstrolabeUI::VerifyAction()
{
   emit VerificationRequested();
}

//! Method to respond to a platform being selected in the platform selector.
//!
//! This method intercepts the user action of selecting a platform to allow
//! the UI to check if there are uncommitted changes to the event sequence
//! for the current platform. If there are, the user is presented an option
//! to save the changes, discard the changes, or cancel the request to load
//! a new platform.
//!
//! \param aPlatformName - the platform selected.
void SpaceTools::AstrolabeUI::OnPlatformSelected(const QString& aPlatformName)
{
   mUI.mDetailsPtr->SaveCurrentModifications();

   bool forwardRequest{true};
   if (mCurrentModelPtr != nullptr && mCurrentModelPtr->HasAnyModifications())
   {
      QString message = QString{"Platform "} + mUI.mPlatformSelector->GetSelectedPlatform() +
                        QString{"'s mission sequence has been modified with Astrolabe. "} +
                        QString{"Apply changes before changing platform selection?"};
      auto response = QMessageBox::question(this,
                                            QString("Apply Changes?"),
                                            message,
                                            QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
      if (response == QMessageBox::Yes)
      {
         OnApply();
      }
      else if (response == QMessageBox::Cancel)
      {
         forwardRequest = false;
      }
   }

   if (forwardRequest)
   {
      mUI.mMissionView->SetCurrentIndex(QModelIndex{});
      emit PlatformRequested(aPlatformName);
   }
}

//! Reload the current platform's mission sequence from the input stack.
//!
//! If the current platform has uncommitted changes, the user will be presented
//! with a notification that those changes will be discarded if they continue.
void SpaceTools::AstrolabeUI::PlatformReload(const QString& aPlatformName)
{
   mUI.mDetailsPtr->SaveCurrentModifications();

   bool forwardRequest{true};
   if (mCurrentModelPtr != nullptr && mCurrentModelPtr->HasAnyModifications())
   {
      QString message = QString{"Reloading "} + mUI.mPlatformSelector->GetSelectedPlatform() +
                        QString{"'s mission sequence would discard changes made with Astrolabe. Reload anyway?"};
      auto response =
         QMessageBox::question(this, QString("Discard Changes?"), message, QMessageBox::Yes | QMessageBox::No);
      if (response == QMessageBox::No)
      {
         forwardRequest = false;
      }
   }

   if (forwardRequest)
   {
      emit PlatformRequested(aPlatformName);
   }
}

//! Reload the mission sequence for the current platform from the input stack.
void SpaceTools::AstrolabeUI::HandleReload()
{
   PlatformReload(mUI.mPlatformSelector->GetSelectedPlatform());
}

void SpaceTools::AstrolabeUI::InitialApplyAction()
{
   if (mCurrentModelPtr)
   {
      wsfg::spaceg::OrbitalModelToSequence transformer{*mScenarioPtr.get()};
      auto                                 sequencePtr = transformer.Transform(mCurrentModelPtr->GetMissionData());
      FinishApply(std::move(sequencePtr));
   }
}

//! Inject the mission sequence into the input stack.
//!
//! Note that this method is called from the template method on the base class.
void SpaceTools::AstrolabeUI::ApplyAction()
{
   mCurrentModelPtr->SaveAllModifications();
   emit OutputRequested();
}
