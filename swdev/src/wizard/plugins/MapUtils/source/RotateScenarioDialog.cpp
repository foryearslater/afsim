// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2020 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "RotateScenarioDialog.hpp"

#include <sstream>
#include <string>

#include "Environment.hpp"
#include "GhostingInterface.hpp"
#include "Message.hpp"
#include "Platform.hpp"
#include "UtQtMemory.hpp"
#include "VaEntity.hpp"
#include "VaUtils.hpp"
#include "VtkInterface.hpp"
#include "WkfAttachmentModel.hpp"
#include "WkfEnvironment.hpp"
#include "WkfScenario.hpp"
#include "WkfUnitsObject.hpp"
#include "WkfViewer.hpp"
#include "route/WkfAttachmentRoute.hpp"
#include "zone/WkfAttachmentZoneCircle.hpp"
#include "zone/WkfAttachmentZoneEllipse.hpp"
#include "zone/WkfAttachmentZonePolygon.hpp"
#include "zone/WkfZonePoint.hpp"

namespace MapUtils
{
namespace ScenarioTransformation
{
namespace RotateScenario
{

const Message Dialog::cROTATE_HELP_TEXT{
   Message::Level::Information,
   "RotateScenario Help",
   "<h3>Purpose:</h3>"
   "<p>This widget allows one to rotate an entire scenario or a selection of objects on the Map Display.</p>"
   "<h3>How to use:</h3>"
   "<p>Either use the line edit or the dial to rotate the scenario/selection."
   "<ul><li>Line Edit:  type in a decimal angle followed by the unit.</li>"
   "<li>Dial:  spin the degree dial.</li></ul></p>"
   "<h3>Change modes:</h3>"
   "<p>Use the combo box to change modes."
   "&nbsp;&nbsp;&nbsp;&nbsp;Below is a list of modes:"
   "<ul><li>Rotate Scenario:  mode to rotate an entire scenario.</li>"
   "<li>Rotate Selection:  mode to rotate a selection of objects.</li></ul></p>"
   "<p>A more detailed help text can be found " +
      QString("<a href=\"%1\">here</a>.")
         .arg("file:///" + QFileInfo(wkfEnv.GetDocumentationDir()).absolutePath() +
              "/documentation/html/docs/wkf_plugin/wiz_rotate.html") +
      "</p>",
   Qt::TextFormat::RichText,
   false};

Dialog::Dialog(QWidget* aParent, Qt::WindowFlags aFlags, const Menu aCurrentMode, const bool aShowHelpButton, const bool aShowModeMenu)
   : DialogInterface(aParent, aFlags, &cROTATE_HELP_TEXT)
   , mAngleManager(nullptr, nullptr, TransformationType::cANGLE, true, DirectionCharacters::cDEFAULT)
{
   setWindowFlag(Qt::WindowContextHelpButtonHint, false);

   // set up the dialog window
   setWindowTitle("Rotate Scenario");

   mUi.setupUi(this);

   // construct the menus
   mUi.mDialogMenuAndButtons->ConstructMenu(cROTATE_MENU);

   // disallow auto popup of the menus
   mUi.mDialogMenuAndButtons->Menu()->SetAutoPopup(false);

   // set up the sliders
   mUi.mAngleDial->setWrapping(true);

   // set up the manager
   mAngleManager.SetLineEdit(mUi.mAngleLineEdit);
   mAngleManager.SetSlider(mUi.mAngleDial);

   // connect the LineEditSliderManager's signals to the appropriate RotateScenario::Dialog slots
   connect(&mAngleManager, &LineEditSliderManager::ValueEditedString, this, &Dialog::AngleModified);
   connect(&mAngleManager, &LineEditSliderManager::UnitChangedString, this, &Dialog::UnitsModified);
   connect(&mAngleManager,
           &LineEditSliderManager::InvalidFormat,
           this,
           [=]() { mUi.mAngleLineEditStatusLabel->setText("Invalid Format"); });
   connect(&mAngleManager,
           &LineEditSliderManager::ValidFormat,
           this,
           [=]() { mUi.mAngleLineEditStatusLabel->setText(""); });
   connect(&mAngleManager, &LineEditSliderManager::HandleMessage, this, &Dialog::HandleMessage);

   // connect the DialogMenuButtonsWidget's signals to the appropriate RotateScenario::Dialog slots
   connect(mUi.mDialogMenuAndButtons->Okay(), &QPushButton::clicked, this, &Dialog::accept);
   connect(mUi.mDialogMenuAndButtons->Cancel(), &QPushButton::clicked, this, &Dialog::reject);

   // hide the "Help" button if requested
   if (aShowHelpButton)
   {
      mUi.mDialogMenuAndButtons->ShowHelpButton();
   }
   else
   {
      mUi.mDialogMenuAndButtons->HideHelpButton();
   }

   // hide the mode menu (combo box) if requested
   if (aShowModeMenu)
   {
      mUi.mDialogMenuAndButtons->ShowMenu();
   }
   else
   {
      mUi.mDialogMenuAndButtons->HideMenu();
   }
   mUi.mDialogMenuAndButtons->UseOkayAndCancel();

   switch (aCurrentMode)
   {
   case Menu::cROTATE_SCENARIO:
      mEntitiesToGhost = MapUtils::PluginUtil::WhatEntities::cSCENARIO;
      break;
   case Menu::cROTATE_SELECTION:
      mEntitiesToGhost = MapUtils::PluginUtil::WhatEntities::cSELECTION;
      break;
   case Menu::cSELECT_A_MODE:
      mEntitiesToGhost = MapUtils::PluginUtil::WhatEntities::cNONE;
      break;
   }
   // default to a mode
   ModeChanged(static_cast<int>(aCurrentMode));
}

void Dialog::accept()
{
   mClickedButton = WhatButton::cOKAY;
   // Set the VtkInterface's move mode to the appropriate Rotate mode
   if (mPreviousMode == Menu::cROTATE_SELECTION)
   {
      wizEnv.SetVtkMoveMode(wizard::VtkInterface::MoveMode::RotateSelection);
   }
   else
   {
      wizEnv.SetVtkMoveMode(wizard::VtkInterface::MoveMode::RotateScenario);
   }
   DialogInterface::AcceptP();
   // Set the VtkInterface's move mode to TranslateSelection (the default)
   wizEnv.SetVtkMoveMode(wizard::VtkInterface::MoveMode::TranslateSelection);
   emit Closed();
}

void Dialog::reject()
{
   mClickedButton = WhatButton::cCANCEL;
   DialogInterface::RejectP();
   emit Closed();
}

void Dialog::AngleModified(const QString& aPropertyValue)
{
   mClickedButton = WhatButton::cNONE;
   if (mAngleManager.IsFormatValid())
   {
      bool   success;
      double val = aPropertyValue.toDouble(&success);
      if (success)
      {
         mCurrentAngle.Set(val, UtUnitAngle::FindUnitName(mAngleManager.GetManagerUnitInt()));
         if (NeedToApplyRotation())
         {
            ApplyRotation();
         }
      }
   }
}

void Dialog::UnitsModified(const QString& aPropertyUnit)
{
   mClickedButton = WhatButton::cNONE;
   if (mAngleManager.IsFormatValid())
   {
      // set the units
      mPreviousAngle.ConvertToUnit(aPropertyUnit.toStdString());
      mCurrentAngle.ConvertToUnit(aPropertyUnit.toStdString());
      // set the dial range
      auto rangeMapIter = MapUtils::ScenarioTransformation::IO_Types::cANGLE_RANGES.find(
         static_cast<UtUnitAngle::BaseUnit>(mCurrentAngle.GetUnit()));
      mAngleManager.SetDefaultRange(UtUnitAngle::ReadUnit(aPropertyUnit.toStdString()), rangeMapIter->second);
   }
   QString propertyValue;
   AngleModified(mAngleManager.GetManagerValueString());
}

void Dialog::ModeChanged(const int aState)
{
   Menu menuState    = static_cast<Menu>(aState);
   int  previousMode = static_cast<int>(mPreviousMode);
   if (mPreviousMode != menuState)
   {
      if (menuState != Menu::cSELECT_A_MODE)
      {
         BeforeModeSwitch();
      }
      if (menuState == Menu::cSELECT_A_MODE)
      {
         setWindowTitle("");
         mUi.mDialogMenuAndButtons->Menu()->setCurrentText(
            mUi.mDialogMenuAndButtons->Menu()->itemData(previousMode, Qt::DisplayRole).toString());
      }
      else if (menuState == Menu::cROTATE_SCENARIO)
      {
         setWindowTitle("Rotate Scenario");
         mPreviousMode  = Menu::cROTATE_SCENARIO;
         mSelectionOnly = false;
      }
      else if (menuState == Menu::cROTATE_SELECTION)
      {
         setWindowTitle("Rotate Selection");
         mPreviousMode  = Menu::cROTATE_SELECTION;
         mSelectionOnly = true;
      }
      if (menuState != Menu::cSELECT_A_MODE)
      {
         AfterModeSwitch();
      }
   }
}

void Dialog::ResetP()
{
   // rotate ghost scenario back to original position
   mAngleManager.SetManagerValueDouble(0.0);
   mCurrentAngle.Set(0.0, mCurrentAngle.GetUnitName());
}

bool Dialog::NeedToApplyRotation()
{
   return MapUtils::PluginUtil::AreYawsDifferent(mPreviousAngle, mCurrentAngle);
}

void Dialog::ApplyRotation()
{
   wkf::Scenario* scenarioPtr = vaEnv.GetStandardScenario();
   // rotate entities about the COR
   UtLLAPos centerPosition;
   mCentroid.GetLLA(centerPosition);

   // compute the difference of the current and previous angles to get the correct angle by which to rotate
   // NOTE:  the current and previous angles are stored as the standard unit, which is in radians.
   // this is a problem because visualization ONLY supports degrees when doing geometric transformation.
   // therefore, the angle values are extracted as degrees before the difference is computed
   double rotateDifference = UtMath::NormalizeAngle0_360(mCurrentAngle.GetAsUnit(UtUnitAngle::BaseUnit::cDEGREES) -
                                                         mPreviousAngle.GetAsUnit(UtUnitAngle::BaseUnit::cDEGREES));
   mPreviousAngle          = mCurrentAngle;

   for (const auto& currentGhost : ghostMan->GetGhostEntities())
   {
      std::string      currentGhostEntityName = currentGhost->GetName();
      vespa::VaEntity* ghostEntityPtr         = scenarioPtr->FindEntity(currentGhostEntityName);
      // check if the ghost entity exists
      if (ghostEntityPtr != nullptr)
      {
         // get the position before rotation
         vespa::VaPosition beforePosition = ghostEntityPtr->GetPosition();
         // rotate the ghost entity
         ghostEntityPtr->Rotate(centerPosition, rotateDifference);
         // get the position after rotation
         vespa::VaPosition afterPosition = ghostEntityPtr->GetPosition();
         // compute the difference between the before and after positions
         // this is used for the zones
         vespa::VaPosition difference(afterPosition.GetX() - beforePosition.GetX(),
                                      afterPosition.GetY() - beforePosition.GetY());

         // process the attachments
         vespa::VaEntity::Attachments& currentAttachments = ghostEntityPtr->GetAttachments();

         for (const auto& attachmentPtr : currentAttachments)
         {
            // attachment is a wkf::AttachmentRoute
            if (attachmentPtr->IsA_TypeOf<wkf::AttachmentRoute>())
            {
               // cast the attachment into a wkf::AttachmentRoute
               auto currentRoute = static_cast<wkf::AttachmentRoute*>(attachmentPtr.get());

               // extract the waypoints from the attachment route
               std::vector<wkf::Waypoint*> waypoints = currentRoute->GetWaypoints();

               // rotate each waypoint
               for (size_t index = 0; index < waypoints.size(); ++index)
               {
                  // update the data layer
                  waypoints[index]->Rotate(centerPosition, rotateDifference);
                  // update the visual layer
                  currentRoute->UpdateWaypoint(index,
                                               waypoints[index]->GetLat(),
                                               waypoints[index]->GetLon(),
                                               waypoints[index]->GetAlt(),
                                               waypoints[index]->GetGoToIndex(),
                                               waypoints[index]->GetLabel());
               }
            }
            // attachment is a wkf::AttachmentZoneCircle
            if (attachmentPtr->IsA_TypeOf<wkf::AttachmentZoneCircle>())
            {
               // cast the attachment into a wkf::AttachmentZoneCircle
               auto currentZoneCircle = static_cast<wkf::AttachmentZoneCircle*>(attachmentPtr.get());

               // apply the rotation to the zone (just a translation)
               currentZoneCircle->SetX(currentZoneCircle->GetX() + difference.GetX());
               currentZoneCircle->SetY(currentZoneCircle->GetY() + difference.GetY());
               // reload the zone
               currentZoneCircle->Load();
            }
            // attachment is a wkf::AttachmentZoneEllipse
            if (attachmentPtr->IsA_TypeOf<wkf::AttachmentZoneEllipse>())
            {
               // cast the attachment into a wkf::AttachmentZoneEllipse
               auto currentZoneEllipse = static_cast<wkf::AttachmentZoneEllipse*>(attachmentPtr.get());

               // apply the rotation to the zone (just a translation)
               currentZoneEllipse->SetX(currentZoneEllipse->GetX() + difference.GetX());
               currentZoneEllipse->SetY(currentZoneEllipse->GetY() + difference.GetY());
               // reload the zone
               currentZoneEllipse->Load();
            }
            // attachment is a wkf::AttachmentZonePolygon
            else if (attachmentPtr->IsA_TypeOf<wkf::AttachmentZonePolygon>())
            {
               // cast the attachment into a wkf::AttachmentZonePolygon
               auto currentZonePolygon = static_cast<wkf::AttachmentZonePolygon*>(attachmentPtr.get());

               // apply the translation to the zone points
               const auto& pointList = currentZonePolygon->GetZonePoints();
               for (const auto& point : pointList)
               {
                  vespa::VaPosition pos = point->GetPosition();
                  pos.Rotate(centerPosition, rotateDifference);
                  point->SetPosition(pos);
               }
               // reload the zone
               currentZonePolygon->Load();
            }
         }
      }
   }
}

void Dialog::InitializeComponents()
{
   // acquire the default unit from the preferences
   auto unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetAngleUnit();
   // run the initialization routines
   InitializeLabel();
   InitializeUnits(unit);
   mAngleManager.Initialize(0.0, mCurrentAngle.GetUnit());
}

void Dialog::InitializeLabel()
{
   mUi.mAngleLineEditStatusLabel->setText("");
}

void Dialog::InitializeUnits(const UtUnitAngle::BaseUnit aUnit)
{
   // set the default units
   mCurrentAngle.Set(0.0, aUnit);
   mPreviousAngle.Set(0.0, aUnit);
   // set the default dial range
   auto rangeMapIter = MapUtils::ScenarioTransformation::IO_Types::cANGLE_RANGES.find(aUnit);
   mAngleManager.SetDefaultRange(aUnit, rangeMapIter->second);
}

void Dialog::UI_Enable()
{
   mUi.mAngleGroupBox->setEnabled(true);
   mUi.mDialogMenuAndButtons->Apply()->setEnabled(true);
   mUi.mDialogMenuAndButtons->Reset()->setEnabled(true);
}

void Dialog::UI_Disable()
{
   mUi.mAngleGroupBox->setEnabled(false);
   mUi.mDialogMenuAndButtons->Apply()->setEnabled(false);
   mUi.mDialogMenuAndButtons->Reset()->setEnabled(false);
}


} // namespace RotateScenario
} // namespace ScenarioTransformation
} // namespace MapUtils
