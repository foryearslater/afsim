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

#include "RevisionDb.hpp"

#include <iostream>

#include <QByteArray>
#include <QDataStream>

#include "TextSourceChange.hpp"
#include "sqlite3.h"

namespace
{
//! The current version of the database. Increment when changing the database
//! tables or other behavior in such a way that backwards compatibility would
//! be lost.
const int DB_VERSION = 2;

// Initial table creation and setup statements. Foreign key constraints must
// be explicitly enabled with a SQL PRAGMA statement in SQLite.
// -- http://www.sqlite.org/foreignkeys.html#fk_enable
// -- http://www.sqlite.org/pragma.html#pragma_foreign_keys
const char* SQL_CREATE_TABLES =
   "CREATE TABLE IF NOT EXISTS"
   "   revision(revNo INTEGER PRIMARY KEY AUTOINCREMENT,"
   "            parentRevNo INTEGER REFERENCES revision(revNo),"
   "            workingDir TEXT,"
   "            timestamp INTEGER);"

   "CREATE TABLE IF NOT EXISTS"
   "   startupFile(idx INTEGER PRIMARY KEY AUTOINCREMENT,"
   "               revNo INTEGER,"
   "               filePath TEXT,"
   "               UNIQUE(revNo, filePath),"
   "               FOREIGN KEY(revNo) REFERENCES revision(revNo) ON DELETE CASCADE ON UPDATE CASCADE);"

   "CREATE TABLE IF NOT EXISTS"
   "   change(chIdx INTEGER PRIMARY KEY AUTOINCREMENT,"
   "          kind INTEGER,"
   "          revNo INTEGER,"
   "          filePath TEXT,"
   "          data BLOB,"
   "          FOREIGN KEY(revNo) REFERENCES revision(revNo) ON DELETE CASCADE ON UPDATE CASCADE);"

   "CREATE TABLE IF NOT EXISTS"
   "   metadata(key TEXT PRIMARY KEY,"
   "            value BLOB);"

   "PRAGMA foreign_keys = ON;";

// Update the database from version 1 to version 2.
const char* SQL_UPDATE_V1_TO_V2 = "ALTER TABLE revision ADD COLUMN workingDir TEXT;";

// Drops (deletes) the tables from the database.
const char* SQL_CLEAR_TABLES = "DELETE FROM revision;"
                               "DELETE FROM change;";

// INSERT a change.
const char* SQL_INSERT_CHANGE = "INSERT INTO change(kind,revNo,filePath,data) VALUES (?,?,?,?);";

// INSERT a revision.
const char* SQL_INSERT_REVISION = "INSERT INTO revision(timestamp,workingDir) VALUES (strftime('%s', 'now'),?);";

// INSERT a startup file for a revision.
const char* SQL_INSERT_STARTUP_FILE = "INSERT INTO startupFile(revNo,filePath) VALUES (?,?);";

// INSERT a key-value pair into the metadata table.
const char* SQL_INSERT_METADATA = "INSERT OR REPLACE INTO metadata(key, value) VALUES (?, ?);";

// DELETE a revision.
const char* SQL_DELETE_REVISION = "DELETE FROM revision WHERE (revNo = ?);";

// SELECT all revisions, ordered from oldest to newest.
const char* SQL_SELECT_REVISIONS =
   "SELECT revNo,parentRevNo,workingDir,timestamp FROM revision ORDER BY timestamp ASC;";

// SELECT all distinct (via GROUP BY) paths of files that are currently
// considered "existing" in the database. That is, files that are in the
// database and whose last recorded change was not a deletion.
//
// (kind != 2) refers to the ViRevisionChange::Type enumeration. 2 is the
// value that represents file removal.
const char* SQL_SELECT_EXISTING_FILES = "SELECT filePath FROM (SELECT filePath,kind,max(chIdx) FROM change WHERE "
                                        "(revNo <= ?) GROUP BY filePath) WHERE (kind != 2);";

// SELECT all distinct file names of files in the database, even ones
// that are considered deleted as of the latest revision.
const char* SQL_SELECT_ALL_FILES = "SELECT DISTINCT filePath FROM change WHERE (revNo <= ?);";

// SELECT all changes for a given file up to a given revision number.
const char* SQL_SELECT_CHANGES_UP_TO =
   "SELECT kind,revNo FROM change WHERE (filePath = ? AND revNo <= ?) ORDER BY revNo DESC;";

// SELECT all changes for a given file between two given revision numbers.
const char* SQL_SELECT_CHANGES_BTW =
   "SELECT kind,revNo,data FROM change WHERE (filePath = ? AND revNo <= ? AND revNo >= ?) ORDER BY chIdx ASC;";

// SELECT all changes for a given revision number.
const char* SQL_SELECT_REVISION_CHANGES =
   "SELECT kind,revNo,filePath,data FROM change WHERE (revNo = ?) ORDER BY filePath ASC;";

// SELECT the latest revision number in the database.
const char* SQL_SELECT_LATEST_REVISION = "SELECT revNo FROM revision ORDER BY revNo DESC LIMIT 1;";

// SELECT all startup files for a revision.
const char* SQL_SELECT_STARTUP_FILES = "SELECT filePath FROM startupFile WHERE (revNo = ?) ORDER BY idx ASC;";

// SELECT the working directory from the given revision.
const char* SQL_SELECT_WORKING_DIR = "SELECT workingDir FROM revision WHERE (revNo = ?) LIMIT 1;";

// SELECT the value for the given key from the metadata table.
const char* SQL_SELECT_METADATA = "SELECT value FROM metadata WHERE (key = ?) LIMIT 1;";
} // namespace

wizard::RevisionDb::RevisionDb()
{
   mSqlDb = nullptr;

   mSqlStmts.push_back(&mDeleteRevStmt);
   mSqlStmts.push_back(&mInsertChangeStmt);
   mSqlStmts.push_back(&mInsertMetadataStmt);
   mSqlStmts.push_back(&mInsertRevStmt);
   mSqlStmts.push_back(&mInsertStartupFileStmt);
   mSqlStmts.push_back(&mSelectAllFilesStmt);
   mSqlStmts.push_back(&mSelectChangesBtwStmt);
   mSqlStmts.push_back(&mSelectChangesUpToStmt);
   mSqlStmts.push_back(&mSelectExistingFilesStmt);
   mSqlStmts.push_back(&mSelectLatestRevStmt);
   mSqlStmts.push_back(&mSelectWorkingDirStmt);
   mSqlStmts.push_back(&mSelectMetadataStmt);
   mSqlStmts.push_back(&mSelectRevChangesStmt);
   mSqlStmts.push_back(&mSelectRevisionsStmt);
   mSqlStmts.push_back(&mSelectStartupFilesStmt);
}

wizard::RevisionDb::~RevisionDb()
{
   Close();
}

//! Creates and/or opens a database at the given file path.
//! @param aDbPath The file path of the database to open.
//! @returns Whether or not the database was successfully opened.
bool wizard::RevisionDb::Open(const std::string& aDbPath)
{
   bool success = false;

   if (DbOpen(aDbPath))
   {
      if (DbCreateTables())
      {
         switch (Version())
         {
         case 1:
            UpdateToV2();
         default:
            PrepareSqlStatements();
            SetVersion(DB_VERSION);
            break;
         }
         success = true;
      }
      else
      {
         DbClose();
      }
   }

   if (!success)
   {
      PrintDbError();
   }

   return success;
}

//! SQLite will create an in-memory database when opening a database with the
//! special name ":memory:". The database will vanish when its connection closes.
//! Particularly useful for tests.
//! @note http://www.sqlite.org/c3ref/open.html
//! @returns Whether or not a database was successfully opened in memory.
bool wizard::RevisionDb::OpenInMemory()
{
   return Open(":memory:");
}

//! Finalizes the SQLite statements and closes the connection to the database.
//! @returns Whether or not the database was successfully closed.
bool wizard::RevisionDb::Close()
{
   bool success = true;

   if (mSqlDb)
   {
      // Any unfinalized prepared statements will prevent the database from
      // being closed. (SQLITE_BUSY will be returned.)
      // -- http://www.sqlite.org/c3ref/close.html
      FinalizeSqlStatements();
      success = DbClose();
      mSqlDb  = nullptr;
   }

   return success;
}

//! Runs a transaction that deletes all data from the tables.
//! @returns Whether or not the transaction was successful.
bool wizard::RevisionDb::ClearTables()
{
   bool success = false;

   if (BeginTransaction())
   {
      success = DbClearTables() && CommitTransaction();
      if (!success)
      {
         RollbackTransaction();
      }
   }

   return success;
}

//! Stores a new revision of changes in the database.
//! @param aChanges The list of file changes to insert for the new revision.
//! @param aWorkingDir The project working directory to insert for the new revision.
//! @param aStartupFiles The list of project startup files to insert for the new revision.
//! @returns The new revision number (if successful) or -1 (if unsuccessful).
int wizard::RevisionDb::StoreNewRevision(const RevisionChangeList&       aChanges,
                                         const std::string&              aWorkingDir,
                                         const std::vector<std::string>& aStartupFiles)
{
   int  newRevNo     = -1;
   bool changesSaved = false;

   // The calls to the Latest* methods here will modify their arguments.
   std::string              latestWorkingDir;
   std::vector<std::string> latestStartupFiles;
   const bool workingDirChanged   = (!LatestWorkingDirectory(latestWorkingDir) || (aWorkingDir != latestWorkingDir));
   const bool startupFilesChanged = (!LatestStartupFiles(latestStartupFiles) || (aStartupFiles != latestStartupFiles));

   if ((!aChanges.empty() || workingDirChanged || startupFilesChanged) && BeginTransaction())
   {
      newRevNo = NewRevisionNo(aWorkingDir);

      changesSaved = ((newRevNo > 0) && InsertStartupFiles(newRevNo, aStartupFiles) &&
                      InsertChanges(newRevNo, aChanges) && CommitTransaction());

      if (!changesSaved)
      {
         RollbackTransaction();
      }
   }

   return (changesSaved ? newRevNo : -1);
}

//! Begins a transaction. All SQL statements executed on the database between
//! the call to this function and the call to commitTransaction() will be batched
//! and happen atomically. This improves performance greatly if you need to do
//! a lot of inserts (for example) at once.
//! @returns Whether or not the transaction was successfully started.
bool wizard::RevisionDb::BeginTransaction()
{
   return RevisionSqlStmt::ExecOk(mSqlDb, "BEGIN TRANSACTION;");
}

//! Commits a transaction. Should be preceded by beginTransaction().
//! @returns Whether or not the commit was successful.
bool wizard::RevisionDb::CommitTransaction()
{
   return RevisionSqlStmt::ExecOk(mSqlDb, "COMMIT TRANSACTION;");
}

//! Aborts/rolls back a transaction.
//! @returns Whether or not the rollback was successful.
bool wizard::RevisionDb::RollbackTransaction()
{
   // Rollbacks will fail with SQLITE_BUSY if there are any pending queries. So
   // reset all statements before rolling back.
   // -- http://www.sqlite.org/lang_transaction.html
   ResetSqlStatements();

   return RevisionSqlStmt::ExecOk(mSqlDb, "ROLLBACK TRANSACTION;");
}

//! @returns The latest revision number in the database.
int wizard::RevisionDb::LatestRevisionNo()
{
   return mSelectLatestRevStmt.ExecuteStepRow() ? mSelectLatestRevStmt.ColInt(0) : -1;
}

//! Creates a new revision number.
//! @returns The newly created revision number
int wizard::RevisionDb::NewRevisionNo(const std::string& aWorkingDir)
{
   mInsertRevStmt.Reset();
   mInsertRevStmt.Bind(1, aWorkingDir);
   return mInsertRevStmt.Step() ? LatestRevisionNo() : -1;
}

//! Deletes a revision from the database. (The deletion will cascade to any
//! associated changes.)
//! @param aRevNo The revision number to delete.
//! @returns Whether or not the delete was successful.
bool wizard::RevisionDb::DeleteRevision(int aRevNo)
{
   mDeleteRevStmt.Reset();
   mDeleteRevStmt.Bind(1, aRevNo);
   return mDeleteRevStmt.Step();
}

//! Retrieves the version of a given file as it existed at a given revision.
//! This is used for restoring files to old versions.
//! @param aRelFilePath The relative path of the file to look up in the database.
//! @param aRevision The revision of the file to fetch.
//! @param aDoc A reference to a document object to which the content of the
//! given file name at the given revision will be written.
//! @param aRevisionsTraversed The number of revisions that needed to be
//! traversed in order to build the file contents.
//! @returns Whether or not the file existed at the given revision.
bool wizard::RevisionDb::GetFileAtRevision(const std::string& aRelFilePath,
                                           int                aRevision,
                                           UtTextDocument&    aDoc,
                                           int*               aRevisionsTraversed)
{
   bool fileExists         = false;
   int  revisionsTraversed = 0;

   aDoc.Clear();
   aDoc.Insert(0, "", 1);

   const int firstFullRev = GetFirstFullRevisionUpTo(aRelFilePath, aRevision);

   if (firstFullRev >= -1)
   {
      // Found revision data, reconstitute
      mSelectChangesBtwStmt.Reset();
      mSelectChangesBtwStmt.Bind(1, aRelFilePath);
      mSelectChangesBtwStmt.Bind(2, aRevision);
      mSelectChangesBtwStmt.Bind(3, firstFullRev);
      while (mSelectChangesBtwStmt.StepRow())
      {
         ++revisionsTraversed;
         RevisionChange::Type kind = RevisionChange::Type(mSelectChangesBtwStmt.ColInt(0));
         switch (kind)
         {
         case RevisionChange::cNEW_FILE:
         case RevisionChange::cCHANGE_FULL:
         {
            QByteArray bytes(mSelectChangesBtwStmt.ColBlobUnzipped(2));
            aDoc.Clear();
            // QByteArrays are null terminated
            aDoc.Insert(0, bytes.constData(), bytes.size() + 1);
            assert(aDoc[aDoc.Size() - 1] == 0);
            fileExists = true;
            break;
         }
         case RevisionChange::cREMOVED_FILE:
            aDoc.Clear();
            fileExists = false;
            break;
         case RevisionChange::cCHANGE_DELTA:
         {
            QByteArray                bytes(mSelectChangesBtwStmt.ColBlobUnzipped(2));
            QDataStream               stream(bytes);
            QVector<TextSourceChange> changes;
            stream >> changes;
            ApplyToDocument(aDoc, changes);
            break;
         }
         default:
            // Should not happen
            assert(false);
            break;
         }
      }
   }

   if (fileExists)
   {
      aDoc.GetFilePath();
   }

   if (aRevisionsTraversed != nullptr)
   {
      *aRevisionsTraversed = revisionsTraversed;
   }

   return fileExists;
}

//! Inserts a change for a given revision into the database.
//! @param aRevNo The revision number that the change belongs to.
//! @param aChange The change to insert.
//! @returns Whether or not the insert was successful.
bool wizard::RevisionDb::InsertChange(int aRevNo, const RevisionChange& aChange)
{
   return InsertChange(aChange.kind, aRevNo, aChange.filePath, aChange.changeBytes);
}

//! Inserts a list of changes for a given revision into the database.
//! @param aRevNo The revision number that the changes belong to.
//! @param aChangeList The list of changes to insert.
//! @returns Whether or not all of the inserts were successful.
bool wizard::RevisionDb::InsertChanges(int aRevNo, const RevisionChangeList& aChangeList)
{
   bool success = true;

   for (RevisionChangeList::const_iterator i = aChangeList.begin(); (i != aChangeList.end()) && success; ++i)
   {
      RevisionChange change = *i;
      success               = InsertChange(aRevNo, change) && success;
   }

   return success;
}

//! Inserts a startup file associated with the given revision.
bool wizard::RevisionDb::InsertStartupFile(int aRevNo, const std::string& aStartupFile)
{
   mInsertStartupFileStmt.Reset();
   mInsertStartupFileStmt.Bind(1, aRevNo);
   mInsertStartupFileStmt.Bind(2, aStartupFile);
   return mInsertStartupFileStmt.Step();
}

//! Inserts a list of startup files associated with the given revision.
bool wizard::RevisionDb::InsertStartupFiles(int aRevNo, const std::vector<std::string>& aStartupFiles)
{
   bool success = true;

   for (size_t i = 0; i < aStartupFiles.size() && success; i++)
   {
      const std::string& startupFile = aStartupFiles[i];
      success                        = InsertStartupFile(aRevNo, startupFile) && success;
   }

   return success;
}

//! @returns Data for all revisions in the database.
wizard::RevisionList wizard::RevisionDb::Revisions()
{
   RevisionList revisions;

   mSelectRevisionsStmt.Reset();

   while (mSelectRevisionsStmt.StepRow())
   {
      const int          revNo       = mSelectRevisionsStmt.ColInt(0);
      const int          parentRevNo = mSelectRevisionsStmt.ColInt(1);
      const std::string& workingDir  = mSelectRevisionsStmt.ColText(2);
      const int          timestamp   = mSelectRevisionsStmt.ColInt(3);

      Revision rev(revNo, parentRevNo, workingDir, timestamp);

      // Get startup files for revision.
      mSelectStartupFilesStmt.Reset();
      mSelectStartupFilesStmt.Bind(1, revNo);
      while (mSelectStartupFilesStmt.StepRow())
      {
         const std::string& startupFile = mSelectStartupFilesStmt.ColText(0);
         rev.startupFiles.push_back(startupFile);
      }

      revisions.push_back(rev);
   }

   return revisions;
}

//! @returns All changes associated with a given revision number.
//! @param aRevNo A revision number.
wizard::RevisionChangeList wizard::RevisionDb::Changes(int aRevNo)
{
   RevisionChangeList revChanges;

   mSelectRevChangesStmt.Reset();
   mSelectRevChangesStmt.Bind(1, aRevNo);

   while (mSelectRevChangesStmt.StepRow())
   {
      RevisionChange change;

      change.kind        = RevisionChange::Type(mSelectRevChangesStmt.ColInt(0));
      change.filePath    = mSelectRevChangesStmt.ColText(2);
      change.changeBytes = mSelectRevChangesStmt.ColBlobUnzipped(3);

      assert(change.kind != RevisionChange::cNO_CHANGE);
      revChanges.push_back(change);
   }

   return revChanges;
}

//! @returns All distinct file paths (relative paths) that exist in the database
//! for the given revision.
//! @param aRevNo A revision number.
wizard::RevisionPathSet wizard::RevisionDb::ExistingFilePaths(int aRevNo)
{
   RevisionPathSet fileSet;

   mSelectExistingFilesStmt.Reset();
   mSelectExistingFilesStmt.Bind(1, aRevNo);

   while (mSelectExistingFilesStmt.StepRow())
   {
      fileSet.insert(mSelectExistingFilesStmt.ColText(0));
   }

   return fileSet;
}

//! @returns All file paths (relative) that have existed in the database up to
//! the given revision number, even if the files are considered to be deleted or
//! non-existent at the revision.
//! @param aRevNo A revision number.
wizard::RevisionPathSet wizard::RevisionDb::AllFilePaths(int aRevNo)
{
   RevisionPathSet fileSet;

   mSelectAllFilesStmt.Reset();
   mSelectAllFilesStmt.Bind(1, aRevNo);

   while (mSelectAllFilesStmt.StepRow())
   {
      fileSet.insert(mSelectAllFilesStmt.ColText(0));
   }

   return fileSet;
}

//! @returns A map of file paths to their contents for all files that exist in
//! the database at the given revision.
//! @param aRevNo A revision number.
wizard::FileMap wizard::RevisionDb::ExistingFiles(int aRevNo)
{
   const RevisionPathSet& filePaths = ExistingFilePaths(aRevNo);
   FileMap                fileMap;

   for (auto&& path : filePaths)
   {
      UtTextDocument file;
      GetFileAtRevision(path, aRevNo, file);
      fileMap[path] = file;
   }

   return fileMap;
}

//! @returns A list of startup files for the given revision number.
bool wizard::RevisionDb::StartupFiles(int aRevNo, std::vector<std::string>& aStartupFiles)
{
   bool startupFilesFound = false;

   if (aRevNo != -1)
   {
      aStartupFiles.clear();
      mSelectStartupFilesStmt.Reset();
      mSelectStartupFilesStmt.Bind(1, aRevNo);
      while (mSelectStartupFilesStmt.StepRow())
      {
         aStartupFiles.push_back(mSelectStartupFilesStmt.ColText(0));
         startupFilesFound = true;
      }
   }

   return startupFilesFound;
}

//! Sets the reference aWorkingDir to the working directory of the given
//! revision number.
//! @returns Whether or not a working directory existed in the database for the
//! given revision.
bool wizard::RevisionDb::WorkingDirectory(int aRevNo, std::string& aWorkingDir)
{
   const char* workingDirCStr = nullptr;

   if (aRevNo != -1)
   {
      mSelectWorkingDirStmt.Reset();
      mSelectWorkingDirStmt.Bind(1, aRevNo);
      workingDirCStr = (mSelectWorkingDirStmt.ExecuteStepRow() ? mSelectWorkingDirStmt.ColCString(0) : nullptr);
      if (workingDirCStr)
      {
         aWorkingDir = std::string(workingDirCStr);
      }
   }

   return (workingDirCStr != nullptr);
}

//! @returns A list of startup files for the latest revision.
bool wizard::RevisionDb::LatestStartupFiles(std::vector<std::string>& aStartupFiles)
{
   return StartupFiles(LatestRevisionNo(), aStartupFiles);
}

//! @returns The working directory for the latest revision.
bool wizard::RevisionDb::LatestWorkingDirectory(std::string& aWorkingDir)
{
   return WorkingDirectory(LatestRevisionNo(), aWorkingDir);
}

//! @returns The latest version of the database currently supported.
int wizard::RevisionDb::LatestSupportedVersion() const
{
   return DB_VERSION;
}

//! This should not be used in the same way the other statements are used with
//! ViSqlStmt objects. The version specifically will have to be checked
//! *before* initializing all the other statements, so that the database can
//! be upgraded from earlier versions if necessary.
//! @returns The current version of the opened database.
int wizard::RevisionDb::Version()
{
   RevisionSqlStmt selectVersionStmt;

   selectVersionStmt.Prepare(mSqlDb, SQL_SELECT_METADATA);
   selectVersionStmt.Reset();
   selectVersionStmt.Bind(1, "version");
   const int version = (selectVersionStmt.Step() ? selectVersionStmt.ColInt(0) : -1);
   selectVersionStmt.Finalize();

   return version;
}

//! @returns Whether or not the version of the opened database is older than
//! the latest available version.
bool wizard::RevisionDb::IsVersionOld()
{
   return (Version() < LatestSupportedVersion());
}

//! @returns Whether or not the database is empty.
//! @note "The ROWID chosen for the new row is at least one larger than the largest
//!        ROWID that has ever before existed in that same table. If the table has
//!        never before contained any data, then a ROWID of 1 is used."
//!        -- http://www.sqlite.org/autoinc.html
bool wizard::RevisionDb::IsEmpty()
{
   return (LatestRevisionNo() < 1);
}

//! @returns Whether or not the database is open.
bool wizard::RevisionDb::IsOpen() const
{
   return (mSqlDb != nullptr);
}

//! Creates the tables for holding revisions and revision data and sets the
//! desired pragmas.
//! @returns Whether or not the tables were created and pragmas were set.
bool wizard::RevisionDb::DbCreateTables()
{
   return (Exec(SQL_CREATE_TABLES) == SQLITE_OK);
}

//! Deletes all data from all tables.
//! @returns Whether or not the deletions were successful.
bool wizard::RevisionDb::DbClearTables()
{
   return (Exec(SQL_CLEAR_TABLES) == SQLITE_OK);
}

//! Opens the SQLite database at the given path.
//! @param dbPath The path of the database to open.
//! @returns Whether or not the database was successfully opened.
bool wizard::RevisionDb::DbOpen(const std::string& aDbPath)
{
   return (sqlite3_open(aDbPath.c_str(), &mSqlDb) == SQLITE_OK);
}

//! Closes the SQLite database.
//! @note http://www.sqlite.org/c3ref/close.html
//! @returns Whether or not the database was successfully closed.
bool wizard::RevisionDb::DbClose()
{
   bool success = true;
   int  rv;

   while ((rv = sqlite3_close(mSqlDb)) != SQLITE_OK)
   {
      if (rv != SQLITE_BUSY)
      {
         success = false;
         PrintDbError();
         break;
      }
   }

   return success;
}

//! Updates the database from version 1 to version 2.
//! @returns Whether or not the update SQL was successfully executed.
bool wizard::RevisionDb::UpdateToV2()
{
   return (Exec(SQL_UPDATE_V1_TO_V2) == SQLITE_OK);
}

//! Search revisions for the given file path, up to and including the given
//! revision number, looking for the earliest revision needed to reconstitute
//! the text.
//! @param aRelFilePath The relative path of the file.
//! @param aRevNo The maximum revision number to check.
//! @returns The earliest revision needed for fully representing the given file.
int wizard::RevisionDb::GetFirstFullRevisionUpTo(const std::string& aRelFilePath, int aRevNo)
{
   int firstFullRev = -1;

   mSelectChangesUpToStmt.Reset();
   mSelectChangesUpToStmt.Bind(1, aRelFilePath);
   mSelectChangesUpToStmt.Bind(2, aRevNo);

   while (mSelectChangesUpToStmt.StepRow())
   {
      RevisionChange::Type kind = RevisionChange::Type(mSelectChangesUpToStmt.ColInt(0));
      if (kind != RevisionChange::cCHANGE_DELTA)
      {
         firstFullRev = mSelectChangesUpToStmt.ColInt(1);
      }
   }

   return firstFullRev;
}

//! Inserts a change (associated with a revision) with the given data into the
//! database.
//! @param aKind The kind of change.
//! @param aRevNo The revision number that this change belongs to.
//! @param aFilePath The path of the file that this change pertains to.
//! @param aData Serialized data that represents the change.
//! @returns Whether or not the insert was successful.
bool wizard::RevisionDb::InsertChange(RevisionChange::Type aKind,
                                      int                  aRevNo,
                                      const std::string&   aFilePath,
                                      const QByteArray&    aData)
{
   const bool changed = (aKind != RevisionChange::cNO_CHANGE);

   if (changed)
   {
      mInsertChangeStmt.Reset();
      mInsertChangeStmt.Bind(1, aKind);
      mInsertChangeStmt.Bind(2, aRevNo);
      mInsertChangeStmt.Bind(3, aFilePath);
      mInsertChangeStmt.BindZipped(4, aData);
   }

   return changed && mInsertChangeStmt.Step();
}

//! Set the version of the opened database to the given number. The number given
//! must be greater than zero.
//! @param aVersion The desired version to set in the database.
//! @returns Whether or not the version was successfully set in the database.
bool wizard::RevisionDb::SetVersion(int aVersion)
{
   const bool validVersion = (aVersion > 0);

   assert(validVersion);
   if (validVersion)
   {
      mInsertMetadataStmt.Reset();
      mInsertMetadataStmt.Bind(1, "version");
      mInsertMetadataStmt.Bind(2, aVersion);
   }

   return validVersion && mInsertMetadataStmt.Step();
}

//! Executes the given SQL text in the database.
//! @returns The SQLite result code of the execution.
int wizard::RevisionDb::Exec(const char* aSql)
{
   return RevisionSqlStmt::Exec(mSqlDb, aSql);
}

//! Prints a database error if available (and asserts when debugging).
void wizard::RevisionDb::PrintDbError()
{
   const char* err = sqlite3_errmsg(mSqlDb);
   std::cerr << err << std::endl;
   // assert(false);
}

//! Prepares SQL statements with the open database so they can be used.
void wizard::RevisionDb::PrepareSqlStatements()
{
   mDeleteRevStmt.Prepare(mSqlDb, SQL_DELETE_REVISION);
   mInsertChangeStmt.Prepare(mSqlDb, SQL_INSERT_CHANGE);
   mInsertMetadataStmt.Prepare(mSqlDb, SQL_INSERT_METADATA);
   mInsertRevStmt.Prepare(mSqlDb, SQL_INSERT_REVISION);
   mInsertStartupFileStmt.Prepare(mSqlDb, SQL_INSERT_STARTUP_FILE);
   mSelectAllFilesStmt.Prepare(mSqlDb, SQL_SELECT_ALL_FILES);
   mSelectChangesBtwStmt.Prepare(mSqlDb, SQL_SELECT_CHANGES_BTW);
   mSelectChangesUpToStmt.Prepare(mSqlDb, SQL_SELECT_CHANGES_UP_TO);
   mSelectExistingFilesStmt.Prepare(mSqlDb, SQL_SELECT_EXISTING_FILES);
   mSelectLatestRevStmt.Prepare(mSqlDb, SQL_SELECT_LATEST_REVISION);
   mSelectWorkingDirStmt.Prepare(mSqlDb, SQL_SELECT_WORKING_DIR);
   mSelectMetadataStmt.Prepare(mSqlDb, SQL_SELECT_METADATA);
   mSelectRevChangesStmt.Prepare(mSqlDb, SQL_SELECT_REVISION_CHANGES);
   mSelectRevisionsStmt.Prepare(mSqlDb, SQL_SELECT_REVISIONS);
   mSelectStartupFilesStmt.Prepare(mSqlDb, SQL_SELECT_STARTUP_FILES);
}

//! Resets all SQL statements.
void wizard::RevisionDb::ResetSqlStatements()
{
   for (auto&& stmt : mSqlStmts)
   {
      stmt->Reset();
   }
}

//! Finalizes all SQL statements.
void wizard::RevisionDb::FinalizeSqlStatements()
{
   for (auto&& stmt : mSqlStmts)
   {
      stmt->Finalize();
   }
}
