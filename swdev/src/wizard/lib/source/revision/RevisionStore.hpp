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

#ifndef REVISIONSTORE_HPP
#define REVISIONSTORE_HPP

#include <map>
#include <set>

#include <QObject>

#include "RevisionDb.hpp"
#include "UtCallbackHolder.hpp"
#include "UtPath.hpp"
#include "ViExport.hpp"

class QStringList;
class QTimer;
class UtTextDocument;
class WsfPProxy;

namespace wizard
{
class TextSource;

typedef std::map<std::string, const UtTextDocument*> RevisionFileMap;

//! A high-level interface into the Change History database.
class VI_EXPORT RevisionStore : public QObject
{
   Q_OBJECT

public:
   explicit RevisionStore();
   ~RevisionStore() override = default;

   void SetProject(const UtPath& aProjectDir, const UtPath& aProjectFile = UtPath());
   void SetProjectWorkingDir(const UtPath& aWorkingDir);

   bool Open();
   bool OpenInMemory();
   bool Close();
   bool IsEmpty();
   bool IsOpen() const;
   bool Exists() const;

   void EnableAutoBackup(int aMin);
   void DisableAutoBackup();

   void QueueFile(const UtTextDocument* aDocPtr);
   bool Restore(int aRevNo, QStringList& aFailedFiles, QWidget* aRequestingWidget = nullptr);
   bool DeleteAllRevisions();

   RevisionList Revisions();
   RevisionDb&  DB();

public slots:
   bool Backup();
   bool RequestBackup();
   void QueueFile(TextSource* aDocPtr);
   void QueueAllProjectFiles();
   void HandleProxyAvailable(WsfPProxy* aProxy);
   void HandleParseUpdated(bool aParsed);
   void HandleProjectStartupFilesChanged(const std::vector<UtPath>& aStartupFiles);

protected:
   RevisionChangeList GenerateFileChanges(const RevisionPathSet& aRemovedPaths = RevisionPathSet());
   bool               StoreNewRevision(const RevisionChangeList&       aChanges,
                                       const std::string&              aWorkingDir,
                                       const std::vector<std::string>& aStartupFiles);

private:
   bool RenameFallbackDb();
   bool ReplaceFile(const UtPath& aFilePath, const char* aFileContent, size_t aFileContentSize);
   bool MakeNewFile(const UtPath& aFilePath, const char* aFileContent);
   bool RestoreIncludedFile(const std::string& aFilePath, const UtTextDocument& aFileContent);
   bool RestoreUnincludedFile(const std::string& aFilePath, const UtTextDocument& aFileContent, QWidget* aRequestingWidget);
   bool           RestoreFile(const std::string& aPath, int aRevNo, QWidget* aRequestingWidget = nullptr);
   bool           AnalyzeAndBackup();
   bool           ProxyUpToDate() const;
   void           AddToQueue(const std::string& aRelFilePath, const UtTextDocument* aDocPtr);
   std::string    RelativeDocPath(const UtTextDocument& aDoc) const;
   RevisionChange AnalyzeFile(const UtTextDocument& aDoc);
   bool           AnalyzeAddedRemovedFiles(RevisionPathSet& aRemovedPaths);

   UtPath              mDbDir;
   UtPath              mDbFile;
   UtPath              mProjectDir;
   UtPath              mProjectWorkingDir;
   std::vector<UtPath> mProjectStartupFiles;
   UtCallbackHolder    mCallbacks;
   RevisionDb          mRevDb;
   RevisionFileMap     mQueuedFiles;
   bool                mBackupScheduled;
   QTimer*             mAutoBackupTimer;
   WsfPProxy*          mProxy;
};
} // namespace wizard
#endif
