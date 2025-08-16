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
#include "AdHocScriptBrowserDockWidget.hpp"

#include "WkPermissions.hpp"
#include "WkSimEnvironment.hpp"
#include "WkfEnvironment.hpp"
#include "WkfPlatform.hpp"

WkAdHocScriptBrowser::DockWidget::DockWidget(QWidget* parent, Qt::WindowFlags f)
   : QDockWidget(parent, f)
   , mSimInterfacePtr(simEnv.GetScriptSimInterface())
{
   mUI.setupUi(this);
   mUI.platformRadioButton->setText("<No Platform Selected>");

   connect(mUI.executePushButton, &QPushButton::clicked, this, &DockWidget::OnExecuteClicked);

   connect(&wkfEnv, &wkf::Environment::PlatformOfInterestChanged, this, &DockWidget::PlatformOfInterestChanged);
}

void WkAdHocScriptBrowser::DockWidget::showEvent(QShowEvent* aEvent)
{
   // Do not show this dialog if File Browsing is locked
   if (wkfEnv.IsFileBrowsingLocked())
   {
      hide();
   }
}

void WkAdHocScriptBrowser::DockWidget::OnExecuteClicked()
{
   warlock::script::Instance adHocScriptInstance;
   if (mUI.globalRadioButton->isChecked())
   {
      adHocScriptInstance.script.context = warlock::script::GLOBAL;
   }
   else
   {
      adHocScriptInstance.script.context = warlock::script::PLATFORM;
      auto* platformPtr                  = wkfEnv.GetPlatformOfInterest();
      if (platformPtr)
      {
         if (!warlock::HasPermissionToControlPlatform(*platformPtr))
         {
            QMessageBox::warning(nullptr,
                                 "Script Not Executed",
                                 "User does not have permissions to modify platform " +
                                    QString::fromStdString(platformPtr->GetName()) + ".");
            return;
         }
         adHocScriptInstance.platformName = platformPtr->GetName();
      }
   }

   adHocScriptInstance.script.text = "void AdHocScript_" + std::to_string(mScriptCount) + "()\n" +
                                     mUI.scriptTextBrowser->toPlainText().toStdString() + "\nend_script";
   mScriptCount++;
   mSimInterfacePtr->ExecuteScript(adHocScriptInstance);
}

void WkAdHocScriptBrowser::DockWidget::PlatformOfInterestChanged(wkf::Platform* aPlatform)
{
   if (aPlatform)
   {
      mUI.platformRadioButton->setText("Execute on " + QString::fromStdString(aPlatform->GetName()));
   }
   else
   {
      mUI.platformRadioButton->setText("<No Platform Selected>");
   }
}
