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
#ifndef SPLAT_PLUGIN_HPP
#define SPLAT_PLUGIN_HPP

#include <QProcess>

#include "Plugin.hpp"
#include "ProxyInterface.hpp"
#include "WsfExecution.hpp"
class WsfPProxyNode;

namespace SPLAT
{
class SensorAnalysisDialog;

class Plugin : public wizard::Plugin
{
public:
   Plugin(const QString& aPluginName, const size_t aUniqueId);
   ~Plugin() override;

   void StartUp(const std::string& aSensorPlotFile, const UtPath& aOutputDirectory);

private:
   void ActionsForNodeRequested(const WsfPProxyNode& aNode, QMenu* aMenuPtr, QWidget* aParentWidgetPtr);
   void ShowDialog();
   void CreateProcess(const QStringList& aArgList);
   void ProcessFinished(int aExitCode, QProcess::ExitStatus aExitStatus);

   wizard::WsfExecution* mSensorPlotProcess; // process that starts sensor_plot.exe
   QProcess*             mGnuplotProcess;
   SensorAnalysisDialog* mDialog;
   QString               mGnuplotDirectory; // where gnuplot.exe is located
   QString               mExeDirectory;
   QString mWorkingDirectory; // where the plot files are going to be saved, also the working directory for gnuplot

   ProxyInterface mProxyInterface;
};
} // namespace SPLAT
#endif
