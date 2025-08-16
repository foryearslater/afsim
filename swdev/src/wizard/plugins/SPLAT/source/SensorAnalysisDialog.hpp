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

#ifndef SENSORANALYSISDIALOG_HPP
#define SENSORANALYSISDIALOG_HPP

#include <QDialog>

#include "UtCallbackHolder.hpp"
class UtPath;

#include "ui_SensorAnalysisDialog.h"

namespace wizard
{
class Project;
}

namespace SPLAT
{
class HorizontalCoverageOptionsDialog;
class HorizontalMapOptionsDialog;
class ProxyInterface;
class TargetPlatformTypeDialog;
class SphericalMapOptionsDialog;
class VerticalCoverageOptionsDialog;
class VerticalMapOptionsDialog;

class SensorAnalysisDialog : public QDialog
{
   Q_OBJECT
public:
   SensorAnalysisDialog(ProxyInterface* aProxyInterface, QWidget* aParent = nullptr);
   ~SensorAnalysisDialog() override;

   void Show();
   void Update();
   bool PlotsRequested();
   void SetStatusMessage(const QString& aMessage);
   void EnableKill(bool aEnable = true);
   void SetSensorId(const QString& aSensorName);

signals:
   void SensorPlotFileReady(const std::string& aSensorPlotFile, const UtPath& aOutputDirectory);
   void KillButtonPressed();

protected:
   void closeEvent(QCloseEvent* aCloseEvent) override;

private:
   void Initialize();
   void Connect();
   void WriteSensorPlotFile();

   void WriteTargetPlatformType(std::ofstream& aStream);
   void WriteSensorPlatformType(std::ofstream& aStream);
   void WritePlatformTypeIncludes(std::ofstream& aStream);
   void WriteHorizontalCoverage(std::ofstream& aStream);
   void WriteHorizontalMap(std::ofstream& aStream);
   void WriteSphericalMap(std::ofstream& aStream);
   void WriteVerticalCoverage(std::ofstream& aStream);
   void WriteVerticalMap(std::ofstream& aStream);

   void OutputBrowsePressedHandler();
   void OutputDirectoryChangedHandler(const QString& aOutputDirectory);

   void ProjectOpened(wizard::Project* aProject);

   Ui::SensorAnalysisDialog         mUi;
   HorizontalCoverageOptionsDialog* mHorizontalCoverageDialog;
   HorizontalMapOptionsDialog*      mHorizontalMapDialog;
   SphericalMapOptionsDialog*       mSphericalMapDialog;
   VerticalCoverageOptionsDialog*   mVerticalCoverageDialog;
   VerticalMapOptionsDialog*        mVerticalMapDialog;
   TargetPlatformTypeDialog*        mTargetPlatformTypeDialog;

   QString mProjectDirectory;
   QString mOutputDirectory;

   ProxyInterface*  mProxyInterface;
   wizard::Project* mProject;

   UtCallbackHolder mCallbacks;
};
} // namespace SPLAT

#endif