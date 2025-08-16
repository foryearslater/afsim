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

#include "CreateZoneDialog.hpp"

#include <QMenu>
#include <QTreeWidgetItem>

#include "ParseResults.hpp"
#include "Project.hpp"
#include "ProjectWorkspace.hpp"
#include "TextSource.hpp"
#include "UtEllipsoidalEarth.hpp"
#include "UtPath.hpp"
#include "UtQt.hpp"
#include "UtQtMemory.hpp"
#include "Util.hpp"
#include "WkfEnvironment.hpp"
#include "WkfScenario.hpp"
#include "WkfUnitsObject.hpp"
#include "WkfVtkEnvironment.hpp"
#include "WsfPM_Root.hpp"
#include "zone/WkfZonePoint.hpp"
#include "zone_browser/WkfZoneBrowserDockWidget.hpp"
#include "zone_browser/WkfZoneBrowserPrefObject.hpp"

ZoneEditor::CreateZoneDialog::CreateZoneDialog(wkf::ZoneBrowserDataContainer& aDataContainer,
                                               WsfPProxy&                     aProxy,
                                               const vespa::VaPosition&       aInitialPosition,
                                               QWidget*                       aParentPtr)
   : QDialog(aParentPtr)
   , mIsClosing(false)
   , mDataContainer(aDataContainer)
   , mProxy(aProxy)
   , mPosition(aInitialPosition)
{
   mUi.setupUi(this);

   mUi.mZoneLayout->setSizeConstraint(QLayout::SetFixedSize);

   PopulateFileList();
   PopulateFileComboBox();
   PopulatePlatformComboBox();
   PopulateZoneComboBox();

   mUi.mZoneTypeBox->setCurrentIndex(0);
   ZoneTypeChanged(mUi.mZoneTypeBox->currentText());

   // Initialize the zone's center point.
   mUi.mLatitudeEdit->SetValue(aInitialPosition.GetLat());
   mUi.mLongitudeEdit->SetValue(aInitialPosition.GetLon());

   mUi.mNameEdit->setValidator(ut::qt::make_qt_ptr<QRegExpValidator>(wizard::Util::wsfValidInputRegexp, this));

   // Initialize values.
   mUi.mMinimumAltitudeEdit->SetValue(
      UtLengthValue(0, wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetAltitudeUnit(0.0)));
   mUi.mMaximumAltitudeEdit->SetValue(
      UtLengthValue(0, wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetAltitudeUnit(0.0)));
   mUi.mHeadingEdit->SetValue(UtAngleValue(0, wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetAngleUnit()));
   mUi.mMinimumRadiusEdit->SetValue(UtLengthValue(0, wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetLengthUnit()));
   mUi.mMaximumRadiusEdit->SetValue(UtLengthValue(0, wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetLengthUnit()));
   mUi.mLateralAxisEdit->SetValue(UtLengthValue(0, wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetLengthUnit()));
   mUi.mLongitudinalAxisEdit->SetValue(UtLengthValue(0, wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetLengthUnit()));
   mUi.mFillColorButton->SetColor(wkfEnv.GetPreferenceObject<wkf::ZoneBrowserPrefObject>()->GetDefaultFillColor());
   mUi.mLineColorButton->SetColor(wkfEnv.GetPreferenceObject<wkf::ZoneBrowserPrefObject>()->GetDefaultLineColor());
   mUi.mFillColorButton->ShowAlpha(true);

   UtAngleValue          startAngle = UtAngleValue(0, UtUnitAngle::cDEGREES);
   UtAngleValue          stopAngle  = UtAngleValue(360, UtUnitAngle::cDEGREES);
   UtUnitAngle::BaseUnit angleUnit  = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetAngleUnit();
   if (angleUnit != UtUnitAngle::BaseUnit::cDEGREES)
   {
      startAngle.ConvertToUnit(angleUnit);
      stopAngle.ConvertToUnit(angleUnit);
   }
   mUi.mStartAngleEdit->SetValue(startAngle);
   mUi.mStopAngleEdit->SetValue(stopAngle);

   mUi.mAbsoluteButton->setChecked(true);

   mUi.mTreeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
   mUi.mTreeWidget->installEventFilter(this);
   connect(mUi.mTreeWidget, &QTreeWidget::customContextMenuRequested, this, &CreateZoneDialog::CreateTreeWidgetContextMenu);
   connect(mUi.mTreeWidget, &QTreeWidget::currentItemChanged, this, &CreateZoneDialog::DisplayPointData);

   // Update the corresponding point value when the user changes an item's latitude/longitude.
   connect(mUi.mPointLatEdit,
           &QLineEdit::editingFinished,
           [this]()
           {
              QTreeWidgetItem* itemPtr = mUi.mTreeWidget->currentItem();

              if (itemPtr != nullptr)
              {
                 mPointsList[mUi.mTreeWidget->indexOfTopLevelItem(itemPtr)].mX = mUi.mPointLatEdit->GetValue();
              }
              UpdatePreview();
           });
   connect(mUi.mPointLonEdit,
           &QLineEdit::editingFinished,
           [this]()
           {
              QTreeWidgetItem* itemPtr = mUi.mTreeWidget->currentItem();

              if (itemPtr != nullptr)
              {
                 mPointsList[mUi.mTreeWidget->indexOfTopLevelItem(itemPtr)].mY = mUi.mPointLonEdit->GetValue();
              }
              UpdatePreview();
           });

   mUi.mPointSelector->SetMultiSelectionEnabled(true);
   connect(mUi.mPointSelector, &wkf::LocationSelector::Selected, this, &CreateZoneDialog::AddPointToTree);
   connect(mUi.mZoneTypeBox,
           QOverload<const QString&>::of(&QComboBox::currentIndexChanged),
           this,
           &CreateZoneDialog::ZoneTypeChanged);
   connect(mUi.mButtonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked, this, &CreateZoneDialog::CreateZone);
   connect(mUi.mButtonBox->button(QDialogButtonBox::Cancel),
           &QPushButton::clicked,
           [&]()
           {
              mIsClosing = true;
              mDataContainer.RemovePreview();
              deleteLater();
           });

   // Update the preview zone each time the user changes a value.
   connect(mUi.mNameEdit, &QLineEdit::editingFinished, this, &CreateZoneDialog::UpdatePreview);
   connect(mUi.mReferencePlatformBox,
           QOverload<const QString&>::of(&QComboBox::currentIndexChanged),
           this,
           [&](const QString& aText)
           {
              if (mUi.mReferencePlatformBox->currentIndex() == 0)
              {
                 mUi.mLatitudeEdit->setEnabled(true);
                 mUi.mLongitudeEdit->setEnabled(true);
              }
              else
              {
                 mUi.mLatitudeEdit->setEnabled(false);
                 mUi.mLongitudeEdit->setEnabled(false);
              }
              UpdatePreview();
           });
   connect(mUi.mHeadingEdit, &QLineEdit::editingFinished, this, &CreateZoneDialog::UpdatePreview);
   connect(mUi.mLatitudeEdit, &QLineEdit::editingFinished, this, &CreateZoneDialog::UpdatePreview);
   connect(mUi.mLongitudeEdit, &QLineEdit::editingFinished, this, &CreateZoneDialog::UpdatePreview);
   connect(mUi.mLateralAxisEdit, &QLineEdit::editingFinished, this, &CreateZoneDialog::UpdatePreview);
   connect(mUi.mLongitudinalAxisEdit, &QLineEdit::editingFinished, this, &CreateZoneDialog::UpdatePreview);
   connect(mUi.mStartAngleEdit, &QLineEdit::editingFinished, this, &CreateZoneDialog::UpdatePreview);
   connect(mUi.mStopAngleEdit, &QLineEdit::editingFinished, this, &CreateZoneDialog::UpdatePreview);
   connect(mUi.mMaximumAltitudeEdit, &QLineEdit::editingFinished, this, &CreateZoneDialog::UpdatePreview);
   connect(mUi.mMinimumAltitudeEdit, &QLineEdit::editingFinished, this, &CreateZoneDialog::UpdatePreview);
   connect(mUi.mMaximumRadiusEdit, &QLineEdit::editingFinished, this, &CreateZoneDialog::UpdatePreview);
   connect(mUi.mMinimumRadiusEdit, &QLineEdit::editingFinished, this, &CreateZoneDialog::UpdatePreview);
   connect(mUi.mRelativeButton,
           &QRadioButton::toggled,
           [&](bool aChecked)
           {
              mUi.mReferencePlatformBox->setEnabled(aChecked);
              UpdatePreview();
           });
   connect(mUi.mFillColorButton, &UtQtColorButton::ColorChanged, this, [&](const QColor& aColor) { UpdatePreview(); });
   connect(mUi.mLineColorButton, &UtQtColorButton::ColorChanged, this, [&](const QColor& aColor) { UpdatePreview(); });

   UpdatePreview();
}

// On closing the dialog, remove the preview from the data container.
void ZoneEditor::CreateZoneDialog::closeEvent(QCloseEvent* aEventPtr)
{
   mIsClosing = true;
   mDataContainer.RemovePreview();
}

// Attempts to add a zone definition given the user's input.
void ZoneEditor::CreateZoneDialog::CreateZone()
{
   // Don't allow the user to create a zone with no name.
   if (!mUi.mNameEdit->text().isEmpty())
   {
      std::string zoneName = mUi.mNameEdit->text().toStdString();

      // Don't allow the user to create a zone with the same name as an existing zone.
      if (!WsfPM_Root(&mProxy).zones().Find(zoneName))
      {
         // Remove the preview zone.
         mDataContainer.RemovePreview();

         std::string fileName = mUi.fileComboBox->currentText().toStdString();
         std::string filePath = UtPath(wizard::Project::Instance()->ProjectDirectory() + fileName).GetSystemPath();
         wizard::TextSource* srcPtr = wizard::ProjectWorkspace::Instance()->FindSource(filePath, false, false);

         if (srcPtr != nullptr)
         {
            std::string       zoneType = mUi.mZoneTypeBox->currentText().toLower().toStdString();
            std::stringstream zoneDef;
            zoneDef << "\n";
            zoneDef << "zone " << zoneName << "\n";
            zoneDef << "   " << zoneType << "\n";

            // Add the line and fill color
            QColor fillColor = mUi.mFillColorButton->GetColor();
            zoneDef << "   fill_color " << fillColor.red() << ' ' << fillColor.green() << ' ' << fillColor.blue() << ' '
                    << fillColor.alpha() << "\n";

            QColor lineColor = mUi.mLineColorButton->GetColor();
            zoneDef << "   line_color " << lineColor.red() << ' ' << lineColor.green() << ' ' << lineColor.blue() << "\n";

            // Add altitudes to the zone definition.
            zoneDef << "      minimum_altitude " << mUi.mMinimumAltitudeEdit->text().toStdString() << "\n";
            zoneDef << "      maximum_altitude " << mUi.mMaximumAltitudeEdit->text().toStdString() << "\n";

            // Set the reference platform. If the zone is an absolue polygonal, this won't be used.
            std::string referencePlatform = mUi.mReferencePlatformBox->currentText().toStdString();
            if (mUi.mReferencePlatformBox->isEnabled())
            {
               // Set the zone's central location and heading. This is only used if there isn't a reference platform specified.
               if (mUi.mReferencePlatformBox->currentIndex() == 0)
               {
                  zoneDef << "      position " << mUi.mLatitudeEdit->text().toStdString() << " "
                          << mUi.mLongitudeEdit->text().toStdString() << "\n";
                  zoneDef << "      heading " << mUi.mHeadingEdit->text().toStdString() << "\n";
               }
               else
               {
                  zoneDef << "      reference_platform " << referencePlatform << "\n";
               }
            }

            if (mUi.mGeometricOptionsBox->isVisible())
            {
               if (zoneType == "circular")
               {
                  zoneDef << "      minimum_radius " << mUi.mMinimumRadiusEdit->text().toStdString() << "\n";
                  zoneDef << "      maximum_radius " << mUi.mMaximumRadiusEdit->text().toStdString() << "\n";
               }
               else if (zoneType == "elliptical")
               {
                  zoneDef << "      lateral_axis " << mUi.mLateralAxisEdit->text().toStdString() << "\n";
                  zoneDef << "      longitudinal_axis " << mUi.mLongitudinalAxisEdit->text().toStdString() << "\n";
               }
               else if (zoneType == "spherical")
               {
                  zoneDef << "      maximum_radius " << mUi.mMaximumRadiusEdit->text().toStdString() << "\n";
               }

               zoneDef << "      start_angle " << mUi.mStartAngleEdit->text().toStdString() << "\n";
               zoneDef << "      stop_angle " << mUi.mStopAngleEdit->text().toStdString() << "\n";
            }
            else if (mUi.mPolygonalOptionsBox->isVisible())
            {
               if (mUi.mAbsoluteButton->isChecked())
               {
                  zoneDef << "      lat_lon\n";

                  for (const auto& point : mPointsList)
                  {
                     zoneDef << "      point " << UtLatPos(point.mX) << " " << UtLonPos(point.mY) << "\n";
                  }
               }
               else
               {
                  auto* scenarioPtr = vaEnv.GetStandardScenario();
                  if (scenarioPtr)
                  {
                     double transECEF[3][3];
                     double refECEF[3];
                     double xyz[3];
                     for (const auto& point : mPointsList)
                     {
                        wkf::Platform* refPlatformPtr = scenarioPtr->FindPlatformByName(referencePlatform);
                        if (refPlatformPtr != nullptr)
                        {
                           const vespa::VaPosition& refPos = refPlatformPtr->GetPosition();
                           UtEllipsoidalEarth::ComputeNEDTransform(refPos.GetLat(),
                                                                   refPos.GetLon(),
                                                                   refPos.GetAlt(),
                                                                   transECEF,
                                                                   refECEF);
                           UtEllipsoidalEarth::ConvertLLAToLocal(refECEF, transECEF, point.mX, point.mY, refPos.GetAlt(), xyz);

                           UtLengthValue xVal = UtLengthValue(xyz[0], UtUnitLength::cMETERS);
                           UtLengthValue yVal = UtLengthValue(xyz[1], UtUnitLength::cMETERS);
                           xVal.ConvertToUnit(wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetLengthUnit());
                           yVal.ConvertToUnit(wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetLengthUnit());

                           zoneDef << "      point " << xVal.GetAsUnit() << " " << yVal.GetAsUnit() << " "
                                   << xVal.GetUnitName() << "\n";
                        }
                     }
                  }
               }
            }

            zoneDef << "end_zone\n";

            if (srcPtr->GetViews().empty())
            {
               srcPtr->NewView(wizard::Project::Instance());
            }

            srcPtr->Append(zoneDef.str());
            deleteLater();
         }
      }
      else
      {
         UtQtShowSimpleMsg("Zone name conflict",
                           "A zone named <b>" + QString::fromStdString(zoneName) + "</b> already exists.",
                           "Please choose a different name or remove the existing zone.",
                           this);
      }
   }
   else
   {
      UtQtShowSimpleMsg("Name required", "Please enter a name for the zone.", "", this);
   }
}

// Shows/hides different options based on what type of zone is selected.
void ZoneEditor::CreateZoneDialog::ZoneTypeChanged(const QString& aText)
{
   if (aText == "Polygonal")
   {
      ShowGeometricCommands(false);
      ShowPolygonalCommands(true);
      mUi.mLatitudeEdit->setDisabled(true);
      mUi.mLongitudeEdit->setDisabled(true);
      if (mUi.mAbsoluteButton->isChecked())
      {
         mUi.mReferencePlatformBox->setDisabled(true);
      }
   }
   else
   {
      ShowGeometricCommands(true);
      ShowPolygonalCommands(false);

      mUi.mLatitudeEdit->setDisabled(false);
      mUi.mLongitudeEdit->setDisabled(false);
      mUi.mReferencePlatformBox->setDisabled(false);

      if (aText == "Circular")
      {
         mUi.mLateralAxisEdit->setEnabled(false);
         mUi.mLongitudinalAxisEdit->setEnabled(false);
         mUi.mMinimumRadiusEdit->setEnabled(true);
         mUi.mMaximumRadiusEdit->setEnabled(true);
      }
      else if (aText == "Elliptical")
      {
         mUi.mMinimumRadiusEdit->setEnabled(false);
         mUi.mMaximumRadiusEdit->setEnabled(false);
         mUi.mLateralAxisEdit->setEnabled(true);
         mUi.mLongitudinalAxisEdit->setEnabled(true);
      }
      else if (aText == "Spherical")
      {
         mUi.mMinimumRadiusEdit->setEnabled(false);
         mUi.mMaximumRadiusEdit->setEnabled(true);
         mUi.mLateralAxisEdit->setEnabled(false);
         mUi.mLongitudinalAxisEdit->setEnabled(false);
      }
   }

   UpdatePreview();
}

// Shows options for geometric zones.
void ZoneEditor::CreateZoneDialog::ShowGeometricCommands(bool aShow)
{
   mUi.mGeometricOptionsBox->setVisible(aShow);
}

// Shows options for polygonal zones.
void ZoneEditor::CreateZoneDialog::ShowPolygonalCommands(bool aShow)
{
   mUi.mPolygonalOptionsBox->setVisible(aShow);
}

// Adds a lat/lon location to the tree and points list.
void ZoneEditor::CreateZoneDialog::AddPointToTree(double aLatitude, double aLongitude, double aAltitude)
{
   wkf::ZonePointStruct point(aLatitude, aLongitude);
   mPointsList.emplace_back(point);

   std::string zoneName{mUi.mNameEdit->text().toStdString()};
   mDataContainer.UpdatePreview(GetCommonVariables(), GetPolygonalVariables(), zoneName, "Global Zones");

   QTreeWidgetItem* treePointPtr = ut::qt::make_qt_ptr<QTreeWidgetItem>(mUi.mTreeWidget);
   treePointPtr->setText(0, QString::fromStdString("Point " + std::to_string(mPointsList.size())));
   mUi.mTreeWidget->addTopLevelItem(treePointPtr);
}

// Displays the latitude and longitude of a selected point. This also gets called when a point is deleted;
// if the point was deleted, clear the line edits as nothing is selected now.
void ZoneEditor::CreateZoneDialog::DisplayPointData(QTreeWidgetItem* aItemPtr, QTreeWidgetItem* aPreviousItemPtr)
{
   if (aItemPtr != nullptr)
   {
      mUi.mPointLatEdit->SetValue(UtLatPos(mPointsList[mUi.mTreeWidget->indexOfTopLevelItem(aItemPtr)].mX));
      mUi.mPointLonEdit->SetValue(UtLonPos(mPointsList[mUi.mTreeWidget->indexOfTopLevelItem(aItemPtr)].mY));
   }
   else
   {
      mUi.mPointLatEdit->clear();
      mUi.mPointLonEdit->clear();
   }
}

// Adds a "remove point" option to the tree widget's context menu
void ZoneEditor::CreateZoneDialog::CreateTreeWidgetContextMenu(const QPoint& aPoint)
{
   QTreeWidgetItem* pointToDeletePtr = mUi.mTreeWidget->itemAt(aPoint);
   if (pointToDeletePtr != nullptr)
   {
      QMenu*   menuPtr         = ut::qt::make_qt_ptr<QMenu>();
      QAction* deleteActionPtr = ut::qt::make_qt_ptr<QAction>("Remove Point", menuPtr);

      connect(deleteActionPtr,
              &QAction::triggered,
              [&]()
              {
                 mPointsList.erase(mPointsList.begin() + mUi.mTreeWidget->indexOfTopLevelItem(pointToDeletePtr));
                 std::string zoneName{mUi.mNameEdit->text().toStdString()};
                 mDataContainer.UpdatePreview(GetCommonVariables(), GetPolygonalVariables(), zoneName, "Global Zones");
                 PopulatePointTree();
              });

      menuPtr->addAction(deleteActionPtr);
      menuPtr->exec(mUi.mTreeWidget->mapToGlobal(aPoint));
      delete menuPtr;
   }
}

// Clear the tree, then add each point sequentially.
void ZoneEditor::CreateZoneDialog::PopulatePointTree()
{
   mUi.mTreeWidget->clear();
   for (unsigned int i = 0; i < mPointsList.size(); i++)
   {
      QTreeWidgetItem* treePointPtr = ut::qt::make_qt_ptr<QTreeWidgetItem>(mUi.mTreeWidget);
      treePointPtr->setText(0, QString::fromStdString("Point " + std::to_string(i + 1)));
      mUi.mTreeWidget->addTopLevelItem(treePointPtr);
   }
}

// Returns the common variables as currently defined in the dialog.
wkf::CommonZoneVariables ZoneEditor::CreateZoneDialog::GetCommonVariables()
{
   wkf::CommonZoneVariables vars;

   if (mUi.mReferencePlatformBox->currentIndex() == 0)
   {
      vars.mReferencePlatformName = "";
      vars.mReferenceLat          = mUi.mLatitudeEdit->GetValue();
      vars.mReferenceLon          = mUi.mLongitudeEdit->GetValue();
      vars.mLocationDefined       = true;
   }
   else
   {
      vars.mReferencePlatformName = mUi.mReferencePlatformBox->currentText().toStdString();
      vars.mLocationDefined       = false;
   }

   // vars.mPlatformName = "Global Zones";  // Not currently supporting creating zones on platforms.
   vars.mZoneName    = mUi.mNameEdit->text().toStdString();
   vars.mHeading     = mUi.mHeadingEdit->GetValue();
   vars.mMinAltitude = mUi.mMinimumAltitudeEdit->GetValue();
   vars.mMaxAltitude = mUi.mMaximumAltitudeEdit->GetValue();

   return vars;
}

// Returns the circular variables as currently defined in the dialog.
wkf::CircularZoneVariables ZoneEditor::CreateZoneDialog::GetCircularVariables()
{
   wkf::CircularZoneVariables vars;
   vars.mRadius     = mUi.mMaximumRadiusEdit->GetValue();
   vars.mStartAngle = mUi.mStartAngleEdit->GetValue();
   vars.mStopAngle  = mUi.mStopAngleEdit->GetValue();

   return vars;
}

// Returns the elliptical variables as currently defined in the dialog.
wkf::EllipticalZoneVariables ZoneEditor::CreateZoneDialog::GetEllipticalVariables()
{
   wkf::EllipticalZoneVariables vars;
   vars.mLatAxis    = mUi.mLateralAxisEdit->GetValue();
   vars.mLonAxis    = mUi.mLongitudinalAxisEdit->GetValue();
   vars.mStartAngle = mUi.mStartAngleEdit->GetValue();
   vars.mStopAngle  = mUi.mStopAngleEdit->GetValue();

   return vars;
}

// Returns the spherical variables as currently defined in the dialog.
wkf::SphericalZoneVariables ZoneEditor::CreateZoneDialog::GetSphericalVariables()
{
   wkf::SphericalZoneVariables vars;
   vars.mRadius     = mUi.mMaximumRadiusEdit->GetValue();
   vars.mMinRadius  = mUi.mMinimumRadiusEdit->GetValue();
   vars.mStartAngle = mUi.mStartAngleEdit->GetValue();
   vars.mStopAngle  = mUi.mStopAngleEdit->GetValue();

   return vars;
}

// Returns the polygonal variables as currently defined in the dialog.
wkf::PolygonalZoneVariables ZoneEditor::CreateZoneDialog::GetPolygonalVariables()
{
   wkf::PolygonalZoneVariables vars;
   vars.mUseLatLon = true;

   for (const auto& point : mPointsList)
   {
      vars.mPoints.emplace_back(point);
   }

   return vars;
}

// Updates the currently previewed zone by grabbing all relevant variables from the dialog and
// sending them to the data container.
void ZoneEditor::CreateZoneDialog::UpdatePreview()
{
   // The editingFinished signal gets called even when closing the dialog, so only update
   // the preview if we aren't closing.
   if (!mIsClosing)
   {
      QString     zoneType = mUi.mZoneTypeBox->currentText();
      std::string zoneName{mUi.mNameEdit->text().toStdString()};
      std::string globalName = "Global Zones";
      // Currently only support added zones at the global level
      if (zoneType == "Circular")
      {
         mDataContainer.UpdatePreview(GetCommonVariables(), GetCircularVariables(), zoneName, globalName);
      }
      else if (zoneType == "Elliptical")
      {
         mDataContainer.UpdatePreview(GetCommonVariables(), GetEllipticalVariables(), zoneName, globalName);
      }
      else if (zoneType == "Spherical")
      {
         mDataContainer.UpdatePreview(GetCommonVariables(), GetSphericalVariables(), zoneName, globalName);
      }
      else if (zoneType == "Polygonal")
      {
         mDataContainer.UpdatePreview(GetCommonVariables(), GetPolygonalVariables(), zoneName, globalName);
      }
   }
}

// Returns a path relative to the project directory.
std::string ZoneEditor::CreateZoneDialog::GetRelativePath(const UtTextDocument* aDocPtr)
{
   std::string path;

   if (aDocPtr != nullptr)
   {
      const UtPath& filePath = aDocPtr->GetFilePath();
      path = wizard::Project::Instance()->ProjectDirectory().GetRelativePath(filePath).GetSystemPath();
   }

   return path;
}

// Populates the file list with all the files in the project, then sorts them alphabetically.
void ZoneEditor::CreateZoneDialog::PopulateFileList()
{
   wizard::ParseResults* resultsPtr = wizard::ProjectWorkspace::Instance()->GetProject()->WaitForParseResults();

   for (const auto& iter : resultsPtr->mFirstIncludes)
   {
      std::string path = GetRelativePath(iter.first);
      if (!path.empty())
      {
         mFileList.emplace_back(path);
      }
   }

   std::sort(mFileList.begin(), mFileList.end(), SortAlphabetical);
}

// Populates the file combo box with all the files in the file list.
void ZoneEditor::CreateZoneDialog::PopulateFileComboBox()
{
   for (const auto& file : mFileList)
   {
      mUi.fileComboBox->addItem(QString::fromStdString(file));
   }
}

// Populates the reference platform combo box with the names of each platform in the scenario.
void ZoneEditor::CreateZoneDialog::PopulatePlatformComboBox()
{
   mUi.mReferencePlatformBox->addItem("<none>");
   for (const auto& platform : WsfPM_Root(&mProxy).platforms())
   {
      mUi.mReferencePlatformBox->addItem(QString::fromStdString(platform.first));
   }
}

// Populates the zones combo box with the types of zones.
void ZoneEditor::CreateZoneDialog::PopulateZoneComboBox()
{
   mUi.mZoneTypeBox->addItem("Circular");
   mUi.mZoneTypeBox->addItem("Elliptical");
   mUi.mZoneTypeBox->addItem("Spherical");
   mUi.mZoneTypeBox->addItem("Polygonal");
}
