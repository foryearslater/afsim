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

#include "RevisionStore.hpp"

#include <algorithm>
#include <string>

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QString>
#include <QStringList>
#include <QTimer>

#include "ParseResults.hpp"
#include "Project.hpp"
#include "ProjectWorkspace.hpp"
#include "ProxyWatcher.hpp"
#include "RunManager.hpp"
#include "Signals.hpp"
#include "TextSource.hpp"
#include "TextSourceCache.hpp"
#include "TextSourceSignals.hpp"
#include "UtAlgorithm.hpp"
#include "UtPath.hpp"
#include "UtQt.hpp"
#include "UtTextDocument.hpp"
namespace
{
//! The legacy/fall-back file name of the database. The database file name is
//! now based on the project name when available. If a project has a name,
//! and if we cannot find a database based on that name, then we will check
//! for this legacy/fall-back database file and rename it. If the project does
//! not have a name, then a database file will be created using this name.
const std::string FALLBACK_DB_FILE_NAME = "wizard_change_history.db";
} // namespace

//! Default constructor.
wizard::RevisionStore::RevisionStore()
   : mBackupScheduled(false)
   , mAutoBackupTimer(nullptr)
   , mProxy(nullptr)
{
   connect(&TextSourceSignals::GetInstance(), &TextSourceSignals::requestBackup, this, &RevisionStore::RequestBackup);
}

//! Sets the project root directory and name. The database will be created in
//! the project directory with a name based on the project name.
//! @param aProjectDir The project directory/root path.
//! @param aProjectFile The project file path.
void wizard::RevisionStore::SetProject(const UtPath& aProjectDir, const UtPath& aProjectFile)
{
   assert(!aProjectDir.GetSystemPath().empty());
   mProjectDir = aProjectDir;

   if (!aProjectFile.GetSystemPath().empty())
   {
      mDbDir = aProjectFile;
      mDbDir.Up();
      mDbFile = UtPath(aProjectFile.GetSystemPath() + ".changes.db");
   }
   else
   {
      mDbDir  = aProjectDir;
      mDbFile = aProjectDir + FALLBACK_DB_FILE_NAME;
   }
}

//! Sets the project working directory. This will be stored as part of the
//! revision.
void wizard::RevisionStore::SetProjectWorkingDir(const UtPath& aWorkingDir)
{
   mProjectWorkingDir = aWorkingDir;
   RequestBackup();
}

//! Opens (or creates) the database at the project root and connects to signals.
//! @returns Whether or not the database was successfully opened.
bool wizard::RevisionStore::Open()
{
   RenameFallbackDb();

   const bool success = mRevDb.Open(mDbFile.GetSystemPath());

   if (success)
   {
      assert(mRevDb.IsOpen());
      mProxy = ProxyWatcher::GetActiveProxy();
      connect(wizSignals,
              &Signals::SourceModifiedStateChange,
              this,
              static_cast<void (RevisionStore::*)(TextSource*)>(&RevisionStore::QueueFile));
      connect(wizSignals, &Signals::ProjectStartupFilesChanged, this, &RevisionStore::HandleProjectStartupFilesChanged);
      mCallbacks += ProxyWatcher::ProxyAvailable.Connect(&RevisionStore::HandleProxyAvailable, this);
      mCallbacks += Project::Instance()->ParseUpdatedDeferred.Connect(&RevisionStore::HandleParseUpdated, this);
   }

   return success;
}

//! Opens the revision database in memory. Useful for testing.
//! @returns Whether or not the database was successfully opened.
bool wizard::RevisionStore::OpenInMemory()
{
   return mRevDb.OpenInMemory();
}

//! Closes the connection to the revision database.
//! @returns Whether or not the database was successfully closed.
bool wizard::RevisionStore::Close()
{
   const bool success = mRevDb.Close();

   if (success)
   {
      assert(!mRevDb.IsOpen());
      mCallbacks.Clear();
   }

   return success;
}

//! @returns Whether or not the revision database is currently empty.
bool wizard::RevisionStore::IsEmpty()
{
   return mRevDb.IsEmpty();
}

//! @returns Whether or not the revision/backup database is currently open.
bool wizard::RevisionStore::IsOpen() const
{
   return mRevDb.IsOpen();
}

//! @returns Whether or not the database file for the revision store exists on
//! the file system.
bool wizard::RevisionStore::Exists() const
{
   return (mDbFile.Stat() == UtPath::cFILE);
}

//! Sets the revision store to automatically backup every X minutes.
//! @param aMin How often (in minutes) an automatic backup should be created.
void wizard::RevisionStore::EnableAutoBackup(int aMin)
{
   if (mAutoBackupTimer)
   {
      DisableAutoBackup();
   }

   const int msec   = aMin * 60 * 1000;
   mAutoBackupTimer = new QTimer(this);
   connect(mAutoBackupTimer, &QTimer::timeout, this, &RevisionStore::RequestBackup);
   mAutoBackupTimer->start(msec);
}

//! Disables the automatic backup feature.
void wizard::RevisionStore::DisableAutoBackup()
{
   if (mAutoBackupTimer)
   {
      mAutoBackupTimer->stop();
      delete mAutoBackupTimer;
      mAutoBackupTimer = nullptr;
   }
}

//! Requests a backup. If the proxy is available and the Wizard is not currently
//! parsing when this method is called, a backup will be created immediately.
//! Otherwise, a flag will be set and another backup will be attempted once
//! parsing is finished and/or the proxy is available.
//! @returns Whether or not a backup has been scheduled or, if the proxy was
//! available and up to date (i.e. the Wizard was not parsing), whether or not the
//! backup was successful.
bool wizard::RevisionStore::RequestBackup()
{
   bool success     = true;
   mBackupScheduled = true;

   if (mRevDb.IsOpen())
   {
      if (ProxyUpToDate() && !mProjectStartupFiles.empty())
      {
         success          = AnalyzeAndBackup();
         mBackupScheduled = !success;
      }
   }
   else
   {
      success          = false;
      mBackupScheduled = false;
   }

   return success;
}

//! Blocks until parsing ends and then immediately attempts to create a backup.
//! This will clear the "backup scheduled" flag if successful.
//! @returns Whether or not the backup was successful.
bool wizard::RevisionStore::Backup()
{
   bool saved = false;

   if (mRevDb.IsOpen())
   {
      Project::Instance()->WaitForAllParsing();
      // assert(ProxyUpToDate());

      if (ProxyUpToDate())
      {
         saved            = AnalyzeAndBackup();
         mBackupScheduled = mBackupScheduled && !saved;
      }
   }

   return saved;
}

//! Queues all included files in the project to be analyzed for changes during
//! the next backup. This is useful upon a project first being opened (and
//! receiving the first available proxy), so that all the files can be
//! scanned for changes in the case that they were modified outside the Wizard
//! since the last time the project was opened.
void wizard::RevisionStore::QueueAllProjectFiles()
{
   Project*                          proj    = Project::Instance();
   const TextSourceCache::SourceMap& sources = proj->GetSourceCache().GetSources();
   for (auto&& sourcePair : sources)
   {
      TextSource* source = sourcePair.second;
      if (source->GetFlags() & TextSource::cINCLUDED)
      {
         QueueFile(source);
      }
   }
}

//! Receives the signal that is sent when the proxy becomes available or
//! unavailable. A backup is performed if one has been scheduled.
//! @param aProxy A valid proxy or NULL.
void wizard::RevisionStore::HandleProxyAvailable(WsfPProxy* aProxy)
{
   if (!mProxy)
   {
      QueueAllProjectFiles();
   }

   mProxy = aProxy;

   if (mRevDb.IsOpen() && mProxy && mBackupScheduled && AnalyzeAndBackup())
   {
      mBackupScheduled = false;
   }
}

//! Receives the signal that is sent when parsing has finished. A backup is
//! performed if one has been scheduled.
//! @param aParsed Whether or not parsing data is up to date.
void wizard::RevisionStore::HandleParseUpdated(bool aParsed)
{
   if (aParsed && mRevDb.IsOpen() && mProxy && mBackupScheduled && AnalyzeAndBackup())
   {
      mBackupScheduled = false;
   }
}

//! Sets the project startup files. These will be associated with the current
//! revision.
void wizard::RevisionStore::HandleProjectStartupFilesChanged(const std::vector<UtPath>& aStartupFiles)
{
   mProjectStartupFiles = aStartupFiles;
   RequestBackup();
}

//! @returns Whether or not the proxy is currently up to date. That is, we have
//! a valid proxy and we are not in the middle of parsing.
bool wizard::RevisionStore::ProxyUpToDate() const
{
   return (mProxy && !ProjectWorkspace::Instance()->IsProcessing());
}

//! Adds a file to the queue. This should be done if the file does not exist in
//! the database or if its content has changed since the last revision.
//! @param aDocPtr The document object of the file to be queued.
void wizard::RevisionStore::QueueFile(const UtTextDocument* aDocPtr)
{
   const std::string& relDocPath = RelativeDocPath(*aDocPtr);
   AddToQueue(relDocPath, aDocPtr);
}

//! Slot used for connecting to "file modified" signals. Any file that's
//! modified gets added to the queue.
//! @param aDocPtr The document object of a file that has been modified.
void wizard::RevisionStore::QueueFile(TextSource* aDocPtr)
{
   QueueFile(static_cast<UtTextDocument*>(aDocPtr));
}

//! Creates a revision at the current point and then attempts to restore the
//! user's files to the given revision number.
//! @warning This function modifies the user's in-memory file content.
//! @param aRevNo The revision number to restore the user's files to.
//! @param aFailedFiles Reference to a string list to hold the paths of files
//! that failed to be restored, potentially due to lack of 'write' file permission.
//! @param aRequestingWidget The widget initiating the restore request.
//! @returns Whether or not all files were successfully restored.
bool wizard::RevisionStore::Restore(int aRevNo, QStringList& aFailedFiles, QWidget* aRequestingWidget)
{
   bool success = false;

   if (mRevDb.IsOpen())
   {
      const RevisionPathSet& revPaths = mRevDb.ExistingFilePaths(aRevNo);
      assert(!revPaths.empty());

      if (!revPaths.empty())
      {
         bool allRestored = true;

         Backup();

         for (auto&& path : revPaths)
         {
            const bool fileRestored = RestoreFile(path, aRevNo, aRequestingWidget);

            if (!fileRestored)
            {
               aFailedFiles << QString::fromStdString(path);
            }

            allRestored = fileRestored && allRestored;
         }

         success = allRestored;
         Project::Instance()->TriggerReparse();
      }

      std::vector<std::string> startupFiles;
      if (mRevDb.StartupFiles(aRevNo, startupFiles))
      {
         std::vector<UtPath> utPathStartupFiles;

         for (std::vector<std::string>::const_iterator i = startupFiles.begin(); i != startupFiles.end(); ++i)
         {
            const UtPath& startupFilePath = mDbDir + *i;
            utPathStartupFiles.push_back(startupFilePath);
         }
         // if the first start-up file changed, set the working directory to the its directory
         if (wizRunMgr.SetWorkingDirectoryToProject(utPathStartupFiles.front()))
         {
            Project::Instance()->SetStartupFiles(utPathStartupFiles);
         }
      }

      std::string workingDir;
      if (mRevDb.WorkingDirectory(aRevNo, workingDir))
      {
         Project::Instance()->SetWorkingDirectory(UtPath(workingDir));
      }
   }

   return success;
}

//! Deletes all revisions (and associated data) in the database.
//! @returns Whether or not the database was open and the data was succesfully
//! cleared.
bool wizard::RevisionStore::DeleteAllRevisions()
{
   const bool success = mRevDb.IsOpen() && mRevDb.ClearTables();

   if (success)
   {
      mBackupScheduled = false;
   }

   return success;
}

//! @returns The data for all available revisions in the database.
wizard::RevisionList wizard::RevisionStore::Revisions()
{
   return mRevDb.IsOpen() ? mRevDb.Revisions() : RevisionList();
}

//! @returns The revision database.
wizard::RevisionDb& wizard::RevisionStore::DB()
{
   return mRevDb;
}

//! Saves a new revision in the database. This is the public-facing interface
//! for creating revisions. The database inserts are batched in a single
//! transaction to improve performance. (It's really slow and will freeze
//! the UI for at least several seconds if you don't.) It also ensures that a
//! revision and all of its associated changes are inserted atomically.
//! @returns Whether or not a new backup was created. A new backup will not be
//! created if there have been no changes since the last backup.
bool wizard::RevisionStore::AnalyzeAndBackup()
{
   RevisionPathSet removedFiles;
   AnalyzeAddedRemovedFiles(removedFiles);

   std::string workingDir = mDbDir.GetRelativePath(mProjectWorkingDir).GetSystemPath();
   if (workingDir.empty())
   {
      workingDir = ".";
   }

   std::vector<std::string> startupFiles;
   for (auto&& startupFile : mProjectStartupFiles)
   {
      startupFiles.push_back(mDbDir.GetRelativePath(startupFile).GetSystemPath());
   }

   const RevisionChangeList& changes = GenerateFileChanges(removedFiles);
   const bool                success = StoreNewRevision(changes, workingDir, startupFiles);

   if (success)
   {
      mQueuedFiles.clear();
   }

   return success;
}

//! Adds a file to the file queue.
//! @param aRelFilePath The relative path of the file.
//! @param aDocPtr A pointer to the document object representing the file.
void wizard::RevisionStore::AddToQueue(const std::string& aRelFilePath, const UtTextDocument* aDocPtr)
{
   assert(aDocPtr);
   mQueuedFiles[aRelFilePath] = aDocPtr;
}

//! @returns The path of a given file relative to the project root.
//! @param aDoc The file/doc to get the relative path of.
std::string wizard::RevisionStore::RelativeDocPath(const UtTextDocument& aDoc) const
{
   return mDbDir.GetRelativePath(aDoc.GetFilePath()).GetSystemPath();
}

//! Compares the current state of a given file with the state of the file at
//! the given revision number and generates a change to represent the difference.
//! @param aDoc The file/document to compare and generate the change for.
//! @returns The aforementioned change.
wizard::RevisionChange wizard::RevisionStore::AnalyzeFile(const UtTextDocument& aDoc)
{
   RevisionChange change;

   const int cMAX_SEQUENTIAL_CHANGESETS = 100;

   const int         revNo      = mRevDb.LatestRevisionNo();
   const std::string relDocPath = RelativeDocPath(aDoc);
   UtTextDocument    oldRevision;
   int               revisionsTraversed;

   if ((revNo < 1) || !mRevDb.GetFileAtRevision(relDocPath, revNo, oldRevision, &revisionsTraversed))
   {
      change = RevisionChange(RevisionChange::cNEW_FILE, relDocPath, aDoc);
   }
   else if (revisionsTraversed > cMAX_SEQUENTIAL_CHANGESETS)
   {
      change = RevisionChange(RevisionChange::cCHANGE_FULL, relDocPath, aDoc);
   }
   else
   {
      QVector<TextSourceChange> changes;
      TextSource::DiffDocuments(oldRevision.GetPointer(), aDoc.GetPointer(), changes);
      if (!changes.isEmpty())
      {
         change = RevisionChange(RevisionChange::cCHANGE_DELTA, relDocPath, changes);
      }
      else
      {
         change = RevisionChange(RevisionChange::cNO_CHANGE, relDocPath);
      }
   }

   return change;
}

//! Generates a list of changes for all queued files and removed files since the
//! last revision. Queued files will included newly added files.
//! @param aRemovedPaths A set of relative paths to files that have been removed
//! since the last revision.
//! @returns A list of changes since the last revision.
wizard::RevisionChangeList wizard::RevisionStore::GenerateFileChanges(const RevisionPathSet& aRemovedPaths)
{
   RevisionChangeList changes;

   for (auto&& qf : mQueuedFiles)
   {
      const UtTextDocument* doc    = qf.second;
      RevisionChange        change = AnalyzeFile(*doc);
      if (change.kind != RevisionChange::cNO_CHANGE)
      {
         changes.push_back(change);
      }
   }

   for (auto&& path : aRemovedPaths)
   {
      changes.push_back(RevisionChange(RevisionChange::cREMOVED_FILE, path));
   }

   return changes;
}

//! Checks for files that have been added or removed since the last revision.
//! Files that have been added will simply be added to the list of queued files.
//! Files that have been removed will be removed from the list of queued files.
//! @param aRemovedPaths A reference to a set that should hold a list of relative
//! paths of files that have been deleted since the last revision.
//! @returns Whether or not any files have been added or removed since the last
//! revision.
bool wizard::RevisionStore::AnalyzeAddedRemovedFiles(RevisionPathSet& aRemovedPaths)
{
   bool                   addedFilesQueued = false;
   const int              revNo            = mRevDb.LatestRevisionNo();
   const RevisionPathSet& dbPaths          = mRevDb.ExistingFilePaths(revNo);
   RevisionPathSet        filePaths;

   const ParseIncludes& projFiles = Project::Instance()->WaitForParseResults()->mFirstIncludes;
   for (auto&& pf : projFiles)
   {
      const UtTextDocument* doc        = pf.first;
      const std::string&    relDocPath = RelativeDocPath(*doc);
      filePaths.insert(relDocPath);
      if (dbPaths.find(relDocPath) == dbPaths.end())
      {
         AddToQueue(relDocPath, doc);
         addedFilesQueued = true;
      }
   }

   // Check for files that are in the database but not in the current set of
   // project files. These are the paths of files that have been removed from
   // the project since the last saved revision.
   std::set_difference(dbPaths.begin(),
                       dbPaths.end(),
                       filePaths.begin(),
                       filePaths.end(),
                       std::insert_iterator<RevisionPathSet>(aRemovedPaths, aRemovedPaths.begin()));

   // Remove deleted files from the list of queued files.
   for (auto&& path : aRemovedPaths)
   {
      mQueuedFiles.erase(path);
   }

   return (addedFilesQueued || !aRemovedPaths.empty());
}

//! Stores a new revision in the database if there are queued files with changes
//! or if files have been added or removed from the project.
//! @param aChanges The list of changes that make up the revision.
//! @param aWorkingDir The name of the working directory.
//! @param aStartupFiles A container of the start-up file names.
//! @returns Whether or not a new revision was successfully stored.
bool wizard::RevisionStore::StoreNewRevision(const RevisionChangeList&       aChanges,
                                             const std::string&              aWorkingDir,
                                             const std::vector<std::string>& aStartupFiles)
{
   const int newRevNo = mRevDb.StoreNewRevision(aChanges, aWorkingDir, aStartupFiles);
   return (newRevNo > 0);
}

//! The database file name is now based on the project name. If we cannot find a
//! database based on the project name, then we will check for a legacy/fallback
//! database file and rename it.
//! @returns Whether or not a legacy/fallback database was found and renamed.
bool wizard::RevisionStore::RenameFallbackDb()
{
   bool fallbackDbFileRenamed = false;

   if (mDbFile.Stat() == UtPath::cSTAT_ERROR)
   {
      const UtPath& fallbackDbFilePath = mProjectDir + FALLBACK_DB_FILE_NAME;
      if (fallbackDbFilePath.Stat() == UtPath::cFILE)
      {
         QFile fallbackDbFile(QString::fromStdString(fallbackDbFilePath.GetSystemPath()));
         fallbackDbFileRenamed = fallbackDbFile.rename(QString::fromStdString(mDbFile.GetSystemPath()));
         // assert(fallbackDbFileRenamed);
      }
   }

   return fallbackDbFileRenamed;
}

//! Replaces the content of a file with new content.
//! @param aFilePath The absolute path of the file to modify.
//! @param aFileContent The data/string to write to the file.
//! @param aFileContentSize The size of the data/string.
bool wizard::RevisionStore::ReplaceFile(const UtPath& aFilePath, const char* aFileContent, size_t aFileContentSize)
{
   TextSourceCache* srcCache = ProjectWorkspace::Instance()->GetSourceCache();
   TextSource*      doc      = srcCache->GetSource(aFilePath);

   if (doc)
   {
      doc->Insert(0, aFileContent, aFileContentSize);
      doc->ReloadViews();
   }

   return (doc != nullptr);
}

//! Creates a new file on the file system and writes data to it.
//! @param aFilePath The absolute path of the new file to create.
//! @param aFileContent The data/string to write to the file.
//! @returns Whether or not the file was written without error.
bool wizard::RevisionStore::MakeNewFile(const UtPath& aFilePath, const char* aFileContent)
{
   QFile      newFile(QString::fromStdString(aFilePath.GetSystemPath().c_str()));
   const bool success = newFile.open(QIODevice::WriteOnly);

   if (success)
   {
      QTextStream stream(&newFile);
      stream << aFileContent;
      newFile.close();
   }

   return success;
}

//! Attempts to restore the file contents of a file currently included in the
//! project.
//! @param aFilePath The path of the file to restore.
//! @param aFileContent The content to place into the file.
//! @returns Whether or not the file was found in the included files and restored.
bool wizard::RevisionStore::RestoreIncludedFile(const std::string& aFilePath, const UtTextDocument& aFileContent)
{
   bool restored = false;

   ParseIncludes& projFiles = Project::Instance()->WaitForParseResults()->mFirstIncludes;
   for (auto&& pf : projFiles)
   {
      TextSource* doc = static_cast<TextSource*>(pf.first);
      if (RelativeDocPath(*doc) == aFilePath)
      {
         doc->Clear();
         doc->Insert(0, aFileContent.GetPointer(), aFileContent.Size());
         doc->ReloadViews();
         restored = true;
         break;
      }
   }

   return restored;
}

//! Attempts to load a file from the file system into the included project files
//! and then restore its contents.
//! @param aFilePath The path of the file to restore.
//! @param aFileContent The content to place into the file.
//! @returns Whether or not the file was loaded and restored successfully.
bool wizard::RevisionStore::RestoreUnincludedFile(const std::string&    aFilePath,
                                                  const UtTextDocument& aFileContent,
                                                  QWidget*              aRequestingWidget)
{
   bool                   restored    = false;
   const UtPath           absFilePath = mDbDir + aFilePath;
   const UtPath::StatType stat        = absFilePath.Stat();

   if (stat == UtPath::cFILE)
   {
      restored = ReplaceFile(absFilePath, aFileContent.GetPointer(), aFileContent.Size());
   }
   else if (stat == UtPath::cSTAT_ERROR)
   {
      restored = MakeNewFile(absFilePath, aFileContent.GetPointer());
   }
   else
   {
      const QString qtFilePath = QString::fromStdString(aFilePath);
      const QString timestamp  = QString::number(QDateTime::currentDateTime().toTime_t());
      const QString newPath    = qtFilePath + "-backup-" + timestamp;

      UtQtShowSimpleMsg("Name collision",
                        "The directory " + qtFilePath + " conflicts with a restored file of the same name.",
                        "The directory will be renamed to " + newPath,
                        aRequestingWidget);

      const QString oldAbsPath = QString::fromStdString(absFilePath.GetSystemPath());
      const QString newAbsPath = QString::fromStdString((mDbDir + newPath.toStdString()).GetSystemPath());

      if (QDir().rename(oldAbsPath, newAbsPath))
      {
         restored = MakeNewFile(absFilePath, aFileContent.GetPointer());
      }
      else
      {
         UtQtShowSimpleMsg("Directory rename failure",
                           "Failed to rename directory " + qtFilePath + " to " + newPath,
                           "Try manually moving or renaming the directory and then performing the restore again.",
                           aRequestingWidget);
      }
   }

   return restored;
}

//! Restores a file to its state at a given revision. If the file has no data
//! for the given revision, it will not be modified and the function will return
//! failure.
//! @param aPath The absolute path of the file to restore.
//! @param aRevNo The revision number to restore the file to.
//! @param aRequestingWidget The widget initiating the restore request.
//! @returns Whether or not the file exists at the given revision number and was
//! successfully restored.
bool wizard::RevisionStore::RestoreFile(const std::string& aPath, int aRevNo, QWidget* aRequestingWidget)
{
   bool           restored = false;
   UtTextDocument fileContent;
   const bool     foundInDb = mRevDb.GetFileAtRevision(aPath, aRevNo, fileContent);
   assert(foundInDb);

   if (foundInDb)
   {
      restored = RestoreIncludedFile(aPath, fileContent);
      if (!restored)
      {
         restored = RestoreUnincludedFile(aPath, fileContent, aRequestingWidget);
      }
   }

   return restored;
}
