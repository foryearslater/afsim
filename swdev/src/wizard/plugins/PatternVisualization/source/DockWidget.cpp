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

#include "DockWidget.hpp"

#include <memory>

#include <QApplication>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QTimer>

#include "Debug.hpp"
#include "Environment.hpp"
#include "Pattern.hpp"
#include "PatternVisualizerWidget.hpp"
#include "PrefObject.hpp"
#include "Session.hpp"
#include "UtInput.hpp"
#include "UtMemory.hpp"
#include "UtPath.hpp"
#include "UtStringUtil.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"
#include "WsfStandardApplication.hpp"

extern void Register_xio_interface(WsfApplication& aApplication);

PatternVisualizer::DockWidget::DockWidget(const QString&  aName,
                                          QMainWindow*    aParent,
                                          Qt::WindowFlags aFlags,
                                          bool            aDefaultStartDocked)
   : wkf::DockWidget(aName, aParent, aFlags, aDefaultStartDocked)
   , mSessionPtr(Session::Create())
   , mScenarioPtr(nullptr)
{
   mSessionPtr->SetDockWidget(this);
   setWindowTitle(aName);

   // Create the container widget.
   mPatternVisualizerWidgetPtr = new PatternVisualizerWidget(*mSessionPtr.get(), this);

   // Hide the pattern tree and data if the widget is starting in a docked position.
   if (!isFloating())
   {
      mPatternVisualizerWidgetPtr->ShowTreeAndSettings(false);
   }
   else
   {
      setGeometry(wkfEnv.GetMainWindow()->x() + 100, wkfEnv.GetMainWindow()->y() + 100, 512, 512);
   }

   setWidget(mPatternVisualizerWidgetPtr);
}

void PatternVisualizer::DockWidget::ReadPatternFile(const QString& aStartupFile, const QString& aSigViewFile)
{
   if (aStartupFile.isEmpty())
   {
      return;
   }
   // Change to the pattern file's directory
   QString fileDirectory = QFileInfo(aStartupFile).path();
   if (QDir(fileDirectory).exists())
   {
      QDir::setCurrent(fileDirectory);
   }

   WsfStandardApplication& standardApp = wizEnv.GetWsfStandardApplication();
   connect(&wizEnv,
           &wizard::Environment::WsfStandardApplicationPendingDestruction,
           [this]()
           {
              mSessionPtr->GetPatternUpdateManager()->StopRunningThread();
              ClearAllPatterns();
              mScenarioPtr.reset();
           });

   mScenarioPtr = ut::make_unique<WsfScenario>(standardApp);

   // Parse the data file
   try
   {
      standardApp.ProcessInputFiles(*mScenarioPtr, std::vector<std::string>{aStartupFile.toStdString()});
   }
   catch (WsfApplication::Exception& e)
   {
      auto errorMessage =
         QString{
            R"(
               Error processing the startup file:
               <br>
               <pre>%1</pre>
            )"}
            .arg(e.what());

      Debug::ShowError(errorMessage);
   }

   // Load the patterns
   try
   {
      mSessionPtr->LoadPatterns(aStartupFile, *mScenarioPtr, aSigViewFile);
   }
   catch (std::exception& e)
   {
      QMessageBox msg;
      msg.setText(e.what());
      msg.exec();
      ut::log::error(e.what());
   }
}

void PatternVisualizer::DockWidget::ClearAllPatterns()
{
   mSessionPtr->Clear();
}

void PatternVisualizer::DockWidget::PrepareForDockStateChange(bool aFloating)
{
   mPatternVisualizerWidgetPtr->ShowTreeAndSettings(aFloating);
}
