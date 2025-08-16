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

#include <iostream>

#include <QApplication>
#include <QDockWidget>
#include <QMessageBox>
#ifdef _WIN32
#include <QMimeDatabase>
#include <QtWinExtras>
#endif

#include "UtException.hpp"
#include "UtPath.hpp"
#include "UtQtException.hpp"
#include "VaPredefine.hpp"
#include "WarlockApplicationExtension.hpp"
#include "WkAppEnvironment.hpp"
#include "WkFactory.hpp"
#include "WkPermissions.hpp"
#include "WkRunManager.hpp"
#include "WkSimEnvironment.hpp"
#include "WkStartupDialog.hpp"
#include "WkfEnvironment.hpp"
#include "WkfGeneralPrefObject.hpp"
#include "WkfMainWindow.hpp"
#include "WkfPermissionDialog.hpp"
#include "WkfPermissionManager.hpp"
#include "WkfVtkEnvironment.hpp"
#include "WsfScenario.hpp"
#include "WsfVersion.hpp"
#include "warlock_version_defines.hpp"

// Includes all of the optional projects which can be compiled with WSF
#include <QDockWidget>
#include <QObject>
#include <QString>
#include <QTextEdit>
#include <QVBoxLayout>

#include "wsf_extensions.hpp"

namespace
{
#ifdef _WIN32
// To enable the application jump list to show the desired recent files, the corresponding file types are
// registered using the following helper function.
bool associateFileTypes()
{
   QString displayName = QGuiApplication::applicationDisplayName();
   QString filePath    = QCoreApplication::applicationFilePath();
   QString fileName    = QFileInfo(filePath).fileName();

   const QString key = QStringLiteral("HKEY_CURRENT_USER\\Software\\Classes\\Applications\\") + fileName;
   QSettings     settings(key, QSettings::NativeFormat);
   if (settings.status() != QSettings::NoError)
   {
      std::cerr << "Cannot access registry key" << key.toStdString();
      return false;
   }
   settings.setValue(QStringLiteral("FriendlyAppName"), displayName);

   settings.beginGroup(QStringLiteral("SupportedTypes"));

   QMimeDatabase     mimeDatabase;
   const QStringList suffixes = mimeDatabase.mimeTypeForName("text/plain").suffixes();
   for (QString suffix : suffixes)
   {
      suffix.prepend('.');
      settings.setValue(suffix, QString());
   }

   settings.endGroup();

   settings.beginGroup(QStringLiteral("shell"));
   settings.beginGroup(QStringLiteral("open"));
   settings.setValue(QStringLiteral("FriendlyAppName"), displayName);
   settings.beginGroup(QStringLiteral("Command"));
   settings.setValue(QStringLiteral("."),
                     QLatin1Char('"') + QDir::toNativeSeparators(filePath) + QStringLiteral("\" \"%1\""));

   return true;
}
#endif

void setupPermissions(const WarlockApplicationExtension* aWarlockExtensionPtr)
{
   warlock::SetupPermissions();

   // If a Permission File was provided
   if (!aWarlockExtensionPtr->GetPermissionFile().empty())
   {
      // Disable the UI so that the user can not change the permissions
      wkfEnv.GetMainWindow()->GetPermissionDialog()->LockPermissions();
   }
}

int wkExecute(WsfStandardApplication& aApp)
{
   qApp->setOrganizationName(WARLOCK_COMPANYNAME_STR);
   qApp->setOrganizationDomain(WARLOCK_COMPANYDOMAIN_STR);
   qApp->setApplicationName(QString::fromStdString(aApp.GetApplicationName()));
   qApp->setApplicationDisplayName(QString::fromStdString(aApp.GetApplicationName()));
   qApp->setApplicationVersion(wsf::version::GetString());
   qApp->setWindowIcon(QIcon(":images/warlock_icon.ico"));

   // Process the command line arguments first as it may change what is loaded or how it is loaded.
   WsfStandardApplication::Options options;
   try
   {
      aApp.ProcessCommandLine(options);
   }
   catch (WsfApplication::Exception& e)
   {
      std::cout << e.what() << std::endl;
      QMessageBox::critical(nullptr,
                            "Invalid Command Line",
                            "An exception reading the command line arguments was thrown.\n\n" + QString(e.what()),
                            QMessageBox::Abort);
      return e.GetReturnCode();
   }

   WarlockApplicationExtension* warlockExtPtr = WarlockApplicationExtension::Find(aApp);
   warlockExtPtr->ProcessDisplayConsoleWindow();

   // Create VTK Environment (vaEnv)
   wkf::VtkEnvironment::Create(new warlock::Factory, "wk");
   // Create WKF Environment (wkfEnv)
   wkf::Environment::Create("warlock",
                            "wk",
                            QString::fromStdString(warlockExtPtr->GetConfigFile()),
                            warlockExtPtr->GetImportConfigFile(),
                            QString::fromStdString(warlockExtPtr->GetPermissionFile()));
   // Create Warlock UI Environment (wkEnv)
   warlock::AppEnvironment::Create();
   // Create the Warlock Simulation Environment (simEnv)
   warlock::SimEnvironment::Create();
   // Create the Warlock Run Manager
   warlock::RunManager::Create(aApp);

   int returnCode = 0;

   if (options.mRunMode == WsfStandardApplication::cRUN_SCENARIO)
   {
      const auto& recents = warlock::RunManager::GetInstance().GetRecentScenarios();

#ifdef _WIN32
      associateFileTypes();

      QWinJumpList          jumplist;
      QWinJumpListCategory* recentCatPtr = jumplist.recent();
      recentCatPtr->setVisible(true);
      // iterate in reverse order so most recent is on top
      for (QList<QStringList>::const_reverse_iterator it = recents.rbegin(); it != recents.rend(); ++it)
      {
         QString text = warlock::RunManager::GetInstance().GetScenarioDisplayName(*it, false);
         recentCatPtr->addLink(text, QCoreApplication::applicationFilePath(), *it);
      }
#endif
      // This needs to be called prior to wkfEnv.StartUp() as that will read the permissions state from file.
      setupPermissions(warlockExtPtr);

      // Launch the WKF environment
      wkfEnv.StartUp();
      // Hide the Main Window until completion of the Start Dialog by the user.
      wkfEnv.GetMainWindow()->hide();

      if (options.mInputFiles.empty())
      {
         if (!warlockExtPtr->GetLoadPreviousScenario() || recents.empty())
         {
            // launch dialog if no scenario was specified and -ups was not given.
            QStringList            inputFiles;
            warlock::StartupDialog dialog(inputFiles, wkfEnv.IsFileBrowsingLocked(), nullptr);
            if (dialog.exec() == QDialog::Rejected)
            {
               return 0;
            }
            else
            {
               for (const QString& file : inputFiles)
               {
                  options.mInputFiles.push_back(file.toStdString());
               }
            }
         }
         else
         {
            for (const QString& file : recents.front())
            {
               options.mInputFiles.push_back(file.toStdString());
            }
         }
      }

      if (warlockExtPtr->GetStartMinimized())
      {
         wkfEnv.GetMainWindow()->showMinimized();
      }

      wkfEnv.GetMainWindow()->show();
      warlock::RunManager::GetInstance().LoadScenario(options);

      // Send any events that were posted before event loop started (e.g. due to race conditions)
      qApp->sendPostedEvents();

      // Enter the main event loop to start event handling.
      // exec() is a blocking call that does not return until quit() is called or last primary window is closed
      returnCode = qApp->exec();
   }
   else if (options.mRunMode == WsfStandardApplication::cSHOW_USAGE)
   {
      ut::log::Publisher::CaptureStdStreams(true);
      ut::log::Subscriber consoleSub{"raw"};

      WsfScenario scenario(aApp);
      aApp.ProcessCommandLineCommands(scenario, options);
      std::cout.flush();

      QString text;
      auto    msgs = consoleSub.TakeQueue();
      while (!msgs.empty())
      {
         // Because consoleSub is only subscribed to "raw", there should be no notes.
         text += QString::fromStdString(msgs.front().mData);
         msgs.pop();
      }

      QMessageBox msgBox;
      msgBox.setWindowTitle("Warlock Usage");
      msgBox.setText(text);
      msgBox.setIcon(QMessageBox::Question);
      msgBox.setStandardButtons(QMessageBox::Close);
      msgBox.setDefaultButton(QMessageBox::Close);
      // Use a Monospace font so that the text stays formatted correctly.
      QFont font("Monospace");
      font.setStyleHint(QFont::TypeWriter);
      msgBox.setFont(font);
      // This is a hack to get QMessageBox to be wider, so that 80 characters from
      //  the simulation output will stay formatted correctly.
      QSpacerItem* horizontalSpacer = new QSpacerItem(650, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
      QGridLayout* layout           = (QGridLayout*)msgBox.layout();
      layout->addItem(horizontalSpacer, layout->rowCount(), 0, 1, layout->columnCount());

      msgBox.exec();
   }
   else if (options.mRunMode == WsfStandardApplication::cWRITE_GRAMMAR)
   {
      WsfScenario scenario(aApp);
      aApp.ProcessCommandLineCommands(scenario, options);
   }

   warlock::RunManager::GetInstance().Shutdown();
   simEnv.Shutdown();
   wkEnv.Shutdown();
   wkfEnv.Shutdown();

   return returnCode;
}

// With the addition of a WarlockApplication, we should consider refactoring WarlockApplicationExtension
// into the WarlockApplication and thus Warlock would be an Application instead of an extension.
class WarlockApplication : public WsfStandardApplication
{
public:
   WarlockApplication(int argc, char* argv[])
      : WsfStandardApplication("Warlock", argc, argv, {"../mission_plugins", "../wsf_plugins"})
   {
   }

protected:
   std::string GetUsageString() const override
   {
      return "\nUsage: " + GetApplicationName() +
             " [input_file...]"
             "\n"
             "\n-es               Use the event-stepped executive. (DEFAULT)"
             "\n-fs               Use the frame-stepped executive."
             "\n-fio              Flush the output buffer when the simulation time is advanced"
             "\n-sm               Suppress the periodic messages indicating advance of the"
             "\n                  simulation time"
             "\n-mi <interval>    Output periodic messages indicating advance of the simulation"
             "\n                  time at the given interval."
             "\n-list-variables   List preprocessor variables used in the input files and quit"
             "\n-?, -h, -help     Display command line options and quit";
   }
};
} // namespace

int main(int argc, char* argv[])
try
    {
   ut::SetApplicationErrorHandling(&ut::qt::TerminateHandler);
   ut::SetupApplicationLog("Warlock", WARLOCK_VERSION, "warlock-exception.log");

   QApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
   QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
   QApplication q(argc, argv);

   WarlockApplication app(argc, argv);

   // Load built-in extensions (defined in wsf_extensions.hpp)
   RegisterBuiltinExtensions(app);

   // Load optional extensions (defined in wsf_extensions.hpp)
   RegisterOptionalExtensions(app);

   // Register the Warlock extension
   WSF_REGISTER_EXTENSION(app, warlock);

   // Register the XIO simulation interface.
   WSF_REGISTER_EXTENSION(app, xio_interface);

   // Inner try-catch to create exception report popup before QApplication close
   try
   {
      return wkExecute(app);
   }
   catch (...)
   {
      ut::qt::ReportException("Warlock", std::current_exception());
      return 1;
   }
}
catch (...)
{
   ut::qt::ReportException("Warlock", std::current_exception());
   return 1;
}
