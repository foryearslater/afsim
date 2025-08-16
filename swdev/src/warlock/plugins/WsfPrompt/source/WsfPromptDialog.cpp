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

#include "WsfPromptDialog.hpp"

#include <QDesktopServices>
#include <QMessageBox>
#include <QPushButton>

#include "UtMemory.hpp"
#include "WkfEnvironment.hpp"
#include "WkfUtils.hpp"
#include "WsfPromptSimCommands.hpp"
#include "WsfScriptPrompt.hpp"

WkWsfPrompt::PromptDialog::PromptDialog(wkf::MainWindow*                        aParent,
                                        const WsfScriptPromptClass::PromptData& aPromptData,
                                        ScriptSimInterfacePtr                   aScriptSimInterfacePtr)
   : QDialog(aParent)
   , mUI()
   , mScriptSimInterfacePtr(aScriptSimInterfacePtr)
   , mPlatformName(aPromptData.platformName)
   , mBlock(aPromptData.block)
{
   mUI.setupUi(this);
   setAttribute(Qt::WA_DeleteOnClose);
   QDialog::setWindowTitle(QString::fromStdString(aPromptData.title));

   QString   formattedText = wkf::util::ParseLinks(QString::fromStdString(aPromptData.message));
   QFileInfo fi(wkfEnv.GetDocumentationDir());
   if (fi.isDir())
   {
      QString dir(fi.absoluteFilePath().append("/html/docs/"));
      formattedText.replace("<a doclink=\"", "<a href=\"" + dir);
   }
   mUI.message->setText(formattedText);
   mUI.message->setTextFormat(Qt::RichText);

   connect(mUI.message,
           &QLabel::linkActivated,
           this,
           [this](const QString& aLink)
           {
              // check if documentation link
              if (aLink.right(4) == "html")
              {
                 if (!QDesktopServices::openUrl(QUrl(aLink)))
                 {
                    QMessageBox::warning(this, "Error", "Could not open html link " + aLink);
                 }
              }
              else
              {
                 // otherwise attempt to open as an AFSIM link command
                 QString errorMsg;
                 if (!wkf::util::FollowLink(aLink, errorMsg))
                 {
                    QFileInfo fi(wkfEnv.GetDocumentationDir());
                    if (fi.isDir())
                    {
                       QString dir(fi.absoluteFilePath().append("/html/docs/wkf_plugin/wk_logs_chat.html"));
                       errorMsg +=
                          "Refer to the <a href=\"" + dir + "\">link documentation</a> for a list of accepted commands.";
                    }
                    QMessageBox::warning(this, "Error", errorMsg);
                 }
              }
           });

   for (const auto& b : aPromptData.scriptButtons)
   {
      QPushButton* button = new QPushButton(QString::fromStdString(b.first));
      mUI.buttonLayout->addWidget(button);
      connect(button, &QPushButton::clicked, this, [b, this]() { ButtonClicked(b.second); });
   }
}

void WkWsfPrompt::PromptDialog::showEvent(QShowEvent* aShowEvent)
{
   if (mBlock)
   {
      mScriptSimInterfacePtr->AddSimCommand(ut::make_unique<PauseCommand>());
   }
}

void WkWsfPrompt::PromptDialog::closeEvent(QCloseEvent* aCloseEvent)
{
   if (mBlock)
   {
      mScriptSimInterfacePtr->AddSimCommand(ut::make_unique<ResumeCommand>());
   }
}

void WkWsfPrompt::PromptDialog::ButtonClicked(const std::string& aScriptName)
{
   // When a button is clicked on the dialog, execute the script associated with the button,
   // and close the dialog.
   warlock::script::Instance scriptInstance;
   scriptInstance.platformName = mPlatformName;

   // An empty script name means that we just want to display a message without executing any scripts
   if (!aScriptName.empty())
   {
      // First check to see if the script is in the platform scope. (If mPlatformName is empty, this will return false)
      if (!mScriptSimInterfacePtr->GetPlatformScriptData(mPlatformName, aScriptName, scriptInstance.script))
      {
         // If the script is not in the platform scope, check the global scope.
         if (!mScriptSimInterfacePtr->GetGlobalScriptData(aScriptName, scriptInstance.script))
         {
            // If it's not in either global or platform scope, display an error message.
            QString errorMessage = "Script \"" + QString::fromStdString(aScriptName) + "\" not found";
            QMessageBox::warning(nullptr, "WsfPrompt", errorMessage);
            close();
            return;
         }
      }
      mScriptSimInterfacePtr->ExecuteScript(scriptInstance);
   }
   close();
}
