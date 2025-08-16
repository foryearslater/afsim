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

#ifndef REVISIONDB_HPP
#define REVISIONDB_HPP

#include <map>
#include <set>
#include <vector>

#include "Revision.hpp"
#include "RevisionChange.hpp"
#include "RevisionSqlStmt.hpp"
#include "UtTextDocument.hpp"

namespace wizard
{
using RevisionPathSet = std::set<std::string>;
using FileMap         = std::map<std::string, UtTextDocument>;

//! A semantic interface into the Change History database for managing revisions,
//! used primarily by ViRevisionStore.
//!
//! @note You should use the higher level ViRevisionStore instead of this to
//! view and manage the Change History database.
class VI_EXPORT RevisionDb
{
public:
   RevisionDb();
   virtual ~RevisionDb();

   bool Open(const std::string& aDbPath);
   bool OpenInMemory();
   bool Close();
   bool ClearTables();

   int StoreNewRevision(const RevisionChangeList&       aChanges,
                        const std::string&              aWorkingDir,
                        const std::vector<std::string>& aStartupFiles);

   int  LatestRevisionNo();
   int  NewRevisionNo(const std::string& aWorkingDir);
   bool DeleteRevision(int aRevNo);
   bool GetFileAtRevision(const std::string& aRelFilePath,
                          int                aRevision,
                          UtTextDocument&    aDoc,
                          int*               aRevisionsTraversed = nullptr);
   bool InsertChange(int aRevNo, const RevisionChange& aChange);
   bool InsertChanges(int aRevNo, const RevisionChangeList& aChangeList);
   bool InsertStartupFile(int aRevNo, const std::string& aStartupFile);
   bool InsertStartupFiles(int aRevNo, const std::vector<std::string>& aStartupFiles);

   RevisionList       Revisions();
   RevisionChangeList Changes(int aRevNo);
   RevisionPathSet    ExistingFilePaths(int aRevNo);
   RevisionPathSet    AllFilePaths(int aRevNo);
   FileMap            ExistingFiles(int aRevNo);
   bool               StartupFiles(int aRevNo, std::vector<std::string>& aStartupFiles);
   bool               WorkingDirectory(int aRevNo, std::string& aWorkingDir);
   bool               LatestStartupFiles(std::vector<std::string>& aStartupFiles);
   bool               LatestWorkingDirectory(std::string& aWorkingDir);

   int  LatestSupportedVersion() const;
   int  Version();
   bool IsVersionOld();

   bool IsEmpty();
   bool IsOpen() const;

private:
   bool DbCreateTables();
   bool DbClearTables();
   bool DbOpen(const std::string& aDbPath);
   bool DbClose();

   bool UpdateToV2();

   int  GetFirstFullRevisionUpTo(const std::string& aRelFilePath, int aRevNo);
   bool InsertChange(RevisionChange::Type aKind, int aRevNo, const std::string& aFilePath, const QByteArray& aData);
   bool SetVersion(int aVersion);
   int  Exec(const char* aSql);
   void PrintDbError();

   bool BeginTransaction();
   bool CommitTransaction();
   bool RollbackTransaction();

   void PrepareSqlStatements();
   void ResetSqlStatements();
   void FinalizeSqlStatements();

   using RevisionSqlStmtList = std::vector<RevisionSqlStmt*>;

   sqlite3*            mSqlDb;
   RevisionSqlStmt     mDeleteRevStmt;
   RevisionSqlStmt     mInsertChangeStmt;
   RevisionSqlStmt     mInsertMetadataStmt;
   RevisionSqlStmt     mInsertRevStmt;
   RevisionSqlStmt     mInsertStartupFileStmt;
   RevisionSqlStmt     mSelectAllFilesStmt;
   RevisionSqlStmt     mSelectChangesBtwStmt;
   RevisionSqlStmt     mSelectChangesUpToStmt;
   RevisionSqlStmt     mSelectExistingFilesStmt;
   RevisionSqlStmt     mSelectLatestRevStmt;
   RevisionSqlStmt     mSelectStartupFilesStmt;
   RevisionSqlStmt     mSelectWorkingDirStmt;
   RevisionSqlStmt     mSelectMetadataStmt;
   RevisionSqlStmt     mSelectRevChangesStmt;
   RevisionSqlStmt     mSelectRevisionsStmt;
   RevisionSqlStmtList mSqlStmts;
};
} // namespace wizard
#endif // REVISIONDB_HPP
