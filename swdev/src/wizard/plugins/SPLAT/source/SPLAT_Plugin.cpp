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

#include "SPLAT_Plugin.hpp"

#include <QMenu>

#include "ProjectWorkspace.hpp"
#include "SensorAnalysisDialog.hpp"
#include "Signals.hpp"
#include "ToolPrefObject.hpp"
#include "WsfExe.hpp"

WKF_PLUGIN_DEFINE_SYMBOLS(SPLAT::Plugin, "SPLAT", "Launches the Sensor Plot Analysis Tool", "wizard")

SPLAT::Plugin::Plugin(const QString& aPluginName, const size_t aUniqueId)
   : wizard::Plugin(aPluginName, aUniqueId)
   , mSensorPlotProcess(nullptr)
   , mGnuplotProcess(nullptr)
   , mDialog(nullptr)
{
   // get bin directory where sensor_plot is located
   UtPath epath = UtPath::GetExePath();
   epath.Up();
   QString exeDir = epath.GetSystemPath().c_str();

#ifdef _WIN32
   mExeDirectory = exeDir + "/sensor_plot.exe";
#else
   mExeDirectory = exeDir + "/sensor_plot";
#endif

   connect(wizSignals, &wizard::Signals::ActionsForNodeRequested, this, &Plugin::ActionsForNodeRequested);
}

SPLAT::Plugin::~Plugin()
{
   delete mDialog;
   delete mGnuplotProcess;
   // mSensorPlotProcess gets deleted by wizard::WsfExecution -- see wizard::WsfExecution::Finished()
}

void SPLAT::Plugin::StartUp(const std::string& aSensorPlotFile, const UtPath& aOutputDirectory)
{
   mDialog->EnableKill(true);
   mDialog->SetStatusMessage("Running");

   mWorkingDirectory = QString::fromStdString(aOutputDirectory.GetNormalizedPath());

   // add the generated sensor plot file to the sensor_plot argument list
   QStringList argList;
   argList.push_back(QString::fromStdString(aSensorPlotFile));

   CreateProcess(argList);
   wizard::ProjectWorkspace::Instance()->StartExecution(mSensorPlotProcess);
}

void SPLAT::Plugin::ActionsForNodeRequested(const WsfPProxyNode& aNode, QMenu* aMenuPtr, QWidget* aParentWidgetPtr)
{
   if (aNode.IsOfType("Sensor") && !aNode.IsOfBasicType() && // not a predefined sensor type (e.g. not WSF_RADAR_SENSOR)
       aNode.GetParent().GetPath().size() ==
          1) // The parent must be the "root"node (i.e. the sensor cannot have a platform or platform_type as parent
   {
      mProxyInterface.SetProxyNode(aNode);

      QObject::connect(aMenuPtr->addAction("Analyze..."), &QAction::triggered, this, [this]() { ShowDialog(); });
   }
}

void SPLAT::Plugin::ShowDialog()
{
   if (!mDialog) // if the dialog hasn't been created, create a new one
   {
      mDialog = new SensorAnalysisDialog(&mProxyInterface);
      connect(mDialog, &SensorAnalysisDialog::SensorPlotFileReady, this, &Plugin::StartUp);
      connect(mDialog, &SensorAnalysisDialog::KillButtonPressed, [this]() { mSensorPlotProcess->Kill(); });
   }

   mDialog->SetSensorId(QString::fromStdString(mProxyInterface.GetProxyNodeName()));
   mDialog->Show();
}

void SPLAT::Plugin::CreateProcess(const QStringList& aArgList)
{
   // Create the process - deleted on QProcess::finished -- see wizard::WsfExecution::Finished
   mSensorPlotProcess = new wizard::WsfExecution("SPLAT",
                                                 new wizard::WsfExe(UtPath(mExeDirectory.toStdString())),
                                                 mWorkingDirectory.toStdString(),
                                                 aArgList);

   // when the sensor_plot process is finished, run gnuplot on resulting files
   connect(mSensorPlotProcess,
           static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
           this,
           &Plugin::ProcessFinished);
}

void SPLAT::Plugin::ProcessFinished(int aExitCode, QProcess::ExitStatus aExitStatus)
{
   mDialog->EnableKill(false);
   if (aExitStatus == QProcess::CrashExit)
   {
      mDialog->SetStatusMessage("Killed");
   }
   else
   {
      wizard::ToolDef gnuPlot = wkfEnv.GetPreferenceObject<wizard::ToolPrefObject>()->FindTool("GNU Plot");
      if (gnuPlot.mAlias.isEmpty())
      {
         gnuPlot = wkfEnv.GetPreferenceObject<wizard::ToolPrefObject>()->FindTool("gnuplot");
      }
      if (mDialog->PlotsRequested()) // only attempt to run gnuplot if user has requested at least one plot
      {
         if (!gnuPlot.mAlias.isEmpty())
         {
            mDialog->SetStatusMessage("Showing Plots...");
            QStringList argList;
            argList.push_back(QString::fromStdString("plot_" + mProxyInterface.GetProxyNodeName() +
                                                     ".gnu")); // script that contains the plots to show

            QProcess* gnuProc = new QProcess(this);
            gnuProc->setWorkingDirectory(mWorkingDirectory);

            // Note: this doesn't seem to work from Visual Studio, but does work for an install.
            gnuProc->start(gnuPlot.mPath, argList);
            if (mGnuplotProcess)
            {
               delete mGnuplotProcess;
            }
            mGnuplotProcess = gnuProc;
         }
         else
         {
            QMessageBox::information(nullptr, "Can't execute", "Unable to produce plots. GNU Plot not configured.");
         }
      }
      mDialog->SetStatusMessage("Finished");
   }
}
