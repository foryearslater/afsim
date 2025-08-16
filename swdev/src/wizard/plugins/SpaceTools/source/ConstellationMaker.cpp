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

#include "ConstellationMaker.hpp"

#include <sstream>

#include <QMenu>
#include <QMessageBox>
#include <QTextFrame>

#include "AdvancedConstellationGenerator.hpp"
#include "ConstellationMakerOptions.hpp"
#include "Editor.hpp"
#include "EditorManager.hpp"
#include "Project.hpp"
#include "ProjectWorkspace.hpp"
#include "Signals.hpp"
#include "UtMemory.hpp"
#include "WkfMainWindow.hpp"
#include "WsfSpaceConstellationGenerator.hpp"

SpaceTools::ConstellationMaker::ConstellationMaker()
   : mConstellationUI_Ptr{ut::make_unique<ConstellationMakerUI>()}
   , mOptions{SpaceTools::ConstellationMakerOptions::DefaultConstellationMakerOptions()}
   , mProjectPtr{nullptr}
{
   connect(mConstellationUI_Ptr.get(),
           &ConstellationMakerUI::GenerateConstellation,
           this,
           &ConstellationMaker::GenerateConstellation);
   connect(mConstellationUI_Ptr.get(), &ConstellationMakerUI::GenerateGenerator, this, &ConstellationMaker::GenerateGenerator);
   connect(mConstellationUI_Ptr.get(), &ConstellationMakerUI::rejected, this, &ConstellationMaker::SaveOptions);

   // This will add the constellation maker to the context menu for relevant files.
   connect(wizSignals, &wizard::Signals::EditorContextMenu, this, &ConstellationMaker::EditorContextMenu);
}

//! Reacts to a project being opened in the Wizard.
void SpaceTools::ConstellationMaker::OnProjectOpened(wizard::Project* aProjectPtr)
{
   mProjectPtr = aProjectPtr;
}

//! Reacts to a project being closed in the Wizard.
void SpaceTools::ConstellationMaker::OnProjectClosed(wizard::Project* aProjectPtr)
{
   mOptions    = SpaceTools::ConstellationMakerOptions::DefaultConstellationMakerOptions();
   mProjectPtr = nullptr;
   mConstellationUI_Ptr->close();
}

//! Save the options from the UI.
//!
//! This is called whenever the dialog is closed so that the options might
//! persist between dialog invocations.
void SpaceTools::ConstellationMaker::SaveOptions()
{
   mConstellationUI_Ptr->SaveValues(mOptions);
}

//! Add an action to an editor's context menu when appropriate.
void SpaceTools::ConstellationMaker::EditorContextMenu(wizard::Editor* aEditorPtr, QMenu& aMenu)
{
   WsfConstellationOptions options = ConstellationMakerOptions::InterpretOptions(aEditorPtr->document());
   if (options.IsSetup())
   {
      auto saveAndShow = [this, options]()
      {
         mOptions = options;
         ShowDialog();
      };
      connect(aMenu.addAction(QIcon::fromTheme("satellite"), "Edit using Constellation Maker"),
              &QAction::triggered,
              saveAndShow);
   }
}

//! Generate the constellation input file.
//!
//! This will generate the constellation input file with options as
//! specified by the dialog. If the requested folder does not exist,
//! this will create it.
void SpaceTools::ConstellationMaker::GenerateConstellation()
{
   SaveOptions();

   std::string relativePath = mOptions.GetGeneratedFilePrefix() + "_autogen.txt";
   UtPath      path         = mProjectPtr->WorkingDirectory() + relativePath;
   UtPath      fileDir(path);
   fileDir.Up();
   bool ok{true};
   if (fileDir.Stat() != UtPath::cDIRECTORY)
   {
      ok = fileDir.Mkdir();
   }

   if (ok)
   {
      // If the file does not currently exist, create it.
      if (!path.Exists())
      {
         std::string   fileName{path.GetSystemPath()};
         std::ofstream outFile(fileName, std::ios::out);
         outFile.close();
         wizSignals->CheckFilesForModification();
         mProjectPtr->TriggerReparse();
      }

      std::stringstream ss{};
      wsf::space::ConstellationGenerator::Generate(mOptions, ss);

      auto editorPtr = wizard::ProjectWorkspace::Instance()->GotoFile(path.GetSystemPath().c_str());
      auto rootFrame = editorPtr->document()->rootFrame();
      auto curs      = rootFrame->firstCursorPosition();
      curs.setPosition(rootFrame->lastPosition(), QTextCursor::KeepAnchor);
      curs.insertText(QString::fromStdString(ss.str()));
   }
}

//! Generate the constellation generator.
//!
//! This will create the script that can generate the constellation input file.
//! If the requested folder does not exist, this will create it. Also, this will
//! save some content from a previously created generator to allow changes to the
//! constellation options to be made without overwriting customization scripts.
void SpaceTools::ConstellationMaker::GenerateGenerator()
{
   SaveOptions();

   bool        ok{true};
   std::string userGeneralContent{};
   std::string userSpecificContent{};

   std::string relativePath = mOptions.GetGeneratedFilePrefix() + "_generator.txt";
   UtPath      path         = mProjectPtr->WorkingDirectory() + relativePath;
   if (path.Stat() == UtPath::cFILE)
   {
      std::string       fileName{path.GetSystemPath()};
      std::ifstream     inFile(fileName, std::ios::in);
      std::stringstream buffer;
      buffer << inFile.rdbuf();
      AdvancedConstellationGenerator::GrabUserContent(buffer.str(), userGeneralContent, userSpecificContent);
   }
   else
   {
      UtPath fileDir(path);
      fileDir.Up();
      if (fileDir.Stat() != UtPath::cDIRECTORY)
      {
         ok = fileDir.Mkdir();
      }
   }

   if (ok)
   {
      // If the file does not exist, create it.
      if (!path.Exists())
      {
         std::string   fileName{path.GetSystemPath()};
         std::ofstream outFile(fileName, std::ios::out);
         outFile.close();
         wizSignals->CheckFilesForModification();
         mProjectPtr->TriggerReparse();
      }

      std::stringstream ss{};
      AdvancedConstellationGenerator::CreateGenerator(mOptions, userGeneralContent, userSpecificContent, ss);

      auto editorPtr = wizard::ProjectWorkspace::Instance()->GotoFile(path.GetSystemPath().c_str());
      auto rootFrame = editorPtr->document()->rootFrame();
      auto curs      = rootFrame->firstCursorPosition();
      curs.setPosition(rootFrame->lastPosition(), QTextCursor::KeepAnchor);
      curs.insertText(QString::fromStdString(ss.str()));
   }
}

//! Display the Constellation Maker dialog.
//!
//! In addition to displaying the dialog, this will scan the active file in the
//! editor to see if it contains a Constellation Maker options comment block.
//! If it does, it will load those values into the dialog to make modification of
//! existing constellations easy.
void SpaceTools::ConstellationMaker::ShowDialogFromMenu()
{
   if (mConstellationUI_Ptr)
   {
      auto current = wizard::ProjectWorkspace::Instance()->GetEditorManager()->GetCurrentEditor();
      if (current != nullptr)
      {
         QTextDocument* doc          = current->document();
         auto           trialOptions = ConstellationMakerOptions::InterpretOptions(doc);
         if (trialOptions.IsSetup())
         {
            mOptions = trialOptions;
         }
         else
         {
            mOptions = ConstellationMakerOptions::DefaultConstellationMakerOptions();
         }
      }

      ShowDialog();
   }
}

//! Display the Constellation Maker dialog.
void SpaceTools::ConstellationMaker::ShowDialog()
{
   if (mConstellationUI_Ptr)
   {
      mConstellationUI_Ptr->LoadValues(mOptions);
      mConstellationUI_Ptr->show();
      mConstellationUI_Ptr->activateWindow();
   }
}

//! Reloads the generated file if it is open in an editor tab.
void SpaceTools::ConstellationMaker::ReloadFileAsNeeded(const UtPath& aFilePath)
{
   wizard::EditorManager* editormgr = wizard::ProjectWorkspace::Instance()->GetEditorManager();
   for (auto&& editor : editormgr->GetEditorMap())
   {
      wizard::TextSource* src = editor.second->GetSource();
      if (src->GetFileName() == aFilePath.GetFileName())
      {
         src->ReadSource();
      }
   }
}
