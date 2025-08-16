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

#include "ProjectWorkspace.hpp"

#include <QMessageBox>
#include <QtConcurrentFilter>

#include "ActionManager.hpp"
#include "ClonePlatformDialog.hpp"
#include "CreateNewFile.hpp"
#include "CreatePlatformDialog.hpp"
#include "CreateTypeDialog.hpp"
#include "Editor.hpp"
#include "EditorDock.hpp"
#include "EditorManager.hpp"
#include "Environment.hpp"
#include "ParseResults.hpp"
#include "Project.hpp"
#include "RevisionStore.hpp"
#include "RunManager.hpp"
#include "Signals.hpp"
#include "TextSourceCache.hpp"
#include "TextSourceView.hpp"
#include "ThreadNotifyEvent.hpp"
#include "UtQt.hpp"
#include "UtSleep.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"
#include "WsfConnector.hpp"
#include "WsfExecution.hpp"
#include "WsfPProxySerialize.hpp"

int wizard::ProjectWorkspace::mMaximumWSF_FileSize = 100 * 1000000; // 100 MB

namespace
{
wizard::ProjectWorkspace* sInstancePtr = nullptr;
}

const int cDEFER_UPDATE_COUNT = 100;
const int cFULL_UPDATE_COUNT  = 1000;

wizard::ProjectWorkspace::ProjectWorkspace()
   : mTouched(false)
   , mModificationSequenceNumberBeingParsed(-2)
   , mModificationSequenceNumber(0)
   , mParsedModificationSequenceNumber(-1)
   , mParseThread()
   , mFullUpdateSequenceNumber(-1)
   , mFullUpdateQuick(true)
   , mPreventUpdates(0)
   , mProjectPtr(nullptr)
   , mEditorPtr(nullptr)
   , mCurrentTaskId(0)
{
   mNewFileDialogPtr = new CreateNewFile(wkfEnv.GetMainWindow(), "Wizard");
   sInstancePtr      = this;
   mEditorPtr        = new EditorManager();
   mActionManagerPtr = new ActionManager();

   mSourceCachePtr = new TextSourceCache(this);

   // Testing shows this this will be run in the main thread, so no synchronization necessary.
   mParseThread.SetRunOnCompletion(std::bind(&ProjectWorkspace::OnParseCompletion, this));

   mParseThread.Start();
   mCallbacks += wizSignals->ApplicationActivated.Connect(&ProjectWorkspace::ApplicationActivated, this);
   mCallbacks +=
      wizSignals->CheckFilesForModification.Connect(&ProjectWorkspace::ScheduleCheckingFilesForModification, this);
   mCallbacks += wizSignals->PasteQuery.Connect(&ClonePlatformDialog::PasteHandler);
   mCallbacks += wizSignals->PasteQuery.Connect(&CreatePlatformDialog::PasteHandler);
   mCallbacks += wizSignals->PasteQuery.Connect(&CreateTypeDialog::PasteHandler);

   mWsfConnectorPtr = new WsfConnector;

   mUpdateTimer.setInterval(10);
   QObject::connect(&mUpdateTimer, &QTimer::timeout, this, &ProjectWorkspace::Update);
   mUpdateTimer.start();

   mDebug = false;
   connect(ProjectWorkspace::Instance(), &ProjectWorkspace::ChangeDebug, this, &ProjectWorkspace::OnChangeDebug);
}

wizard::ProjectWorkspace::~ProjectWorkspace()
{
   delete mEditorPtr;
   StopAllExecutions();
   if (mProjectPtr)
   {
      RemoveProject(mProjectPtr);
   }
   mParseThread.JoinThread();
   delete mSourceCachePtr;
   delete mWsfConnectorPtr;
   sInstancePtr = nullptr;
}

void wizard::ProjectWorkspace::ProjectAdded(wizard::Project* aProjectPtr)
{
   if (mProjectPtr == nullptr)
   {
      mProjectPtr = aProjectPtr;
      SetModified(true);
      emit ProjectOpened(aProjectPtr);
      emit ActiveProjectChanged(aProjectPtr);
   }
}

bool wizard::ProjectWorkspace::RequestClose(bool aForce)
{
   bool closing = true;
   if (mProjectPtr)
   {
      closing = mProjectPtr->CloseProject(aForce);
   }
   return closing;
}

void wizard::ProjectWorkspace::RemoveProject(wizard::Project* aProjectPtr)
{
   if (mProjectPtr == aProjectPtr &&
       // set the working directory back to the application directory
       wizRunMgr.SetWorkingDirectoryToApplication())
   {
      emit ProjectClosed(mProjectPtr);
      emit ActiveProjectChanged(nullptr);
      WaitForAbortParsing();
      // The update timer may go off, this prevents updating while removing the project
      ++mPreventUpdates;
      delete mProjectPtr;
      mProjectPtr = nullptr;
      mSourceCachePtr->Clear();
      --mPreventUpdates;
   }
}

// For each scenario, determine if that scenario is affected by the changes to the source code.
// if (Scenario Parse Up To Date) and (Modified files are NOT in scenario)
//     mark as up to date
// else
//     if (Scenario is active)
//         Add scenario to parse batch
//     else
//         mark as not up to date
void wizard::ProjectWorkspace::BeginUpdateParse(bool aTesting)
{
   if (aTesting)
   {
      mTouched = true;
   }
   if (!DelayedFileModificationCheck())
   {
      return;
   }
   // Start a new parse task.  Attempts to avoid a full reparse.
   ProjectParseThread::Task task;
   if (mModificationSequenceNumber != mParsedModificationSequenceNumber || mTouched)
   {
      if (mProjectPtr)
      {
         // Add all scenarios which need parsing.  Put the active scenario first.
         if (!mProjectPtr->GetParseResults() || mTouched)
         {
            task.mScenarioPtr = mProjectPtr;
         }
      }
      SyncModifiedWsfSources();
      mModificationSequenceNumberBeingParsed = mModificationSequenceNumber;
      task.mTestingParse                     = aTesting;
      task.mTaskId                           = ++mCurrentTaskId;
      task.mModificationSequenceNumber       = mModificationSequenceNumber;
      mParseThread.SetTask(task);
   }
   mTouched = false;
}

void wizard::ProjectWorkspace::Update()
{
   if (mPreventUpdates != 0)
   {
      return;
   }
   ++mPreventUpdates;
   mWsfConnectorPtr->Update();

   if (!DelayedFileModificationCheck())
   {
      --mPreventUpdates;
      return;
   }

   ProjectParseThread::TaskPhase phase = mParseThread.GetPhase();
   if (phase == ProjectParseThread::cSERIALIZE_DONE)
   {
   }
   if (phase == ProjectParseThread::cPARSE_DONE)
   {
      // Make sure we have processed the events from the thread
      QApplication::processEvents();
      if (!mParseThread.IsAborting())
      {
         mParsedModificationSequenceNumber = mModificationSequenceNumberBeingParsed;
         mFullUpdateSequenceNumber         = mParsedModificationSequenceNumber;
         mFullUpdateTime                   = QTime::currentTime();
         if (!mFullUpdateQuick)
         {
            mFullUpdateTime = mFullUpdateTime.addMSecs(2000);
         }
      }
      mParseThread.SetComplete();
   }
   if (phase == ProjectParseThread::cNOT_WORKING)
   {
      BeginUpdateParse(false);
      phase = mParseThread.GetPhase();
   }
   if (phase == ProjectParseThread::cNOT_WORKING)
   {
      // If non-Wsf files are modified, we still need to sync changes even though re-parsing isn't occurring
      SyncModifiedWsfSources();
      InitializeNewSources();
      if (mFullUpdateSequenceNumber >= 0 && QTime::currentTime() > mFullUpdateTime)
      {
         if (mProjectPtr && mFullUpdateSequenceNumber == mModificationSequenceNumber)
         {
            mProjectPtr->DeferUpdate(true);
            emit FullParseComplete(mProjectPtr);
            mProjectPtr->FullUpdate();
         }
         mFullUpdateSequenceNumber = -1;
         mFullUpdateQuick          = false;
      }
   }
   --mPreventUpdates;
}

void wizard::ProjectWorkspace::RequestReparseWsf(TextSource* aSourcePtr)
{
   mModifiedWsfSources.insert(aSourcePtr);
   SetModified();
}

void wizard::ProjectWorkspace::SyncModifiedWsfSources()
{
   for (auto&& it : mModifiedWsfSources)
   {
      TextSource* src = it;
      src->SyncChangesToSource();
   }
   mModifiedWsfSources.clear();
}

//! Returns when the source has all queued changes applied.  This may require waiting until parsing is complete.
void wizard::ProjectWorkspace::WaitForSourceModifications(TextSource* aSourcePtr)
{
   if (mModifiedWsfSources.find(aSourcePtr) != mModifiedWsfSources.end())
   {
      WaitForAbortParsing();
   }
}

//! Waits until the parsing thread is complete.  This should only be called from the main thread.
// public
void wizard::ProjectWorkspace::WaitForParsing()
{
   if (mParseThread.GetPhase() == ProjectParseThread::cNOT_WORKING)
   {
      return;
   }
   while (mParseThread.GetPhase() == ProjectParseThread::cPARSING)
   {
      UtSleep::Sleep(.001);
   }
   //   CompleteParsing();
}

void wizard::ProjectWorkspace::WaitForAbortParsing()
{
   mParseThread.Abort();
   if (mParseThread.GetPhase() == ProjectParseThread::cNOT_WORKING)
   {
      return;
   }
   while (mParseThread.GetPhase() == ProjectParseThread::cPARSING)
   {
      UtSleep::Sleep(.001);
   }
   SyncModifiedWsfSources();
}

void wizard::ProjectWorkspace::ReparseAll()
{
   SetModified();
   mTouched = true;
}

void wizard::ProjectWorkspace::ViewOpened(TextSourceView* aViewPtr)
{
   mOpenViews.push_back(aViewPtr);
   mEditorPtr->AddTab(aViewPtr->mEditorPtr);
}

void wizard::ProjectWorkspace::ViewClosed(TextSourceView* aViewPtr)
{
   mOpenViews.erase(std::remove(mOpenViews.begin(), mOpenViews.end(), aViewPtr), mOpenViews.end());
}

void wizard::ProjectWorkspace::ParseStatistics()
{
   // complete existing parse run, and do another parse if necessary
   WaitForParsing();
   BeginUpdateParse(false);
   WaitForParsing();

   mParsedModificationSequenceNumber = -1;
   SetModified();
   BeginUpdateParse(true);
   WaitForParsing();
}

bool wizard::ProjectWorkspace::ParseIsUpToDate()
{
   return mProjectPtr ? mProjectPtr->ParseIsUpToDate() : true;
}

double wizard::ProjectWorkspace::GetLastGrammarParseTime() const noexcept
{
   return mProjectPtr ? mProjectPtr->GetLastGrammarParseTime() : std::numeric_limits<double>::max();
}

double wizard::ProjectWorkspace::GetLastInputParseTime() const noexcept
{
   return mProjectPtr ? mProjectPtr->GetLastInputParseTime() : std::numeric_limits<double>::max();
}

double wizard::ProjectWorkspace::GetLastProxyDeserializeTime() const noexcept
{
   return mProjectPtr ? mProjectPtr->GetLastProxyDeserializeTime() : std::numeric_limits<double>::max();
}

double wizard::ProjectWorkspace::GetLastProxyMergeGUI_Time() const noexcept
{
   return mProjectPtr ? mProjectPtr->GetLastProxyMergeGUI_Time() : std::numeric_limits<double>::max();
}

namespace
{
bool fileNeedsUpdated(const wizard::ProjectWorkspace::FileStat& fs)
{
   wizard::FileSignature newSig = fs.mOldSignature;
   return wizard::FileSignature::cNO_CHANGE != newSig.Update(fs.mPath);
}
} // namespace

void wizard::ProjectWorkspace::ScheduleCheckingFilesForModification()
{
   if (mParseThread.GetPhase() == ProjectParseThread::cPARSING)
   {
      mCheckFilesAfterParse = true;
   }
   else
   {
      CheckFilesForModificationInternal();
   }
}

void wizard::ProjectWorkspace::CheckFilesForModification()
{
   WaitForParsing();
   CheckFilesForModificationInternal();
}

void wizard::ProjectWorkspace::CheckFilesForModificationInternal()
{
   // It is too expensive to check each file one-by-one because of networked drives
   // Instead, files are checked concurrently and a future is checked later.

   // Get a list of file paths & previous mod dates.
   QList<FileStat>                   fileStats;
   const TextSourceCache::SourceMap& sources = mSourceCachePtr->GetSources();
   for (auto&& it : sources)
   {
      TextSource* src = it.second;
      if (src->IsLoaded())
      {
         FileStat fs = {src->FileSignatureAtLoad(), src->GetFilePath()};
         fileStats.push_back(fs);
      }
   }

   // We want many threads because each file check may have high latency
   QThreadPool::globalInstance()->setMaxThreadCount(20);
   mFileModCheckFuture.cancel();
   mFileModCheckFuture = QtConcurrent::filtered(fileStats, fileNeedsUpdated);
}

void wizard::ProjectWorkspace::ApplicationActivated()
{
   wizSignals->CheckFilesForModification();
}

wizard::Project* wizard::ProjectWorkspace::GetProject()
{
   return mProjectPtr;
}

void wizard::ProjectWorkspace::SaveAll()
{
   if (mProjectPtr)
   {
      mProjectPtr->SaveAll();
   }
}

void wizard::ProjectWorkspace::OnParseCompletion()
{
   // This is called in the GUI thread.
   if (mCheckFilesAfterParse)
   {
      mCheckFilesAfterParse = false;
      CheckFilesForModificationInternal();
   }
}

QString wizard::ProjectWorkspace::OutputPerformanceString(Project* aProjectPtr, int aTimeSpentParsing, int aTimeSpentProxying)
{
   std::stringstream ss;

   // Parse
   std::set<UtTextDocument*> documents;
   int                       nodeCount = 0, tokenCount = 0;
   Project*                  scenarioPtr = aProjectPtr;
   ParseResults*             resultsPtr  = scenarioPtr->mParseResultsPrivatePtr;
   WsfParseNode*             nodePtr     = resultsPtr->parseTree();
   while (nodePtr)
   {
      ++nodeCount;
      if (nodePtr->mValue.Valid())
      {
         documents.insert(nodePtr->mValue.mSource);
         ++tokenCount;
      }
      nodePtr = nodePtr->Next();
   }

   // Project
   size_t lineCount       = 0;
   size_t fileBytes       = 0;
   size_t allocationBytes = 0;
   for (auto&& it : documents)
   {
      lineCount += it->GetLineCount();
      fileBytes += it->Size();
      allocationBytes += it->GetText().GetAllocationSize();
   }

   ss << "Project Parse Stats\n";
   ss << "  File Count:\t\t" << documents.size() << "\n";
   ss << "  Line Count:\t\t" << lineCount << "\n";
   ss << "  Memory Used:\t" << fileBytes / 1000 << " KB\n";
   ss << "  Memory Allocated:\t" << allocationBytes / 1000 << " KB\n\n";

   ss << "Parse Tree Stats\n";
   ss << "  Node Count:\t\t" << nodeCount << "\n";
   ss << "  Token Count:\t" << tokenCount << "\n";
   ss << "  Time to Process:\t" << aTimeSpentParsing / 10.0 << " ms\n";
   ss << "  Memory Used:\t" << (sizeof(WsfParseNode) * nodeCount) / 1000 << " KB\n\n";

   // Proxy
   ss << "Proxy Stats\n";
   ss << "  Time to Process:\t" << aTimeSpentProxying / 10.0 << " ms\n";
   WsfPProxyValue root = resultsPtr->ProxyRoot();
   if (root)
   {
      auto usage = root.GetType()->MemoryUsage(root.GetDataPtr());
      ss << "  Memory Used:\t" << usage / 1000 << " KB\n";
   }
   ss << "\n";

   return QString::fromStdString(ss.str());
}

wizard::TextSource* wizard::ProjectWorkspace::FindSource(const std::string& aFilePath, bool aCreate, bool aRead)
{
   if (!mProjectPtr)
   {
      return nullptr;
   }
   if (aCreate)
   {
      TextSource* src = mSourceCachePtr->FindSource(aFilePath, aRead);
      if (!src)
      {
         src = mSourceCachePtr->GetSource(aFilePath, aRead);
         // Some sources aren't created until they are requested by the user, in this case try to initialize the source
         // fully if possible
         if (mParseThread.GetPhase() == ProjectParseThread::cNOT_WORKING)
         {
            InitializeNewSources();
         }
      }
      return src;
   }
   else
   {
      return mSourceCachePtr->FindSource(aFilePath, aRead);
   }
}

//! Attempts to open or show an already open source file.
wizard::Editor* wizard::ProjectWorkspace::OpenSource(TextSource* aSourcePtr)
{
   Editor* editPtr = nullptr;
   if (aSourcePtr)
   {
      if (!aSourcePtr->GetViews().empty())
      {
         editPtr = aSourcePtr->GetView(nullptr)->mEditorPtr;
      }
      if (!editPtr && GetProject())
      {
         TextSourceView* viewPtr = aSourcePtr->NewView(GetProject());
         if (viewPtr)
         {
            editPtr = viewPtr->mEditorPtr;
         }
      }
   }
   if (editPtr)
   {
      mEditorPtr->SetCurrent(editPtr);
   }
   return editPtr;
}

//! Open the file and make it active.
//! @param aSourcePtr         The source to open / activate.
//! @param aRecordLocation    If true, records the location for forward/backward navigation.
wizard::Editor* wizard::ProjectWorkspace::GotoFile(UtTextDocument* aSourcePtr, bool aRecordLocation)
{
   if (!mProjectPtr)
   {
      return nullptr;
   }
   Editor*         controlPtr = nullptr;
   TextSource*     sourcePtr  = static_cast<TextSource*>(aSourcePtr);
   TextSourceView* viewPtr    = sourcePtr->GetView(mProjectPtr);
   if (viewPtr)
   {
      GetEditorManager()->SetCurrent(viewPtr->mEditorPtr);
      if (aRecordLocation)
      {
         GetEditorManager()->RecordCurrentLocation();
      }
      viewPtr->mEditorPtr->setFocus();
      controlPtr = viewPtr->mEditorPtr;
      // Enables/disables fullscreen for the dock if the debug run is active
      if (mDebug)
      {
         controlPtr->GetDock()->DisableFullscreen();
      }
      else
      {
         controlPtr->GetDock()->EnableFullscreen();
      }
   }
   return controlPtr;
}

wizard::Editor* wizard::ProjectWorkspace::GotoFile(const std::string& aFilePath, bool aRecordLocation)
{
   TextSource* sourcePtr = FindSource(aFilePath, true, true);
   if (sourcePtr)
   {
      return GotoFile(sourcePtr, aRecordLocation);
   }
   return nullptr;
}

wizard::Editor* wizard::ProjectWorkspace::GotoRange(const UtTextDocumentRange& aRange)
{
   Editor* editPtr = OpenSource(static_cast<TextSource*>(aRange.mSource));
   if (editPtr)
   {
      editPtr->SetSelection(aRange);
      editPtr->RecordCurrentLocation();
      editPtr->setFocus();
   }
   return editPtr;
}

wizard::Editor* wizard::ProjectWorkspace::GotoRange(const std::string& aFilePath, size_t aLineNumber)
{
   TextSource* source = FindSource(aFilePath, false, true);
   if (source)
   {
      Editor* editorPtr = GotoFile(source, false);
      if (editorPtr)
      {
         editorPtr->GoToLine(aLineNumber);
         editorPtr->RecordCurrentLocation();
         return editorPtr;
      }
   }
   return nullptr;
}

void wizard::ProjectWorkspace::NotifyOfSourceChange(TextSource* aSourcePtr)
{
   // A file has changed.  Any scenario including the file must be reparsed, invalidate the parse results now.
   if (mProjectPtr)
   {
      ParseResults* resultsPtr = mProjectPtr->GetParseResults();
      if (resultsPtr && resultsPtr->IncludesFile(aSourcePtr))
      {
         mProjectPtr->TakeParseResults();
      }
   }
   RequestReparseWsf(aSourcePtr);
}

void wizard::ProjectWorkspace::SetModified(bool aUpdateQuick)
{
   ++mModificationSequenceNumber;
   if (aUpdateQuick)
   {
      mFullUpdateQuick = true;
   }
   ++mCurrentTaskId;
   mParseThread.Abort();
}

void wizard::ProjectWorkspace::TouchParse(bool aUpdateQuick)
{
   mTouched = true;
   SetModified(aUpdateQuick);
}

void wizard::ProjectWorkspace::RestyleViews()
{
   QList<TextSource*> textSources;

   for (auto&& view : mOpenViews)
   {
      textSources.append(view->mSourcePtr);
   }

   GetEditorManager()->ProjectClosing(GetProject());

   while (!textSources.isEmpty())
   {
      textSources.takeFirst()->NewView(mProjectPtr);
   }
}

void wizard::ProjectWorkspace::StartExecution(WsfExecution* aExecutionPtr)
{
   mExecutions.push_back(aExecutionPtr);
   connect(aExecutionPtr, &WsfExecution::destroyed, this, &ProjectWorkspace::ExecutionDestroyed);
   StartingExecution(aExecutionPtr);
   auto* proj = Project::Instance();
   aExecutionPtr->Start(proj->GetStartupFiles());
}

void wizard::ProjectWorkspace::StopAllExecutions()
{
   for (auto&& exe : mExecutions)
   {
      exe->Kill();
   }
}

void wizard::ProjectWorkspace::CreateA_NewFile(Project* aProject, const UtPath& aDir)
{
   mNewFileDialogPtr->Open(Project::Instance(), aDir);
}

void wizard::ProjectWorkspace::ExecutionDestroyed(QObject* aObjectPtr)
{
   WsfExecution* execPtr = static_cast<WsfExecution*>(aObjectPtr);
   for (size_t i = 0; i < mExecutions.size(); ++i)
   {
      if (mExecutions[i] == execPtr)
      {
         mExecutions.erase(mExecutions.begin() + i);
      }
   }
   // Check for modified files and re-parse
   CheckFilesForModification();
   emit wizSignals->FileCheckRequested();
   ReparseAll();
}

void wizard::ProjectWorkspace::customEvent(QEvent* aEventPtr)
{
   if (aEventPtr->type() == ThreadNotifyEvent::cTHREAD_NOTIFY_EVENT_ID)
   {
      ++mPreventUpdates;
      ThreadNotifyEvent* evtPtr = static_cast<ThreadNotifyEvent*>(aEventPtr);
      if (evtPtr->mTaskId == mCurrentTaskId && mProjectPtr == evtPtr->mProjectPtr)
      {
         if (evtPtr->mNotificationType == ThreadNotifyEvent::cFAILED)
         {
            // Abort the worker thread and show a message
            mParseThread.Abort();
            QMessageBox::critical(nullptr, "Parse Error", "Failed to allocate memory!", QMessageBox::Ok);

            // This is abrupt and may cause a crash on the Qt side
            exit(1);
         }
         else if (evtPtr->mNotificationType == ThreadNotifyEvent::cPARSE_RESULTS_READY)
         {
            InitializeNewSources();
            PerformanceResult(evtPtr->mProjectPtr, "parse", evtPtr->mTimeToCompleteParseMs);
            evtPtr->mProjectPtr->GiveParseResults();
            evtPtr->mProjectPtr->P_NotifyParseUpdated();
            // Discard history of changes we don't need
            TextSource::SetObsoleteSequenceNumber(mProjectPtr->GetMinimumSequenceNumber());
         }
         else if (evtPtr->mNotificationType == ThreadNotifyEvent::cPROXY_READY)
         {
            if (mProjectPtr == evtPtr->mProjectPtr)
            {
               PerformanceResult(evtPtr->mProjectPtr, "proxySync", evtPtr->mTimeToCompleteProxyMs);
               evtPtr->mProjectPtr->ProxyReady(std::move(evtPtr->mProxy), std::move(evtPtr->mProxyHash));
            }
         }
         else if (evtPtr->mNotificationType == ThreadNotifyEvent::cWORK_COMPLETE)
         {
            // No action at this point
         }
         else if (evtPtr->mNotificationType == ThreadNotifyEvent::cPERFORMANCE_TEST_RESULTS_READY)
         {
            ParseStatisticsReady(evtPtr->mProjectPtr,
                                 OutputPerformanceString(evtPtr->mProjectPtr,
                                                         evtPtr->mTimeToCompleteParseMs,
                                                         evtPtr->mTimeToCompleteProxyMs));
         }
         else if (evtPtr->mNotificationType == ThreadNotifyEvent::cSERIALIZE_COMPLETED)
         {
            std::vector<UtTextDocument*> changedDocuments;
            evtPtr->mSerializeToComplete->GetChangedFiles(changedDocuments);
            for (auto&& doc : changedDocuments)
            {
               TextSource* textSource = static_cast<TextSource*>(doc);
               OpenSource(textSource);
            }

            wizSignals->emitEditModeSwitch(Vi::cEDIT_MODE_TEXT);

            evtPtr->mSerializeToComplete->Finalize();

            for (auto&& doc : changedDocuments)
            {
               TextSource* textSource = static_cast<TextSource*>(doc);
               textSource->ReloadViews();
            }
            ReparseAll();

            mProjectPtr->GetRevisionStore().RequestBackup();
            mParseThread.SetComplete();
         }

         // parse and proxy may update styling
         if (evtPtr->mNotificationType == ThreadNotifyEvent::cPARSE_RESULTS_READY ||
             evtPtr->mNotificationType == ThreadNotifyEvent::cPROXY_READY)
         {
            for (auto&& view : mOpenViews)
            {
               if (view->mEditorPtr->isVisible())
               {
                  view->mEditorPtr->UpdateStyle();
               }
            }
         }
      }
      --mPreventUpdates;
   }
   else
   {
      QObject::customEvent(aEventPtr);
   }
}

void wizard::ProjectWorkspace::InitializeNewSources()
{
   if (!mSourceCachePtr->mNewSources.empty())
   {
      std::set<TextSource*> newSources;
      std::swap(newSources, mSourceCachePtr->mNewSources);
      for (auto&& it : newSources)
      {
         TextSource* src = it;
         emit        NewTextSource(src);
         src->AddedToWorkspace();
      }
   }
}

std::vector<wizard::TextSource*> wizard::ProjectWorkspace::GetSourceList()
{
   const TextSourceCache::SourceMap& sources = mSourceCachePtr->GetSources();
   std::vector<TextSource*>          srcList;
   for (auto&& it : sources)
   {
      srcList.push_back(it.second);
   }
   return srcList;
}

wizard::ProjectWorkspace* wizard::ProjectWorkspace::Instance()
{
   return sInstancePtr;
}

void wizard::ProjectWorkspace::BeginSerialize()
{
   WaitForParsing();

   // UtTextDocument isn't read-only thread-safe due to the way GetPointer() works.
   // Calling GetPointer() here will prime the documents for thread-safe read-only use.
   const TextSourceCache::SourceMap& sources = mSourceCachePtr->GetSources();
   for (auto&& it : sources)
   {
      TextSource* src = it.second;
      if (src->IsLoaded())
      {
         src->GetPointer();
      }
   }

   // wizSignals->emitEditModeSwitch(Vi::cEDIT_MODE_SWITCHING_TO_TEXT);

   ProjectParseThread::Task task;
   task.mTaskType    = ProjectParseThread::Task::cTT_SERIALIZE;
   task.mScenarioPtr = mProjectPtr;
   task.mTaskId      = ++mCurrentTaskId;
   mParseThread.SetTask(task);
}

bool wizard::ProjectWorkspace::DelayedFileModificationCheck()
{
   if (!mFileModCheckFuture.isCanceled() && mFileModCheckFuture.isFinished())
   {
      bool hasMods = false;
      for (int i = 0; i < mFileModCheckFuture.resultCount(); ++i)
      {
         TextSource* src = FindSource(mFileModCheckFuture.resultAt(i).mPath.GetSystemPath(), false, true);
         if (src)
         {
            bool          alwaysFullyReload = true;
            ParseResults* resultsPtr        = mProjectPtr->GetParseResults();
            if (resultsPtr && resultsPtr->IncludesFile(src))
            {
               // If the file is included in the current project don't automatically do a full reload
               alwaysFullyReload = false;
            }
            hasMods = src->CheckForFileModifications(alwaysFullyReload) || hasMods;
         }
      }
      mFileModCheckFuture.cancel();
      if (hasMods)
      {
         ReparseAll();
      }
   }
   return mFileModCheckFuture.isCanceled();
}

bool wizard::ProjectWorkspace::IsProcessing() const
{
   return (mParseThread.GetPhase() != ProjectParseThread::cNOT_WORKING);
}

void wizard::ProjectWorkspace::NewProject()
{
   // Use the location of the last opened project
   // otherwise use the user's home directory
   // else use the location of the executable
   QString newLocationDir = QDir::currentPath();
   if (!QDir::current().exists())
   {
      // Absolute path of the user's home directory
      newLocationDir = QDir::toNativeSeparators(QDir::homePath()); // Home directory
      // openLocationDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);   // Documents directory

      if (newLocationDir.isEmpty())
      {
         newLocationDir = QString::fromStdString(UtPath::GetExePath().GetSystemPath());
      }
   }

   // Get the new project file name
   QString fileName = QFileDialog::getSaveFileName(nullptr, "New Project", newLocationDir, "AFSIM Project (*.afproj)");
   if (!fileName.isEmpty() &&
       // set the working directory to the new project's directory
       wizRunMgr.SetWorkingDirectoryToProject(fileName))
   {
      UtPath fileNamePath(fileName.toStdString());
      UtPath filePath = fileNamePath;
      filePath.Up();
      Project* projectPtr = new Project(this);
      projectPtr->SetProjectLocation(filePath.GetSystemPath(), fileNamePath);
   }
}

void wizard::ProjectWorkspace::EmitProxyUpdated(ProxyMerge* aProxyMerge)
{
   emit ProxyUpdated(aProxyMerge);
}

void wizard::ProjectWorkspace::OnChangeDebug(bool aIsActive)
{
   mDebug = aIsActive;
}
