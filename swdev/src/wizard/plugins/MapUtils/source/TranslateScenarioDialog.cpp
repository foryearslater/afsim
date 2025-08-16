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

#include "TranslateScenarioDialog.hpp"

#include <cctype>
#include <iostream>
#include <string>

#include "Environment.hpp"
#include "GhostingInterface.hpp"
#include "Message.hpp"
#include "Platform.hpp"
#include "VaEntity.hpp"
#include "VaUtils.hpp"
#include "VtkInterface.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"
#include "WkfScenario.hpp"
#include "WkfUnitsObject.hpp"
#include "route/WkfAttachmentRoute.hpp"
#include "zone/WkfAttachmentZoneCircle.hpp"
#include "zone/WkfAttachmentZoneEllipse.hpp"
#include "zone/WkfAttachmentZonePolygon.hpp"
#include "zone/WkfZonePoint.hpp"

namespace MapUtils
{
namespace ScenarioTransformation
{
namespace TranslateScenario
{

const Message Dialog::cTRANSLATE_HELP_TEXT{
   Message::Level::Information,
   "TranslateScenario Help",
   "<h3>Purpose:</h3>"
   "<p>This widget allows one to translate a scenario or a selection of objects on the Map Display.</p>"
   "<h3>How to use:</h3>"
   "<p>Use the line edits or the sliders to translate the scenario/selection.</p>"
   "&nbsp;&nbsp;&nbsp;&nbsp;Absolute Translation:"
   "<ul><li>Line Edits:  type in a latitude/longitude using the DMS format.</li>"
   "<li>Sliders:  move the slider to adjust the latitude/longitude degree.</li></ul>"
   "&nbsp;&nbsp;&nbsp;&nbsp;Relative Translation:"
   "<ul><li>Line Edits:  type in a decimal distance followed by the unit.</li></ul>"
   "<h3>Change modes:</h3>"
   "<p>Use the combo box to change modes."
   "&nbsp;&nbsp;&nbsp;&nbsp;Below is a list of modes:"
   "<ul><li>Translate Scenario to/by:  modes to translate an entire scenario to a location/by a certain amount.</li>"
   "<li>Translate Selection to/by:  modes to translate a selection of objects to a location/by a certain "
   "amount.</li></ul></p>"
   "<p>A more detailed help text can be found " +
      QString("<a href=\"%1\">here</a>.")
         .arg("file:///" + QFileInfo(wkfEnv.GetDocumentationDir()).absolutePath() +
              "/documentation/html/docs/wkf_plugin/wiz_translate.html") +
      "</p>",
   Qt::TextFormat::RichText,
   false};

Dialog::Dialog(QWidget* aParent, Qt::WindowFlags aFlags, const Menu aCurrentMode, const bool aShowHelpButton, const bool aShowModeMenu)
   : DialogInterface(aParent, aFlags, &cTRANSLATE_HELP_TEXT)
   , mLatitudeManager(nullptr, nullptr, TransformationType::cLATITUDE, false, {'n', 's'})
   , mLongitudeManager(nullptr, nullptr, TransformationType::cLONGITUDE, false, {'e', 'w'})
   , mVerticalManager(nullptr, nullptr, TransformationType::cLENGTH, true, DirectionCharacters::cDEFAULT)
   , mHorizontalManager(nullptr, nullptr, TransformationType::cLENGTH, true, DirectionCharacters::cDEFAULT)
{
   setWindowFlag(Qt::WindowContextHelpButtonHint, false);
   setAttribute(Qt::WA_DeleteOnClose);

   // set up the dialog window
   setWindowTitle("Translate Scenario");

   mUi.setupUi(this);

   // construct the menus
   mUi.mDialogMenuAndButtons->ConstructMenu(cTRANSLATE_MENU);

   // disallow auto popup of the menus
   mUi.mDialogMenuAndButtons->Menu()->SetAutoPopup(false);

   mLatitudeManager.SetLineEdit(mUi.mLatitudeLineEdit);
   mLatitudeManager.SetSlider(mUi.mLatitudeSlider);
   mLongitudeManager.SetLineEdit(mUi.mLongitudeLineEdit);
   mLongitudeManager.SetSlider(mUi.mLongitudeSlider);
   mVerticalManager.SetLineEdit(mUi.mVerticalLineEdit);
   mHorizontalManager.SetLineEdit(mUi.mHorizontalLineEdit);

   // connect the LineEditSliderManager's signals to the appropriate TranslateScenario::Dialog slots
   connect(&mLatitudeManager, &LineEditSliderManager::ValueEditedString, this, &Dialog::LatitudeModified);
   connect(&mLongitudeManager, &LineEditSliderManager::ValueEditedString, this, &Dialog::LongitudeModified);
   connect(&mVerticalManager, &LineEditSliderManager::ValueEditedString, this, &Dialog::VerticalLengthModified);
   connect(&mVerticalManager, &LineEditSliderManager::UnitChangedString, this, &Dialog::UpdateVerticalUnits);
   connect(&mHorizontalManager, &LineEditSliderManager::ValueEditedString, this, &Dialog::HorizontalLengthModified);
   connect(&mHorizontalManager, &LineEditSliderManager::UnitChangedString, this, &Dialog::UpdateHorizontalUnits);
   connect(&mLatitudeManager,
           &LineEditSliderManager::InvalidFormat,
           this,
           [=]() { mUi.mLatitudeLineEditStatusLabel->setText("Invalid Format"); });
   connect(&mLatitudeManager,
           &LineEditSliderManager::ValidFormat,
           this,
           [=]() { mUi.mLatitudeLineEditStatusLabel->setText(""); });
   connect(&mLongitudeManager,
           &LineEditSliderManager::InvalidFormat,
           this,
           [=]() { mUi.mLongitudeLineEditStatusLabel->setText("Invalid Format"); });
   connect(&mLongitudeManager,
           &LineEditSliderManager::ValidFormat,
           this,
           [=]() { mUi.mLongitudeLineEditStatusLabel->setText(""); });
   connect(&mVerticalManager,
           &LineEditSliderManager::InvalidFormat,
           this,
           [=]() { mUi.mLatitudeLineEditStatusLabel->setText("Invalid Format"); });
   connect(&mVerticalManager,
           &LineEditSliderManager::ValidFormat,
           this,
           [=]() { mUi.mLatitudeLineEditStatusLabel->setText(""); });
   connect(&mHorizontalManager,
           &LineEditSliderManager::InvalidFormat,
           this,
           [=]() { mUi.mLongitudeLineEditStatusLabel->setText("Invalid Format"); });
   connect(&mHorizontalManager,
           &LineEditSliderManager::ValidFormat,
           this,
           [=]() { mUi.mLongitudeLineEditStatusLabel->setText(""); });
   connect(&mLatitudeManager, &LineEditSliderManager::HandleMessage, this, &Dialog::HandleMessage);
   connect(&mLongitudeManager, &LineEditSliderManager::HandleMessage, this, &Dialog::HandleMessage);

   // connect the DialogMenuButtonsWidget's signals to the appropriate TranslateScenario::Dialog slots
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
   case Menu::cTRANSLATE_SCENARIO_BY:
   case Menu::cTRANSLATE_SCENARIO_TO:
      mEntitiesToGhost = MapUtils::PluginUtil::WhatEntities::cSCENARIO;
      break;
   case Menu::cTRANSLATE_SELECTION_BY:
   case Menu::cTRANSLATE_SELECTION_TO:
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
   // Set the VtkInterface's move mode to TranslateScenario
   if (mPreviousMode == Menu::cTRANSLATE_SCENARIO_TO)
   {
      wizEnv.SetVtkMoveMode(wizard::VtkInterface::MoveMode::TranslateScenario);
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

void Dialog::LatitudeModified(const QString& aPropertyValue)
{
   mClickedButton = WhatButton::cNONE;
   if (mLatitudeManager.IsFormatValid())
   {
      // aPropertyValue is in double format
      bool   success;
      double val = aPropertyValue.toDouble(&success);
      if (success)
      {
         UtLatPos latitude{val, mAbsoluteFormat};
         TranslateLatitudeAbsolute(latitude);
      }
   }
}

void Dialog::LongitudeModified(const QString& aPropertyValue)
{
   mClickedButton = WhatButton::cNONE;
   if (mLongitudeManager.IsFormatValid())
   {
      // aPropertyValue is in double format
      bool   success;
      double val = aPropertyValue.toDouble(&success);
      if (success)
      {
         UtLonPos longitude{val, mAbsoluteFormat};
         TranslateLongitudeAbsolute(longitude);
      }
   }
}

void Dialog::VerticalLengthModified(const QString& aPropertyValue)
{
   mClickedButton = WhatButton::cNONE;
   if (mVerticalManager.IsFormatValid())
   {
      bool   success;
      double val = aPropertyValue.toDouble(&success);
      if (success)
      {
         UtLengthValue vertical(val, mVerticalManager.GetManagerUnitInt());
         TranslateLatitudeRelative(vertical);
      }
   }
}

void Dialog::HorizontalLengthModified(const QString& aPropertyValue)
{
   mClickedButton = WhatButton::cNONE;
   if (mHorizontalManager.IsFormatValid())
   {
      bool   success;
      double val = aPropertyValue.toDouble(&success);
      if (success)
      {
         UtLengthValue horizontal(val, mHorizontalManager.GetManagerUnitInt());
         TranslateLongitudeRelative(horizontal);
      }
   }
}

void Dialog::UpdateVerticalUnits(const QString& aPropertyUnit)
{
   mClickedButton = WhatButton::cNONE;
   if (UtUnitLength::IsUnitValid(UtUnitLength::ReadUnit(aPropertyUnit.toStdString())))
   {
      // set the units
      mPreviousVerticalLength.ConvertToUnit(aPropertyUnit.toStdString());
   }
   VerticalLengthModified(mVerticalManager.GetManagerValueString());
}

void Dialog::UpdateHorizontalUnits(const QString& aPropertyUnit)
{
   mClickedButton = WhatButton::cNONE;
   if (UtUnitLength::IsUnitValid(UtUnitLength::ReadUnit(aPropertyUnit.toStdString())))
   {
      // set the units
      mPreviousHorizontalLength.ConvertToUnit(aPropertyUnit.toStdString());
   }
   HorizontalLengthModified(mHorizontalManager.GetManagerValueString());
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
      else if (menuState == Menu::cTRANSLATE_SCENARIO_TO)
      {
         setWindowTitle("Translate Scenario");
         mPreviousMode  = Menu::cTRANSLATE_SCENARIO_TO;
         mRelative      = false;
         mSelectionOnly = false;
      }
      else if (menuState == Menu::cTRANSLATE_SCENARIO_BY)
      {
         setWindowTitle("Translate Scenario");
         mPreviousMode  = Menu::cTRANSLATE_SCENARIO_BY;
         mRelative      = true;
         mSelectionOnly = false;
      }
      else if (menuState == Menu::cTRANSLATE_SELECTION_TO)
      {
         setWindowTitle("Translate Selection");
         mPreviousMode  = Menu::cTRANSLATE_SELECTION_TO;
         mRelative      = false;
         mSelectionOnly = true;
      }
      else if (menuState == Menu::cTRANSLATE_SELECTION_BY)
      {
         setWindowTitle("Translate Selection");
         mPreviousMode  = Menu::cTRANSLATE_SELECTION_BY;
         mRelative      = true;
         mSelectionOnly = true;
      }
      if (menuState != Menu::cSELECT_A_MODE)
      {
         // update the UI view
         if (!mRelative)
         {
            SwitchToAbsolute();
         }
         else
         {
            SwitchToRelative();
         }
         AfterModeSwitch();
      }
   }
}

void Dialog::ResetP()
{
   if (!mRelative)
   {
      // reset the latitude manager value
      mLatitudeManager.SetManagerValueDouble(mCentroid.GetLat());
      // reset the longitude manager value
      mLongitudeManager.SetManagerValueDouble(mCentroid.GetLon());
      // reset the centroids to the original value
      mCurrentPosition = mPreviousPosition = mCentroid;
   }
   else
   {
      // reset the vertical manager value
      mVerticalManager.SetManagerValueDouble(0.0);
      // reset the horizontal manager value
      mHorizontalManager.SetManagerValueDouble(0.0);
      // reset the previous vertical and horizontal lengths to zero
      mPreviousVerticalLength   = 0.0;
      mPreviousHorizontalLength = 0.0;
   }
}

void Dialog::TranslateLatitudeAbsolute(const UtLatPos aLatitude)
{
   // update the previous centroid's latitude with the specified latitude
   mCurrentPosition.SetLat(aLatitude);

   if (NeedToApplyTranslation())
   {
      // translate the units "to" the specified latitude
      ApplyTranslation();
   }
}

void Dialog::TranslateLatitudeRelative(const UtLengthValue aVerticalLength)
{
   // reset the ghost scenario back to the stating position
   ghostMan->ResetGhostEntitiesAndAttachments();

   // convert the distance (difference) to latitude
   UtLLAPos lla = DistanceToPosition({0.0, aVerticalLength.GetAsUnit(UtUnitLength::BaseUnit::cMETERS)},
                                     mPreviousPosition,
                                     ExtrapolationMode::cGREAT_ELLIPSE);

   // update the previous centroid's latitude with the latitude from the extrapolated position from the great ellipse
   mCurrentPosition.SetLat(lla.GetLat());
   // update the previous centroid's longitude if the longitude flipped signs due to a hemisphere crossing
   mCurrentPosition.SetLon(lla.GetLon());
   mPreviousVerticalLength = aVerticalLength;

   if (NeedToApplyTranslation())
   {
      // translate the units "by" the specified latitude
      ApplyTranslation();
   }
}

void Dialog::TranslateLongitudeAbsolute(const UtLonPos aLongitude)
{
   // update the previous centroid's longitude with the specified longitude
   mCurrentPosition.SetLon(aLongitude);

   if (NeedToApplyTranslation())
   {
      // translate the units "to" the specified longitude
      ApplyTranslation();
   }
}

void Dialog::TranslateLongitudeRelative(const UtLengthValue aHorizontalLength)
{
   // reset the ghost scenario back to the stating position
   ghostMan->ResetGhostEntitiesAndAttachments();

   // convert the distance (difference) to longitude
   UtLLAPos lla = DistanceToPosition({aHorizontalLength.GetAsUnit(UtUnitLength::BaseUnit::cMETERS), 0.0},
                                     mPreviousPosition,
                                     ExtrapolationMode::cGREAT_ELLIPSE);

   // update the previous centroid's latitude if the latitude flipped signs due to a hemisphere crossing
   mCurrentPosition.SetLat(lla.GetLat());
   // update the previous centroid's longitude with the longitude from the extrapolated position from the great ellipse
   mCurrentPosition.SetLon(lla.GetLon());
   mPreviousHorizontalLength = aHorizontalLength;

   if (NeedToApplyTranslation())
   {
      // translate the units "by" the specified longitude
      ApplyTranslation();
   }
}

void Dialog::InitializeComponents()
{
   // acquire the default unit from the preferences
   auto unit       = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetLengthUnit();
   mAbsoluteFormat = static_cast<UtAngle::Fmt>(wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetLatLonFormatting());
   InitializeUnits(unit);
   InitializeLabels();
   InitializeManagers();
}

void Dialog::InitializeLabels()
{
   QSignalBlocker latBlock(mUi.mLatitudeLineEditLabel);
   QSignalBlocker lonBlock(mUi.mLongitudeLineEditLabel);
   mUi.mLatitudeLineEditStatusLabel->setText("");
   mUi.mLongitudeLineEditStatusLabel->setText("");
   if (!mRelative)
   {
      mUi.mLatitudeLineEditLabel->setText("Latitudinal Coordinate:");
      mUi.mLongitudeLineEditLabel->setText("Longitudinal Coordinate:");
   }
   else
   {
      mUi.mLatitudeLineEditLabel->setText("Latitudinal Distance:");
      mUi.mLongitudeLineEditLabel->setText("Longitudinal Distance:");
   }
}

void Dialog::InitializeManagers()
{
   // initialize the line edits for absolute translation
   if (!mRelative)
   {
      // create a stringstream for conversion of default value for latitude line edit
      std::stringstream latitudeStream;
      // create a stringstream for conversion of default value for longitude line edit
      std::stringstream longitudeStream;
      // create a string for initialization of default value for latitude line edit
      std::string latitudeStr;
      // create a string for initialization of default value for longitude line edit
      std::string longitudeStr;

      // use stringstreams (absolute translation mode)
      latitudeStream << mCentroid.GetLat();
      latitudeStream >> latitudeStr;
      longitudeStream << mCentroid.GetLon();
      longitudeStream >> longitudeStr;
      // initialize the default values for the absolute translation mode
      mLatitudeManager.Initialize(QString::fromStdString(latitudeStr), 0);
      mLongitudeManager.Initialize(QString::fromStdString(longitudeStr), 0);
   }
   // initialize the line edits for relative translation
   else
   {
      // initialize the default values for the relative translation mode
      mVerticalManager.Initialize(0.0, mPreviousVerticalLength.GetUnit());
      mHorizontalManager.Initialize(0.0, mPreviousHorizontalLength.GetUnit());
   }

   // get location data to determine boundary positions
   ghost::Manager::VaPositionList positionList;
   for (const auto& currentEntity : ghostMan->GetGhostEntities())
   {
      positionList.push_back(currentEntity->GetPosition());
   }
   // sort the positionList in ascending order based on latitudes
   std::sort(positionList.begin(),
             positionList.end(),
             [](const vespa::VaPosition& aLhs, const vespa::VaPosition& aRhs) { return aLhs.GetLat() < aRhs.GetLat(); });
   // set the minimum and maximum positions from the sorted positionList
   mMinimumPosition = positionList.front();
   mMaximumPosition = positionList.back();
}

void Dialog::InitializeUnits(const UtUnitLength::BaseUnit aUnit)
{
   // set the previous and current positions of the centroid as the initial position of the centroid
   mPreviousPosition = mCentroid;
   mCurrentPosition  = mCentroid;
   if (!mRelative)
   {
      // set the default ranges
      mLatitudeManager.SetDefaultRange(0, MapUtils::ScenarioTransformation::IO_Types::cLATITUDE_RANGE);
      mLongitudeManager.SetDefaultRange(0, MapUtils::ScenarioTransformation::IO_Types::cLONGITUDE_RANGE);
   }
   else
   {
      // set the default units
      mPreviousVerticalLength.Set(0.0, aUnit);
      mPreviousHorizontalLength.Set(0.0, aUnit);
      // set the default ranges
      mVerticalManager.SetDefaultRange(0, MapUtils::ScenarioTransformation::IO_Types::cVERTICAL_RANGE);
      mHorizontalManager.SetDefaultRange(0, MapUtils::ScenarioTransformation::IO_Types::cHORIZONTAL_RANGE);
   }
}

bool Dialog::WithinBounds(const UtLatPos aLatitude) const
{
   // define a boolean to denote if translation hit the upper boundary (north pole)
   bool hitLatitudeLowerBound = false;
   // define a boolean to denote if translation hit the lower boundary (south pole)
   bool hitLatitudeUpperBound = false;

   // check if translation hit the lower boundary (south pole)
   if (mMinimumPosition.GetLat() <= 0)
   {
      // translation will hit the lower boundary if and only if the minimum position's latitude <= zero
      hitLatitudeLowerBound = UtLatPos(MapUtils::ScenarioTransformation::IO_Types::cLATITUDE_RANGE.mMinimum) >=
                              mMinimumPosition.GetLat() + aLatitude;
   }
   else
   {
      hitLatitudeLowerBound = false;
   }
   if (mMaximumPosition.GetLat() > 0)
   {
      // translation will hit the upper boundary if and only if the maximum position's latitude > zero
      hitLatitudeUpperBound = UtLatPos(MapUtils::ScenarioTransformation::IO_Types::cLATITUDE_RANGE.mMaximum) <
                              mMaximumPosition.GetLat() + aLatitude;
   }
   else
   {
      hitLatitudeUpperBound = false;
   }

   return !(hitLatitudeLowerBound || hitLatitudeUpperBound);
}

bool Dialog::NeedToApplyTranslation() const
{
   return MapUtils::PluginUtil::AreLatitudesDifferent(mPreviousPosition.GetLat(), mCurrentPosition.GetLat()) ||
          MapUtils::PluginUtil::AreLongitudesDifferent(mPreviousPosition.GetLon(), mCurrentPosition.GetLon()) ||
          MapUtils::PluginUtil::AreAltitudesDifferent(mPreviousPosition.GetAlt(), mCurrentPosition.GetAlt());
   // the other three degrees of freedom should NEVER change with a translation
}

void Dialog::ApplyTranslation()
{
   // get the LLA of the old centroid position
   UtLLAPos oldCentroid;
   mPreviousPosition.GetLLA(oldCentroid);
   // get the LLA of the new centroid position
   UtLLAPos newCentroid;
   mCurrentPosition.GetLLA(newCentroid);

   // apply the translation
   // absolute and relative does not matter here, it is all relative to the centroid (line 887)
   wkf::Scenario* scenarioPtr = vaEnv.GetStandardScenario();
   for (const auto& currentGhost : ghostMan->GetGhostEntities())
   {
      std::string      currentGhostEntityName = currentGhost->GetName();
      vespa::VaEntity* ghostEntityPtr         = scenarioPtr->FindEntity(currentGhostEntityName);
      // check if the ghost entity exists
      if (ghostEntityPtr != nullptr)
      {
         vespa::VaPosition ghostEntityPosition = ghostEntityPtr->GetPosition();
         // update the position
         if (Translate(oldCentroid, newCentroid, &ghostEntityPosition, TranslationMode::cRELATIVE_XYA))
         {
            ghostEntityPtr->SetPosition(ghostEntityPosition);
         }

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

               // apply the translation to each waypoint
               for (size_t index = 0; index < waypoints.size(); ++index)
               {
                  // update the data layer
                  if (Translate(oldCentroid,
                                newCentroid,
                                static_cast<vespa::VaPosition*>(waypoints[index]),
                                TranslationMode::cRELATIVE_XYA))
                  {
                     // update the visual layer
                     currentRoute->UpdateWaypoint(index,
                                                  waypoints[index]->GetLat(),
                                                  waypoints[index]->GetLon(),
                                                  waypoints[index]->GetAlt(),
                                                  waypoints[index]->GetGoToIndex(),
                                                  waypoints[index]->GetLabel());
                  }
               }
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
                  vespa::VaPosition pointPos = point->GetPosition();
                  // update the position
                  if (Translate(oldCentroid, newCentroid, &pointPos, TranslationMode::cRELATIVE_XYA))
                  {
                     point->SetPosition(pointPos);
                  }
               }
               // reload the zone
               currentZonePolygon->Load();
            }
         }
      }
   }

   // set the previous centroid position as the current centroid position
   mPreviousPosition = mCurrentPosition;
}

void Dialog::UI_Enable()
{
   // enable certain UI components
   mUi.mLatitudeGroupBox->setEnabled(true);
   mUi.mLongitudeGroupBox->setEnabled(true);
   mUi.mDialogMenuAndButtons->Apply()->setEnabled(true);
   mUi.mDialogMenuAndButtons->Reset()->setEnabled(true);
}

void Dialog::UI_Disable()
{
   // set the stack widgets to the default page (nothing)
   mUi.mLatitudeVerticalLineEdit->setCurrentIndex(static_cast<int>(Page::cDEFAULT));
   mUi.mLongitudeHorizontalLineEdit->setCurrentIndex(static_cast<int>(Page::cDEFAULT));
   // disable certain UI components
   mUi.mLatitudeGroupBox->setEnabled(false);
   mUi.mLongitudeGroupBox->setEnabled(false);
   mUi.mDialogMenuAndButtons->Apply()->setEnabled(false);
   mUi.mDialogMenuAndButtons->Reset()->setEnabled(false);
}

void Dialog::SwitchToAbsolute()
{
   mUi.mLatitudeVerticalLineEdit->setCurrentIndex(static_cast<int>(Page::cABSOLUTE));
   // modify visibility
   mUi.mLatitudeSliderLabel->setVisible(true);
   mUi.mLatitudeSlider->setVisible(true);
   mUi.mLongitudeHorizontalLineEdit->setCurrentIndex(static_cast<int>(Page::cABSOLUTE));
   // modify visibility
   mUi.mLongitudeSliderLabel->setVisible(true);
   mUi.mLongitudeSlider->setVisible(true);
}

void Dialog::SwitchToRelative()
{
   mUi.mLatitudeVerticalLineEdit->setCurrentIndex(static_cast<int>(Page::cRELATIVE));
   // modify visibility
   mUi.mLatitudeSliderLabel->setVisible(false);
   mUi.mLatitudeSlider->setVisible(false);
   mUi.mLongitudeHorizontalLineEdit->setCurrentIndex(static_cast<int>(Page::cRELATIVE));
   // modify visibility
   mUi.mLongitudeSliderLabel->setVisible(false);
   mUi.mLongitudeSlider->setVisible(false);
}

} // namespace TranslateScenario
} // namespace ScenarioTransformation
} // namespace MapUtils
