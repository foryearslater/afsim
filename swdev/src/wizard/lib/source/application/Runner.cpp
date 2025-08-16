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

#include "Runner.hpp"

#include <QMessageBox>
#include <QResource>

#include "Environment.hpp"
#include "Factory.hpp"
#include "RunEnvManager.hpp"
#include "RunManager.hpp"
#include "Signals.hpp"
#include "StartupDialog.hpp"
#include "Undo.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"
#include "WkfVtkEnvironment.hpp"

wizard::Runner::Runner(int&        argc,
                       char*       argv[],
                       const char* aCompanyStr,
                       const char* aCompanyDomainStr,
                       const char* aProductNameStr,
                       const char* aProductVersionStr)
{
   // Get the executable path
   // Trim the name of the executable
   mBinDir = UtPath::GetExePath();
   mBinDir.Up();

   QApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
   QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
   mApp = ut::make_unique<QApplication>(argc, argv);
   mApp->setQuitOnLastWindowClosed(true);
   mApp->setWindowIcon(QIcon(":icons/wizard_logo_icon.png"));
   qApp->setOrganizationName(aCompanyStr);
   qApp->setOrganizationDomain(aCompanyDomainStr);
   qApp->setApplicationName(aProductNameStr);
   qApp->setApplicationDisplayName(aProductNameStr);
   qApp->setApplicationVersion(aProductVersionStr);
}

void wizard::Runner::run()
{
   std::string errorString;
   // Given the path to the executable determine the other directory paths
   if (!RunEnvManager::Initialize(mBinDir, errorString))
   {
      QMessageBox::critical(nullptr, QObject::tr("AFSIM Wizard"), QObject::tr(errorString.c_str()), QMessageBox::Ok);
      return;
   }
   // Expected locations for rcc files ...
   // - Install: bin/../resources/rcc directory
   // - Debug:   Debug/../rcc/ directory
   mRccPath = (RunEnvManager::GetDataDir() + "../rcc/wizard.rcc").GetSystemPath();
   if (!QResource::registerResource(QString::fromStdString(mRccPath)))
   {
      // paths for developer
      mRccPath = (RunEnvManager::GetBinDir() + "../rcc/wizard.rcc").GetSystemPath();
      if (!QResource::registerResource(QString::fromStdString(mRccPath)))
      {
         mRccPath = (RunEnvManager::GetBinDir() + "rcc/wizard.rcc").GetSystemPath();
         QResource::registerResource(QString::fromStdString(mRccPath));
      }
   }

   new Undo();

   wkf::VtkEnvironment::Create(new Factory);
   wkf::Environment::Create("wizard", "wiz");
   // Redirect standard output
   Environment::Create();
   RunManager::Create();

   wkfEnv.StartUp();
   wizEnv.UpdateMenus();
   wkfEnv.GetMainWindow()->setWindowTitle("wizard");
   wkfEnv.GetMainWindow()->centralWidget()->setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::North);

   if (!qApp->arguments().empty())
   {
      wizRunMgr.LoadScenario(qApp->arguments());
   }
   else
   {
      QStringList files;
      // launch dialog if no scenario was specified.
      StartupDialog dialog(files, nullptr, Qt::MSWindowsFixedSizeDialogHint);
      if (dialog.exec() != QDialog::Rejected)
      {
         wizRunMgr.LoadScenario(files);
      }
   }

   mApp->exec();

   // cleanup the environment
   wizRunMgr.Shutdown();
   wizEnv.Shutdown();
   wkfEnv.Shutdown();
   return;
}

wizard::Runner::~Runner()
{
   // Unregister the QResource from initialization
   QResource::unregisterResource(QString::fromStdString(mRccPath));
}
