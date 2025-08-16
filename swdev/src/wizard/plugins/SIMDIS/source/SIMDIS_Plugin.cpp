// ****************************************************************************
// CUI//REL TO USA ONLY
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2018 - 2019 Lockheed Martin Corporation. All Rights Reserved.
// Lockheed Martin Corporation hereby provides the below source code to the
// United States Government with Unlimited Rights as such is defined in
// DFARS 252.227 - 7014.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "SIMDIS_Plugin.hpp"

#include <iomanip>
#include <QColorDialog>
#include <QFileDialog>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QString>
#include <QTextBrowser>
#include <QVBoxLayout>

#include "ParseResults.hpp"
#include "Project.hpp"
#include "TextSource.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"
#include "WsfEditor.hpp"
//#include "ViActionSet.hpp"
//#include "ViSettings.hpp"
//#include "ViSignals.hpp"
//#include "ViToolProcessManager.hpp"

WKF_PLUGIN_DEFINE_SYMBOLS(wizard::SIMDIS::Plugin, "SIMDIS Launcher", "Launches SIMDIS and provides UI for editing SIMDIS parameters in AFSIM input", "wizard")

namespace {
   struct ReplaceRangeAction
   {
      UtTextDocumentRange  mRange;
      std::string          mText;
      ReplaceRangeAction(UtTextDocumentRange aRange, const std::string& aText) : mRange(aRange), mText(aText) {}
      void operator()()
      {
         wizard::TextSource* sourcePtr = (wizard::TextSource*)mRange.mSource;
         if (sourcePtr != nullptr)
         {
            sourcePtr->Replace(mRange, mText);
         }
      }
   };
}

wizard::SIMDIS::Plugin::Plugin(const QString& aPluginName,
                               const size_t   aUniqueId)
   : wizard::Plugin(aPluginName, aUniqueId)
   , mActionName("Tools.Plugin.ConfigureSIMDIS")
   , mModelMenu("SIMDIS Models")
   , mExePath("")
{
   QMenu* toolsMenuPtr = wkfEnv.GetMainWindow()->GetMenuByName("Tools");
   if (toolsMenuPtr)
   {
      connect(toolsMenuPtr->addAction("Configure SIMDIS..."), &QAction::triggered, this, &Plugin::ConfigureSIMDIS);
   }

   // sign up for editor right clicks
   connect(wizSignals, &wizard::Signals::EditorContextMenu, this, &Plugin::onEditorContextMenu);
   // sign up for file right clicks
   connect(wizSignals, &wizard::Signals::ActionsForFileRequested, this, &Plugin::ActionsForFileRequested);
   // sign up for settings
}

// this is called when the user right clicks on a file in the project browser
void wizard::SIMDIS::Plugin::ActionsForFileRequested(const QFileInfo& aFileInfo, QMenu* aMenu)
{
   // if the file has an ASI suffix
   if (aFileInfo.completeSuffix() == "asi")
   {
      // and we have a SIMDIS install configured
      if (!mExePath.isEmpty())
      {
         // provide an option to launch in the file's context menu
         connect(aMenu->addAction(QIcon::fromTheme("show"), "Open with SIMDIS"), &QAction::triggered, [this, aFileInfo]() { Launch(aFileInfo); });
      }
   }
}

// lets find SIMDIS
void wizard::SIMDIS::Plugin::ConfigureSIMDIS()
{
   // clear out our data
   mModelMenu.clear();
   mExePath = "";

   // ask the user for the path to SIMDIS
   QString SIMDISpath = QFileDialog::getOpenFileName(wkfEnv.GetMainWindow(), "Navigate to the SIMDIS Executable", QString(), "simdis.exe;; All Files (*.*)");
   // if we got a path
   if (!SIMDISpath.isEmpty())
   {
      // try to find the model directory
      UtPath path(SIMDISpath.toStdString());
      path.Up();
      path.Up();
      path.Up();
      path = path.GetSystemPath() + "/data/models";
      // if there is a model directory
      if (path.Exists())
      {
         // configure SIMDIS with the tool manager
         mExePath = SIMDISpath;
         // build a list of models (opt and 3db for now)
         BuildModelList(path, mModelMenu);
      }
      else
      {
         // let the user know we didn't find any models and ask how to proceed
         if (QMessageBox::Yes == QMessageBox::warning(wkfEnv.GetMainWindow(), "Could not find SIMDIS models", "Could not find the SIMDIS models, would you like to find them manually?  (This is may not work.)", QMessageBox::Yes, QMessageBox::No))
         {
            // ask the user to find the model directory
            QString modelPath = QFileDialog::getExistingDirectory(wkfEnv.GetMainWindow(), "Navigate to the SIMDIS Model Directory", QString());
            if (!modelPath.isEmpty())
            {
               // configure SIMDIS with the tool manager
               mExePath = SIMDISpath;
               // configure SIMDIS with the tool manager
               BuildModelList(UtPath(modelPath.toStdString()), mModelMenu);
            }
            else
            {
               // I would suspect at this point that the user pointed to the wrong thing (maybe twice!), but just in case we allow the following
               if (QMessageBox::Yes == QMessageBox::warning(wkfEnv.GetMainWindow(), "Could not find SIMDIS models", "Would you like to continue to configure SIMDIS?", QMessageBox::Yes, QMessageBox::No))
               {
                  // configure SIMDIS with the tool manager
                  mExePath = SIMDISpath;
               }
            }
         }
      }
   }
}

// lets figure out what models are available in a directory
void wizard::SIMDIS::Plugin::BuildModelList(const UtPath& aPath, QMenu& aMenu)
{
   std::vector<std::string> files;
   std::vector<std::string> dirs;
   // get files and models in the directory
   aPath.ListDir(files, dirs);
   // for every file
   for (auto&& file : files)
   {
      // add the file (without extension) to the list
      file.resize(file.size() - 4);
      aMenu.addAction(file.c_str());
   }
   // for every directory
   for (auto&& dir : dirs)
   {
      // if it isn't one of these
      if ((dir != "..") && (dir != "."))
      {
         // add a submenu
         QMenu* newMenu = aMenu.addMenu(dir.c_str());
         // populate it with the models within
         BuildModelList(UtPath(aPath.GetSystemPath() + '/' + dir), *newMenu);
         // if it is empty, get rid of it
         if (newMenu->isEmpty())
         {
            aMenu.removeAction(newMenu->menuAction());
            delete newMenu;
         }
      }
   }
}

// user right clicked on some text in an editor
void wizard::SIMDIS::Plugin::onEditorContextMenu(wizard::Editor* aEditControlPtr,
                                                 QMenu&          aMenu)
{
   // if we are configured
   if (!mModelMenu.isEmpty())
   {
      // find a parse node associated with the file location clicked
      wizard::WsfEditor* editorPtr = dynamic_cast<wizard::WsfEditor*>(aEditControlPtr);
      if (!editorPtr) return;
      wizard::ParseResults* parseResultsPtr = editorPtr->GetProject()->GetParseResults();
      if (!parseResultsPtr || parseResultsPtr->ProxyRegistry() == nullptr) return;
      if (editorPtr->GetContextMenuCharacterPosition() < 0) return;
      UtTextDocumentRange range(editorPtr->GetSource(), editorPtr->GetContextMenuCharacterPosition(), editorPtr->GetContextMenuCharacterPosition());
      WsfParseNode* nodePtr = parseResultsPtr->FindLastParseNode(range);
      if (!nodePtr) return;

      WsfParseNode* n = nodePtr;
      while (n)
      {
         WsfParseRule* rulePtr = n->GetTopLevelRule();
         // if the node is on a platform
         if (rulePtr && rulePtr->GetRuleDescription() == "<Platform>")
         {
            // if the type is reported as 'icon'
            if (strcmp(n->mType, "icon") == 0)
            {
               // get the range of the value
               UtTextDocumentRange range = n->SubtreeRange();
               if (range.Valid())
               {
                  // add a menu option to swap out the current value with a model name
                  aMenu.addMenu(&mModelMenu);
                  mModelMenu.disconnect();
                  QObject::connect(&mModelMenu, &QMenu::triggered, [this, range](QAction* aAction) {ReplaceRangeAction(range, aAction->text().toStdString())(); });
                  return;
               }
            }
         }
         if (rulePtr && rulePtr->GetRuleDescription() == "<simdis-interface-command>")
         {
            if (strcmp(n->mType, "icon") == 0)
            {
               // get the range of the value
               UtTextDocumentRange range = n->SubtreeRange();
               if (range.Valid())
               {
                  // add a menu option to swap out the current value with a model name
                  aMenu.addMenu(&mModelMenu);
                  mModelMenu.disconnect();
                  QObject::connect(&mModelMenu, &QMenu::triggered, [this, range](QAction* aAction) {ReplaceRangeAction(range, aAction->text().toStdString())(); });
                  return;
               }
            }
            else if (strcmp(n->mType, "color") == 0)
            {
               // get the range of the value
               UtTextDocumentRange range = n->SubtreeRange();
               if (range.Valid())
               {
                  QObject::connect(aMenu.addAction("Set SIMDIS Beam Color..."), &QAction::triggered, [this, range]() { ReplaceRangeWithColor(range); });
                  return;
               }
            }
         }
         n = n->GetParent();
      }
   }
}

// run simdis on the file provided
void wizard::SIMDIS::Plugin::Launch(const QFileInfo& aPath)
{
   QString workingDir = aPath.absolutePath();
   QStringList argList;
   argList.push_back("--file");
   argList.push_back(aPath.absoluteFilePath());
   if (!QProcess::startDetached(mExePath, argList, workingDir))
   {
      QMessageBox::information(nullptr, "Can't execute", "Cannot execute " + mExePath);
   }
}

void wizard::SIMDIS::Plugin::LoadSettings(QSettings& aSettings)
{
   // clear our data
   mExePath = "";
   mModelMenu.clear();

   // load out data from file
   aSettings.beginGroup("SIMDIS");
   mExePath = aSettings.value("path").toString();
   int modelCount = aSettings.beginReadArray("models");
   for (int i = 0; i < modelCount; ++i)
   {
      aSettings.setArrayIndex(i);
      QStringList address = aSettings.value("model").toStringList();
      QString model = address.back();
      address.pop_back();
      QMenu* menu = &mModelMenu;
      for (auto&& it : address)
      {
         bool found = false;
         for (auto&& action : menu->actions())
         {
            if ((action->text() == it) && (action->menu()))
            {
               menu = action->menu();
               found = true;
            }
         }
         if (!found)
         {
            menu = menu->addMenu(it);
         }
      }
      menu->addAction(model);
   }
   aSettings.endArray();
   aSettings.endGroup();
}

void wizard::SIMDIS::Plugin::SaveSettings(QSettings& aSettings)
{
   // save our data to file
   aSettings.beginGroup("SIMDIS");
   aSettings.setValue("path", mExePath);
   int idx = 0;
   aSettings.beginWriteArray("models");
   RecordModels(&mModelMenu, aSettings, idx, QStringList());
   aSettings.endArray();

   aSettings.endGroup();
}

void wizard::SIMDIS::Plugin::RecordModels(QMenu*             aMenu,
                                          QSettings&         aSettings,
                                          int&               aIndex,
                                          const QStringList& aPrefix)
{
   // save the models to ini file
   for (auto&& action : aMenu->actions())
   {
      QStringList address = aPrefix;
      address.push_back(action->text());
      if (action->menu())
      {
         RecordModels(action->menu(), aSettings, aIndex, address);
      }
      else
      {
         aSettings.setArrayIndex(aIndex++);
         aSettings.setValue("model", address);
      }
   }
}

void wizard::SIMDIS::Plugin::ReplaceRangeWithColor(const UtTextDocumentRange& aRange)
{
   // ask the user for a color, and convert it to a string
   QColor out = QColorDialog::getColor(Qt::yellow, wkfEnv.GetMainWindow(), "Choose Color", QColorDialog::ShowAlphaChannel);
   if (out.isValid())
   {
      std::stringstream ss;
      unsigned char alpha = out.alphaF() * 255;
      unsigned int red = out.redF() * 255;
      unsigned int green = out.greenF() * 255;
      unsigned int blue = out.blueF() * 255;
      unsigned int color = (alpha * 256 * 256 * 256) + (blue * 256 * 256) + (green * 256) + red;
      ss << "0x" << std::setfill('0') << std::setw(8) << std::hex << color;
      wizard::TextSource* sourcePtr = (wizard::TextSource*)aRange.mSource;
      if (sourcePtr != nullptr)
      {
         sourcePtr->Replace(aRange, ss.str());
      }
   }
}
