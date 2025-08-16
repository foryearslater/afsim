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
#ifndef MODELIMPORT_PLUGIN_HPP
#define MODELIMPORT_PLUGIN_HPP

#include <QDateTime>
#include <QPointer>
#include <QProgressDialog>
#include <QStringList>

#include "ModelImportDirectoryReaderThread.hpp"
#include "ModelImportDockWidget.hpp"
#include "ModelImportPrefObject.hpp"
#include "ModelImportPrefWidget.hpp"
#include "ModelImportUtility.hpp"
#include "Plugin.hpp"
#include "Project.hpp"

namespace ModelImport
{
enum class Reimport
{
   cUNDECLARED,
   cYES,
   cNO,
   cYESTOALL,
   cNOTOALL
};

class Plugin : public wizard::Plugin
{
   Q_OBJECT
public:
   Plugin(const QString& aPluginName, const size_t aUniqueId);
   ~Plugin() noexcept override = default;

   //! Provides a constant reference to the preferences object.
   const ModelImport::PrefObject& GetPrefObject() const;

   // Needed for adding the widget to the Preferences menu.
   virtual QList<wkf::PrefWidget*> GetPreferencesWidgets() const override;

   //! Returns the search path from the PrefObject.
   const QString& GetPath() const noexcept;

   //! Returns the meta file name from the PrefObject.
   const QString& GetMetaFileName() const noexcept;

   //! Returns the imports file name from the PrefObject.
   const QString& GetImportsFileName() const noexcept;

   //! Returns the errors file name from the PrefObject.
   const QString& GetErrorsFileName() const noexcept;

   //! Returns the display mode from the PrefObject.
   const QString& GetDisplayMode() const noexcept;

   //! Returns the path to the current open project.
   std::string GetProjectPath() const noexcept;

private:
   PluginUiPointer<DockWidget>      mDockWidget;
   PluginUiPointer<PrefWidget>      mPrefWidgetPtr;
   ParseData                        mFileData;
   ParseData                        mOldData;
   PluginUiPointer<QProgressDialog> mProgress;
   DirectoryReaderThread            mReadingThread;
   Reimport                         mReimportSelection;
   QStringList              mImportedFiles;  // Contains a list of files being imported to avoid asking for re-import.
   std::vector<std::string> mNamesInProject; // Contains a list of names in the Project that are not imported.
   bool                     mValidImport = true;

   //! Updates the display in the dock widget.
   void Display();

   //! Reads the meta object file. Results are stored in mFileData.
   void ReadJson();

   //! Parses the directory. Results are stored in mFileData.
   //! @note The actual parsing takes place on a separate thread. The calling thread creates a loading bar that blocks
   //! the GUI until parsing finishes.
   void ReadDirectory();

   //! Handles the finishing steps for parsing the directory.
   void ReadDirectoryComplete();

   //! Writes the contents of mFileData to the meta object file.
   void WriteJson();

   //! Prepares the Plugin to merge parse results with previous data. Parses the directory.
   void UpdateMerge();

   //! Prepares the Plugin to overwrite previous data. Parses the directory.
   void UpdateOverwrite();

   //! Merges two sets of file data.
   //! @param aOldData This is the previous data to merge from.
   //! @param aNewData This is the new data to merge into.
   //! @note The old data is mutable because it will be sorted to improve runtime from O(n^2) to O(n*log n).
   static void MergeFileData(ModelImport::ParseData& aOldData, ModelImport::ParseData& aNewData);

   //! Imports a file and its dependencies.
   //! @param aPathToFile This is the path to the file to be imported. Relative to Model Directory.
   void Import(const QString& aPathToFile);

   //! This is used to recursively import files and all their dependencies.
   //! @param aFilePath The path to the file to import. Relative to Model Directory.
   //! @param aImportToDir The directory to import to.
   void ImportRecursionHelper(const QString& aFilePath, const QDir& aImportToDir);

   //! Checks imports directory for missing dependencies and name collision with existing entities.
   //! @param aFilePath This is the path to the root file to be checked. Relative to Model Directory.
   //! @param aCheckedFiles These are the files that have already been checked. This argument is used to escape include loops.
   //! @param aIgnore If this is true, function returns true immediately.
   //! @returns True if the file is okay to import, False otherwise.
   bool ImportOkay(const QString& aFilePath, QStringList& aCheckedFiles, bool& aIgnore);

   //! Finds a ModelImport::FileInfo object based on its file path.
   //! @param aFilePath This is the path to the file to import.
   //! @throws QString If file is not listed in mFileData.
   const ModelImport::FileData& LookupFileData(const QString& aFilePath) const;
};
} // namespace ModelImport
#endif
