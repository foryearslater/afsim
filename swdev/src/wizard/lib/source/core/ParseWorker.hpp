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

#ifndef PARSEWORKER_HPP
#define PARSEWORKER_HPP

#include <map>
#include <memory>
#include <vector>

#include <QEvent>

#include "ProxyMerge.hpp"
#include "ThreadNotifyEvent.hpp"
#include "UtMemory.hpp"
#include "UtPath.hpp"
#include "WsfPProxy.hpp"
#include "WsfPProxyRegistry.hpp"
#include "WsfPProxySerialize.hpp"
#include "WsfPProxyType.hpp"
#include "WsfParseDefinitions.hpp"
#include "WsfParseNode.hpp"

class WsfParseSourceInclude;
class WsfParser;

namespace wizard
{
class ParseResults;
class Project;
class WsfScriptTypes;

/////////////////////////////////////////////////////////////////////////////////
// ParseWorker -- Manages parse related tasks which are done in a helper thread
//   For thread safety, this class only uses data that it owns.
//   Exceptions:
//     ParseDefinitions -- read-only, shared from WsfExe
//
//
/////////////////////////////////////////////////////////////////////////////////
class ParseWorker
{
public:
   friend class ViProjectParseThread;
   const static int cMAX_ERROR_COUNT = 100;

   explicit ParseWorker(Project* aProjectPtr);
   ~ParseWorker();

   // methods to be called from the main thread:
   //! Primes the worker to begin a new parse
   void Start(ParseResults* aResultsPtr, int aTaskId, bool aTestingParse);

   std::vector<UtTextDocument*> StartSerialize(int                                         aTaskId,
                                               const std::map<WsfPProxyPath, std::string>& aRenameMap,
                                               const std::map<WsfPProxyPath, std::string>& aSaveFiles);

   void UpdateParseDefinitions();

   enum Result
   {
      cFAILED,
      cABORTED,
      cPARSE_COMPLETE,
      cPROXY_SYNC_COMPLETE,
      cPROXY_SERIALIZE_COMPLETE,
      cWORK_DONE
   };

   // This method may be called in another thread.
   // Result Run();
   Result Execute();

   Result ExecuteParse();

   volatile bool* mAbortSwitch;

   bool IsExecuting() { return mIsExecuting; }

   // ViProxyDataTemplated* TakeProxyPtr() { ViProxyDataTemplated* proxyPtr = mProxyPtr; mProxyPtr = 0; return proxyPtr; }

private:
   enum TaskType
   {
      cTASK_PARSE,
      cTASK_SERIALIZE
   };

   struct ParseState
   {
      ParseState()
         : parseTreePtr(nullptr)
      {
      }
      WsfParseNode* parseTreePtr;
   };

   // The following members are copied here by the main thread
   struct TaskData
   {
      TaskData()
         : mTaskType(cTASK_PARSE)
         , mMainFiles()
         , mParseResultsPtr(nullptr)
         , mSequenceNumber(-1)
         , mTaskId(-1)
         , mTestMode(false)
      {
      }
      TaskType            mTaskType;
      std::vector<UtPath> mMainFiles;
      ParseResults*       mParseResultsPtr;
      long long           mSequenceNumber;
      int                 mTaskId;
      bool                mTestMode;
      WsfPProxyValue      proxyToSerialize;
   };

   void   SetDefinitions(const std::shared_ptr<WsfParseDefinitions>& aDefinitionsPtr);
   Result ParsePhase(ParseState& parseState);
   Result ProxySyncPhase(ParseState& parseState, const std::unique_ptr<WsfPProxy>& aProxyState);
   void   UpdateIncludeData(WsfParseSourceInclude*                             aIncludePtr,
                            std::map<UtTextDocument*, WsfParseSourceInclude*>& aFirstIncludes);
   Result ExecuteSerialize();

   TaskData mTaskData;

   // Parse definitions on loan from WsfExe.
   std::shared_ptr<WsfParseDefinitions> mDefinitions;
   std::shared_ptr<WsfPProxyRegistry>   mProxyRegistry;

   Project*                             mProjectPtr;
   std::unique_ptr<WsfParser>           mParserPtr;
   WsfScriptTypes*                      mScriptDataBuildPtr;
   WsfParseNodePool                     mRootNodePool;
   std::map<WsfPProxyPath, std::string> mSaveFiles;
   volatile bool                        mAborting;
   bool                                 mFirstDefinitions;
   volatile bool                        mIsExecuting;
};
} // namespace wizard
#endif
