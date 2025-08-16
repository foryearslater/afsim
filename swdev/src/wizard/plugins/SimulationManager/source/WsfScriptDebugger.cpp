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

#include "WsfScriptDebugger.hpp"

#include <algorithm>
#include <iterator>

#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>
#include <QToolTip>
#include <QTreeView>

#include "Editor.hpp"
#include "EditorManager.hpp"
#include "Environment.hpp"
#include "ParseResults.hpp"
#include "Project.hpp"
#include "ProjectWorkspace.hpp"
#include "Signals.hpp"
#include "SimulationManagerPlugin.hpp"
#include "TextSource.hpp"
#include "UtCast.hpp"
#include "WkfMainWindow.hpp"
#include "WsfConnector.hpp"
#include "WsfDebugValueModel.hpp"
#include "WsfEditor.hpp"
#include "WsfScriptDebuggerWatchControl.hpp"
#include "WsfScriptLookup.hpp"
#include "xio/WsfXIO_Interface.hpp"
#include "xio/WsfXIO_Request.hpp"

namespace SimulationManager
{
class DebugRequest : public WsfXIO_Request
{
public:
   WsfScriptDebugger* mDebuggerPtr;
   bool               mStarted;
   DebugRequest(WsfXIO_Connection* aConnectionPtr, WsfScriptDebugger* aDebuggerPtr)
      : WsfXIO_Request(aConnectionPtr)
      , mDebuggerPtr(aDebuggerPtr)
      , mStarted(false)
   {
   }
   void Initialized() override
   {
      mStarted = false;
      WsfXIO_RequestScriptSessionPkt pkt;
      SendRequest(pkt);
   }

   ~DebugRequest() override { mDebuggerPtr->DebugSessionEnded(); }

   void HandleResponse(WsfXIO_ResponsePkt& aPkt) override
   {
      if (!mStarted)
      {
         mDebuggerPtr->DebugSessionStarted();
      }
      mStarted = true;
      if (WsfXIO_RequestScriptSessionResponsePkt::cPACKET_ID == aPkt.ID())
      {
         WsfXIO_RequestScriptSessionResponsePkt& pkt = (WsfXIO_RequestScriptSessionResponsePkt&)aPkt;
         mDebuggerPtr->HandlePacket(pkt);
      }
   }
};

class CallstackRequest : public WsfXIO_Request
{
public:
   int                mBreakSequenceNumber;
   WsfScriptDebugger* mDebuggerPtr;
   CallstackRequest(WsfXIO_Connection* aConnectionPtr, WsfScriptDebugger* aDebuggerPtr)
      : WsfXIO_Request(aConnectionPtr)
      , mDebuggerPtr(aDebuggerPtr)
   {
      mBreakSequenceNumber = mDebuggerPtr->BreakSequenceNumber();
   }
   void Initialized() override
   {
      WsfXIO_RequestScriptDataPkt requestCallstack;
      requestCallstack.mSubscriptionRequestId = 0;
      requestCallstack.mInformationType       = WsfXIO_RequestScriptDataPkt::cCALLSTACK_LISTING;
      SendRequest(requestCallstack);
   }

   void HandleResponse(WsfXIO_ResponsePkt& aPkt) override
   {
      if (WsfXIO_RequestScriptDataResponsePkt::cPACKET_ID == aPkt.ID())
      {
         if (mBreakSequenceNumber == mDebuggerPtr->BreakSequenceNumber())
         {
            WsfXIO_RequestScriptDataResponsePkt& pkt = (WsfXIO_RequestScriptDataResponsePkt&)aPkt;
            mDebuggerPtr->HandleCallstackData(pkt);
         }
      }
   }
};

class ScriptInfoRequest : public WsfXIO_Request
{
public:
   int                   mBreakSequenceNumber;
   WsfScriptDebugger*    mDebuggerPtr;
   WsfXIO_EncodedPointer mScriptPtr;
   ScriptInfoRequest(WsfXIO_Connection* aConnectionPtr, WsfScriptDebugger* aDebuggerPtr, WsfXIO_EncodedPointer aScriptPtr)
      : WsfXIO_Request(aConnectionPtr)
      , mBreakSequenceNumber(0)
      , mDebuggerPtr(aDebuggerPtr)
      , mScriptPtr(aScriptPtr)
   {
   }
   ~ScriptInfoRequest() override { mDebuggerPtr->ScriptInfoRequestDone(); }
   void Initialized() override
   {
      WsfXIO_RequestScriptDataPkt requestCallstack;
      requestCallstack.mSubscriptionRequestId = 0;
      requestCallstack.mInformationType       = WsfXIO_RequestScriptDataPkt::cSCRIPT_DETAILS;
      requestCallstack.mScript                = mScriptPtr;
      SendRequest(requestCallstack);
   }

   void HandleResponse(WsfXIO_ResponsePkt& aPkt) override
   {
      if (WsfXIO_RequestScriptDataResponsePkt::cPACKET_ID == aPkt.ID())
      {
         WsfXIO_RequestScriptDataResponsePkt& pkt = (WsfXIO_RequestScriptDataResponsePkt&)aPkt;
         if (pkt.mScriptDetailsPtr)
         {
            mDebuggerPtr->ScriptInfoReceived(mScriptPtr, pkt.mScriptDetailsPtr);
         }
      }
   }
};

class HoverVariableValueRequest : public WsfXIO_Request
{
public:
   WsfScriptDebugger* mDebuggerPtr;
   std::string        mVariableName;
   HoverVariableValueRequest(WsfXIO_Connection* aConnectionPtr, WsfScriptDebugger* aDebuggerPtr, const std::string& aVariableName)
      : WsfXIO_Request(aConnectionPtr)
      , mDebuggerPtr(aDebuggerPtr)
      , mVariableName(aVariableName)
      , mBreakSequenceNumber(-1)
   {
   }

   void HandleResponse(WsfXIO_ResponsePkt& aPkt) override
   {
      if (WsfXIO_RequestScriptDataResponsePkt::cPACKET_ID == aPkt.ID())
      {
         std::stringstream                    ss;
         WsfXIO_RequestScriptDataResponsePkt& pkt = (WsfXIO_RequestScriptDataResponsePkt&)aPkt;
         assert(pkt.mVariableValues.size() == mRequest.mVariableQueries.size());
         for (size_t i = 0; i < mRequest.mVariableQueries.size(); ++i)
         {
            WsfXIO_RequestScriptDataPkt::VariableQuery&               query        = mRequest.mVariableQueries[i];
            WsfXIO_RequestScriptDataResponsePkt::VariableQueryResult* queryResults = pkt.mVariableValues[i];
            for (size_t j = 0; j < query.mEntryAddresses.size(); ++j)
            {
               WsfDebugValueModelVariableValue                     valdata;
               WsfXIO_RequestScriptDataResponsePkt::VariableValue& val = queryResults->mValues[j];
               if (!val.IsNull())
               {
                  if (j > 0)
                  {
                     ss << '\n';
                     for (size_t k = 0; k < query.mEntryAddresses[j].size(); ++k)
                     {
                        ss << ' ';
                     }
                     ss << val.mEntryName << ": ";
                  }
                  ss << val.mValueText << "   " << val.mTypeName << '\n';
               }
            }
         }
         mDebuggerPtr->HoverValueReceived(ss.str());
      }
   }
   void Initialized() override
   {
      mRequest.mInformationType = WsfXIO_RequestScriptDataPkt::cVARIABLE_QUERY;

      WsfXIO_RequestScriptDataPkt::VariableQuery query;
      query.mEntryAddresses.emplace_back(std::vector<int>());
      query.mStackFrame                = mDebuggerPtr->CurrentCallFrameIndex();
      query.mExpressionReferenceNumber = 0;
      query.mVariableName              = mVariableName;
      mRequest.mVariableQueries.push_back(query);

      SendRequest(mRequest);
   }

   WsfXIO_RequestScriptDataPkt mRequest;
   int                         mBreakSequenceNumber;
};
} // namespace SimulationManager

SimulationManager::WsfScriptDebugger::WsfScriptDebugger(wizard::Project*      aProjectPtr,
                                                        wizard::WsfConnector* aConnectorPtr,
                                                        Plugin*               aPluginPtr)
   : wizard::ScriptDebugger()
   , mIsAtBreak(false)
   , mBreakSequenceNumber(0)
   , mExceptionalScriptInfoRequests(0)
   , mCurrentCallStackFrame(-1)
   , mPluginPtr(aPluginPtr)
{
   mHasSession   = false;
   mToggleAllBP  = false;
   mRequestId    = -1;
   mProjectPtr   = aProjectPtr;
   mConnectorPtr = aConnectorPtr;
   connect(mConnectorPtr, &wizard::WsfConnector::ConnectedToWsf, this, &WsfScriptDebugger::ConnectedToWsf);
   connect(wizSignals, &wizard::Signals::EditorToolTipEvent, this, &WsfScriptDebugger::EditorToolTipEvent);
   connect(wizSignals, &wizard::Signals::EditorContextMenu, this, &WsfScriptDebugger::EditorContextMenu);
   connect(wizSignals, &wizard::Signals::BreakpointsChanged, this, &WsfScriptDebugger::CheckBreakpointsEnabled);
   connect(wizSignals, &wizard::Signals::TextModified, this, &WsfScriptDebugger::TextChanged);
   connect(wizSignals, &wizard::Signals::EditorOpened, this, &WsfScriptDebugger::EditorOpened);

   mCallbackPtr = new UtCallbackHolder();
   mCallbackPtr->Add(wizard::TextSource::TextSourceCharDeleted.Connect(&WsfScriptDebugger::TextDeletedCB, this));
   mCallbackPtr->Add(UtTextDocument::TextDocumentChanged.Connect(&WsfScriptDebugger::TextDocumentChangedCB, this));
}

SimulationManager::WsfScriptDebugger::~WsfScriptDebugger()
{
   if (mRequestId >= 0)
   {
      mConnectorPtr->Xio()->GetRequestManager().RemoveRequest(mRequestId);
   }

   delete mCallbackPtr;
}

void SimulationManager::WsfScriptDebugger::ToggleBreakpoint(wizard::TextSource* aSourcePtr, size_t aLine)
{
   Breakpoint* bpPtr = FindBreakpoint(aSourcePtr, aLine);
   if (bpPtr)
   {
      RemoveBreakpoint(bpPtr->mId);
   }
   else
   {
      AddBreakpoint(aSourcePtr, aLine);
   }
}

bool SimulationManager::WsfScriptDebugger::AddBreakpoint(wizard::TextSource* aSourcePtr, size_t aLine)
{
   Breakpoint* bpPtr = FindBreakpoint(aSourcePtr, aLine);
   if (!bpPtr)
   {
      Breakpoint b;
      b.mPath      = aSourcePtr->GetSystemPath();
      b.mPosition  = aSourcePtr->GetLinePosition(aLine);
      b.mSourcePtr = aSourcePtr;
      auto it      = std::find(mModifiedFiles.begin(), mModifiedFiles.end(), aSourcePtr);
      if (it != mModifiedFiles.end())
      {
         b.SetIsEnabled(false);
         b.mToggleLocked = true;
      }
      mBreakpoints[b.mId] = b;
      SendBreakpoint(b);
      emit wizSignals->BreakpointsChanged();
      return true;
   }
   return false;
}

void SimulationManager::WsfScriptDebugger::CheckBreakpointsEnabled()
{
   bool anyDisabled = false;
   bool anyUnlocked = false;
   for (auto& bp : mBreakpoints)
   {
      if (!bp.second.mToggleLocked)
      {
         anyUnlocked = true;
         if (!bp.second.IsEnabled())
         {
            anyDisabled = true;
            break;
         }
      }
   }

   mToggleAllBP = anyDisabled;
   if (!anyUnlocked)
   {
      emit BreakpointsToggled(ToggleState::cALL_LOCKED);
   }
   else if (anyDisabled)
   {
      emit BreakpointsToggled(ToggleState::cTOGGLE_ENABLE);
   }
   else
   {
      emit BreakpointsToggled(ToggleState::cTOGGLE_DISABLE);
   }
}

void SimulationManager::WsfScriptDebugger::TextChanged(wizard::TextSource* aSourcePtr)
{
   auto it = std::find(mModifiedFiles.begin(), mModifiedFiles.end(), aSourcePtr);
   if (it == mModifiedFiles.end())
   {
      bool paused = false;
      if (!mIsAtBreak)
      {
         Pause();
         paused = true;
      }
      int optionSelected =
         QMessageBox::warning(nullptr,
                              "File Text Modified",
                              "Breakpoints in the modified file will no longer execute properly. Do you want to abort "
                              "the current run or apply the changes and continue with breakpoints inactive?",
                              QMessageBox::Abort,
                              QMessageBox::Apply);
      if (optionSelected == QMessageBox::Abort)
      {
         Stop();
      }
      else if (optionSelected == QMessageBox::Apply)
      {
         mModifiedFiles.push_back(aSourcePtr);
         bool mutated = false;
         for (auto& bp : mBreakpoints)
         {
            if (bp.second.mSourcePtr == aSourcePtr)
            {
               bp.second.mWasUserEnabled = bp.second.IsEnabled();
               if (bp.second.IsEnabled())
               {
                  bp.second.SetIsEnabled(false);
                  SendBreakpoint(bp.second);
               }
               bp.second.mToggleLocked = true;
               mutated                 = true;
            }
         }

         if (mutated)
         {
            emit wizSignals->BreakpointsChanged();
         }
         if (paused)
         {
            ContinueRealtime();
         }
      }
   }
}

void SimulationManager::WsfScriptDebugger::EditorOpened()
{
   wizSignals->DebugSessionActive(this, mHasSession);
}

bool SimulationManager::WsfScriptDebugger::RemoveBreakpoint(int aBreakpointId)
{
   auto it = mBreakpoints.find(aBreakpointId);
   if (it != mBreakpoints.end())
   {
      SendBreakpoint(it->second, true);
      mBreakpoints.erase(it);
      emit wizSignals->BreakpointsChanged();
      return true;
   }
   return false;
}

void SimulationManager::WsfScriptDebugger::RemoveBreakpoints(const std::vector<int>& aBreakpointIds)
{
   bool mutated = false;
   for (int id : aBreakpointIds)
   {
      auto it = mBreakpoints.find(id);
      if (it != mBreakpoints.end())
      {
         SendBreakpoint(it->second, true);
         mBreakpoints.erase(it);
         mutated = true;
      }
   }

   if (mutated)
   {
      emit wizSignals->BreakpointsChanged();
   }
}

std::vector<SimulationManager::WsfScriptDebugger::Breakpoint*>
SimulationManager::WsfScriptDebugger::BreakpointsInFile(wizard::TextSource* aSourcePtr)
{
   std::vector<Breakpoint*> points;
   for (auto&& it : mBreakpoints)
   {
      if (it.second.mPath == aSourcePtr->GetSystemPath())
      {
         points.push_back(&it.second);
      }
   }
   return points;
}

std::vector<size_t> SimulationManager::WsfScriptDebugger::BreakpointLines(wizard::TextSource* aSourcePtr)
{
   std::vector<size_t> lines;
   for (auto&& bp : mBreakpoints)
   {
      if (bp.second.mPath == aSourcePtr->GetSystemPath())
      {
         lines.push_back(bp.second.GetLine());
      }
   }
   std::sort(lines.begin(), lines.end());
   return lines;
}

void SimulationManager::WsfScriptDebugger::ConnectedToWsf()
{
   DebugRequest* requestPtr = new DebugRequest(mConnectorPtr->Connection(), this);
   mConnectorPtr->Xio()->GetRequestManager().AddRequest(requestPtr);
   mRequestId = requestPtr->GetRequestId();

   mPluginPtr->ShowDebugWindows(true);
}

void SimulationManager::WsfScriptDebugger::SendBreakpoint(const Breakpoint& aBreak, bool aClear)
{
   if (mConnectorPtr->Connection())
   {
      WsfXIO_ScriptCommandPkt pkt;
      if (aClear || !aBreak.IsEnabled())
      {
         pkt.mCommandType = WsfXIO_ScriptCommandPkt::cCLEAR_BREAKPOINT;
      }
      else
      {
         pkt.mCommandType = WsfXIO_ScriptCommandPkt::cSET_BREAKPOINT;
      }
      pkt.mFilePath     = aBreak.mPath;
      pkt.mLineNumber   = aBreak.GetLine();
      pkt.mBreakpointId = aBreak.mId;
      pkt.mCondition    = aBreak.mCondition;
      mConnectorPtr->Connection()->Send(pkt);
   }
}

void SimulationManager::WsfScriptDebugger::TextDocumentChangedCB(const UtTextDocument* aTextDocument,
                                                                 size_t                aPosition,
                                                                 int                   aCount)
{
   bool breakpointsChanged = false;

   for (auto&& bp : mBreakpoints)
   {
      if (bp.second.mSourcePtr == aTextDocument &&
          ((aPosition < bp.second.mPosition) || (aPosition == bp.second.mPosition && aCount > 0)))
      {
         bp.second.mPosition += aCount;
         if (bp.second.mPosition <= aPosition)
         {
            bp.second.mPosition =
               bp.second.mSourcePtr->GetLinePosition(bp.second.mSourcePtr->PositionToLineNumber(aPosition));
         }
         breakpointsChanged = true;
      }
   }

   if (breakpointsChanged)
   {
      // it's possible that the changes place multiple breakpoints on the same line
      // dedupe if they did
      for (auto it = mBreakpoints.begin(); it != mBreakpoints.end(); ++it)
      {
         size_t              itLine      = it->second.GetLine();
         wizard::TextSource* itSourcePtr = it->second.mSourcePtr;
         for (auto jt = std::next(it); jt != mBreakpoints.end();)
         {
            if (itLine == jt->second.GetLine() && itSourcePtr == jt->second.mSourcePtr)
            {
               jt = mBreakpoints.erase(jt);
            }
            else
            {
               ++jt;
            }
         }
      }
      emit wizSignals->BreakpointsChanged();
   }
}

void SimulationManager::WsfScriptDebugger::DebugSessionStarted()
{
   mHasSession = true;
   mIsAtBreak  = false;

   wizSignals->DebugSessionActive(this, true);
   for (auto&& bp : mBreakpoints)
   {
      SendBreakpoint(bp.second);
   }
   emit wizSignals->BreakpointsChanged();
}

void SimulationManager::WsfScriptDebugger::DebugSessionEnded()
{
   mGlobalWatchesAvailable.clear();
   mPluginPtr->GetDebugWatchControl()->setGlobalWatches(mGlobalWatchesAvailable);
   mHasSession = false;
   wizSignals->DebugSessionActive(this, false);
   mRequestId = -1;

   if (mActiveDebugSource)
   {
      mActiveDebugLine   = 0;
      mActiveDebugSource = nullptr;
   }
   mCallstack.clear();
   wizSignals->DebugCallstackUpdated(this);
   wizSignals->CurrentDebugLineChanged();

   for (auto& bp : mBreakpoints)
   {
      bp.second.mActualLine = ut::npos;
      if (bp.second.mToggleLocked)
      {
         bp.second.mToggleLocked = false;
         bp.second.SetIsEnabled(bp.second.mWasUserEnabled);
      }
   }

   emit wizSignals->BreakpointsChanged();
   mModifiedFiles.clear();
}

void SimulationManager::WsfScriptDebugger::HandlePacket(WsfXIO_RequestScriptSessionResponsePkt& aPkt)
{
   if (aPkt.mInitialData)
   {
      mGlobalWatchesAvailable = aPkt.mInitialData->mGlobalEnumerateScripts;
      mPluginPtr->GetDebugWatchControl()->setGlobalWatches(mGlobalWatchesAvailable);
   }
   if (aPkt.mState == WsfXIO_RequestScriptSessionResponsePkt::cBREAKPOINT_ADDED)
   {
      auto it = mBreakpoints.find(aPkt.mBreakpointId);
      if (it != mBreakpoints.end())
      {
         if (ut::npos != aPkt.mLineNumber)
         {
            if (it->second.GetLine() != aPkt.mLineNumber)
            {
               wizard::TextSource* txtPtr = it->second.mSourcePtr;
               RemoveBreakpoint(it->second.mId);
               AddBreakpoint(txtPtr, aPkt.mLineNumber);
            }
            auto pit = mBreakpoints.find(aPkt.mBreakpointId);
            if (pit != mBreakpoints.end())
            {
               pit->second.mActualLine = aPkt.mLineNumber;
               emit wizSignals->BreakpointsChanged();
            }
         }
         else if (mHasSession)
         {
            RemoveBreakpoint(it->second.mId);
         }
      }
   }
   else
   {
      mIsAtBreak = (aPkt.mState != WsfXIO_RequestScriptSessionResponsePkt::cRUNNING) && !aPkt.mInitialData;
      if (mIsAtBreak)
      {
         ++mBreakSequenceNumber;
         wizard::TextSource* src = mProjectPtr->GetWorkspace()->FindSource(aPkt.mFilePath, false, true);
         if (src)
         {
            wizard::Editor* editorPtr = mProjectPtr->GotoFile(src, true);
            if (editorPtr)
            {
               mActiveDebugLine   = aPkt.mLineNumber;
               mActiveDebugSource = src;
               Breakpoint* bpPtr  = FindBreakpoint(mActiveDebugSource, mActiveDebugLine);
               if (bpPtr)
               {
                  emit wizSignals->BreakpointHit(bpPtr);
               }
               wizSignals->CurrentDebugLineChanged();
               editorPtr->GoToLine(aPkt.mLineNumber);
            }
         }
         if (aPkt.mState == WsfXIO_RequestScriptSessionResponsePkt::cAT_EXCEPTION)
         {
            QMessageBox::warning(nullptr, "Script Exception", aPkt.mExceptionDescription.c_str());
         }
         {
            mConnectorPtr->Xio()->GetRequestManager().AddRequest(new CallstackRequest(Connection(), this));
         }
      }
      else
      {
         if (mActiveDebugSource)
         {
            mActiveDebugLine   = 0;
            mActiveDebugSource = nullptr;
            mCallstack.clear();
            wizSignals->CurrentDebugLineChanged();
            wizSignals->DebugCallstackUpdated(this);
         }
      }
      wizSignals->DebugAtBreak(this, mIsAtBreak);
   }
   mConnectorPtr->UpdateTimeInfo(aPkt.mSimTime, mConnectorPtr->mEndTime);
}

void SimulationManager::WsfScriptDebugger::ForceDetatch()
{
   if (mRequestId >= 0)
   {
      mConnectorPtr->Xio()->GetRequestManager().RemoveRequest(mRequestId);
   }
}

void SimulationManager::WsfScriptDebugger::Resume()
{
   if (!mConnectorPtr->Connection())
   {
      return;
   }
   WsfXIO_ScriptCommandPkt pkt;
   pkt.mCommandType = WsfXIO_ScriptCommandPkt::cRESUME;
   mConnectorPtr->Connection()->Send(pkt);
}

std::vector<size_t> SimulationManager::WsfScriptDebugger::ActiveLines(wizard::TextSource* aSourcePtr)
{
   std::vector<size_t> lines;
   if (mHasSession)
   {
      for (auto&& call : mCallstack)
      {
         if (call.mSourcePtr == aSourcePtr)
         {
            lines.push_back(call.mLineNumber);
         }
      }
   }
   return lines;
}

void SimulationManager::WsfScriptDebugger::StepOver()
{
   if (!mConnectorPtr->Connection())
   {
      return;
   }
   if (!mIsAtBreak)
   {
      return;
   } // should not be possible
   WsfXIO_ScriptCommandPkt pkt;
   pkt.mCommandType = WsfXIO_ScriptCommandPkt::cSTEP_OVER;
   mConnectorPtr->Connection()->Send(pkt);
}

void SimulationManager::WsfScriptDebugger::StepOut()
{
   if (!mConnectorPtr->Connection())
   {
      return;
   }
   if (!mIsAtBreak)
   {
      return;
   } // should not be possible
   WsfXIO_ScriptCommandPkt pkt;
   pkt.mCommandType = WsfXIO_ScriptCommandPkt::cSTEP_OUT;
   mConnectorPtr->Connection()->Send(pkt);
}

void SimulationManager::WsfScriptDebugger::StepInto()
{
   if (!mConnectorPtr->Connection())
   {
      return;
   }
   if (!mIsAtBreak)
   {
      return;
   } // should not be possible
   if (IsModifiedFile())
   {
      QMessageBox::warning(nullptr,
                           "Stepping into modified script / file",
                           "This script / file has been modified during the current debug session and cannot be "
                           "stepped into. Stepping out now.",
                           QMessageBox::Ok);
      StepOut();
      return;
   }
   WsfXIO_ScriptCommandPkt pkt;
   pkt.mCommandType = WsfXIO_ScriptCommandPkt::cSTEP_INTO;
   mConnectorPtr->Connection()->Send(pkt);
}

bool SimulationManager::WsfScriptDebugger::IsModifiedFile()
{
   std::string scriptPath = CurrentCallFrame()->mSourcePtr->GetFilePath().GetNormalizedPath();
   for (const auto& itr : mModifiedFiles)
   {
      std::string modFilePath = UtPath(itr->GetSystemPath()).GetNormalizedPath();
      if (modFilePath == scriptPath)
      {
         return true;
      }
   }
   return false;
}

void SimulationManager::WsfScriptDebugger::UpdateBreakpointLines(const std::vector<size_t>& aLines,
                                                                 wizard::TextSource*        aSourcePtr)
{
   std::vector<size_t> oldLines = BreakpointLines(aSourcePtr);
   for (size_t i = 0; i < std::min(oldLines.size(), aLines.size()); ++i)
   {
      if (oldLines[i] != aLines[i])
      {
         Breakpoint* bpPtr = FindBreakpoint(aSourcePtr, oldLines[i]);
         if (bpPtr)
         {
            bpPtr->SetPositionByLineNumber(aLines[i]);
         }
      }
   }
}

const SimulationManager::WsfScriptDebugger::Breakpoint* SimulationManager::WsfScriptDebugger::BreakpointFromId(int aBreakpointId)
{
   return (mBreakpoints.count(aBreakpointId) ? &mBreakpoints[aBreakpointId] : nullptr);
}

SimulationManager::WsfScriptDebugger::Breakpoint*
SimulationManager::WsfScriptDebugger::FindBreakpoint(wizard::TextSource* aSource, size_t aLine)
{
   std::string path = aSource->GetSystemPath();
   for (auto& bp : mBreakpoints)
   {
      if (bp.second.GetLine() == aLine && path == bp.second.mPath)
      {
         return &bp.second;
      }
   }

   return nullptr;
}

bool SimulationManager::WsfScriptDebugger::HasBreakpoints(wizard::TextSource* aSourcePtr)
{
   std::string path = aSourcePtr->GetSystemPath();
   for (auto&& bp : mBreakpoints)
   {
      if (bp.second.mPath == path)
      {
         return true;
      }
   }
   return false;
}

SimulationManager::WsfScriptDebugger::BreakpointState SimulationManager::WsfScriptDebugger::GetBreakpointState(Breakpoint* aBp)
{
   if (aBp->mToggleLocked)
   {
      return cBP_INVALID;
   }
   if (!aBp->IsEnabled())
   {
      return cBP_DISABLED;
   }
   else if (!mHasSession)
   {
      return cBP_NORMAL;
   }
   return cBP_NORMAL;
}

const size_t SimulationManager::WsfScriptDebugger::ArrowDrawHelper(const wizard::TextSource* aDrawFile)
{
   if (mHasSession && mIsAtBreak && mActiveDebugSource == aDrawFile)
   {
      return mActiveDebugLine;
   }
   else
   {
      return ut::npos;
   }
}

WsfXIO_Connection* SimulationManager::WsfScriptDebugger::Connection()
{
   return mConnectorPtr->Connection();
}

void SimulationManager::WsfScriptDebugger::HandleCallstackData(WsfXIO_RequestScriptDataResponsePkt& aPkt)
{
   if (!aPkt.mCallstackData)
   {
      return;
   }

   mCallstack.clear();

   mAwaitingCallstackBroadcast = true;
   ++mExceptionalScriptInfoRequests;

   for (auto&& bp : aPkt.mCallstackData->mCallstackEntries)
   {
      CallstackEntry entry;
      entry.mSourcePtr    = mProjectPtr->GetWorkspace()->FindSource(bp.mFilePath, false, true);
      entry.mLineNumber   = bp.mLineNumber;
      entry.mScriptHandle = bp.mScriptId;
      if (mScriptInfo.find(entry.mScriptHandle) == mScriptInfo.end())
      {
         ++mExceptionalScriptInfoRequests;
         mConnectorPtr->Xio()->GetRequestManager().AddRequest(
            new ScriptInfoRequest(Connection(), this, entry.mScriptHandle));
      }
      mCallstack.push_back(entry);
   }

   ScriptInfoRequestDone();
}

void SimulationManager::WsfScriptDebugger::ScriptInfoRequestDone()
{
   // information about a script has been received.  Send out a call-stack update if one is waiting
   if (0 == --mExceptionalScriptInfoRequests)
   {
      wizSignals->CurrentDebugLineChanged();

      if (mAwaitingCallstackBroadcast)
      {
         wizSignals->DebugCallstackUpdated(this);
         mAwaitingCallstackBroadcast = false;
      }
   }
}

SimulationManager::WsfScriptDebugger::ScriptInfo* SimulationManager::WsfScriptDebugger::FindScriptInfo(int64_t aScriptHandle)
{
   std::map<int64_t, ScriptInfo>::iterator i = mScriptInfo.find(aScriptHandle);
   if (i != mScriptInfo.end())
   {
      return &i->second;
   }
   return nullptr;
}

void SimulationManager::WsfScriptDebugger::SelectStackFrame(int aFrameIndex)
{
   if (aFrameIndex >= (int)mCallstack.size())
   {
      return;
   }

   // -1 is an alias for the top frame
   if (aFrameIndex + 1 == (int)mCallstack.size())
   {
      aFrameIndex = -1;
   }

   if (mCurrentCallStackFrame != aFrameIndex)
   {
      mCurrentCallStackFrame               = aFrameIndex;
      const CallstackEntry* callstackEntry = CurrentCallFrame();
      if (callstackEntry && callstackEntry->mSourcePtr)
      {
         wizard::Editor* editorPtr = mProjectPtr->GotoFile(callstackEntry->mSourcePtr, true);
         if (editorPtr)
         {
            editorPtr->GoToLine(callstackEntry->mLineNumber);
         }
      }

      wizSignals->DebugCallStackFrameSwitch(this, mCurrentCallStackFrame);
   }
}

const SimulationManager::WsfScriptDebugger::CallstackEntry* SimulationManager::WsfScriptDebugger::CurrentCallFrame()
{
   if (mCallstack.empty())
   {
      return nullptr;
   }
   if (mCurrentCallStackFrame == -1)
   {
      return &mCallstack.back();
   }
   return &mCallstack[mCurrentCallStackFrame];
}

void SimulationManager::WsfScriptDebugger::EditorToolTipEvent(wizard::WsfEditor* aEditControl, QHelpEvent* aEvent)
{
   if (!mIsAtBreak)
   {
      return;
   }
   size_t      offset     = aEditControl->GetPosition(aEvent->pos());
   std::string identifier = ScriptIdentifierAtPosition(aEditControl, ut::cast_to_int(offset));
   if (!identifier.empty() && Connection())
   {
      mHoverPosition = aEvent->globalPos();
      Connection()->GetInterface().GetRequestManager().AddRequest(
         new HoverVariableValueRequest(Connection(), this, identifier));
   }
}

void SimulationManager::WsfScriptDebugger::HoverValueReceived(const std::string& aValueText)
{
   QToolTip::showText(mHoverPosition, QString::fromStdString(aValueText), wkfEnv.GetMainWindow());
}

void SimulationManager::WsfScriptDebugger::EditorContextMenu(wizard::Editor* aEditorPtr, QMenu& aMenu)
{
   wizard::WsfEditor* editorPtr = dynamic_cast<wizard::WsfEditor*>(aEditorPtr);
   if (editorPtr && mIsAtBreak)
   {
      mWatchVariableContextSuggestion =
         ScriptIdentifierAtPosition(editorPtr, editorPtr->GetContextMenuCharacterPosition());

      if (!mWatchVariableContextSuggestion.empty())
      {
         QString txt = "Add Watch for '";
         txt += mWatchVariableContextSuggestion.c_str();
         txt += "'";
         connect(aMenu.addAction(txt), &QAction::triggered, this, &SimulationManager::WsfScriptDebugger::AddWatchFromContextMenu);
      }
   }
}

std::string SimulationManager::WsfScriptDebugger::ScriptIdentifierAtPosition(wizard::Editor* aEditorPtr, int aPos)
{
   int offset = aPos;
   if (offset >= 0)
   {
      wizard::ParseResults* parseResults = aEditorPtr->GetProject()->GetParseResults();
      if (parseResults && parseResults->mScriptDataPtr)
      {
         size_t line, col;
         aEditorPtr->GetLineColumn(ut::cast_to_size_t(offset), line, col);
         std::string text = aEditorPtr->GetLineText(line);
         if (text.empty())
            col = 0;
         else
         {
            col = std::min(col, text.size() - 1);
         }

         size_t start = col;
         while (start != 0)
         {
            char c = text[start];
            if (!(isalpha(c) || isdigit(c) || c == '_'))
            {
               ++start;
               break;
            }
            --start;
         }

         size_t end = col;
         while (end < text.size())
         {
            char c = text[end];
            if (!(isalpha(c) || isdigit(c) || c == '_'))
            {
               break;
            }
            ++end;
         }
         if (!text.empty())
         {
            end = std::min(end, text.size() - 1);
         }

         if (end > start)
         {
            std::string identifier = text.substr(start, end - start);
            if (!isdigit(identifier[0]))
            {
               wizard::WsfScriptLookup lookup(aEditorPtr->GetProject());
               lookup.SetLocation(UtTextDocumentLocation(aEditorPtr->GetSource(), offset));
               wizard::WsfScriptLookup::UpdateResult result = lookup.Update();
               if (result != wizard::WsfScriptLookup::cNO_SCRIPT)
               {
                  return identifier;
               }
            }
         }
      }
   }
   return std::string();
}

void SimulationManager::WsfScriptDebugger::AddWatch(const std::string& aVar)
{
   mPluginPtr->GetDebugWatchControl()->addNewWatch(aVar);
}

void SimulationManager::WsfScriptDebugger::TextDeletedCB(wizard::TextSource* aSourcePtr,
                                                         const size_t        aPosition,
                                                         const int           aCount)
{
   std::vector<Breakpoint*> bpinfile = BreakpointsInFile(aSourcePtr); // vector of breakpoints in the file
   std::vector<int>         bpidToBeDeleted; // vector of breakpoint id's that are going to be deleted
   std::size_t              startRow;
   std::size_t              startColumn;
   std::size_t              endRow;
   std::size_t              endColumn;
   aSourcePtr->PositionToLineColumn(aPosition, startRow, startColumn); // getting variable info
   aSourcePtr->PositionToLineColumn(aPosition + aCount, endRow, endColumn);
   if (!bpinfile.empty())
   {
      std::vector<size_t> linesDeleted; // line numbers of lines in file that are deleted
      // checks different cases where breakpoints should be removed
      if (aCount == 1) // if deleting just one character
      {
         // deleting second last character of line
         if (startRow == endRow && startColumn == 0 && endColumn == 1 && aSourcePtr->GetLineLength(startRow) == 2)
         {
            linesDeleted.push_back(startRow);
         }
         // deleting last character of line
         else if (startRow < endRow && endColumn == 0 && aSourcePtr->GetLineLength(endRow) == 1)
         {
            linesDeleted.push_back(endRow);
         }
      }
      else
      {
         // if entire row is highlighted and deleted
         if (startRow == endRow && startColumn == 0 && endColumn + 1 == aSourcePtr->GetLineLength(endRow))
         {
            linesDeleted.push_back(endRow);
         }
         // if multiple rows are highlighted and deleted
         else if (startRow < endRow && startColumn == 0)
         {
            for (; startRow < endRow; startRow++)
            {
               linesDeleted.push_back(startRow);
            }
            if (endColumn + 1 == aSourcePtr->GetLineLength(endRow))
            {
               linesDeleted.push_back(endRow);
            }
         }
         else if (startRow < endRow && startColumn != 0 && endColumn + 1 != aSourcePtr->GetLineLength(endRow))
         {
            for (startRow++; startRow < endRow; startRow++)
            {
               linesDeleted.push_back(startRow);
            }
         }
         else if (startRow < endRow && startColumn != 0 && endColumn + 1 == aSourcePtr->GetLineLength(endRow))
         {
            for (startRow++; startRow <= endRow; startRow++)
            {
               linesDeleted.push_back(startRow);
            }
         }
      }
      for (size_t j = 0; j < bpinfile.size(); j++)
      {
         if (std::find(linesDeleted.begin(), linesDeleted.end(), bpinfile[j]->GetLine()) != linesDeleted.end())
         {
            bpidToBeDeleted.push_back(bpinfile[j]->mId);
         }
      }
      RemoveBreakpoints(bpidToBeDeleted);
   }
}

void SimulationManager::WsfScriptDebugger::SetBreakpointEnabled(int aBreakpointId, bool aIsEnabled)
{
   // if the breakpoint exists and if it's not already at the desired state
   auto it = mBreakpoints.find(aBreakpointId);
   if (it != mBreakpoints.end() && it->second.IsEnabled() != aIsEnabled)
   {
      it->second.SetIsEnabled(aIsEnabled);
      SendBreakpoint(it->second);
      emit wizSignals->BreakpointsChanged();
   }
}

void SimulationManager::WsfScriptDebugger::SetBreakpointsEnabled(const std::vector<int>& aBreakpointIds, bool aIsEnabled)
{
   bool mutated = false;
   for (int id : aBreakpointIds)
   {
      auto it = mBreakpoints.find(id);
      if (it != mBreakpoints.end() && it->second.IsEnabled() != aIsEnabled)
      {
         it->second.SetIsEnabled(aIsEnabled);
         SendBreakpoint(it->second);
         mutated = true;
      }
   }

   if (mutated)
   {
      emit wizSignals->BreakpointsChanged();
   }
}

void SimulationManager::WsfScriptDebugger::ToggleAllBreakpoints()
{
   bool mutated = false;
   for (auto& bp : mBreakpoints)
   {
      if (bp.second.IsEnabled() != mToggleAllBP)
      {
         bp.second.SetIsEnabled(mToggleAllBP);
         SendBreakpoint(bp.second);
         mutated = true;
      }
   }

   if (mutated)
   {
      emit wizSignals->BreakpointsChanged();
   }
}

void SimulationManager::WsfScriptDebugger::DeleteAllBreakpoints()
{
   if (!mBreakpoints.empty())
   {
      for (auto& bp : mBreakpoints)
      {
         SendBreakpoint(bp.second, true);
      }
      mBreakpoints.clear();
      emit wizSignals->BreakpointsChanged();
   }
}

void SimulationManager::WsfScriptDebugger::SetBreakpointCondition(int aBreakpointId, const std::string& aCondition)
{
   auto it = mBreakpoints.find(aBreakpointId);
   if (it != mBreakpoints.end() && it->second.mCondition != aCondition)
   {
      it->second.mCondition = aCondition;
      SendBreakpoint(it->second, false);
      emit wizSignals->BreakpointsChanged();
   }
}
void SimulationManager::WsfScriptDebugger::ContinueRealtime()
{
   mConnectorPtr->ResumeRealtime();
}
void SimulationManager::WsfScriptDebugger::ContinueNonRealtime()
{
   mConnectorPtr->ResumeConstructive();
}

void SimulationManager::WsfScriptDebugger::MarginRightClick(QWidget* aParent, wizard::TextSource* aSourcePtr, size_t aLine)
{
   Breakpoint* bpPtr = FindBreakpoint(aSourcePtr, aLine);
   QMenu       menu;

   if (bpPtr)
   {
      // user right-clicked a breakpoint
      QAction* toggleBp = menu.addAction(bpPtr->IsEnabled() ? "Disable Breakpoint" : "Enable Breakpoint");
      connect(toggleBp,
              &QAction::triggered,
              [this, aSourcePtr, aLine, bpPtr]() { SetBreakpointEnabled(bpPtr->mId, !bpPtr->IsEnabled()); });

      QAction* editBpCondition = menu.addAction("Edit Breakpoint Condition...");
      connect(editBpCondition,
              &QAction::triggered,
              [this, aParent, bpPtr]()
              {
                 bool    ok;
                 QString condition = QInputDialog::getText(aParent,
                                                           "Breakpoint Condition",
                                                           "Condition",
                                                           QLineEdit::Normal,
                                                           QString::fromStdString(bpPtr->mCondition),
                                                           &ok);
                 if (ok)
                 {
                    SetBreakpointCondition(bpPtr->mId, condition.toStdString());
                 }
              });
   }
   else
   {
      // user right-clicked on a line with no breakpoint
      QAction* setBp = menu.addAction("Set Breakpoint");
      connect(setBp, &QAction::triggered, [this, aSourcePtr, aLine]() { ToggleBreakpoint(aSourcePtr, aLine); });

      QAction* setConditionalBp = menu.addAction("Set Conditional Breakpoint...");
      connect(setConditionalBp,
              &QAction::triggered,
              [this, aParent, aSourcePtr, aLine]()
              {
                 ToggleBreakpoint(aSourcePtr, aLine);
                 bool    ok;
                 QString condition =
                    QInputDialog::getText(aParent, "Breakpoint Condition", "Condition", QLineEdit::Normal, "", &ok);
                 Breakpoint* bpPtr = FindBreakpoint(aSourcePtr, aLine);
                 if (ok)
                 {
                    SetBreakpointCondition(bpPtr->mId, condition.toStdString());
                 }
              });
   }

   menu.exec(QCursor::pos());
}

std::vector<SimulationManager::WsfScriptDebugger::Breakpoint> SimulationManager::WsfScriptDebugger::AllBreakpoints() const
{
   std::vector<Breakpoint> bps;
   bps.reserve(mBreakpoints.size());
   for (auto& bp : mBreakpoints)
   {
      bps.push_back(bp.second);
   }
   return bps;
}

void SimulationManager::WsfScriptDebugger::Pause()
{
   if (mConnectorPtr)
   {
      mConnectorPtr->Pause();
   }
}

void SimulationManager::WsfScriptDebugger::Stop()
{
   if (mConnectorPtr)
   {
      mConnectorPtr->Stop();
   }
}

SimulationManager::WsfScriptDebugger::Breakpoint::Breakpoint()
{
   static int nextId = 1;
   mId               = nextId++;
}
