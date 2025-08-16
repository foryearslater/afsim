// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2016 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "SensorAnalysisDialog.hpp"

#include <QFileDialog>
#include <QMessageBox>

#include "HorizontalCoverageOptionsDialog.hpp"
#include "HorizontalMapOptionsDialog.hpp"
#include "OutputVariablesWidget.hpp"
#include "Project.hpp"
#include "ProjectWorkspace.hpp"
#include "ProxyInterface.hpp"
#include "Signals.hpp"
#include "SphericalMapOptionsDialog.hpp"
#include "TargetPlatformTypeDialog.hpp"
#include "VerticalCoverageOptionsDialog.hpp"
#include "VerticalMapOptionsDialog.hpp"

SPLAT::SensorAnalysisDialog::SensorAnalysisDialog(ProxyInterface* aProxyInterface, QWidget* aParent /*= nullptr*/)
   : QDialog(aParent)
   , mHorizontalCoverageDialog(new HorizontalCoverageOptionsDialog(this))
   , mHorizontalMapDialog(new HorizontalMapOptionsDialog(this))
   , mSphericalMapDialog(new SphericalMapOptionsDialog(this))
   , mVerticalCoverageDialog(new VerticalCoverageOptionsDialog(this))
   , mVerticalMapDialog(new VerticalMapOptionsDialog(this))
   , mTargetPlatformTypeDialog(new TargetPlatformTypeDialog(this))
   , mProxyInterface(aProxyInterface)
   , mProject(wizard::Project::Instance())
{
   mUi.setupUi(this);
   mUi.mGenerateHCD->setChecked(true);
   mUi.mGenerateHM->setChecked(true);
   mUi.mGenerateVCD->setChecked(true);
   mUi.mGenerateVM->setChecked(true);
   mUi.mGenerateSM->setChecked(true);
   mUi.mPlotHCD->setChecked(true);
   mUi.mPlotHM->setChecked(true);
   mUi.mPlotVCD->setChecked(true);
   mUi.mPlotVM->setChecked(true);
   mUi.mPlotSM->setChecked(true);
   mUi.mUseDefaultRadarFrequencyCheckbox->setChecked(true);
   mUi.mShowCalibrationDataCheckBox->setChecked(true);
   mProjectDirectory = QString::fromStdString(mProject->ProjectDirectory().GetNormalizedPath());
   mOutputDirectory  = mProjectDirectory;
   Connect();
}

SPLAT::SensorAnalysisDialog::~SensorAnalysisDialog()
{
   delete mHorizontalCoverageDialog;
   delete mHorizontalMapDialog;
   delete mSphericalMapDialog;
   delete mVerticalCoverageDialog;
   delete mVerticalMapDialog;
   delete mTargetPlatformTypeDialog;
}

void SPLAT::SensorAnalysisDialog::Show()
{
   // We need to call raise() and activateWindow() because calling show()
   // will not bring the non-modal dialog to the front
   show();
   raise();
   activateWindow();
   Initialize();
}

void SPLAT::SensorAnalysisDialog::Update()
{
   mHorizontalCoverageDialog->UpdateModes(
      mProxyInterface->GetModeNames()); // signal for HCD dialog to update approprate widget
   mHorizontalCoverageDialog->UpdatePlatformTypes(
      mProxyInterface->GetPlatformTypes()); // signal for HCD dialog to update approprate widget
   mHorizontalCoverageDialog->CheckValidity();

   mHorizontalMapDialog->UpdateModes(
      mProxyInterface->GetModeNames()); // signal for HorizontalMap dialog to update approprate widget
   mHorizontalMapDialog->UpdatePlatformTypes(
      mProxyInterface->GetPlatformTypes()); // signal for HorizontalMap dialog to update approprate widget
   mHorizontalMapDialog->CheckValidity();

   mSphericalMapDialog->UpdateModes(
      mProxyInterface->GetModeNames()); // signal for HorizontalMap dialog to update approprate widget
   mSphericalMapDialog->UpdatePlatformTypes(
      mProxyInterface->GetPlatformTypes()); // signal for HorizontalMap dialog to update approprate widget
   mSphericalMapDialog->CheckValidity();

   mVerticalMapDialog->UpdateModes(
      mProxyInterface->GetModeNames()); // signal for VerticalMap dialog to update approprate widget
   mVerticalMapDialog->UpdatePlatformTypes(
      mProxyInterface->GetPlatformTypes()); // signal for VerticalMap dialog to update approprate widget
   mVerticalMapDialog->CheckValidity();

   mVerticalCoverageDialog->UpdateModes(mProxyInterface->GetModeNames()); // signal for VCD dialog to update approprate widget
   mVerticalCoverageDialog->UpdatePlatformTypes(
      mProxyInterface->GetPlatformTypes()); // signal for VCD dialog to update approprate widget
   mVerticalCoverageDialog->CheckValidity();

   mTargetPlatformTypeDialog->UpdatePlatformTypes(mProxyInterface->GetPlatformTypes());
   mTargetPlatformTypeDialog->UpdateRadarSignatures(mProxyInterface->GetRadarSignatures());
   mTargetPlatformTypeDialog->UpdateInfraredSignatures(mProxyInterface->GetInfraredSignatures());
   mTargetPlatformTypeDialog->UpdateOpticalSignatures(mProxyInterface->GetOpticalSignatures());
   mTargetPlatformTypeDialog->UpdateAcousticSignatures(mProxyInterface->GetAcousticSignatures());
   mTargetPlatformTypeDialog->CheckValidity();
}

bool SPLAT::SensorAnalysisDialog::PlotsRequested()
{
   return mUi.mPlotHM->isChecked() || mUi.mPlotVM->isChecked() || mUi.mPlotHCD->isChecked() ||
          mUi.mPlotVCD->isChecked() || mUi.mPlotSM->isChecked();
}

void SPLAT::SensorAnalysisDialog::SetStatusMessage(const QString& aMessage)
{
   mUi.mStatusMessage->setText(aMessage);
}

void SPLAT::SensorAnalysisDialog::EnableKill(bool aEnable /*= true*/)
{
   mUi.mKillButton->setEnabled(aEnable);
   mHorizontalCoverageDialog->close();
   mHorizontalMapDialog->close();
   mSphericalMapDialog->close();
   mVerticalCoverageDialog->close();
   mVerticalMapDialog->close();
   mTargetPlatformTypeDialog->close();
}

void SPLAT::SensorAnalysisDialog::SetSensorId(const QString& aSensorName)
{
   mHorizontalCoverageDialog->SetSensorId(aSensorName);
   mHorizontalMapDialog->SetSensorId(aSensorName);
   mSphericalMapDialog->SetSensorId(aSensorName);
   mVerticalCoverageDialog->SetSensorId(aSensorName);
   mVerticalMapDialog->SetSensorId(aSensorName);
}

void SPLAT::SensorAnalysisDialog::closeEvent(QCloseEvent* aCloseEvent)
{
   mProxyInterface->ResetProxyNode();

   // If close is called programmatically on ViSensorAnalysisDialog (like on ProjectClosing),
   // we need to call close on the dialogs owned by it
   mHorizontalCoverageDialog->close();
   mHorizontalMapDialog->close();
   mSphericalMapDialog->close();
   mVerticalCoverageDialog->close();
   mVerticalMapDialog->close();
   mTargetPlatformTypeDialog->close();
}

void SPLAT::SensorAnalysisDialog::Initialize()
{
   // When ProjectOpened is called, the project directory of the new project isn't initialized yet
   // On ProjectOpened() we set mOutputDirectory to empty.
   // If we haven't changed projects, we still want the dialog to remember the output directory that the user may have modified.
   if (mOutputDirectory.isEmpty())
   {
      mProjectDirectory = QString::fromStdString(mProject->ProjectDirectory().GetNormalizedPath());
      mOutputDirectory  = mProjectDirectory;
   }
   mUi.mOutputDirectoryEdit->setText(mOutputDirectory);
   mUi.mKillButton->setEnabled(false);
   mUi.mStatusMessage->setText("");

   QString sensorName = QString::fromStdString(mProxyInterface->GetProxyNodeName());
   setWindowTitle(sensorName + " Analysis");
}

void SPLAT::SensorAnalysisDialog::Connect()
{
   connect(mProxyInterface, &ProxyInterface::ProxyUpdated, this, &SensorAnalysisDialog::Update);

   mCallbacks.Add(wizard::Project::ProjectClosing.Connect([this](wizard::Project* aProject) { close(); }));
   connect(wizard::ProjectWorkspace::Instance(),
           &wizard::ProjectWorkspace::ProjectOpened,
           this,
           &SensorAnalysisDialog::ProjectOpened);

   connect(mUi.mOutputDirectoryEdit, &QLineEdit::textChanged, this, &SensorAnalysisDialog::OutputDirectoryChangedHandler);
   connect(mUi.mBrowseButton, &QPushButton::pressed, this, &SensorAnalysisDialog::OutputBrowsePressedHandler);

   connect(mUi.mHorizontalCoverageOptionsButton,
           &QPushButton::pressed,
           this,
           [this]() { mHorizontalCoverageDialog->show(); });
   connect(mUi.mHorizontalMapOptionsButton, &QPushButton::pressed, this, [this]() { mHorizontalMapDialog->show(); });
   connect(mUi.mVerticalCoverageOptionsButton, &QPushButton::pressed, this, [this]() { mVerticalCoverageDialog->show(); });
   connect(mUi.mVerticalMapOptionsButton, &QPushButton::pressed, this, [this]() { mVerticalMapDialog->show(); });
   connect(mUi.mSphericalMapOptionsButton, &QPushButton::pressed, this, [this]() { mSphericalMapDialog->show(); });
   connect(mUi.mTargetPlatformTypeButton, &QPushButton::pressed, this, [this]() { mTargetPlatformTypeDialog->show(); });

   connect(mUi.mRunButton, &QPushButton::pressed, this, &SensorAnalysisDialog::WriteSensorPlotFile);
   connect(mUi.mKillButton, &QPushButton::pressed, this, [this]() { emit KillButtonPressed(); });
}

void SPLAT::SensorAnalysisDialog::WriteSensorPlotFile()
{
   // Create a new file, open for writing ---------------------------------------------
   std::string outputFilename =
      mOutputDirectory.toStdString() + "/sp_" + mProxyInterface->GetProxyNodeName() + "_input.txt";
   std::ofstream stream(outputFilename, std::ofstream::out | std::ofstream::trunc); // write only, truncate

   if (stream.good())
   {
      stream << "# New file created by SPLAT Plugin\n\n";
   }
   stream.close();

   // Reopen file for appending -------------------------------------------------------
   stream.open(outputFilename, std::ofstream::app); // append

   // include neccessary files (platform defs, sensor defs, etc)
   WritePlatformTypeIncludes(stream);

   UtPath sensorFile = mProxyInterface->GetProxyNodeFile();
   UtPath projectDir = mOutputDirectory.toStdString();
   UtPath relPath    = projectDir.GetRelativePath(sensorFile);
   if (stream.good())
   {
      stream << "include_once " << relPath.GetNormalizedPath() << "\n";

      if (mUi.mUseDefaultRadarFrequencyCheckbox->isChecked())
      {
         stream << "\nuse_default_radar_frequencies true\n";
      }
   }

   WriteTargetPlatformType(stream);
   WriteSensorPlatformType(stream);

   bool checkFiles = false;

   if (mUi.mGenerateHM->isChecked())
   {
      WriteHorizontalMap(stream);
      checkFiles = true;
   }

   if (mUi.mGenerateVM->isChecked())
   {
      WriteVerticalMap(stream);
      checkFiles = true;
   }

   if (mUi.mGenerateHCD->isChecked())
   {
      WriteHorizontalCoverage(stream);
      checkFiles = true;
   }

   if (mUi.mGenerateVCD->isChecked())
   {
      WriteVerticalCoverage(stream);
      checkFiles = true;
   }

   if (mUi.mGenerateSM->isChecked())
   {
      WriteSphericalMap(stream);
      checkFiles = true;
   }

   if (checkFiles)
   {
      wizSignals->CheckFilesForModification();
   }

   if (PlotsRequested())
   {
      // Write GNU Plot script (.gnu file)
      std::string gnuFilename = mOutputDirectory.toStdString() + "/plot_" + mProxyInterface->GetProxyNodeName() + ".gnu";
      std::ofstream gnuFileStream(gnuFilename, std::ofstream::out); // write only
      if (gnuFileStream.good())
      {
         gnuFileStream << "# Created by SPLAT, user modifications may be overwritten\n";
      }
      else
      {
         QMessageBox::critical(this, "Error", "GNU Plot File Stream is BAD");
      }

      size_t plotIndex = 0;

      if (mUi.mPlotHCD->isChecked())
      {
         QString filename = mHorizontalCoverageDialog->GetOutputFilename();
         if (!filename.isEmpty())
         {
            gnuFileStream << "\nset terminal wxt " << plotIndex++ << "\n";
            gnuFileStream << "reset\n";
            gnuFileStream << "set grid\n";
            gnuFileStream << "set size square 1,1\n";
            gnuFileStream << "set view equal xy\n";
            gnuFileStream << "plot \"" << filename.toStdString() << "\" using 3:4 with lines\n\n";

            gnuFileStream << "set terminal wxt " << plotIndex++ << "\n";
            gnuFileStream << "reset\n";
            gnuFileStream << "set autoscale\n";
            gnuFileStream << "set polar\n";
            gnuFileStream << "set size square 1,1\n";
            gnuFileStream << "set angles degrees\n";
            // gnuFileStream << "set rrange[0:500]\n";
            gnuFileStream << "set grid polar 30\n";
            gnuFileStream << "plot \"" << filename.toStdString() << "\" using 1:2 with lines\n\n";
         }
      }

      if (mUi.mPlotHM->isChecked())
      {
         QString filename = mHorizontalMapDialog->GetGnuplotFilename();
         if (!filename.isEmpty())
         {
            QString cleanFilename = filename;
            cleanFilename.replace('_', "\\_", Qt::CaseInsensitive);
            QStringList variables =
               static_cast<OutputVariablesWidget*>(mHorizontalMapDialog->GetWidget("OutputSelection"))->GetSelectedVariables();
            int varIndex = 3;
            for (auto&& var : variables)
            {
               QString cleanVarName = var;
               cleanVarName.replace('_', "\\_", Qt::CaseInsensitive);
               gnuFileStream << "\nset terminal wxt " << plotIndex++ << "\n";
               gnuFileStream << "reset\n";
               gnuFileStream << "unset surface\n";
               gnuFileStream << "set pm3d\n";
               gnuFileStream << "set view 0,0\n";
               gnuFileStream << "set size square 1,1\n";
               gnuFileStream << "set view equal xy\n";
               gnuFileStream << "set autoscale fix\n";
               gnuFileStream << "set zrange [-299:299]\n";
               gnuFileStream << "set xlabel \"Down Range\"\n";
               gnuFileStream << "set ylabel \"Cross Range\" offset 1,1 rotate parallel\n";
               gnuFileStream << "splot \"" << filename.toStdString() << "\" using 1:2:" << varIndex << " with pm3d \\\n";
               gnuFileStream << "      title '" << cleanFilename.toStdString() << " using 1:2:" << varIndex << " ("
                             << cleanVarName.toStdString() << ")'\n";

               ++varIndex;
            }
         }
      }

      if (mUi.mPlotSM->isChecked())
      {
         QString filename = mSphericalMapDialog->GetGnuplotFilename();
         if (!filename.isEmpty())
         {
            QString cleanFilename = filename;
            cleanFilename.replace('_', "\\_", Qt::CaseInsensitive);
            QStringList variables =
               static_cast<OutputVariablesWidget*>(mSphericalMapDialog->GetWidget("OutputSelection"))->GetSelectedVariables();
            int varIndex = 3;
            for (auto&& var : variables)
            {
               QString cleanVarName = var;
               cleanVarName.replace('_', "\\_", Qt::CaseInsensitive);
               gnuFileStream << "\nset terminal wxt " << plotIndex++ << "\n";
               gnuFileStream << "reset\n";
               gnuFileStream << "unset surface\n";
               gnuFileStream << "set pm3d\n";
               gnuFileStream << "set angles degrees\n";
               gnuFileStream << "set mapping spherical\n";
               gnuFileStream << "set xlabel \"Azimuth\"\n";
               gnuFileStream << "set ylabel \"Elevation\"\n";
               gnuFileStream << "splot \"" << filename.toStdString() << "\" using 1:2:" << varIndex << " with pm3d \\\n";
               gnuFileStream << "      title '" << cleanFilename.toStdString() << " using 1:2:" << varIndex << " ("
                             << cleanVarName.toStdString() << ")'\n";

               ++varIndex;
            }
         }
      }

      if (mUi.mPlotVCD->isChecked())
      {
         QString filename = mVerticalCoverageDialog->GetOutputFilename();
         if (!filename.isEmpty())
         {
            gnuFileStream << "\nset terminal wxt " << plotIndex++ << "\n";
            gnuFileStream << "reset\n";
            gnuFileStream << "set grid\n";
            gnuFileStream << "plot \"" << filename.toStdString() << "\" using 3:4 with lines\n\n";
         }
      }

      if (mUi.mPlotVM->isChecked())
      {
         QString filename = mVerticalMapDialog->GetGnuplotFilename();
         if (!filename.isEmpty())
         {
            QString cleanFilename = filename;
            cleanFilename.replace('_', "\\_", Qt::CaseInsensitive);
            QStringList variables =
               static_cast<OutputVariablesWidget*>(mVerticalMapDialog->GetWidget("OutputSelection"))->GetSelectedVariables();
            int varIndex = 3;
            for (auto&& var : variables)
            {
               QString cleanVarName = var;
               cleanVarName.replace('_', "\\_", Qt::CaseInsensitive);
               gnuFileStream << "\nset terminal wxt " << plotIndex++ << "\n";
               gnuFileStream << "reset\n";
               gnuFileStream << "unset surface\n";
               gnuFileStream << "set pm3d\n";
               gnuFileStream << "set view 0,0\n";
               gnuFileStream << "set zrange [-299:299]\n";
               gnuFileStream << "set xlabel \"Range\"\n";
               gnuFileStream << "set ylabel \"Altitude\" offset 1,1 rotate parallel\n";
               gnuFileStream << "splot \"" << filename.toStdString() << "\" using 1:2:" << varIndex << " with pm3d \\\n";
               gnuFileStream << "      title '" << cleanFilename.toStdString() << " using 1:2:" << varIndex << " ("
                             << cleanVarName.toStdString() << ")'\n";

               ++varIndex;
            }
         }
      }

      gnuFileStream << "\npause -1\n";
      gnuFileStream.close();
   }

   wizSignals->CheckFilesForModification();
   emit SensorPlotFileReady(outputFilename, mOutputDirectory.toStdString());

   SetSensorId(QString::fromStdString(mProxyInterface->GetProxyNodeName()));
}

void SPLAT::SensorAnalysisDialog::WriteTargetPlatformType(std::ofstream& aStream)
{
   if (aStream.good())
   {
      std::string targetPlatformFile = "";
      if (!mProxyInterface->ContainsPlatformType("TARGET_PLATFORM_TYPE"))
      {
         mTargetPlatformTypeDialog->WriteData(aStream);
      }
   }

   mHorizontalMapDialog->SetTargetPlatformType(mTargetPlatformTypeDialog->GetTargetPlatformType());
   mVerticalMapDialog->SetTargetPlatformType(mTargetPlatformTypeDialog->GetTargetPlatformType());
}

void SPLAT::SensorAnalysisDialog::WriteSensorPlatformType(std::ofstream& aStream)
{
   if (aStream.good() && (!mProxyInterface->ContainsPlatformType("SENSOR_PLATFORM_TYPE")))
   {
      aStream << "\nplatform_type SENSOR_PLATFORM_TYPE WSF_PLATFORM\n";
      aStream << "   sensor sensor-1 " << mProxyInterface->GetProxyNodeName() << "\n";
      if (mUi.mShowCalibrationDataCheckBox->isChecked())
      {
         aStream << "      show_calibration_data\n";
      }
      aStream << "   end_sensor\n";
      aStream << "end_platform_type\n";
   }
}

void SPLAT::SensorAnalysisDialog::WritePlatformTypeIncludes(std::ofstream& aStream)
{
   if (aStream.good())
   {
      UtPath projectDir = mOutputDirectory.toStdString();
      UtPath targetPlatformFile;
      if (!mProxyInterface->ContainsPlatformType("TARGET_PLATFORM_TYPE"))
      {
         QString targetType            = mTargetPlatformTypeDialog->GetTargetPlatformType();
         QString radarSignatureType    = mTargetPlatformTypeDialog->GetRadarSignatureType();
         QString infraredSignatureType = mTargetPlatformTypeDialog->GetInfraredSignatureType();
         QString opticalSignatureType  = mTargetPlatformTypeDialog->GetOpticalSignatureType();
         QString acousticSignatureType = mTargetPlatformTypeDialog->GetAcousticSignatureType();

         QString radarSignatureValue    = mTargetPlatformTypeDialog->GetRadarSignatureValue();
         QString infraredSignatureValue = mTargetPlatformTypeDialog->GetInfraredSignatureValue();
         QString opticalSignatureValue  = mTargetPlatformTypeDialog->GetOpticalSignatureValue();
         QString acousticSignatureValue = mTargetPlatformTypeDialog->GetAcousticSignatureValue();

         // Find the target platform that the user specified, include the file it's in
         if (!targetType.isEmpty()) // the user specified a target platform
         {
            targetPlatformFile = mProxyInterface->GetPlatformTypeFile(targetType);
            if (targetPlatformFile.Exists())
            {
               aStream << "include_once " << projectDir.GetRelativePath(targetPlatformFile).GetNormalizedPath() << "\n";
            }
         }
         else // we need to write a platform_type definition for TARGET_PLATFORM_TYPE
         {
            // check to see what signatures are entered in dialog - find what file they're in, and #include it.
            if (!radarSignatureType.isEmpty())
            {
               UtPath radarSignatureFile = mProxyInterface->GetRadarSignatureFile(radarSignatureType);
               if (radarSignatureFile.Exists())
               {
                  aStream << "include_once " << projectDir.GetRelativePath(radarSignatureFile).GetNormalizedPath() << "\n";
               }
            }
            if (!infraredSignatureType.isEmpty())
            {
               UtPath infraredSignatureFile = mProxyInterface->GetInfraredSignatureFile(infraredSignatureType);
               if (infraredSignatureFile.Exists())
               {
                  aStream << "include_once " << projectDir.GetRelativePath(infraredSignatureFile).GetNormalizedPath()
                          << "\n";
               }
            }
            if (!opticalSignatureType.isEmpty())
            {
               UtPath opticalSignatureFile = mProxyInterface->GetOpticalSignatureFile(opticalSignatureType);
               if (opticalSignatureFile.Exists())
               {
                  aStream << "include_once " << projectDir.GetRelativePath(opticalSignatureFile).GetNormalizedPath()
                          << "\n";
               }
            }
            // if (!acousticSignatureType.isEmpty())
            //{
            //    UtPath acousticSignatureFile = mProxyInterface->GetAcousticSignatureFile(acousticSignatureType);
            //    if (acousticSignatureFile.Exists())
            //    {
            //       aStream << "include_once " << projectDir.GetRelativePath(acousticSignatureFile).GetNormalizedPath()
            //       << "\n";
            //    }
            // }
         }
      }
      else // find where TARGET_PLATFORM_TYPE is defined and #include it
      {
         targetPlatformFile = mProxyInterface->GetPlatformTypeFile("TARGET_PLATFORM_TYPE");
         if (targetPlatformFile.Exists())
         {
            aStream << "include_once " << projectDir.GetRelativePath(targetPlatformFile).GetNormalizedPath() << "\n";
         }
      }

      if (mProxyInterface->ContainsPlatformType(
             "SENSOR_PLATFORM_TYPE")) // find where SENSOR_PLATFORM_TYPE is defined and #include it
      {
         UtPath sensorPlatformFile = mProxyInterface->GetPlatformTypeFile("SENSOR_PLATFORM_TYPE");
         if (sensorPlatformFile.Exists())
         {
            aStream << "include_once " << projectDir.GetRelativePath(sensorPlatformFile).GetNormalizedPath() << "\n";
         }
      }
   }
}

void SPLAT::SensorAnalysisDialog::WriteHorizontalCoverage(std::ofstream& aStream)
{
   mHorizontalCoverageDialog->ApplyAllChanges();
   mHorizontalCoverageDialog->WriteData(aStream);
}

void SPLAT::SensorAnalysisDialog::WriteHorizontalMap(std::ofstream& aStream)
{
   mHorizontalMapDialog->ApplyAllChanges();
   mHorizontalMapDialog->WriteData(aStream);
}

void SPLAT::SensorAnalysisDialog::WriteSphericalMap(std::ofstream& aStream)
{
   mSphericalMapDialog->ApplyAllChanges();
   mSphericalMapDialog->WriteData(aStream);
}

void SPLAT::SensorAnalysisDialog::WriteVerticalCoverage(std::ofstream& aStream)
{
   mVerticalCoverageDialog->ApplyAllChanges();
   mVerticalCoverageDialog->WriteData(aStream);
}

void SPLAT::SensorAnalysisDialog::WriteVerticalMap(std::ofstream& aStream)
{
   mVerticalMapDialog->ApplyAllChanges();
   mVerticalMapDialog->WriteData(aStream);
}

void SPLAT::SensorAnalysisDialog::OutputBrowsePressedHandler()
{
   QFileDialog dlg(nullptr, "Select Directory", mOutputDirectory, tr("Folder"));
   dlg.setAcceptMode(QFileDialog::AcceptOpen);
   dlg.setFileMode(QFileDialog::Directory);

   QStringList selFiles;
   if (dlg.exec())
   {
      selFiles = dlg.selectedFiles();
      if (selFiles.size() == 1)
      {
         mUi.mOutputDirectoryEdit->setText(selFiles[0]);
      }
   }
}

void SPLAT::SensorAnalysisDialog::OutputDirectoryChangedHandler(const QString& aOutputDirectory)
{
   mOutputDirectory = mUi.mOutputDirectoryEdit->text();
}

void SPLAT::SensorAnalysisDialog::ProjectOpened(wizard::Project* aProject)
{
   if (aProject)
   {
      mProject         = aProject;
      mOutputDirectory = ""; // wizard::Project::mProjectDirectory isn't initialized yet, so just set to empty for now.
   }
}
