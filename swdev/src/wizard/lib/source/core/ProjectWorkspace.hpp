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

#ifndef PROJECTWORKSPACE_HPP
#define PROJECTWORKSPACE_HPP

#include <set>

#include <QFuture>
#include <QThread>
#include <QTime>
#include <QTimer>

#include "FileSignature.hpp"
#include "ProjectParseThread.hpp"
#include "UtCallbackHolder.hpp"
#include "UtMemory.hpp"
#include "UtPath.hpp"
#include "UtSemaphore.hpp"
#include "UtThread.hpp"

class UtTextDocumentRange;
class ViMainWindow;
class UtTextDocument;

namespace wizard
{
class ActionManager;
class CreateNewFile;
class EditorManager;
class Project;
class ProxyMerge;
class TextSource;
class TextSourceCache;
class TextSourceView;
class WsfConnector;
class WsfExecution;

class VI_EXPORT ProjectWorkspace : public QObject
{
   Q_OBJECT
public:
   ProjectWorkspace();
   ~ProjectWorkspace() override;

   static ProjectWorkspace* Instance();

   TextSourceCache* GetSourceCache() const { return mSourceCachePtr; }

   Project* GetProject();

   void   Update();
   QTimer mUpdateTimer;

   void customEvent(QEvent*) override;

   void WaitForParsing();
   void WaitForAbortParsing();
   // void CompleteParsing();
   void RequestReparseWsf(TextSource* aSourcePtr);

   void WaitForSourceModifications(TextSource* aSourcePtr);

   void           ProjectAdded(Project* aProjectPtr);
   void           RemoveProject(Project* aProjectPtr);
   void           NotifyOfSourceChange(TextSource* aSourcePtr);
   void           ViewOpened(TextSourceView* aViewPtr);
   void           ViewClosed(TextSourceView* aViewPtr);
   ActionManager* GetActionManager() const { return mActionManagerPtr; }
   EditorManager* GetEditorManager() const { return mEditorPtr; }
   bool           ParseIsUpToDate();
   //! Get the time the WsfParseDefinitions spent parsing the WsfGrammar
   //! @return the latest grammar parsing time
   double GetLastGrammarParseTime() const noexcept;
   //! Get the time the WsfParser spent parsing the input files
   //! @return the latest input parsing time
   double GetLastInputParseTime() const noexcept;
   //! Get the time the WsfPProxyDeserialize spent deserializing the parse tree
   //! @return the latest proxy deserializing time
   double GetLastProxyDeserializeTime() const noexcept;
   //! Get the time the ProxyMerge spent merging the old and new WsfPProxys/Wizard spent updating its GUIs
   //! @return the latest proxy merging/GUI updating time
   double GetLastProxyMergeGUI_Time() const noexcept;
   void   ReparseAll();

   // This version will block while parsing finishing and then run.
   // Prefer ScheduleCheckingFilesForModification() where possible.
   void CheckFilesForModification();
   // This version will set a flag to check files after parsing finishes if parsing is currently
   // happening or now if no parsing is happening.
   void ScheduleCheckingFilesForModification();

   void ApplicationActivated();
   void SaveAll();
   void ParseStatistics();

   TextSource*              FindSource(const std::string& aPath, bool aCreate, bool aRead);
   std::vector<TextSource*> GetSourceList();
   wizard::Editor*          OpenSource(TextSource* aSourcePtr);
   wizard::Editor*          GotoRange(const UtTextDocumentRange& aRange);
   wizard::Editor*          GotoRange(const std::string& aFilePath, size_t aLineNumber);
   wizard::Editor*          GotoFile(UtTextDocument* aSourcePtr, bool aRecordLocation);
   wizard::Editor*          GotoFile(const std::string& aFilePath, bool aRecordLocation = true);
   bool                     RequestClose(bool aForce = false);

   void ProjectIncrementParseNumber() { ++mModificationSequenceNumber; }
   void TouchParse(bool aUpdateQuick = false);
   void RestyleViews();

   void                              StartExecution(WsfExecution* aExecutionPtr);
   void                              StopAllExecutions();
   const std::vector<WsfExecution*>& GetExecutions() const { return mExecutions; }

   VI_DEPRECATED void EmitProxyUpdated(ProxyMerge* aProxyMerge);

   struct FileStat
   {
      FileSignature mOldSignature;
      UtPath        mPath;
   };

   void BeginSerialize();

   WsfConnector* GetWsfConnector() { return mWsfConnectorPtr; }

   bool IsProcessing() const;

   void NewProject();
   void CreateA_NewFile(Project* aProject, const UtPath& aDir);

signals:
   void ProjectOpened(Project* aProjectPtr);
   void ProjectClosed(Project* aProjectPtr);
   void ActiveProjectChanged(Project* aProjectPtr);
   void StartingExecution(WsfExecution* aExecutionPtr);
   void ParseStatisticsReady(Project* aProjectPtr, QString aReportText);

   void SourceRemoved(TextSource* aSourcePtr);
   void ProxyUpdated(ProxyMerge* aProxyMerge);

   //! The project has been fully parsed and proxied
   void FullParseComplete(Project* aProjectPtr);

   //! A piece of work has completed.  This is used to record timing information.
   void PerformanceResult(Project* aProjectPtr, QString aWorkType, int aTimeToCompleteMS);

   void NewTextSource(TextSource* aSourcePtr);

   // Debug session has started/ended
   void ChangeDebug(bool aIsActive);

protected slots:
   void ExecutionDestroyed(QObject* aObjectPtr);
   void OnChangeDebug(bool aIsActive);

private:
   void OnParseCompletion();
   void CheckFilesForModificationInternal();

   QString OutputPerformanceString(Project* aProjectPtr, int aTimeSpentParsing, int aTimeSpentProxying);
   void    SetModified(bool aUpdateQuick = false);
   void    BeginUpdateParse(bool aTesting);
   bool    DelayedFileModificationCheck();

   void SyncModifiedWsfSources();

   void InitializeNewSources();

   //! Indicate that scenarios should be reevaluated for changes
   bool mTouched;

   // Files that have pending modifications (usually from the text editor)
   std::set<TextSource*> mModifiedWsfSources;
   //! The sequence number for the current parse.
   volatile int mModificationSequenceNumberBeingParsed;
   //! The number of times modifications are made.  Updated as the user makes changes.
   volatile int mModificationSequenceNumber;
   //! The sequence number for the last time parsing was done.
   int mParsedModificationSequenceNumber;

   TextSourceCache*   mSourceCachePtr;
   ProjectParseThread mParseThread;

   // After a parse is completed, there are many things that need updated.
   // These members are used to do a 'full update' at a later time:
   int   mFullUpdateSequenceNumber;
   QTime mFullUpdateTime;
   bool  mFullUpdateQuick;
   int   mPreventUpdates;

   bool mTriggerParseAll;
   //! True if the previous parse phase resulted in a complete reparse

   Project* mProjectPtr;

   ActionManager*               mActionManagerPtr;
   EditorManager*               mEditorPtr;
   std::vector<TextSourceView*> mOpenViews;

   UtCallbackHolder mCallbacks;

   std::vector<WsfExecution*> mExecutions;

   int mCurrentTaskId;

   // Future to grab file stats when application focus is lost and regained
   QFuture<FileStat> mFileModCheckFuture;

   WsfConnector* mWsfConnectorPtr;

   CreateNewFile* mNewFileDialogPtr;

   bool mCheckFilesAfterParse = false;

public:
   // STATIC Access
   static int  GetMaximumWSF_FileSize() { return mMaximumWSF_FileSize; }
   static void SetMaximumWSF_FileSize(int aFileSize) { mMaximumWSF_FileSize = aFileSize; }

private:
   static int mMaximumWSF_FileSize;
   bool       mDebug; // True if debug session is active
};
} // namespace wizard

Q_DECLARE_METATYPE(wizard::ProjectWorkspace*);

#endif
