// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2003-2015 The Boeing Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "ExportFiles.hpp"

#include <map>

#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include <QUrl>

#include "RevisionStore.hpp"
#include "UtQt.hpp"

//! Initializing constructor.
//! @param aProjectDir The root directory of the project.
//! @param aRequestingWidget The widget requesting the export. This will be set
//! as the parent widget of the file dialog where the user selects an export
//! directory.
wizard::ExportFiles::ExportFiles(const UtPath& aProjectDir, QWidget* aRequestingWidget)
   : mProjectDir(aProjectDir)
   , mRequestingWidget(aRequestingWidget)
{
}

//! Exports the given project includes. Presumably the includes represent the
//! current state of the open project. The includes are used directly. That is,
//! the file contents are not fetched off the disk. Unsaved changes will
//! therefore potentially be exported.
//! @param aIncludes The files included in the project.
//! @returns Whether or not the export was successful.
bool wizard::ExportFiles::ExportProject(const ParseIncludes& aIncludes)
{
   bool           exportSuccessful = false;
   const QString& exportDirPath    = PromptForExportDir();

   if (!exportDirPath.isEmpty())
   {
      FileMap files;
      for (auto&& inc : aIncludes)
      {
         files[inc.first->GetFilePath().GetSystemPath()] = *(inc.first);
      }
      exportSuccessful = Export(exportDirPath, files);
   }

   return exportSuccessful;
}

//! Exports the project files as it existed at the given revision number.
//! @param aRevStore The project revision store.
//! @param aRevNo The revision number to export.
//! @returns Whether or not the export was successful.
bool wizard::ExportFiles::ExportRevision(RevisionStore& aRevStore, int aRevNo)
{
   bool           exportSuccessful = false;
   const QString& exportDirPath    = PromptForExportDir();

   if (!exportDirPath.isEmpty())
   {
      const bool dbIsOpen = aRevStore.IsOpen();

      if (!dbIsOpen)
      {
         aRevStore.Open();
      }

      const FileMap& files = aRevStore.DB().ExistingFiles(aRevNo);
      exportSuccessful     = Export(exportDirPath, files);

      if (!dbIsOpen)
      {
         aRevStore.Close();
      }
   }

   return exportSuccessful;
}

//! Given an export directory path and a file path (relative unless on another
//! drive), return the path that the file should be exported to.
//! @param aExportDirPath The directory to export files to.
//! @param aFilePath The file path to compute an export file path for.
//! @returns The export path for the given file path.
QString wizard::ExportFiles::GetExportPath(const QString& aExportDirPath, const QString& aFilePath) const
{
   UtPath       exportPath(aExportDirPath.toStdString());
   const UtPath filePath = aFilePath.toStdString();

   std::string              prefix;
   std::vector<std::string> toks;
   filePath.Tokenize(prefix, toks);

   if (prefix.empty())
   {
      exportPath += filePath;
   }
   else
   {
      exportPath += "_external_items";

      // Handle absolute paths. On Windows, we want to preserve the drive letter
      // in the export path.
      if (!prefix.empty() && (prefix != "/") && (prefix != "//"))
      {
         std::string driveLetter;
         driveLetter += prefix.at(0);
         exportPath += driveLetter;
      }

      for (auto&& tok : toks)
      {
         exportPath += tok;
      }
   }

   return QString::fromStdString(exportPath.GetSystemPath());
}

//! Prompts the user to choose a directory to export files to.
//! @returns The chosen directory path.
QString wizard::ExportFiles::PromptForExportDir() const
{
   return QDir::toNativeSeparators(QFileDialog::getExistingDirectory(mRequestingWidget, "Select Export Directory"));
}

//! Prompts the user about whether or not to overwrite a file that already
//! exists when doing an export.
//! @param aFilePath The path of the file to prompt about.
//! @returns The button selected by the user.
QMessageBox::StandardButton wizard::ExportFiles::PromptForOverwrite(const QString& aFilePath) const
{
   QMessageBox msg(mRequestingWidget);

   msg.setWindowTitle("File Already Exists");
   msg.setText(QString("The file %1 already exists. Would you like to overwrite it?").arg(aFilePath));
   msg.addButton(QMessageBox::Abort);
   msg.addButton(QMessageBox::Yes);
   msg.addButton(QMessageBox::YesToAll);
   msg.addButton(QMessageBox::No);
   msg.addButton(QMessageBox::NoToAll);
   msg.setEscapeButton(QMessageBox::No);
   msg.setDefaultButton(QMessageBox::No);

   return QMessageBox::StandardButton(msg.exec());
}

//! This will adjust the project export directory to account for any external
//! items that have been included in the project, i.e. files that exist outside
//! the project directory and its subdirectories, so long as they exist on the
//! same drive (on Windows - Linux doesn't have this problem).
//! @param aExportDirPath The original project export directory chosen by the user.
//! @param aFileMap The files to export; a map from file paths to file contents.
//! @returns The export directory adjusted for relative external item locations.
QString wizard::ExportFiles::AdjustExportDirPath(const QString& aExportDirPath, const FileMap& aFileMap)
{
   UtPath startDir = mProjectDir;
   int    levelsUp = 0;

   std::string              projPrefix;
   std::vector<std::string> projToks;
   mProjectDir.Tokenize(projPrefix, projToks);

   for (auto&& i : aFileMap)
   {
      const UtPath relFilePath = mProjectDir.GetRelativePath(UtPath(i.first));
      UtPath       filePath    = mProjectDir + relFilePath;

      if (!mProjectDir.IsAncestorOf(filePath))
      {
         filePath = mProjectDir + i.first;
      }

      std::string              prefix;
      std::vector<std::string> toks;
      filePath.Tokenize(prefix, toks);

      // If the prefix of the file does not match the prefix of the project
      // directory, then it is on another drive; e.g., the project directory is
      // on C:\ and the included file is on D:\. Such files will go into the
      // special _external_items directory, and we don't want to use them in
      // determining the overall export directory structure for all the other
      // files. That is enforced here.
      if (prefix.empty() || (prefix == projPrefix))
      {
         const UtPath startDirSave = startDir;
         const int    levelsUpSave = levelsUp;
         bool         pathIsEmpty;

         while (!(pathIsEmpty = startDir.GetSystemPath().empty()) && !startDir.IsAncestorOf(filePath))
         {
            startDir.Up();
            levelsUp++;
         }

         if (pathIsEmpty)
         {
            startDir = startDirSave;
            levelsUp = levelsUpSave;
         }
      }
   }

   UtPath adjustedExportDirPath;
   for (int i = 0; i < levelsUp; i++)
   {
      adjustedExportDirPath = UtPath(projToks.back()) + adjustedExportDirPath;
      projToks.erase(projToks.end() - 1);
   }

   const UtPath origExportDirPath(aExportDirPath.toStdString());
   adjustedExportDirPath = origExportDirPath + adjustedExportDirPath;
   assert((adjustedExportDirPath == origExportDirPath) || origExportDirPath.IsAncestorOf(adjustedExportDirPath));

   const QString adjustedExportDirPathStr = QString::fromStdString(adjustedExportDirPath.GetSystemPath());
   assert(adjustedExportDirPathStr.startsWith(aExportDirPath));

   return adjustedExportDirPathStr;
}

//! Exports the given files to the specified directory. See documentation of the
//! class for details.
//! @param aExportDirPath The directory to export files to.
//! @param aFileMap The files to export; a map from file paths to file contents.
//! @returns Whether or not the export was successful.
bool wizard::ExportFiles::Export(const QString& aExportDirPath, const FileMap& aFileMap)
{
   QStringList badFiles;

   if (!aExportDirPath.isEmpty())
   {
      const QString&              adjustedExportDirPath = AdjustExportDirPath(aExportDirPath, aFileMap);
      QMessageBox::StandardButton userChoice            = QMessageBox::NoButton;

      for (auto&& i : aFileMap)
      {
         UtPath relFilePath = mProjectDir.GetRelativePath(UtPath(i.first));
         UtPath filePath    = mProjectDir + relFilePath;

         if (!mProjectDir.IsAncestorOf(filePath))
         {
            filePath    = mProjectDir + i.first;
            relFilePath = mProjectDir.GetRelativePath(filePath);
         }

         const QString&        filePathStr = QString::fromStdString(relFilePath.GetSystemPath());
         const UtTextDocument& fileContent = i.second;

         const QString& fileExportPath = GetExportPath(adjustedExportDirPath, filePathStr);
         assert(fileExportPath.startsWith(aExportDirPath));

         QStringList fileExportPathLs = fileExportPath.split(QDir::separator());
         fileExportPathLs.removeLast();

         const QString& destDirPath = fileExportPathLs.join(QDir::separator());
         const QDir     destDir(destDirPath);
         destDir.mkpath(destDir.path());

         QFile file(fileExportPath);

         if (file.exists() && (userChoice != QMessageBox::YesToAll) && (userChoice != QMessageBox::NoToAll))
         {
            userChoice = PromptForOverwrite(fileExportPath);
            if (userChoice == QMessageBox::Abort)
            {
               break;
            }
         }

         if (!file.exists() || (file.exists() && (userChoice != QMessageBox::No) && (userChoice != QMessageBox::NoToAll)))
         {
            if (file.open(QIODevice::WriteOnly))
            {
               QTextStream stream(&file);
               stream << fileContent.GetPointer();
               file.close();
            }
            else
            {
               badFiles << fileExportPath;
            }
         }
      }

      if (!badFiles.empty())
      {
         UtQtShowSimpleMsg("Failed to export",
                           QString("Failed to export the following files:\n%1").arg(badFiles.join("\n")),
                           "Please ensure you have appropriate read/write file permissions and try again.",
                           mRequestingWidget);
      }

      OpenDir(aExportDirPath);
   }

   return (!aExportDirPath.isEmpty() && !aFileMap.empty() && badFiles.empty());
}

//! Opens the given directory in the system file browser.
//! @param aExportDirPath The path to the directory to open.
void wizard::ExportFiles::OpenDir(const QString& aExportDirPath) const
{
   const QUrl pathUrl("file:///" + aExportDirPath);
   QDesktopServices::openUrl(pathUrl);
}
