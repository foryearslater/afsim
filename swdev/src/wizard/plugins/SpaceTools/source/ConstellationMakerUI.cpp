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

#include "ConstellationMakerUI.hpp"

#include <QIntValidator>
#include <QMessageBox>
#include <QPushButton>

#include "ProxyWatcher.hpp"
#include "UtMath.hpp"
#include "WsfConstellationOptions.hpp"
#include "WsfPM_Root.hpp"
#include "WsfPProxy.hpp"
#include "WsfSpaceConstellationConjunction.hpp"

SpaceTools::ConstellationMakerUI::ConstellationMakerUI()
{
   mUI.setupUi(this);
   mUI.mConstellationPushButton->setDefault(true);

   mUI.mAdvancedWidget->setVisible(false);

   SetValidators();

   setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

   connect(mUI.mConstellationLineEdit, &QLineEdit::textChanged, this, &ConstellationMakerUI::NameChanged);
   connect(mUI.mConstellationPushButton, &QPushButton::clicked, this, &ConstellationMakerUI::AcceptConstellation);
   connect(mUI.mGeneratorPushButton, &QPushButton::clicked, this, &ConstellationMakerUI::AcceptGenerator);
   connect(mUI.mCancelPushButton, &QPushButton::clicked, this, &ConstellationMakerUI::reject);
   connect(mUI.mAdvancedPushButton, &QAbstractButton::toggled, mUI.mAdvancedWidget, &QWidget::setVisible);
   connect(mUI.mPathLineEdit, &QLineEdit::textChanged, this, &ConstellationMakerUI::PathChanged);

   connect(mUI.mInclinationLineEdit, &QLineEdit::textChanged, this, &ConstellationMakerUI::RecomputeConjunction);
   connect(mUI.mSatellitesPerPlaneLineEdit, &QLineEdit::textChanged, this, &ConstellationMakerUI::RecomputeConjunction);
   connect(mUI.mNumberOfPlanesLineEdit, &QLineEdit::textChanged, this, &ConstellationMakerUI::RecomputeConjunction);
   connect(mUI.mRAAN_RangeLineEdit, &QLineEdit::textChanged, this, &ConstellationMakerUI::RecomputeConjunction);
   connect(mUI.mAnomalyAliasLineEdit, &QLineEdit::textChanged, this, &ConstellationMakerUI::RecomputeConjunction);
}

//! Load options into the fields in the dialog.
void SpaceTools::ConstellationMakerUI::LoadValues(const WsfConstellationOptions& aOptions)
{
   mUI.mConstellationLineEdit->setText(aOptions.GetConstellationName().c_str());
   mUI.mPlatformTypeLineEdit->setText(aOptions.GetPlatformType().c_str());
   mUI.mPathLineEdit->setText(aOptions.GetBasePath().GetFileName().c_str());
   mUI.mOrbitSize->SetValue(aOptions);
   mUI.mInclinationLineEdit->SetValue(aOptions.GetInclination());
   mUI.mNumberOfPlanesLineEdit->setText(QString::number(aOptions.GetNumPlanes()));
   mUI.mSatellitesPerPlaneLineEdit->setText(QString::number(aOptions.GetSatsPerPlane()));
   mUI.mInitialRAAN_LineEdit->SetValue(aOptions.GetInitialRAAN());
   mUI.mRAAN_RangeLineEdit->SetValue(aOptions.GetRAAN_Range());
   mUI.mInitialAnomalyLineEdit->SetValue(aOptions.GetInitialAnomaly());
   mUI.mAnomalyAliasLineEdit->SetValue(aOptions.GetAnomalyAlias());
}

//! Save options from the fields in the dialog.
void SpaceTools::ConstellationMakerUI::SaveValues(WsfConstellationOptions& aOptions)
{
   unsigned int p         = mUI.mNumberOfPlanesLineEdit->text().toUInt();
   unsigned int s         = mUI.mSatellitesPerPlaneLineEdit->text().toUInt();
   UtAngleValue raanRange = mUI.mRAAN_RangeLineEdit->GetValue();
   UtAngleValue anomAlias = mUI.mAnomalyAliasLineEdit->GetValue();
   aOptions               = WsfConstellationOptions::CreateGeneral(p, s, anomAlias, raanRange);

   aOptions.SetConstellationName(mUI.mConstellationLineEdit->text().toStdString());
   aOptions.SetPlatformType(mUI.mPlatformTypeLineEdit->text().toStdString());
   aOptions.SetBasePath(mUI.mPathLineEdit->text().toStdString());
   mUI.mOrbitSize->GetValue(aOptions);
   aOptions.SetInclination(mUI.mInclinationLineEdit->GetValue());
   aOptions.SetInitialRAAN(mUI.mInitialRAAN_LineEdit->GetValue());
   aOptions.SetInitialAnomaly(mUI.mInitialAnomalyLineEdit->GetValue());
}

void SpaceTools::ConstellationMakerUI::showEvent(QShowEvent* aEvent)
{
   // show map-orbits
   QDialog::showEvent(aEvent);
}

void SpaceTools::ConstellationMakerUI::hideEvent(QHideEvent* aEvent)
{
   // hide map-orbits
   QDialog::hideEvent(aEvent);
}

//! Slot to update the presented output filenames when the constellation name is modified.
void SpaceTools::ConstellationMakerUI::NameChanged(const QString& aName)
{
   QString labelConstellation{};
   QString labelGenerator{};
   if (!aName.isEmpty())
   {
      labelConstellation = QString("%1%2_autogen.txt").arg(mPath).arg(aName);
      labelGenerator     = QString("%1%2_generator.txt").arg(mPath).arg(aName);
   }
   else
   {
      labelConstellation = mPath;
      labelGenerator     = mPath;
   }
   mUI.mConstellationOutputLabel->setText(labelConstellation);
   mUI.mGeneratorOutputLabel->setText(labelGenerator);
}

// Slot to update the presented output filenames when the generation path is modified.
void SpaceTools::ConstellationMakerUI::PathChanged(const QString& aPath)
{
   mPath = aPath;
   if (!aPath.isEmpty())
   {
      mPath += "/";
   }
   while (mPath.indexOf("//") != -1)
   {
      mPath.replace(QRegularExpression("//"), "/");
   }
   NameChanged(mUI.mConstellationLineEdit->text());
}

//! Slot called when the user activates the Constellation push button.
//!
//! This only activates the QDialog accept behavior if the input fully validates.
void SpaceTools::ConstellationMakerUI::AcceptConstellation()
{
   if (ValidateInput())
   {
      emit GenerateConstellation();
   }
}

//! Slot called when the user activates the Generator push button.
//!
//! This only activates the QDialog accept behavior if the input fully validates.
void SpaceTools::ConstellationMakerUI::AcceptGenerator()
{
   if (ValidateInput())
   {
      emit GenerateGenerator();
   }
}

void SpaceTools::ConstellationMakerUI::RecomputeConjunction()
{
   int  numPlanes    = mUI.mNumberOfPlanesLineEdit->text().toInt();
   int  satsPerPlane = mUI.mSatellitesPerPlaneLineEdit->text().toInt();
   auto inclination  = mUI.mInclinationLineEdit->GetValue();
   auto raanRange    = mUI.mRAAN_RangeLineEdit->GetValue();
   auto anomalyAlias = mUI.mAnomalyAliasLineEdit->GetValue();
   bool validInput =
      mUI.mInclinationLineEdit->IsValid() && mUI.mAnomalyAliasLineEdit->IsValid() && mUI.mRAAN_RangeLineEdit->IsValid();
   if (validInput)
   {
      bool willCollide =
         wsf::space::ConstellationConjunction::Assess(numPlanes, satsPerPlane, inclination, raanRange, anomalyAlias);
      UpdateConjunctionWarning(willCollide);
   }
}

//! Update the conjunction warning.
void SpaceTools::ConstellationMakerUI::UpdateConjunctionWarning(bool aWarn)
{
   if (aWarn)
   {
      mUI.mWarningLabel->setText(QString{"<span style = \"color: yellow\">WARNING:</span> Constellation will suffer a "
                                         "conjunction between its member platforms."});
   }
   else
   {
      mUI.mWarningLabel->setText("");
   }
}

//! Utility method to set validators on the various inputs in the dialog.
void SpaceTools::ConstellationMakerUI::SetValidators()
{
   mUI.mConstellationLineEdit->setValidator(new QRegExpValidator(QRegExp("[a-zA-Z]\\w*"), mUI.mConstellationLineEdit));
   mUI.mPlatformTypeLineEdit->setValidator(new QRegExpValidator(QRegExp("[a-zA-Z]\\w*"), mUI.mPlatformTypeLineEdit));

   mUI.mInclinationLineEdit->SetBounds(UtAngleValue{0.0, UtUnitAngle::cDEGREES},
                                       UtAngleValue{180.0, UtUnitAngle::cDEGREES});
   mUI.mNumberOfPlanesLineEdit->setValidator(new QIntValidator(1, 360, mUI.mNumberOfPlanesLineEdit));
   mUI.mSatellitesPerPlaneLineEdit->setValidator(new QIntValidator(1, 360, mUI.mSatellitesPerPlaneLineEdit));
   mUI.mInitialRAAN_LineEdit->SetBounds(UtAngleValue{0.0, UtUnitAngle::cDEGREES},
                                        UtAngleValue{360.0, UtUnitAngle::cDEGREES});
   mUI.mRAAN_RangeLineEdit->SetBounds(UtAngleValue{0.0, UtUnitAngle::cDEGREES}, UtAngleValue{360.0, UtUnitAngle::cDEGREES});
   mUI.mInitialAnomalyLineEdit->SetBounds(UtAngleValue{0.0, UtUnitAngle::cDEGREES},
                                          UtAngleValue{360.0, UtUnitAngle::cDEGREES});
   mUI.mAnomalyAliasLineEdit->SetBounds(UtAngleValue{0.0, UtUnitAngle::cDEGREES},
                                        UtAngleValue{360.0, UtUnitAngle::cDEGREES});
}

//! Validates the input.
//!
//! This is needed because QValidators can be in an intermediate state, and so that we can be sure
//! that there is something entered for the constellation name and the platform type.
//! In addition to the validation, this method will provide feedback to the user in the case of a
//! failed validation highlighting the problems in the input.
//!
//! \return - true if all input validates; false otherwise.
bool SpaceTools::ConstellationMakerUI::ValidateInput()
{
   QStringList errors;
   if (mUI.mConstellationLineEdit->text().isEmpty())
   {
      errors.push_back(QString("You must enter a name for the constellation."));
   }
   if (mUI.mPlatformTypeLineEdit->text().isEmpty())
   {
      errors.push_back(QString("You must enter a platform type."));
   }
   else
   {
      if (!CheckPlatformTypeHasSpaceMover(mUI.mPlatformTypeLineEdit->text()))
      {
         errors.push_back(QString("Platform type '") + mUI.mPlatformTypeLineEdit->text() +
                          QString("' does not exist, or does not have a space mover."));
      }
   }
   if (mUI.mPathLineEdit->text().isEmpty())
   {
      errors.push_back(QString("You must enter a file generation path."));
   }
   mUI.mOrbitSize->ValidateInput(errors);
   if (!mUI.mInclinationLineEdit->IsValid())
   {
      errors.push_back(QString("Inclination must be in the range [0.0, 180.0] degrees."));
   }
   if (!mUI.mNumberOfPlanesLineEdit->hasAcceptableInput())
   {
      errors.push_back(QString("Number of planes must be in the range [1, 360]."));
   }
   if (!mUI.mSatellitesPerPlaneLineEdit->hasAcceptableInput())
   {
      errors.push_back(QString("Number of satellites per plane must be in the range [1, 360]."));
   }
   if (!mUI.mInitialRAAN_LineEdit->IsValid())
   {
      errors.push_back(QString("Initial RAAN must be in the range [0.0, 360.0] degrees."));
   }
   if (!mUI.mRAAN_RangeLineEdit->IsValid())
   {
      errors.push_back(QString("RAAN range must be in the range [0.0, 360.0] degrees."));
   }
   if (!mUI.mInitialAnomalyLineEdit->IsValid())
   {
      errors.push_back(QString("Initial anomaly must be in the range [0.0, 360.0] degrees."));
   }
   if (!mUI.mAnomalyAliasLineEdit->IsValid())
   {
      errors.push_back(QString("Anomaly alias must be in the range [0.0, 360.0] degrees."));
   }

   if (errors.size() == 1)
   {
      QMessageBox::warning(this, QString("Error in Constellation Options"), errors[0], QMessageBox::Ok);
   }
   else if (errors.size() > 1)
   {
      QMessageBox::warning(this, QString("Errors in Constellation Options"), errors.join("\n"), QMessageBox::Ok);
   }

   return errors.isEmpty();
}

bool SpaceTools::ConstellationMakerUI::CheckPlatformTypeHasSpaceMover(const QString& aPlatformType)
{
   bool retval{false};

   WsfPProxy*        proxyPtr = wizard::ProxyWatcher::GetActiveProxy();
   WsfPM_Root        root{proxyPtr};
   WsfPM_PlatformMap platforms = root.platformTypes();
   WsfPM_Platform    type      = platforms.FindT(aPlatformType.toStdString());
   if (type.IsValid())
   {
      WsfPM_Mover moverNode = type.Mover();
      retval                = moverNode.IsOfType("WSF_SPACE_MOVER_BASE");
   }

   return retval;
}
