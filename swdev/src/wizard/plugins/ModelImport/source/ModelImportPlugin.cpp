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

#include "ModelImportPlugin.hpp"

#include <QDir>
#include <QMessageBox>
#include <QProgressDialog>

#include "JsonRule.hpp"
#include "ModelImportDirectoryReaderThread.hpp"
#include "ParseResults.hpp"
#include "Signals.hpp"
#include "UtQtMemory.hpp"
#include "WkfMainWindow.hpp"
#include "WkfPlugin.hpp"

WKF_PLUGIN_DEFINE_SYMBOLS(ModelImport::Plugin,
                          "Model Import Beta",
                          "Provides an interface for copying files and their dependencies from an external data base.",
                          "wizard")

//! Compares two ModelImport::ItemData objects.
//! Weak ordering is based on the name of the items.
//! @param aLeft The left input.
//! @param aRight The right input.
//! @returns -1 if left <  right
//!           0 if left == right
//!          +1 if left >  right
//! @note Intended for use in MergeData().
static int ItemDataCompare(const ModelImport::ItemData& aLeft, const ModelImport::ItemData& aRight)
{
   if (aLeft.mName < aRight.mName)
   {
      return -1;
   }
   if (aLeft.mName > aRight.mName)
   {
      return 1;
   }
   return 0;
};

// ================================================================================================
//! Moves the labels from one ModelImport::ItemData to another.
//! @param aNew The item that labels are being moved into.
//! @param aOld The item that labels are being moved from.
//! @note Intended for use in MergeData().
static void ItemDataMerge(ModelImport::ItemData& aNew, ModelImport::ItemData& aOld)
{
   aNew.mLabels = std::move(aOld.mLabels);
};

// ================================================================================================
//! Compares two ModelImport::ItemData objects.
//! Weak ordering is based on the name of the files, with path being used secondarily.
//! @param aLeft The left input.
//! @param aRight The right input.
//! @returns -1 if left <  right
//!           0 if left == right
//!          +1 if left >  right
static int FileDataCompare(const ModelImport::FileData& aLeft, const ModelImport::FileData& aRight)
{
   if (aLeft.mFile < aRight.mFile)
   {
      return -1;
   }
   if (aLeft.mFile > aRight.mFile)
   {
      return 1;
   }
   if (aLeft.mPath < aRight.mPath)
   {
      return -1;
   }
   if (aLeft.mPath > aRight.mPath)
   {
      return 1;
   }
   return 0;
};

// ================================================================================================
//! Moves the additional dependencies from one ModelImport::FileData to another, and merges the items it defines.
//! @param aNew The file that labels are being moved into.
//! @param aOld The file that labels are being moved from.
//! @note Intended for use in MergeData().
static void FileDataMerge(ModelImport::FileData& aNew, ModelImport::FileData& aOld)
{
   aNew.mAdditionalDependencies = std::move(aOld.mAdditionalDependencies);

   MergeData(aOld.mDefines, aNew.mDefines, ItemDataCompare, ItemDataMerge);
};

// ================================================================================================

ModelImport::Plugin::Plugin(const QString& aPluginName, const size_t aUniqueId)
   : wizard::Plugin(aPluginName, aUniqueId)
   , mDockWidget(new DockWidget)
   , mPrefWidgetPtr(new PrefWidget(nullptr))
   , mProgress(ut::qt::make_qt_ptr<QProgressDialog>("Generating Model Mapping File...",
                                                    QString(),
                                                    0,
                                                    0,
                                                    mPrefWidgetPtr,
                                                    Qt::CustomizeWindowHint | Qt::WindowTitleHint))
{
   wkf::MainWindow* mainWindowPtr = wkfEnv.GetMainWindow();
   mainWindowPtr->addDockWidget(Qt::RightDockWidgetArea, mDockWidget);
   mDockWidget->hide();

   mProgress->setWindowModality(Qt::ApplicationModal);
   mProgress->reset();

   // Preferences updated
   connect(&GetPrefObject(), &ModelImport::PrefObject::Update, this, &ModelImport::Plugin::ReadJson);
   connect(&GetPrefObject(), &ModelImport::PrefObject::UpdateDisplay, this, &ModelImport::Plugin::Display);
   connect(mPrefWidgetPtr.data(), &ModelImport::PrefWidget::ClearData, this, [this]() { mFileData.clear(); });

   // Update Json file
   connect(mPrefWidgetPtr.data(), &ModelImport::PrefWidget::UpdateWithMerge, this, &ModelImport::Plugin::UpdateMerge);
   connect(mPrefWidgetPtr.data(), &ModelImport::PrefWidget::UpdateWithOverwrite, this, &ModelImport::Plugin::UpdateOverwrite);

   // Reload Json file
   connect(mPrefWidgetPtr.data(),
           &ModelImport::PrefWidget::ReloadJson,
           this,
           [this]()
           {
              ReadJson();
              Display();
           });

   // Item selected for import
   connect(mDockWidget.data(), &ModelImport::DockWidget::ImportItem, this, &ModelImport::Plugin::Import);
   connect(mDockWidget.data(),
           &ModelImport::DockWidget::BeginImport,
           this,
           [this]()
           {
              mImportedFiles.clear();
              mReimportSelection = Reimport::cUNDECLARED;
              mValidImport       = true;
           });
   connect(mDockWidget.data(), &ModelImport::DockWidget::SearchModified, this, &ModelImport::Plugin::Display);

   // Read Directory
   connect(&mReadingThread, &ModelImport::DirectoryReaderThread::FinishedFile, mProgress.data(), &QProgressDialog::setValue);
   connect(&mReadingThread,
           &ModelImport::DirectoryReaderThread::finished,
           this,
           [this]()
           {
              mProgress->hide();

              if (mReadingThread.Errors() || mReadingThread.Warnings())
              {
                 QString warning =
                    Qt::convertFromPlainText("%1 errors encountered.\n%2 warnings encountered.\nSee %3 for details.");
                 warning =
                    warning.arg(mReadingThread.Errors()).arg(mReadingThread.Warnings()).arg("<a href=\"file:///%1/%2\">%2</a>");
                 warning = warning.arg(GetPath(), GetErrorsFileName());
                 QMessageBox::warning(nullptr, QString(), warning);
              }

              ReadDirectoryComplete();
           });
}

// ================================================================================================

const ModelImport::PrefObject& ModelImport::Plugin::GetPrefObject() const
{
   return *(mPrefWidgetPtr->GetPreferenceObject());
}

// ================================================================================================

QList<wkf::PrefWidget*> ModelImport::Plugin::GetPreferencesWidgets() const
{
   return {mPrefWidgetPtr};
}

// ================================================================================================

const QString& ModelImport::Plugin::GetPath() const noexcept
{
   return GetPrefObject().GetPreferences().mPath;
}

// ================================================================================================

const QString& ModelImport::Plugin::GetMetaFileName() const noexcept
{
   return GetPrefObject().GetPreferences().mMetaFile;
}

// ================================================================================================

const QString& ModelImport::Plugin::GetImportsFileName() const noexcept
{
   return GetPrefObject().GetPreferences().mImportsFile;
}

// ================================================================================================

const QString& ModelImport::Plugin::GetErrorsFileName() const noexcept
{
   return GetPrefObject().GetPreferences().mErrorsFile;
}

// ================================================================================================

const QString& ModelImport::Plugin::GetDisplayMode() const noexcept
{
   return GetPrefObject().GetPreferences().mDisplayMode;
}

// ================================================================================================

std::string ModelImport::Plugin::GetProjectPath() const noexcept
{
   // Check for open project
   if (!wizard::Project::Instance())
   {
      return std::string();
   }

   return wizard::Project::Instance()->ProjectDirectory().GetNormalizedPath();
}

// ================================================================================================

void ModelImport::Plugin::Display()
{
   mDockWidget->SetPath(GetPath());
   mDockWidget->DisplayData(mFileData, GetDisplayMode());
}

// ================================================================================================

void ModelImport::Plugin::ReadJson()
{
   mFileData.clear();
   Json::ReadFile(Path(GetPath(), GetMetaFileName()).toStdString(), mFileData);
}

// ================================================================================================

void ModelImport::Plugin::ReadDirectory()
{
   mFileData.clear();

   int numFiles = ModelImport::TotalDirectorySize(GetPath());

   mReadingThread.SetFields(GetPath(), mFileData, numFiles, Path(GetPath(), GetErrorsFileName()).toStdString());
   mProgress->setMaximum(numFiles);
   mProgress->setValue(0);

   mProgress->show();

   mReadingThread.start();
}

// ================================================================================================

void ModelImport::Plugin::ReadDirectoryComplete()
{
   if (!mOldData.empty())
   {
      MergeFileData(mOldData, mFileData);
   }

   WriteJson();
   mDockWidget->DisplayData(mFileData, GetDisplayMode());
}

// ================================================================================================

void ModelImport::Plugin::WriteJson()
{
   Json::WriteFile(Path(GetPath(), GetMetaFileName()).toStdString(), mFileData);
}

// ================================================================================================

void ModelImport::Plugin::UpdateMerge()
{
   mPrefWidgetPtr->GetPreferenceObject()->SetUpdateFlag();
   mOldData = std::move(mFileData);
   ReadDirectory();
}

// ================================================================================================

void ModelImport::Plugin::UpdateOverwrite()
{
   mPrefWidgetPtr->GetPreferenceObject()->SetUpdateFlag();
   mOldData.clear();
   ReadDirectory();
}

// ================================================================================================
// static
void ModelImport::Plugin::MergeFileData(ParseData& aOldData, ParseData& aNewData)
{
   MergeData(aOldData, aNewData, FileDataCompare, FileDataMerge);
}

// ================================================================================================

void ModelImport::Plugin::Import(const QString& aPathToFile)
{
   if (!mValidImport)
   {
      return;
   }

   // Check for open project
   if (GetProjectPath().empty())
   {
      QMessageBox::critical(nullptr, QString(), "A project must be open to import files.");
      mValidImport = false;
      return;
   }
   try
   {
      bool        ignore = false;
      QStringList checked;
      if (!ImportOkay(aPathToFile, checked, ignore))
      {
         mValidImport = false;
         return;
      }
   }
   catch (const QString& errorMessage)
   {
      QMessageBox::critical(nullptr, "File not found.", errorMessage);
      mValidImport = false;
      return;
   }

   QString importsDirectory = GetImportsFileName().left(GetImportsFileName().lastIndexOf('/'));

   // Open project directory and prepare "imports" directory
   QDir projectDirectory(Path(GetProjectPath()));
   projectDirectory.mkpath(Path(GetProjectPath(), importsDirectory));
   projectDirectory.cd(importsDirectory);

   bool importsFileExists = true;

   ImportRecursionHelper(aPathToFile, projectDirectory);

   if (!QFile::exists(Path(GetProjectPath(), GetImportsFileName())))
   {
      importsFileExists = false;
   }

   // Open imports.txt (or custom file) in APPEND mode
   std::ofstream importsTxt(Path(GetProjectPath(), GetImportsFileName()).toStdString(), std::ios::app);

   if (!importsFileExists)
   {
      importsTxt << "# This is a generated file containing all imported files.\n";
      importsTxt << "# Include this into your scenario to gain access to imported items.\n\n";
   }

   importsTxt << "include_once " << aPathToFile << "\n";

   if (!importsFileExists)
   {
      QMessageBox::information(nullptr,
                               "Reminder!",
                               QString("Your imports file (%1) has been generated. It is up to you to include it into "
                                       "your scenario. Wizard will not include it for you.")
                                  .arg(GetImportsFileName()));
   }

   wizSignals->CheckFilesForModification();
}

// ================================================================================================

void ModelImport::Plugin::ImportRecursionHelper(const QString& aFilePath, const QDir& aImportToDir)
{
   // This should not throw because the FileData's existence was checked for in ImportOkay().
   const ModelImport::FileData& file = LookupFileData(Path(GetPath(), aFilePath));

   aImportToDir.mkpath(file.PathToDir()); // Create subdirectories if necessary

   // Check if file was already imported
   if (QFile::exists(Path(aImportToDir.absolutePath(), aFilePath)) && !mImportedFiles.contains(aFilePath))
   {
      switch (mReimportSelection)
      {
      case Reimport::cYESTOALL:
         break;
      case Reimport::cNOTOALL:
         return;
      default:
         switch (QMessageBox::question(
            nullptr,
            QString(),
            QString("The file \"%1\" is already imported. Would you like Wizard to re-import it?").arg(aFilePath),
            QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No | QMessageBox::StandardButton::YesToAll |
               QMessageBox::StandardButton::NoToAll,
            QMessageBox::StandardButton::YesToAll))
         {
         case QMessageBox::StandardButton::Yes:
            break;
         case QMessageBox::StandardButton::No:
            return;
         case QMessageBox::StandardButton::YesToAll:
            mReimportSelection = Reimport::cYESTOALL;
            break;
         case QMessageBox::StandardButton::NoToAll:
            mReimportSelection = Reimport::cNOTOALL;
            return;
         default:
            break;
         }
      }

      mImportedFiles << aFilePath;

      // Copy file
      QFile::copy(Path(GetPath(), aFilePath), Path(aImportToDir.absolutePath(), aFilePath));

      // Import dependencies
      for (const QString& dependency : file.mDependencies)
      {
         ImportRecursionHelper(dependency, aImportToDir);
      }

      // Import additional dependencies
      for (const QString& dependency : file.mAdditionalDependencies)
      {
         ImportRecursionHelper(dependency, aImportToDir);
      }
   }
}

// ================================================================================================

bool ModelImport::Plugin::ImportOkay(const QString& aFilePath, QStringList& aCheckedFiles, bool& aIgnore)
{
   if (aIgnore)
   {
      return true;
   }

   if (aCheckedFiles.contains(aFilePath))
   {
      return true;
   }

   aCheckedFiles << aFilePath;

   const ModelImport::FileData& file = LookupFileData(Path(GetPath(), aFilePath));
   // Project was checked to be open at this function's call site in Import().
   wizard::ParseResults* results = wizard::Project::Instance()->WaitForParseResults();

   for (const ModelImport::ItemData& item : file.mDefines)
   {
      WsfParseNodeList referenceList;

      results->FindDefinitions(item.mName.toStdString(), std::string(), referenceList);

      for (const WsfParseNode* reference : referenceList)
      {
         const std::string importsDir = GetImportsFileName().left(GetImportsFileName().lastIndexOf('/')).toStdString();
         if ((reference->mValue.mSource != nullptr) &&
             (reference->mValue.mSource->GetFilePath().GetNormalizedPath().find(importsDir) == std::string::npos))
         {
            QString warningText = Qt::convertFromPlainText(
               "An item named \"%1\" already exists in your project.\nFound in project: %2\nFound in imports: %3");
            warningText = warningText.arg(item.mName);
            warningText = warningText.arg("<a href=\"file:///%1\">here</a>")
                             .arg(QString::fromStdString(reference->mValue.mSource->GetFilePath().GetNormalizedPath()));
            warningText = warningText.arg("<a href=\"file:///%1\">here</a>").arg(Path(GetPath(), file.PathToFile()));
            QMessageBox warningBox(QMessageBox::Icon::Warning, "Potential naming conflict!", warningText);

            QPushButton* ignoreButton    = warningBox.addButton("Ignore Once", QMessageBox::ButtonRole());
            QPushButton* ignoreAllButton = warningBox.addButton("Ignore All", QMessageBox::ButtonRole());
            QPushButton* cancelButton    = warningBox.addButton("Cancel Import", QMessageBox::ButtonRole());

            warningBox.exec();

            if (warningBox.clickedButton() == ignoreButton)
            {
               continue;
            }
            else if (warningBox.clickedButton() == ignoreAllButton)
            {
               aIgnore = true;
               return true;
            }
            else if (warningBox.clickedButton() == cancelButton)
            {
               return false;
            }
            else
            {
               return false;
            }
         }
      }
   }

   for (const QString& dependency : file.mDependencies)
   {
      if (!ImportOkay(dependency, aCheckedFiles, aIgnore))
      {
         return false;
      }
   }

   for (const QString& dependency : file.mAdditionalDependencies)
   {
      if (!ImportOkay(dependency, aCheckedFiles, aIgnore))
      {
         return false;
      }
   }

   return true;
}

// ================================================================================================

const ModelImport::FileData& ModelImport::Plugin::LookupFileData(const QString& aFilePath) const
{
   for (const ModelImport::FileData& file : mFileData)
   {
      if (Path(GetPath(), file.PathToFile()) == aFilePath)
      {
         return file;
      }
   }

   throw QString("The file \"%1\" does not exist.\nAborting import.").arg(aFilePath);
}
