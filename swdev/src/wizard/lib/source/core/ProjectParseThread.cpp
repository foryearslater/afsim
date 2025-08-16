// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2016 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "ProjectParseThread.hpp"

#include "ParseResults.hpp"
#include "ParseWorker.hpp"
#include "TextSource.hpp"
#include "UtSleep.hpp"

wizard::ProjectParseThread::ProjectParseThread()
   : mSem(1)
   , mParsePhase(cNOT_WORKING)
   , mThreadExit(false)
   , mAborting(false)
{
}

void wizard::ProjectParseThread::JoinThread()
{
   Abort();
   mThreadExit = true;
   mSem.Release(1);
   Join();
}

void wizard::ProjectParseThread::Run()
{
   while (!mThreadExit)
   {
      switch (mParsePhase)
      {
      case cPARSING:
      {
         Reparse(&mCurrentTask);

         // Notify the sources that the parsing is complete, this will allow syntax highlighting to update
         mParsePhase = cPARSE_DONE;
         mSem.Acquire();
         break;
      }
      case cSERIALIZING:
      {
         Serialize();
         mParsePhase = cSERIALIZE_DONE;
         mSem.Acquire();
         break;
      }
      default:
         UtSleep::Sleep(.01);
         break;
      }
   }
}

void wizard::ProjectParseThread::Reparse(Task* aTaskPtr)
{
   if (aTaskPtr->mScenarioPtr)
   {
      ParseWorker* workerPtr = aTaskPtr->mScenarioPtr->GetParseWorker();
      // workerPtr->Start(workerPtr->mParseResultsPtr, aTaskPtr->mTaskId, aTaskPtr->mTestingParse);
      workerPtr->mAbortSwitch = &mAborting;
      if (workerPtr->Execute() == ParseWorker::cWORK_DONE)
      {
         // success
      }
      else
      {
         aTaskPtr->mScenarioPtr->P_ParsingAborted();
      }
   }
}

void wizard::ProjectParseThread::Abort()
{
   mAborting = true;
}

void wizard::ProjectParseThread::SetTask(const Task& aTask)
{
   assert(mParsePhase == cNOT_WORKING);
   mCurrentTask = aTask;
   switch (mCurrentTask.mTaskType)
   {
   case Task::cTT_PARSE:
   {
      mAborting = false;
      if (aTask.mScenarioPtr)
      {
         aTask.mScenarioPtr->GetParseWorker()->Start(aTask.mScenarioPtr->TakeParseResults(),
                                                     mCurrentTask.mTaskId,
                                                     mCurrentTask.mTestingParse);
      }
      mParsePhase = cPARSING;
      mSem.Release();
   }
   break;
   case Task::cTT_SERIALIZE:
   {
      mAborting = false;
      // should be only one scenario...
      if (aTask.mScenarioPtr && aTask.mScenarioPtr->GetParseResults())
      {
         std::vector<UtTextDocument*> changedDocuments = aTask.mScenarioPtr->GetParseWorker()->StartSerialize(
            mCurrentTask.mTaskId,
            aTask.mScenarioPtr->GetParseResults()->Proxy()->RenameMappingOldToNew(),
            aTask.mScenarioPtr->GetParseResults()->Proxy()->SaveFiles());
         for (auto&& doc : changedDocuments)
         {
            std::cout << doc->GetText().GetPointer();
            TextSource* textSource = static_cast<TextSource*>(doc);

            textSource->ReloadViews();
         }
      }
      mParsePhase = cSERIALIZING;
      mSem.Release();
   }
   break;
   }
}

void wizard::ProjectParseThread::SetComplete()
{
   assert(mParsePhase == cPARSE_DONE || mParsePhase == cSERIALIZE_DONE);
   mParsePhase = cNOT_WORKING;
   mAborting   = false;
   mRunOnCompletion();
}

void wizard::ProjectParseThread::Serialize()
{
   if (mCurrentTask.mScenarioPtr)
   {
      ParseWorker* workerPtr = mCurrentTask.mScenarioPtr->GetParseWorker();
      // workerPtr->Start(workerPtr->mParseResultsPtr, aTaskPtr->mTaskId, aTaskPtr->mTestingParse);
      workerPtr->mAbortSwitch = &mAborting;
      if (workerPtr->Execute() == ParseWorker::cPROXY_SERIALIZE_COMPLETE)
      {
         // success
      }
      else
      {
         // fail
      }
   }
}
