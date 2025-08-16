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

#include "Dialog.hpp"

#include <QFileInfo>

#include "Editor.hpp"
#include "EventWidget.hpp"
#include "PluginUtil.hpp"
#include "ProjectWorkspace.hpp"
#include "SettingValueWidget.hpp"
#include "SettingWidget.hpp"
#include "Signals.hpp"
#include "Tokenizer.hpp"
#include "WkfEnvironment.hpp"
#include "editor/Editor.hpp"
#include "editor/EditorManager.hpp"

namespace wizard
{
namespace EventOutput
{

const QSize cWINDOW_SIZE{650, 700};

const QString cCSV_EVENT_OUTPUT{"csv_event_output"};
const QString cEVT_EVENT_OUTPUT{"event_output"};

const editor::tokenizer::BlockTags cCSV_EVENT_OUTPUT_TAGS{cCSV_EVENT_OUTPUT, "end_" + cCSV_EVENT_OUTPUT};
const editor::tokenizer::BlockTags cEVT_EVENT_OUTPUT_TAGS{cEVT_EVENT_OUTPUT, "end_" + cEVT_EVENT_OUTPUT};

const QString cHELP_TEXT{
   "<h3>Purpose:</h3>"
   "<p>This widget allows one to edit event_output blocks.</p>"
   "<h3>How to use:</h3>"
   "<p>Click on the event_output type check boxes to toggle the visibility of the tabs.</p>"
   "<h3>Settings</h3>"
   "<p>Toggle the Add? check box to add a particular setting to the block."
   "Change the value of the setting with the widget in the Value column.</p>"
   "<h3>Events</h3>"
   "<p>Check the disable check box to write disable <event> to the block."
   "Check the enable check box to write enable <event> to the block.</p>"
   "<p><strong>Note:</strong> Both of the check boxes unchecked does not write the event to the block.</p>"
   "<h3>Documentation</h3>"
   "<p>Right-click the setting or event command to bring up a context menu."
   "Clicking the context menu item will bring open the documentation for that command in a web browser.</p>"
   "<p><strong>Note:</strong> The links work best on Chrome and Firefox.  Edge and Internet Explorer seem to have "
   "issues with it...</p>"
   "<p>A more detailed help text can be found " +
   QString("<a href=\"%1\">here</a>.")
      .arg("file:///" + QFileInfo(wkfEnv.GetDocumentationDir()).absolutePath() +
           "/documentation/html/docs/wkf_plugin/wiz_" + cEVT_EVENT_OUTPUT + ".html") +
   "</p>"};

Dialog::Dialog(bool aShowHelpButton, QWidget* aParent, Qt::WindowFlags aFlags)
   : QDialog(aParent, std::move(aFlags))
{
   // set some properties of the dialog
   setWindowFlag(Qt::WindowContextHelpButtonHint, false);
   setMinimumSize(cWINDOW_SIZE);

   // set up the UI
   mUi.setupUi(this);
   mUi.mCSV_Settings->BuildTable(cCSV_EVENT_OUTPUT_TAGS);
   mUi.mEVT_Settings->BuildTable(cEVT_EVENT_OUTPUT_TAGS);

   // disable/enable the appropriate widgets based on the default state of the check boxes
   if (mUi.mCSV->isChecked())
   {
      EnableCSV_EventOutput();
   }
   else
   {
      DisableCSV_EventOutput();
   }
   if (mUi.mEVT->isChecked())
   {
      EnableEVT_EventOutput();
   }
   else
   {
      DisableEVT_EventOutput();
   }

   // hide the Help button by default
   // Note:  if the standard in AFSIM Wizard changes to include a Help button, show this
   if (aShowHelpButton)
   {
      mUi.mMainButtons->ShowHelpButton();
   }
   else
   {
      mUi.mMainButtons->HideHelpButton();
   }

   // do the necessary connections (yes...there are a lot)
   connect(mUi.mCSV, &QCheckBox::stateChanged, this, &Dialog::OnCSV_Selected);
   connect(mUi.mEVT, &QCheckBox::stateChanged, this, &Dialog::OnEVT_Selected);
   connect(mUi.mFile, &FileLocatorWidget::FileChanged, this, &Dialog::LoadEventOutput);
   connect(this, &QDialog::rejected, this, &Dialog::OnCancelClick);
   connect(mUi.mMainButtons, &DialogButtonsWidget::OkayClicked, this, &Dialog::OnOkayClick);
   connect(mUi.mMainButtons, &DialogButtonsWidget::CancelClicked, this, &Dialog::OnCancelClick);
   connect(mUi.mMainButtons, &DialogButtonsWidget::HelpClicked, this, &Dialog::OnHelpClick);
   connect(mUi.mMainButtons, &DialogButtonsWidget::RestoreDefaultsClicked, this, &Dialog::OnRestoreDefaults);
   mEditorClosedCB = wizSignals->EditorClosed.Connect(&Dialog::OnEditorClosed, this);
   /*connect(wizSignals, &wizard::Signals::ActiveQtDocumentLoaded,
           this,       &Dialog::OnActiveQtDocumentLoaded);*/
   connect(evtOutSignals, &PluginUtil::Signals::AcquireSettings, this, &Dialog::OnAcquireSettings);
   connect(evtOutSignals, &PluginUtil::Signals::AcquireCommandInputType, this, &Dialog::OnAcquireCommandInputType);

   // default to the current file that is in focus in AFSIM Wizard
   mUi.mFile->SwitchToFile(ProjectWorkspace::Instance()->GetEditorManager()->GetCurrentEditor());
}

Dialog::~Dialog()
{
   // do this just in case the metaobject tries to invoke the callback when the object has been deleted
   /*disconnect(wizSignals, &wizard::Signals::ActiveQtDocumentLoaded,
              this,       &Dialog::OnActiveQtDocumentLoaded);*/
   // notify the editor::Editors that the Dialog is closing
   for (auto& elem : mSourceFiles)
   {
      elem.second.OnEditorClose(nullptr);
   }
}

void Dialog::OnCSV_Selected(int aState)
{
   switch (aState)
   {
   case Qt::Checked:
      // enable the tabs (without modifying visibility)
      EnableCSV_EventOutput();
      if (mCurrentFile)
      {
         // enable the "need to save" property
         mCurrentFile->SetNeedToSave("CSV", true);
      }
      break;
   case Qt::Unchecked:
      // disable the tabs (without modifying visibility)
      DisableCSV_EventOutput();
      if (mCurrentFile)
      {
         // disable the "need to save" property
         mCurrentFile->SetNeedToSave("CSV", false);
      }
      break;
   // should NEVER get here
   default:
      break;
   }
   LoadEventOutput(mCurrentFile->GetFilePath());
}

void Dialog::OnEVT_Selected(int aState)
{
   switch (aState)
   {
   case Qt::Checked:
      // enable the tabs (without modifying visibility)
      EnableEVT_EventOutput();
      if (mCurrentFile)
      {
         // enable the "need to save" property
         mCurrentFile->SetNeedToSave("EVT", true);
      }
      break;
   case Qt::Unchecked:
      // disable the tabs (without modifying visibility)
      DisableEVT_EventOutput();
      if (mCurrentFile)
      {
         // disable the "need to save" property
         mCurrentFile->SetNeedToSave("EVT", false);
      }
      break;
   // should NEVER get here
   default:
      break;
   }
   LoadEventOutput(mCurrentFile->GetFilePath());
}

void Dialog::OnRestoreDefaults()
{
   // restore the CSV defaults
   if (mUi.mCSV->isChecked())
   {
      mUi.mCSV_Settings->RestoreDefaults();
      mUi.mCSV_Events->RestoreDefaults();
   }
   // restore the EVT defaults
   if (mUi.mEVT->isChecked())
   {
      mUi.mEVT_Settings->RestoreDefaults();
      mUi.mEVT_Events->RestoreDefaults();
   }
}

void Dialog::OnOkayClick()
{
   // build the "cache" string
   CacheEventOutput(false);
   mCurrentFile = nullptr;
   for (auto& elem : mSourceFiles)
   {
      // apply changes to all editor::Editors
      if (!elem.second.Save())
      {
         // display a message if the save fails for any reason
         QMessageBox::warning(this, "Error Saving File", "Could not save file");
         return;
      }
   }

   // notify connected objects that the Dialog is closing
   emit Closed();
}

void Dialog::OnCancelClick()
{
   // notify connected objects that the Dialog is closing
   emit Closed();
}

void Dialog::OnHelpClick()
{
   // create a QMessageBox (information-level message with rich text format) with the help text and display it
   QMessageBox mb(QMessageBox::Icon::Information,
                  "(csv_)" + cEVT_EVENT_OUTPUT + " Editor Help",
                  cHELP_TEXT,
                  QMessageBox::StandardButton::Ok);
   mb.setTextFormat(Qt::TextFormat::RichText);
   mb.exec();
}

void Dialog::OnAcquireSettings(setting::Widget*& aWidget, const QString& aType)
{
   // acquire the appropriate setting::Widget
   if (aType == cCSV_EVENT_OUTPUT)
   {
      aWidget = mUi.mCSV_Settings;
   }
   else if (aType == cEVT_EVENT_OUTPUT)
   {
      aWidget = mUi.mEVT_Settings;
   }
   else
   {
      aWidget = nullptr;
   }
}

void Dialog::OnAcquireCommandInputType(setting::ValueWidget::Type& aInputType, const QString& aCommand)
{
   // acquire the appropriate ValueWidget::Type
   const auto& csvSettings = mUi.mCSV_Settings->GetAvailableSettings();
   auto        csvIter     = csvSettings.find(aCommand);
   if (csvIter != csvSettings.end())
   {
      aInputType = csvIter->mValueType;
      return;
   }
   const auto& evtSettings = mUi.mEVT_Settings->GetAvailableSettings();
   auto        evtIter     = evtSettings.find(aCommand);
   if (evtIter != evtSettings.end())
   {
      aInputType = evtIter->mValueType;
      return;
   }
}

void Dialog::DisableCSV_EventOutput()
{
   // set up the tooltip because it is important
   mUi.mCSV->setToolTip("Do not edit the csv_event_output block in the current file.");
   // disable the tabs
   if (!mUi.mCSV->isChecked() && !mUi.mEVT->isChecked() && mDummyTabInWidget == -1)
   {
      mUi.tabWidget->insertTab(static_cast<int>(Page::cDUMMY), mUi.mDummyTab, "");
      mDummyTabInWidget = 0;
      mUi.tabWidget->setCurrentIndex(static_cast<int>(Page::cDUMMY));
   }
   mUi.tabWidget->tabBar()->setTabEnabled(static_cast<int>(Page::cCSV_EVENTS_LIST) + mDummyTabInWidget, false);
   mUi.mCSV_Events->Disable();
   mUi.tabWidget->tabBar()->setTabEnabled(static_cast<int>(Page::cCSV_SETTINGS_LIST) + mDummyTabInWidget, false);
   mUi.mCSV_Settings->Disable();
}

void Dialog::EnableCSV_EventOutput()
{
   // set up the tooltip because it is important
   mUi.mCSV->setToolTip("Edit the csv_event_output block in the current file.");
   // enable the tabs
   if ((mUi.mCSV->isChecked() || mUi.mEVT->isChecked()) && mDummyTabInWidget == 0)
   {
      mUi.tabWidget->removeTab(static_cast<int>(Page::cDUMMY));
      mDummyTabInWidget = -1;
      mUi.tabWidget->setCurrentIndex(static_cast<int>(Page::cCSV_EVENTS_LIST) + mDummyTabInWidget);
   }
   mUi.tabWidget->tabBar()->setTabEnabled(static_cast<int>(Page::cCSV_EVENTS_LIST) + mDummyTabInWidget, true);
   mUi.mCSV_Events->Enable();
   mUi.tabWidget->tabBar()->setTabEnabled(static_cast<int>(Page::cCSV_SETTINGS_LIST) + mDummyTabInWidget, true);
   mUi.mCSV_Settings->Enable();
}

void Dialog::DisableEVT_EventOutput()
{
   // set up the tooltip because it is important
   mUi.mEVT->setToolTip("Do not edit the event_output block in the current file.");
   // disable the tabs
   if (!mUi.mCSV->isChecked() && !mUi.mEVT->isChecked() && mDummyTabInWidget == -1)
   {
      mUi.tabWidget->insertTab(static_cast<int>(Page::cDUMMY), mUi.mDummyTab, "");
      mDummyTabInWidget = 0;
      mUi.tabWidget->setCurrentIndex(static_cast<int>(Page::cDUMMY));
   }
   mUi.tabWidget->tabBar()->setTabEnabled(static_cast<int>(Page::cEVT_EVENTS_LIST) + mDummyTabInWidget, false);
   mUi.mEVT_Events->Disable();
   mUi.tabWidget->tabBar()->setTabEnabled(static_cast<int>(Page::cEVT_SETTINGS_LIST) + mDummyTabInWidget, false);
   mUi.mEVT_Settings->Disable();
}

void Dialog::EnableEVT_EventOutput()
{
   // set up the tooltip because it is important
   mUi.mEVT->setToolTip("Edit the event_output block in the current file.");
   // enable the tabs
   if ((mUi.mCSV->isChecked() || mUi.mEVT->isChecked()) && mDummyTabInWidget == 0)
   {
      mUi.tabWidget->removeTab(static_cast<int>(Page::cDUMMY));
      mDummyTabInWidget = -1;
      mUi.tabWidget->setCurrentIndex(static_cast<int>(Page::cEVT_EVENTS_LIST) + mDummyTabInWidget);
   }
   mUi.tabWidget->tabBar()->setTabEnabled(static_cast<int>(Page::cEVT_EVENTS_LIST) + mDummyTabInWidget, true);
   mUi.mEVT_Events->Enable();
   mUi.tabWidget->tabBar()->setTabEnabled(static_cast<int>(Page::cEVT_SETTINGS_LIST) + mDummyTabInWidget, true);
   mUi.mEVT_Settings->Enable();
}

void Dialog::OnEditorClosed(wizard::Editor* aEditorPtr)
{
   auto filePath = QString::fromStdString(aEditorPtr->GetSource()->GetFilePath().GetNormalizedPath());
   auto file     = mSourceFiles.find(filePath);
   if (file != mSourceFiles.end())
   {
      mSourceFiles.erase(file);
      mFileHistory.removeOne(filePath);
   }
   if (mSourceFiles.empty())
   {
      mCurrentFile = nullptr;
      if (mUi.mCSV->isChecked())
      {
         mUi.mCSV_Events->RestoreDefaults();
         mUi.mCSV_Settings->RestoreDefaults();
      }
      if (mUi.mEVT->isChecked())
      {
         mUi.mEVT_Events->RestoreDefaults();
         mUi.mEVT_Settings->RestoreDefaults();
      }
   }
   else
   {
      auto prevFile = mSourceFiles.find(mFileHistory.first());
      if (prevFile != mSourceFiles.end())
      {
         mCurrentFile = &prevFile->second;
      }
   }
   mUi.mFile->SwitchToFile(mCurrentFile);
}

void Dialog::LoadEventOutput(const QString& aFilePath)
{
   // short circuit if the file is empty or in "Select a File" state
   if (aFilePath.isEmpty() || aFilePath == "Select a File")
   {
      return;
   }

   // build the "cache" string (prompting user explicitly)
   CacheEventOutput(true);
   // reset the state of the Dialog components
   OnRestoreDefaults();

   // find the file and set it as the current
   auto file = mSourceFiles.find(aFilePath);
   if (file != mSourceFiles.end())
   {
      mCurrentFile = &file->second;
      mCurrentFile->GiveFocus();
   }
   // create the editor::Editor and set it as the current
   else
   {
      mSourceFiles.insert({aFilePath, editor::Editor{aFilePath.toStdString()}});
      mCurrentFile = &mSourceFiles.find(aFilePath)->second;
   }
   // update the file access history
   auto fileHist = mFileHistory.indexOf(aFilePath);
   if (fileHist > -1)
   {
      auto elem = mFileHistory.takeAt(fileHist);
      mFileHistory.prepend(elem);
   }
   else
   {
      mFileHistory.prepend(aFilePath);
   }

   // acquire the CSV tokenizer::Tokenizer or create it if it does not exist
   editor::tokenizer::Tokenizer* csvTok{nullptr};
   csvTok = mCurrentFile->GetTokenizer("CSV");
   if (!csvTok)
   {
      if (mUi.mCSV->isChecked())
      {
         mCurrentFile->AddTokenizer("CSV", cCSV_EVENT_OUTPUT_TAGS);
         csvTok = mCurrentFile->GetTokenizer("CSV");
      }
      else if (mCurrentFile->RemoveTokenizer("CSV"))
      {
         csvTok = nullptr;
      }
   }

   // acquire the EVT tokenizer::Tokenizer or create it if it does not exist
   editor::tokenizer::Tokenizer* evtTok{nullptr};
   evtTok = mCurrentFile->GetTokenizer("EVT");
   if (!evtTok)
   {
      if (mUi.mEVT->isChecked())
      {
         mCurrentFile->AddTokenizer("EVT", cEVT_EVENT_OUTPUT_TAGS);
         evtTok = mCurrentFile->GetTokenizer("EVT");
      }
      else if (mCurrentFile->RemoveTokenizer("EVT"))
      {
         evtTok = nullptr;
      }
   }

   // attempt to open the file; display a message if it fails for any reason
   if (!mCurrentFile->Open(aFilePath.toStdString()))
   {
      QMessageBox::warning(this, "Error Opening File", "Could not open file:  " + aFilePath);
      return;
   }

   // validate the token::Tokens in the CSV tokenizer::Tokenizer and read the settings into the CSV settings and events lists
   if (csvTok)
   {
      csvTok->ValidateSettings(mUi.mCSV_Settings->GetAvailableCommands());
      csvTok->ValidateEvents(mUi.mCSV_Events->GetAvailableCommands());
      mUi.mCSV_Settings->ReadSettings(csvTok);
      mUi.mCSV_Settings->SetModifiedByUser(false);
      mUi.mCSV_Events->ReadEvents(csvTok);
      mUi.mCSV_Events->SetModifiedByUser(false);
   }

   // validate the token::Tokens in the EVT tokenizer::Tokenizer and read the settings into the EVT settings and events lists
   if (evtTok)
   {
      evtTok->ValidateSettings(mUi.mEVT_Settings->GetAvailableCommands());
      evtTok->ValidateEvents(mUi.mEVT_Events->GetAvailableCommands());
      mUi.mEVT_Settings->ReadSettings(evtTok);
      mUi.mEVT_Settings->SetModifiedByUser(false);
      mUi.mEVT_Events->ReadEvents(evtTok);
      mUi.mEVT_Events->SetModifiedByUser(false);
   }
}

void Dialog::CacheEventOutput(bool aPromptUser)
{
   if (mCurrentFile)
   {
      editor::tokenizer::Tokenizer* csvTok{nullptr};
      bool                          cacheCSV{false};
      // prompt the user to stash changes if they uncheck the CSV check box after changing the state of either the CSV
      // settings or events list
      if (aPromptUser && (mUi.mCSV_Settings->ModifiedByUser() || mUi.mCSV_Events->ModifiedByUser()) &&
          !mUi.mCSV->isChecked())
      {
         auto button = QMessageBox::question(
            this,
            "csv_event_output and event_output Editor:  Temporary Changes",
            "There are temporary changes to the event_output tabs.  Do you want to stash the changes?");
         if (button == QMessageBox::Yes)
         {
            cacheCSV = true;
         }
      }
      // stash changes if necessary
      if (mUi.mCSV->isChecked() || cacheCSV)
      {
         csvTok = mCurrentFile->GetTokenizer("CSV");
         if (csvTok)
         {
            mUi.mCSV_Settings->WriteSettings(csvTok);
            mUi.mCSV_Events->WriteEvents(csvTok);
         }
      }

      // prompt the user to stash changes if they uncheck the EVT check box after changing the state of either the EVT
      // settings or events list
      editor::tokenizer::Tokenizer* evtTok{nullptr};
      bool                          cacheEVT{false};
      if (aPromptUser && (mUi.mEVT_Settings->ModifiedByUser() || mUi.mEVT_Events->ModifiedByUser()) &&
          !mUi.mEVT->isChecked())
      {
         auto button = QMessageBox::question(
            this,
            "csv_event_output and event_output Editor:  Temporary Changes",
            "There are temporary changes to the event_output tabs.  Do you want to stash the changes?");
         if (button == QMessageBox::Yes)
         {
            cacheEVT = true;
         }
      }
      // stash changes if necessary
      if (mUi.mEVT->isChecked() || cacheEVT)
      {
         evtTok = mCurrentFile->GetTokenizer("EVT");
         if (evtTok)
         {
            mUi.mEVT_Settings->WriteSettings(evtTok);
            mUi.mEVT_Events->WriteEvents(evtTok);
         }
      }

      // attempt to build the "cache" string; display a message if it fails for any reason
      if (!mCurrentFile->Cache())
      {
         QMessageBox::warning(this, "Error Caching File", "Could not cache file");
         return;
      }
   }
}

} // namespace EventOutput
} // namespace wizard
