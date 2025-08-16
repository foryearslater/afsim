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

#include "AstrolabeDockWidget.hpp"

#include <QMessageBox>
#include <QSize>

#include "AstrolabeConfig.hpp"
#include "AstrolabeMissionView.hpp"
#include "AstrolabeSimCommands.hpp"
#include "AstrolabeSimInterface.hpp"
#include "OrbitalModelToSequence.hpp"
#include "PlatformSelector.hpp"
#include "UtMemory.hpp"
#include "WkPermissions.hpp"
#include "WkfPlatform.hpp"
#include "WkfScenario.hpp"
#include "WkfVtkEnvironment.hpp"

WkAstrolabe::DockWidget::DockWidget(const wsfg::spaceg::AstrolabeConfig& aConfig,
                                    SimInterface*                        aSimInterfacePtr,
                                    QWidget*                             aParentPtr,
                                    Qt::WindowFlags                      aFlags)
   : AstrolabeDockWidgetBase(aConfig, aParentPtr, aFlags)
   , mSimInterfacePtr{aSimInterfacePtr}
   , mRelevantPlatforms{}
{
   if (aConfig.IsValid())
   {
      connect(this, &AstrolabeDockWidgetBase::PlatformSelected, this, &DockWidget::HandlePlatformSelectionRequest);
      connect(mUI.mCancelButton, &QPushButton::clicked, this, &DockWidget::CancelMissionSequence);
      connect(mUI.mDiscardButton, &QPushButton::clicked, this, &DockWidget::DiscardMissionSequence);
      UpdateActionButtons();
   }
}

//! Set the availability of the given platform.
//!
//! Set the availability of the platform with the given @p aPlatformName to @p aAvailable.
//! If the user is currently working on the platform with the given @p aPlatformName, then
//! this will notify the user that the platform has been removed from the simulation.
//!
//! \param aPlatformName - the name of the platform being added to, or removed from the simulation.
//! \param aAvailable    - true if the platform has become available, false if it has been removed.
void WkAstrolabe::DockWidget::SetPlatformAvailability(const QString& aPlatformName, bool aAvailable)
{
   if (aAvailable)
   {
      mUI.mPlatformSelector->PlatformAdded(aPlatformName);
      mRelevantPlatforms.insert(aPlatformName);
   }
   else
   {
      mUI.mPlatformSelector->PlatformRemoved(aPlatformName);
      mRelevantPlatforms.remove(aPlatformName);
      if (aPlatformName == mUI.mPlatformSelector->GetSelectedPlatform())
      {
         ClearCurrentSelection();
         SetWarningMessage("The platform '" + aPlatformName + "' was removed from the simulation.");
      }
   }
}

bool WkAstrolabe::DockWidget::IsRelevantPlatform(const QString& aPlatformName) const
{
   return mRelevantPlatforms.contains(aPlatformName);
}

//! Respond to a user request to select a platform.
//!
//! This will respond to a request from the user to select the platform with the
//! given @p aPlatformName. If Astrolabe currently has another platform selected,
//! the user will be asked if they want to discard any work on the current selection
//! before selecting the given platform.
//!
//! \param aPlatformName - the name of the platform that was requested.
void WkAstrolabe::DockWidget::HandlePlatformSelectionRequest(const QString& aPlatformName)
{
   setVisible(true);
   show();
   raise();

   if (aPlatformName == GetSelectedPlatform())
   {
      return;
   }

   if (VerifyDiscardFromUser())
   {
      if (aPlatformName.isEmpty())
      {
         SetModel(nullptr);
      }
      else
      {
         SetModel(GetEmptyMission());
      }
      mUI.mPlatformSelector->SelectPlatform(aPlatformName);
   }
}

//! Set the model that Astrolabe is currently presenting to the user.
void WkAstrolabe::DockWidget::SetModel(std::unique_ptr<wsfg::spaceg::OrbitalMissionModel> aMissionModelPtr)
{
   mUI.mMissionView->SetModel(aMissionModelPtr.get());
   mUI.mDetailsPtr->SetModel(aMissionModelPtr.get());
   mCurrentModelPtr = std::move(aMissionModelPtr);
   mPreviewManager.ClearOrbitPreviews();
   UpdateActionButtons();
   ClearStatusMessage();
}

std::unique_ptr<wsfg::spaceg::OrbitalMissionModel> WkAstrolabe::DockWidget::GetEmptyMission() const
{
   std::unique_ptr<wsfg::spaceg::OrbitalMissionModel> retvalPtr = ut::make_unique<wsfg::spaceg::OrbitalMissionModel>();
   retvalPtr->SetItemData(retvalPtr->GetSequenceIndex(), wsfg::spaceg::AstrolabeConfig::DefaultEventProperties("SEQUENCE"));
   return retvalPtr;
}

//! Clears the current selection and model.
void WkAstrolabe::DockWidget::ClearCurrentSelection()
{
   SetModel(nullptr);
   mUI.mPlatformSelector->SelectPlatform(QString{});
   ClearStatusMessage();
}

//! Cancel the mission sequence for the currently selected platform.
void WkAstrolabe::DockWidget::CancelMissionSequence()
{
   wkf::Platform* platformPtr = vaEnv.GetStandardScenario()->FindPlatformByName(GetSelectedPlatform());
   if (platformPtr && !warlock::HasPermissionToControlPlatform(*platformPtr))
   {
      QMessageBox::warning(nullptr,
                           "Cancel Sequence Not Executed",
                           "User does not have permissions to modify platform " + GetSelectedPlatform() + ".");
   }
   else
   {
      mSimInterfacePtr->AddSimCommand(ut::make_unique<CancelManeuversCommand>(GetSelectedPlatform()));
      SetStatusMessage("Canceled mission sequence for " + GetSelectedPlatform());
   }
}

//! Discard the mission sequence that is currently being designed in Astrolabe.
void WkAstrolabe::DockWidget::DiscardMissionSequence()
{
   if (VerifyDiscardFromUser())
   {
      SetModel(GetEmptyMission());
   }
}

//! Send a sim command to create a mission sequence from the mission data.
//!
//! This will ultimately result in a response sim event that will continue the created sequence
//! to FinishVerify.
void WkAstrolabe::DockWidget::InitialVerifyAction()
{
   if (mCurrentModelPtr)
   {
      mSimInterfacePtr->AddSimCommand(
         ut::make_unique<CreateMissionCommand>(mCurrentModelPtr->GetMissionData(), true, mSimInterfacePtr));
   }
}

//! Verify the mission sequence.
//!
//! Note that this is called from the template method in the base class.
void WkAstrolabe::DockWidget::VerifyAction()
{
   wkf::Platform* platformPtr = vaEnv.GetStandardScenario()->FindPlatformByName(GetSelectedPlatform());
   if (platformPtr && !warlock::HasPermissionToControlPlatform(*platformPtr))
   {
      QMessageBox::warning(nullptr,
                           "Verify Action Not Executed",
                           "User does not have permissions to modify platform " + GetSelectedPlatform() + ".");
   }
   else
   {
      mSimInterfacePtr->AddSimCommand(ut::make_unique<VerifyMissionCommand>(GetSelectedPlatform(),
                                                                            mCurrentModelPtr->GetMissionData(),
                                                                            mSimInterfacePtr));
   }
}

//! Send a sim command to create a mission sequence from the mission data.
//!
//! This will ultimately result in a response sim event that will continue the created
//! sequence to FinishApply.
void WkAstrolabe::DockWidget::InitialApplyAction()
{
   if (mCurrentModelPtr)
   {
      mSimInterfacePtr->AddSimCommand(
         ut::make_unique<CreateMissionCommand>(mCurrentModelPtr->GetMissionData(), false, mSimInterfacePtr));
   }
}

//! Inject the designed mission sequence into the simulation for the currently selected platform.
//!
//! Note that this is called from the template method in the base class.
void WkAstrolabe::DockWidget::ApplyAction()
{
   wkf::Platform* platformPtr = vaEnv.GetStandardScenario()->FindPlatformByName(GetSelectedPlatform());
   if (platformPtr && !warlock::HasPermissionToControlPlatform(*platformPtr))
   {
      QMessageBox::warning(nullptr,
                           "Inject Sequence Not Executed",
                           "User does not have permissions to modify platform " + GetSelectedPlatform() + ".");
   }
   else
   {
      mSimInterfacePtr->AddSimCommand(
         ut::make_unique<InjectMissionSequenceCommand>(GetSelectedPlatform(), mCurrentModelPtr->GetMissionData()));
      SetModel(GetEmptyMission());
      SetStatusMessage("Mission sequence assigned to platform '" + GetSelectedPlatform() + "'.");
   }
}

//! Verify with the user that they want to discard any current mission sequence.
bool WkAstrolabe::DockWidget::VerifyDiscardFromUser()
{
   bool retval{false};
   if (GetSelectedPlatform().isEmpty())
   {
      // No platform selected; discard is safe.
      retval = true;
   }
   else if (mCurrentModelPtr)
   {
      mUI.mDetailsPtr->SaveCurrentModifications();
      if (mCurrentModelPtr->rowCount(mCurrentModelPtr->GetSequenceIndex()) || mCurrentModelPtr->HasAnyModifications())
      {
         // Ask the user
         QMessageBox::StandardButton response =
            QMessageBox::question(this,
                                  QString("Discard Mission Sequence?"),
                                  QString("Do you wish to discard the current mission sequence for platform '") +
                                     GetSelectedPlatform() + QString("'?"),
                                  QMessageBox::Yes | QMessageBox::No);
         retval = (response == QMessageBox::Yes);
      }
      else
      {
         // Empty sequence; discard is safe.
         retval = true;
      }
   }
   return retval;
}

void WkAstrolabe::DockWidget::UpdateActionButtons()
{
   AstrolabeDockWidgetBase::UpdateActionButtons();
   bool enabled = mCurrentModelPtr != nullptr;
   mUI.mCancelButton->setEnabled(enabled);
   mUI.mDiscardButton->setEnabled(enabled);
}
