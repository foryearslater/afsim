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

#ifndef WSFSCRIPTDEBUGGER_HPP
#define WSFSCRIPTDEBUGGER_HPP

#include <cstdint>
#include <map>

#include <QObject>
#include <QPoint>

#include "ScriptDebugger.hpp"
#include "TextSource.hpp"
#include "UtOptional.hpp"
#include "xio/WsfXIO_PacketRegistry.hpp"

class QMenu;
class WsfXIO_RequestScriptSessionResponsePkt;
class WsfXIO_RequestScriptDataResponsePkt;
class WsfXIO_Connection;
class QHelpEvent;

namespace wizard
{
class Editor;
class Project;
class WsfConnector;
class WsfEditor;
} // namespace wizard

enum ToggleState
{
   cALL_LOCKED     = 0,
   cTOGGLE_ENABLE  = 1,
   cTOGGLE_DISABLE = 2
};

namespace SimulationManager
{
class Plugin;
class ScriptInfoRequest;

class WsfScriptDebugger : public QObject, public wizard::ScriptDebugger
{
   Q_OBJECT
public:
   friend class ScriptInfoRequest;

   WsfScriptDebugger(wizard::Project* aProjectPtr, wizard::WsfConnector* aConnectorPtr, Plugin* aPluginPtr);
   ~WsfScriptDebugger() override;
   std::vector<Breakpoint*> BreakpointsInFile(wizard::TextSource* aSourcePtr);
   std::vector<size_t>      BreakpointLines(wizard::TextSource* aSourcePtr);
   std::vector<size_t>      ActiveLines(wizard::TextSource* aSourcePtr);
   std::vector<Breakpoint>  AllBreakpoints() const;
   const Breakpoint*        BreakpointFromId(int aBreakpointId);
   void                     SetBreakpointEnabled(int aBreakpointId, bool aIsEnabled);
   void                     SetBreakpointsEnabled(const std::vector<int>& aBreakpointIds, bool aIsEnabled);
   void                     SetBreakpointCondition(int aBreakpointId, const std::string& aCondition);
   void                     ToggleBreakpoint(wizard::TextSource* aSource, size_t aLine);
   void                     CheckBreakpointsEnabled();
   bool                     RemoveBreakpoint(int aBreakpointId);
   void                     RemoveBreakpoints(const std::vector<int>& aBreakpointIds);
   bool                     HasSession() const { return mHasSession; }
   void                     DebugSessionStarted();
   void                     DebugSessionEnded();
   bool                     HasBreakpoints(wizard::TextSource* aSourcePtr);
   void                     HandlePacket(WsfXIO_RequestScriptSessionResponsePkt& aPkt);
   void                     UpdateBreakpointLines(const std::vector<size_t>& aLines, wizard::TextSource* aSourcePtr);
   bool                     IsAtBreak() const override { return mIsAtBreak; }

   void ForceDetatch() override;
   void Resume() override;

   void StepOver() override;
   void StepOut() override;
   void StepInto() override;

   void HoverValueReceived(const std::string& aValueText);

   void MarginRightClick(QWidget* aParent, wizard::TextSource* aSourcePtr, size_t aLine);

   enum BreakpointState
   {
      cBP_NORMAL,
      cBP_INVALID,
      cBP_DISABLED
      // tbd...
   };

   BreakpointState                                      GetBreakpointState(Breakpoint* aBp);
   const size_t                                         ArrowDrawHelper(const wizard::TextSource* aDrawFile);
   std::vector<wizard::ScriptDebugger::CallstackEntry>& Callstack() override { return mCallstack; }
   const wizard::ScriptDebugger::CallstackEntry*        CurrentCallFrame() override;
   int                                 CurrentCallFrameIndex() const override { return mCurrentCallStackFrame; }
   int                                 BreakSequenceNumber() override { return mBreakSequenceNumber; }
   WsfXIO_Connection*                  Connection() override;
   void                                HandleCallstackData(WsfXIO_RequestScriptDataResponsePkt& aPkt);
   wizard::ScriptDebugger::ScriptInfo* FindScriptInfo(int64_t aScriptHandle) override;

   void SelectStackFrame(int aFrameIndex) override;

signals:
   void BreakpointsToggled(int aToggleState);

public slots:
   void DeleteAllBreakpoints();
   void ToggleAllBreakpoints();
   void Pause();
   void Stop();
   void ContinueRealtime();
   void ContinueNonRealtime();
   void TextChanged(wizard::TextSource* aSourcePtr);
   bool IsModifiedFile();
   void EditorOpened();

protected slots:
   void EditorToolTipEvent(wizard::WsfEditor*, QHelpEvent*);
   void ConnectedToWsf();

   void EditorContextMenu(wizard::Editor* aEditorPtr, QMenu& aMenu);
   void AddWatch(const std::string& aVar);
   void AddWatchFromContextMenu()
   {
      AddWatch(mWatchVariableContextSuggestion);
      mWatchVariableContextSuggestion.clear();
   }

private:
   std::string ScriptIdentifierAtPosition(wizard::Editor* aEditorPtr, int aPos);
   void        ScriptInfoReceived(WsfXIO_EncodedPointer aScriptPtr, ScriptInfo* aScriptInfoPtr)
   {
      mScriptInfo[aScriptPtr] = *aScriptInfoPtr;
   }
   void ScriptInfoRequestDone();

   Breakpoint* FindBreakpoint(wizard::TextSource* aSource, size_t aLine);

   void SendBreakpoint(const Breakpoint& aBreak, bool aClear = false);

   bool AddBreakpoint(wizard::TextSource* aSource, size_t aLine);

   std::vector<CallstackEntry>      mCallstack;
   wizard::Project*                 mProjectPtr;
   std::map<int, Breakpoint>        mBreakpoints;
   wizard::WsfConnector*            mConnectorPtr;
   bool                             mHasSession;
   int                              mRequestId;
   bool                             mIsAtBreak;
   bool                             mToggleAllBP;
   std::vector<wizard::TextSource*> mModifiedFiles;

   size_t              mActiveDebugLine;
   wizard::TextSource* mActiveDebugSource;

   // Incremented at each break, used to identify watch values that are out of date
   int mBreakSequenceNumber;

   int                           mExceptionalScriptInfoRequests;
   bool                          mAwaitingCallstackBroadcast;
   int                           mCurrentCallStackFrame;
   std::map<int64_t, ScriptInfo> mScriptInfo;
   QPoint                        mHoverPosition;
   std::string                   mWatchVariableContextSuggestion;
   std::vector<std::string>      mGlobalWatchesAvailable;

   void              TextDeletedCB(wizard::TextSource* aSourcePtr, const size_t aPosition, const int aCount);
   void              TextDocumentChangedCB(const UtTextDocument* aTextDocument, size_t aPosition, int aCount);
   UtCallbackHolder* mCallbackPtr;

   Plugin* mPluginPtr;
};
} // namespace SimulationManager
#endif
