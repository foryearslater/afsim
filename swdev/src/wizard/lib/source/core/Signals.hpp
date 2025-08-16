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
#ifndef SIGNALS_HPP
#define SIGNALS_HPP

#include <QObject>

#include "ScriptDebugger.hpp"
#include "UtCallback.hpp"
#include "Vi.hpp"
#include "ViExport.hpp"
#include "WsfPProxyNode.hpp"
#include "WsfPProxyPath.hpp"

class QFileInfo;
class QHelpEvent;
class QMenu;
class QMimeData;
class QString;

namespace wizard
{
class Editor;
class PasteContext;
class Project;
class TextSource;
class WsfEditor;
class WsfExe;

class VI_EXPORT Signals : public QObject
{
   Q_OBJECT

public:
   explicit Signals(QObject* p)
      : QObject(p)
      , gEditMode(Vi::cEDIT_MODE_TEXT)
   {
   }

   UtCallbackListN<void(ScriptDebugger*, bool)> DebugSessionActive; /*(aDebugger, aIsActive)*/
   UtCallbackListN<void()>                      CurrentDebugLineChanged;
   UtCallbackListN<void(ScriptDebugger*, bool)> DebugAtBreak;
   // The selected callstack frame has changed.  The callstack frame resets to the top (-1) whenever a new break occurs
   // ( signal DebugAtBreak ) This signal is only emitted when the user changes the frame.
   UtCallbackListN<void(ScriptDebugger*, int)> DebugCallStackFrameSwitch;
   UtCallbackListN<void(ScriptDebugger*)>      DebugCallstackUpdated;

   // called by auto-complete to find plug-in suggestions for icons
signals:
   void RequestNameSuggestions(const std::string& aNameType, std::vector<std::string>& aNameList);
   void EpochUpdated(const UtCalendar& aEpoch);


   // Called when the editor's context menu is requested
   void EditorContextMenu(Editor* aEditorPtr, QMenu& aMenu);

   void URL_Process(const QUrl& aUrl);

   void EditorOpened();

   // Some files are likely to have changed.
   void FileCheckRequested();

   void ProjectDirectoriesChanged(const std::vector<UtPath>& aDirectoryList);
   void ProjectDirectoryChanged(const UtPath& aDirectory);
   void EditModeSwitch(Vi::EditMode aEditMode);
   void ProjectStartupFilesChanged(const std::vector<UtPath>& aMainFiles);
   void SourceModifiedStateChange(TextSource*);

   // Signaled when the application attempts to clean up unused text sources
   void SourceCleanupEligibility(QSet<TextSource*>&);

   void SourceDeleted(TextSource*);
   void BreakpointsChanged();
   void BreakpointHit(const wizard::ScriptDebugger::Breakpoint* aBreakpoint);
   void EditorToolTipEvent(WsfEditor*, QHelpEvent*);
   void ParseResultsInvalidate(Project*);
   void TextModified(TextSource* aSourcePtr);

   //! These signals are emitted when e.g. the user double-clicks or
   //! right-clicks on a file in the Project Browser. This allows plug-ins (or
   //! anything else) to add actions to the menu that will be presented for
   //! the given file or associated proxy node. Subscribers to this signal
   //! should place the primary or default action first.
   void ActionsForFileRequested(const QFileInfo& aFileInfo, QMenu* aMenu);
   void ActionsForCommandContentsRequested(const std::string& aContent,
                                           const std::string& aPath,
                                           QMenu*             aMenu,
                                           QWidget*           aParentWidget);
   void ActionsForNodeRequested(const WsfPProxyNode& aNode, QMenu* aMenu, QWidget* aParentWidget);

   // The active WSF executable has changed
   void WsfExeChanged(WsfExe*);
   void VisualModeChanged();
   void ApplyProxy();
   // changes in application state may necessitate a change in action state
   void ActionsStateChange();
   //! Signal emitted when the active wizard::Editor has been changed
   //! @param aEditorPtr is the wizard::Editor now active
   void ActiveEditorChanged(Editor* aEditorPtr);

public:
   VI_DEPRECATED void emitEpochUpdated(const UtCalendar& aEpoch) { emit EpochUpdated(aEpoch); }

   VI_DEPRECATED void emitEditorContextMenu(Editor* aEditorPtr, QMenu& aMenu)
   {
      emit EditorContextMenu(aEditorPtr, aMenu);
   }

   VI_DEPRECATED void emitURL_Process(const QUrl& aUrl) { emit URL_Process(aUrl); }

   VI_DEPRECATED void emitFileCheckRequested() { emit FileCheckRequested(); }

   VI_DEPRECATED void emitProjectDirectoriesChanged(const std::vector<UtPath>& aDirectoryList)
   {
      emit ProjectDirectoriesChanged(aDirectoryList);
   }

   VI_DEPRECATED void emitProjectDirectoryChanged(const UtPath& aDirectory)
   {
      emit ProjectDirectoryChanged(aDirectory);
   }

   //////////////////////////////////////////////////////////
   ////                                                  ////
   ////    NOTE: THIS DOES MORE THAN JUST EMIT SIGNAL    ////
   ////                                                  ////
   //////////////////////////////////////////////////////////
   void emitEditModeSwitch(Vi::EditMode aEditMode)
   {
      gEditMode = aEditMode;
      emit EditModeSwitch(aEditMode);
   }

   VI_DEPRECATED void emitProjectStartupFilesChanged(const std::vector<UtPath>& aMainFiles)
   {
      emit ProjectStartupFilesChanged(aMainFiles);
   }

   VI_DEPRECATED void emitSourceModifiedStateChange(TextSource* aSourcePtr)
   {
      emit SourceModifiedStateChange(aSourcePtr);
   }

   VI_DEPRECATED void emitSourceCleanupEligibility(QSet<TextSource*>& aEligibleSources)
   {
      emit SourceCleanupEligibility(aEligibleSources);
   }

   VI_DEPRECATED void emitSourceDeleted(TextSource* aSourcePtr) { emit SourceDeleted(aSourcePtr); }

   VI_DEPRECATED void emitBreakpointsChanged() { emit BreakpointsChanged(); }

   VI_DEPRECATED void emitEditorToolTipEvent(WsfEditor* aEditorPtr, QHelpEvent* aToolTipEventPtr)
   {
      emit EditorToolTipEvent(aEditorPtr, aToolTipEventPtr);
   }

   VI_DEPRECATED void emitParseResultsInvalidate(Project* aProjectPtr) { emit ParseResultsInvalidate(aProjectPtr); }

   VI_DEPRECATED void emitActionsForFileRequested(const QFileInfo& aFileInfo, QMenu* aMenu)
   {
      emit ActionsForFileRequested(aFileInfo, aMenu);
   }

   VI_DEPRECATED void emitActionsForCommandContentsRequested(const std::string& aContent,
                                                             const std::string& aPath,
                                                             QMenu*             aMenu,
                                                             QWidget*           aParentWidget)
   {
      emit ActionsForCommandContentsRequested(aContent, aPath, aMenu, aParentWidget);
   }

   VI_DEPRECATED void emitActionsForNodeRequested(const WsfPProxyNode& aNode, QMenu* aMenu, QWidget* aParentWidget)
   {
      emit ActionsForNodeRequested(aNode, aMenu, aParentWidget);
   }

   //////////////////////////////////////////////////////////
   ////                                                  ////
   ////    NOTE: THIS DOES MORE THAN JUST EMIT SIGNAL    ////
   ////                                                  ////
   //////////////////////////////////////////////////////////
   void emitWsfExeChanged(WsfExe* aExePtr)
   {
      ProxyInvalidate();
      emit WsfExeChanged(aExePtr);
   }

   VI_DEPRECATED void emitVisualModeChanged() { emit VisualModeChanged(); }

   VI_DEPRECATED void emitApplyProxy() { emit ApplyProxy(); }

   VI_DEPRECATED void emitActionsStateChange() { emit ActionsStateChange(); }

   VI_DEPRECATED void emitActiveEditorChanged(Editor* aEditorPtr) { emit ActiveEditorChanged(aEditorPtr); }

   UtCallbackListN<void()>                                 CheckFilesForModification;
   UtCallbackListN<void()>                                 ApplicationActivated;
   UtCallbackListN<void(Editor*)>                          EditorClosed;
   UtCallbackListN<void(Editor*)>                          EditorSelectionChanged;
   UtCallbackListN<void(Project&)>                         ProjectOpen;
   UtCallbackListN<void(Project&)>                         ProjectSettingsOpen;
   UtCallbackListN<void(Project*, bool)>                   ParseUpdatedDeferred;
   UtCallbackListN<void(TextSource*, UtTextDocumentRange)> OpenCommandDocumentation;
   UtCallbackListN<void(QString)>                          StyleSheetReload;
   UtCallbackListN<void()>                                 ShuttingDown;
   UtCallbackListN<void()>                                 ShuttingDown2;

   // PasteQuery and PasteExecute allow for custom paste and drag->drop operations and allows for extension by plugins
   // PasteQuery(.., .., false) - Invoked before a paste operation is executed.  Callee should use
   // PasteContext::AddAvailableAction()
   //              if the paste may be handled by them.
   // PasteQuery(.., .., true) - Invoked after PasteQuery if some actions were declared available.  Callee should
   // execute the paste
   //              operation if PasteContext::IsActionChosen() returns true
   UtCallbackListN<void(const QMimeData*, const PasteContext&, bool)> PasteQuery;

   // The proxy has been invalidated
   UtCallbackListN<void()> ProxyInvalidate;

   Vi::EditMode GetProxyEditMode() const { return gEditMode; }
   bool         InVisualMode() const { return gEditMode == Vi::cEDIT_MODE_VISUAL; }

   void editProxy();
   void applyProxy();
   void revertVisualModeChanges();
   void handleProjectClose();

protected:
   void switchToEditModeP();

   Vi::EditMode gEditMode;
};
} // namespace wizard

extern VI_EXPORT wizard::Signals* wizSignals;

#endif
