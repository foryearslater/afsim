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

#ifndef PROJECTPARSETHREAD_HPP
#define PROJECTPARSETHREAD_HPP

#include "Project.hpp"
#include "UtSemaphore.hpp"
#include "UtThread.hpp"

namespace wizard
{
//! Thread for parsing the WSF source in the background.
class ProjectParseThread : public UtThread
{
public:
   ProjectParseThread();
   ~ProjectParseThread() override = default;
   void JoinThread();

   enum TaskPhase
   {
      //! No parsing is being requested.
      cNOT_WORKING,
      //! Parsing has started.  Transition to this phase is always done by the main thread.
      cPARSING,
      //! Parsing is complete.  Transition to this phase is always done by parsing thread.
      cPARSE_DONE,

      cSERIALIZING,

      cSERIALIZE_DONE
   };

   struct Task
   {
      Task()
         : mTaskType(cTT_PARSE)
         , mScenarioPtr(nullptr)
         , mTestingParse(false)
         , mTaskId(-1)
         , mModificationSequenceNumber(-1)
      {
      }

      enum TaskType
      {
         cTT_PARSE,
         cTT_SERIALIZE
      };
      TaskType mTaskType;
      // Inputs:
      Project* mScenarioPtr;
      //! True if running a parse test.
      bool mTestingParse;
      // Outputs:
      std::string mPerformanceTestResults;

      int mTaskId;

      int mModificationSequenceNumber;
   };

   void Run() override;

   void Abort();
   bool IsAborting() const { return mAborting; }
   void SetTask(const Task& aTask);
   void SetComplete();
   void SetRunOnCompletion(std::function<void()> onCompletion) { mRunOnCompletion = onCompletion; }

   TaskPhase GetPhase() const { return mParsePhase; }

private:
   void Reparse(Task* aTaskPtr);

   void Serialize();

   UtSemaphore        mSem;
   volatile TaskPhase mParsePhase;

   //! True if the thread should exit
   volatile bool mThreadExit;

   volatile bool         mAborting;
   Task                  mCurrentTask;
   std::function<void()> mRunOnCompletion = []() { /* no-op */ };
};
} // namespace wizard
#endif
