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

#include "ParseWorker.hpp"

#include <ostream>

#include <QApplication>
#include <QTime>

#include "ParseResults.hpp"
#include "Project.hpp"
#include "ProjectWorkspace.hpp"
#include "ProxyMerge.hpp"
#include "TextSource.hpp"
#include "TextSourceCache.hpp"
#include "WsfExe.hpp"
#include "WsfPProxyDeserialize.hpp"
#include "WsfPProxySerialize.hpp"
#include "WsfParseIndex.hpp"
#include "WsfParseSourceInclude.hpp"
#include "WsfParser.hpp"
#include "WsfScriptData.hpp"

wizard::ParseWorker::ParseWorker(Project* aProjectPtr)
   : mDefinitions(nullptr)
   , mProjectPtr(aProjectPtr)
   , mParserPtr(ut::make_unique<WsfParser>(aProjectPtr->SourceProvider(), aProjectPtr->WorkingDirectory().GetSystemPath()))
   , mScriptDataBuildPtr(nullptr)
   , mAborting(false)
   , mFirstDefinitions(true)
   , mIsExecuting(false)
{
   mAbortSwitch = &mAborting;
}

wizard::ParseWorker::~ParseWorker()
{
   // The definitions are owned elsewhere, so make sure they aren't deleted here.
   SetDefinitions(nullptr);
   // mParserPtr->SwapDefinitions(nullDefs);
}


// After setting a task on the parse thread and then
// tells the parse worker to start
void wizard::ParseWorker::Start(ParseResults* aResultsPtr, int aTaskId, bool aTestingParse)
{
   TaskData newTask;
   newTask.mParseResultsPtr                  = aResultsPtr;
   newTask.mParseResultsPtr->mSequenceNumber = TextSource::GetNextSequenceNumber() - 1;

   newTask.mMainFiles = mProjectPtr->GetStartupFiles();

   mProjectPtr->P_GrammarParsingStarted();
   // Update the parse definitions from the selected executable
   UpdateParseDefinitions();
   mProjectPtr->P_CompleteGrammarParsing();

   mParserPtr->SetWorkingDirectory(mProjectPtr->WorkingDirectory());
   newTask.mTaskId   = aTaskId;
   newTask.mTestMode = aTestingParse;
   newTask.mTaskType = cTASK_PARSE;
   mTaskData         = newTask;
}


void wizard::ParseWorker::UpdateIncludeData(WsfParseSourceInclude*                             aIncludePtr,
                                            std::map<UtTextDocument*, WsfParseSourceInclude*>& aFirstIncludes)
{
   if (aIncludePtr == nullptr)
   {
      return;
   }
   if (aIncludePtr->mIncludeCount == 1)
   {
      aFirstIncludes[aIncludePtr->mSourcePtr] = aIncludePtr;
   }
   for (auto&& it : aIncludePtr->mEntries)
   {
      if (it.mIncludePtr != nullptr)
      {
         UpdateIncludeData(it.mIncludePtr.get(), aFirstIncludes);
      }
   }
}

wizard::ParseWorker::Result wizard::ParseWorker::Execute()
{
   mIsExecuting = true;
   try
   {
      switch (mTaskData.mTaskType)
      {
      case cTASK_PARSE:
         return ExecuteParse();
      case cTASK_SERIALIZE:
         return ExecuteSerialize();
      }
   }
   catch (const std::bad_alloc& e)
   {
      // Notify the main thread that parsing has failed
      std::cerr << e.what() << std::endl;
      ThreadNotifyEvent* evt = new ThreadNotifyEvent(mProjectPtr, ThreadNotifyEvent::cFAILED);
      evt->mTaskId           = mTaskData.mTaskId;
      QApplication::postEvent(mProjectPtr->GetWorkspace(), evt);
   }
   // All cases should be handled
   return cFAILED;
}

wizard::ParseWorker::Result wizard::ParseWorker::ParsePhase(ParseState& parseState)
{
   // Reset the parser back to default stat
   mParserPtr->Reset();

   // Get the startup files form the current project
   std::vector<UtPath> mainFiles = mProjectPtr->GetStartupFiles();

   if (!mainFiles.empty())
   {
      // Push the first file
      mParserPtr->PushSource(mainFiles[0].GetSystemPath(), true);

      // Add the rest of project's start-up files to the deferred list
      for (size_t i = 1; i < mainFiles.size(); ++i)
      {
         mParserPtr->PushDeferredSource(mainFiles[i]);
      }
   }

   // Get root information
   WsfParseRule*          rootReaderPtr  = mParserPtr->GetDefinitions()->GetRoot();
   WsfParseSourceInclude* rootIncludePtr = mParserPtr->GetCurrentInclude();
   WsfParseNode*          rootNodePtr    = mRootNodePool.NewNode(nullptr, "root-commands");

   // Create a stub node for quick insertions to the linked list of siblings
   WsfParseNode* nextNodePtr = mRootNodePool.NewNode();
   rootNodePtr->Add(nextNodePtr);

   // Parse
   for (; !*mAbortSwitch;)
   {
      UtTextDocumentRange token = mParserPtr->ReadWord();
      if (token.Valid())
      {
         mParserPtr->UndoRead();
         WsfParseNode* commandTreePtr = nullptr;
         if (rootReaderPtr->Read(*mParserPtr, commandTreePtr) && commandTreePtr != nullptr)
         {
            nextNodePtr->InsertAfter(commandTreePtr);
            nextNodePtr = commandTreePtr;
         }
         else
         {
            UtTextDocumentRange range = mParserPtr->ReadWord();
            if (mParserPtr->GetErrors().size() < (size_t)cMAX_ERROR_COUNT)
            {
               mParserPtr->AddError(rootReaderPtr, range);
            }
         }
      }
      else
      {
         break;
      }
   }

   mParserPtr->ResolveDelayLoad();

   // Check for an abort
   if (*mAbortSwitch)
   {
      mParserPtr->Reset();
      mTaskData.mParseResultsPtr->Clear();
      return cABORTED;
   }

   // Remove stub node
   rootNodePtr->Down()->Detatch();

   // mIncludes = mParserPtr->GetParseIndex()->GetIncludes();
   // mIncludePaths = mParserPtr->GetParseIndex()->GetIncludePath();
   {
      std::map<UtTextDocument*, WsfParseSourceInclude*> firstIncludes;
      UpdateIncludeData(rootIncludePtr, firstIncludes);

      WsfParseNode* lastNodePtr = nextNodePtr;
      if (lastNodePtr)
      {
         lastNodePtr = lastNodePtr->GetRightmostLeaf();
      }
      mTaskData.mParseResultsPtr
         ->AssignResults(mProjectPtr, mParserPtr.get(), rootIncludePtr, rootNodePtr, lastNodePtr, firstIncludes);
   }

   // Check for an abort
   if (*mAbortSwitch)
   {
      mParserPtr->Reset();
      mTaskData.mParseResultsPtr->Clear();
      return cABORTED;
   }

   mParserPtr->FinalizeParseTree(rootNodePtr, mTaskData.mParseResultsPtr->mFileTransitionNodes);
   mTaskData.mParseResultsPtr->IndexNames();

   parseState.parseTreePtr = mTaskData.mParseResultsPtr->mParseTreePtr;

   if (mScriptDataBuildPtr && parseState.parseTreePtr)
   {
      WsfScriptData* scriptDataPtr =
         new WsfScriptData(mScriptDataBuildPtr, mParserPtr->GetParseIndex(), mParserPtr->GetDefinitions());
      WsfScriptParser parser(mScriptDataBuildPtr, scriptDataPtr);
      parser.Rebuild(parseState.parseTreePtr, mParserPtr->GetParseIndex()->mUserTypes);
      if (!mTaskData.mTestMode)
      {
         mTaskData.mParseResultsPtr->mScriptDataPtr  = scriptDataPtr;
         mTaskData.mParseResultsPtr->mScriptTypesPtr = mScriptDataBuildPtr;
         mScriptDataBuildPtr                         = nullptr;
      }
   }

   return cPARSE_COMPLETE;
}

wizard::ParseWorker::Result wizard::ParseWorker::ProxySyncPhase(ParseState&                       parseState,
                                                                const std::unique_ptr<WsfPProxy>& aProxy)
{
   if (parseState.parseTreePtr && mProxyRegistry != nullptr)
   {
      aProxy->mIndex                     = ut::make_unique<WsfPProxyIndex>();
      aProxy->mIndex->mRecordCurrentPath = true;
      aProxy->Registry(mProxyRegistry);
      aProxy->mBasicRoot = mProxyRegistry->GetBasicRoot();
      aProxy->mRoot      = WsfPProxyValue::ConstructNew(mProxyRegistry->GetType("root"));

      WsfPProxyUndoValue proxyUndoRoot(aProxy->mRoot);
      proxyUndoRoot.DisableUndo();

      WsfPProxyDeserialize des(mProxyRegistry.get(), aProxy->mIndex.get());
      des.mAbortSwitch = mAbortSwitch;
      des.Deserialize(proxyUndoRoot, parseState.parseTreePtr);

      aProxy->BuildPathMap();

      if (!*mAbortSwitch)
      {
         aProxy->mIndex->BuildReverseIndex();
         return cPROXY_SYNC_COMPLETE;
      }
      return cABORTED;
   }
   return cFAILED;
}

void wizard::ParseWorker::SetDefinitions(const std::shared_ptr<WsfParseDefinitions>& aDefinitionsPtr)
{
   if (aDefinitionsPtr != mDefinitions)
   {
      WsfParseDefinitions* newDefs = aDefinitionsPtr.get();
      mParserPtr->SwapDefinitions(newDefs);
      if (mFirstDefinitions)
      {
         delete newDefs;
         mFirstDefinitions = false;
      }
      mDefinitions = aDefinitionsPtr;
   }
}

std::vector<UtTextDocument*> wizard::ParseWorker::StartSerialize(int                                         aTaskId,
                                                                 const std::map<WsfPProxyPath, std::string>& aRenameMap,
                                                                 const std::map<WsfPProxyPath, std::string>& aSaveFiles)
{
   TaskData newTask;
   newTask.mTaskId    = aTaskId;
   newTask.mMainFiles = mProjectPtr->GetStartupFiles();

   UpdateParseDefinitions();

   mParserPtr->SetWorkingDirectory(mProjectPtr->WorkingDirectory());
   newTask.proxyToSerialize.Delete();
   newTask.proxyToSerialize = mProjectPtr->GetParseResults()->ProxyRoot().Copy();
   mProjectPtr->TakeParseResults();
   newTask.mTaskType = cTASK_SERIALIZE;

   mTaskData = newTask;

   std::vector<UtTextDocument*> changedFiles;
   if (!aRenameMap.empty())
   {
      auto serializerPtr = ut::make_unique<WsfPProxySerialize>();
      serializerPtr->SetRenameMap(aRenameMap);
      serializerPtr->Initialize(mProxyRegistry.get(), mParserPtr.get(), mTaskData.mMainFiles);
      serializerPtr->PreSerialize(changedFiles);
   }
   mSaveFiles = aSaveFiles;
   return changedFiles;
}

//! Updates the parse definitions from the project's selected executable
void wizard::ParseWorker::UpdateParseDefinitions()
{
   WsfExe* exePtr = mProjectPtr->GetExe();
   if (exePtr != nullptr)
   {
      SetDefinitions(exePtr->GetParseDefinitions());
      mProxyRegistry = exePtr->GetProxyRegistry();
      if (!mScriptDataBuildPtr)
      {
         mScriptDataBuildPtr = exePtr->TakeScriptTypes();
      }
   }
}

wizard::ParseWorker::Result wizard::ParseWorker::ExecuteParse()
{
   int timeSpentParsing(0);
   int timeSpentProxying(0);

   // Treat the last iteration as a normal iteration when testing
   bool  testIteration = mTaskData.mTestMode;
   QTime time1         = QTime::currentTime();
   QTime time2;

   // Do the parsing
   ParseState parseState;
   mProjectPtr->P_InputParsingStarted();
   ParseWorker::Result res = ParsePhase(parseState);
   mProjectPtr->P_CompleteInputParsing();

   std::unique_ptr<WsfPProxy> proxyStatePtr = nullptr;
   bool                       workLeft      = true;
   while (!*mAbortSwitch && workLeft)
   {
      switch (res)
      {
      case ParseWorker::cFAILED:
      case ParseWorker::cABORTED:
         workLeft = false;
         break;
      case ParseWorker::cPARSE_COMPLETE:
      {
         time2 = QTime::currentTime();
         timeSpentParsing += time1.msecsTo(time2);
         time1 = time2;

         proxyStatePtr = ut::make_unique<WsfPProxy>();
         mProjectPtr->P_ProxyDeserializeStarted();
         res = ProxySyncPhase(parseState, proxyStatePtr);
         mProjectPtr->P_CompleteProxyDeserialize();

         if (!testIteration)
         {
            ThreadNotifyEvent* evt      = new ThreadNotifyEvent(mProjectPtr, ThreadNotifyEvent::cPARSE_RESULTS_READY);
            evt->mTaskId                = mTaskData.mTaskId;
            evt->mTimeToCompleteParseMs = timeSpentParsing;
            QApplication::postEvent(mProjectPtr->GetWorkspace(), evt);
         }

         break;
      }
      case ParseWorker::cPROXY_SYNC_COMPLETE:
      {
         time2 = QTime::currentTime();
         timeSpentProxying += time1.msecsTo(time2);
         time1 = time2;
         if (!testIteration)
         {
            ThreadNotifyEvent* evt = new ThreadNotifyEvent(mProjectPtr, ThreadNotifyEvent::cPROXY_READY);

            evt->mTaskId                = mTaskData.mTaskId;
            evt->mTimeToCompleteProxyMs = timeSpentProxying;
            evt->mProxyHash             = ut::make_unique<ProxyHash>(proxyStatePtr->mRoot);
            evt->mProxy                 = std::move(proxyStatePtr);

            QApplication::postEvent(mProjectPtr->GetWorkspace(), evt);
         }
         res = cWORK_DONE;
         break;
      }
      case ParseWorker::cWORK_DONE:
      {
         mProjectPtr->P_CompleteAllParsing();
         if (!testIteration)
         {
            ThreadNotifyEvent* evt = new ThreadNotifyEvent(mProjectPtr, ThreadNotifyEvent::cWORK_COMPLETE);
            evt->mTaskId           = mTaskData.mTaskId;
            QApplication::postEvent(mProjectPtr->GetWorkspace(), evt);
         }

         workLeft = false;
         break;
      }
      default:
         break;
      }
   }

   // if (mTaskData.mTestMode && !*mAbortSwitch)
   if (!*mAbortSwitch)
   {
      ThreadNotifyEvent* evt = new ThreadNotifyEvent(mProjectPtr, ThreadNotifyEvent::cPERFORMANCE_TEST_RESULTS_READY);
      evt->mTaskId           = mTaskData.mTaskId;
      evt->mTimeToCompleteParseMs = timeSpentParsing;
      evt->mTimeToCompleteProxyMs = timeSpentProxying;
      QApplication::postEvent(mProjectPtr->GetWorkspace(), evt);
      // aTaskPtr->mPerformanceTestResults = OutputPerformanceString(aTaskPtr, timeSpentParsing, timeSpentProxying);
   }

   mIsExecuting = false;

   return res;
}

wizard::ParseWorker::Result wizard::ParseWorker::ExecuteSerialize()
{
   auto serializerPtr = ut::make_unique<WsfPProxySerialize>();
   serializerPtr->Initialize(mProxyRegistry.get(), mParserPtr.get(), mTaskData.mMainFiles);
   serializerPtr->SetSaveFiles(mSaveFiles);
   serializerPtr->Serialize(mTaskData.proxyToSerialize);
   mTaskData.proxyToSerialize.Delete();

   ThreadNotifyEvent* evt    = new ThreadNotifyEvent(mProjectPtr, ThreadNotifyEvent::cSERIALIZE_COMPLETED);
   evt->mTaskId              = mTaskData.mTaskId;
   evt->mSerializeToComplete = std::move(serializerPtr);
   QApplication::postEvent(mProjectPtr->GetWorkspace(), evt);

   return cPROXY_SERIALIZE_COMPLETE;
}
