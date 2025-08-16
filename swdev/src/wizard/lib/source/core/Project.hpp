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

#ifndef PROJECT_HPP
#define PROJECT_HPP

#include <memory>
#include <string>
#include <vector>

#include <QMetaType>
#include <QObject>
#include <QString>

#include "ScriptDebugger.hpp"
#include "TerrainMonitor.hpp"
#include "UtCalendar.hpp"
#include "UtCallback.hpp"
#include "UtCallbackHolder.hpp"
#include "UtPath.hpp"
#include "UtReferenceTracked.hpp"
#include "UtWallClock.hpp"
#include "ViExport.hpp"

class UtQtXmlWriter;
class UtQtXmlReader;

class QAction;
class QMenu;
class UtTextDocument;
class UtTextDocumentRange;
class ViFileItem;
class WsfPProxy;
class WsfPProxyNode;
class WsfPProxyPath;

namespace wizard
{
class AttributeSet;
class CacheSourceProvider;
class Editor;
class ParseResults;
class ParseResultsTiny;
class ParseWorker;
class ProjectComponent;
class ProjectParseThread;
class ProjectWorkspace;
class ProxyChange;
class ProxyHash;
class RevisionStore;
class TextSource;
class TextSourceCache;
class WsfExe;
class XML_Context;

//! A WSF 'project'.  Manages WSF sources files in a WSF project.
class VI_EXPORT Project : public QObject, public UtReferenceTracked
{
   Q_OBJECT

public:
   friend class ParseWorker;
   friend class ProjectWorkspace;

   typedef UtCallbackN<void()> ParseCompleteCallback;

   explicit Project(ProjectWorkspace* aWorkspacePtr);
   Project(const Project&) = delete;
   Project(Project&&)      = default;
   Project& operator=(const Project&) = delete;
   Project& operator=(Project&&) = default;
   ~Project() override;

   static Project* Instance();

   QString Name() const;

   TextSourceCache& GetSourceCache();

   void AddComponent(const std::string& aName, std::unique_ptr<ProjectComponent> aComponent);

   ProjectComponent* FindComponent(const std::string& aName);

   //! Returns the directory WSF is expected to run from.  This is important for
   //! finding includes with relative paths.
   const UtPath& ProjectDirectory() const { return mProjectDirectory; }
   bool          SetProjectLocation(const UtPath& aProjDirPath, const UtPath& aProjFilePath);
   bool          SetProjectDirectory(const UtPath& aPath);
   bool          SetProjectFilePath(const UtPath& aPath);
   long long     GetMinimumSequenceNumber();

   AttributeSet* FindOptions(const WsfPProxyNode& aNode);
   AttributeSet* FindOptions(const WsfPProxyPath& aPath);
   AttributeSet* GetOptions(const WsfPProxyNode& aNode);

   //! Paths to be searched for map data relating to this project
   std::vector<UtPath> MapSearchPath() const;

   RevisionStore& GetRevisionStore();

   //! Called whenever a parse is complete, as many as once per source modification
   UtCallbackListN<void(bool)> ParseUpdated;

   //! Called after a short period of delay after editing when the parse is complete.
   //! This should be used if more expensive computations are required
   UtCallbackListN<void(bool)> ParseUpdatedDeferred;

   UtCallbackListN<void()>                MainFileListChange;
   static UtCallbackListN<void(Project*)> ProjectClosing;
   static UtCallbackListN<void(Project*)> RegisterComponents;

   void    GotoRange(const UtTextDocumentRange& aRange);
   Editor* GotoFile(UtTextDocument* aSourcePtr, bool aRecordLocation = false);
   Editor* GotoFile(const std::string& aFilePath, bool aRecordLocation = true);

   void WaitForParsing();
   void SaveAll(bool aSkipProject = true);
   void TriggerReparse();
   void InvalidateScenario(bool aInvalidateProxy = false);
   void FindIncludes(TextSource* aSourcePtr);

   bool SourceReloaded(TextSource* aSourcePtr);

   ProjectWorkspace* GetWorkspace() const { return mWorkspacePtr; }

   const std::string& GetMapItemsXML() const { return mMapItemsXML; }
   void               SetMapItemsXML(const std::string& aXml) { mMapItemsXML = aXml; }

   const std::vector<UtPath>& GetStartupFiles();
   void                       SetStartupFiles(const std::vector<UtPath>& aMainFiles);
   const std::vector<UtPath>& GetProjectDirectories() const { return mProjectDirectories; }


   bool                   AddProjectDirectory(const UtPath& aPath);
   void                   RemoveProjectDirectory(const UtPath& aPath);
   bool                   IsDirectoryInProject(const UtPath& aPath) const;
   void                   ShowSettings();
   void                   SetWorkingDirectory(const UtPath& aPath);
   WsfExe*                GetExe() const;
   void                   SetExe(WsfExe* aExePtr, bool aEmit);
   void                   SetCommandLineArgs(const QString& aArgs);
   static QString         GetDefaultCommandLineArgs();
   void                   ExeUpdated(WsfExe* aExePtr);
   void                   WaitForAllParsing();
   ParseResults*          TakeParseResults();
   bool                   IncludesFile(TextSource* aSourcePtr);
   bool                   IncludesStartupFile(TextSource* aSourcePtr);
   void                   GiveParseResults();
   void                   ProxyReady(std::unique_ptr<WsfPProxy> aProxy, std::unique_ptr<ProxyHash> aProxyHashPtr);
   void                   ExportProjectAction();
   ParseWorker*           GetParseWorker() { return mParseWorker; }
   void                   InitParseResults();
   ParseCompleteCallback* ExecuteWhenParseComplete(ParseCompleteCallback* aCallbackPtr);
   const UtPath&          WorkingDirectory() const;
   bool                   ParseIsUpToDate() const { return mParseResultsPtr != nullptr; }
   //! Get the time the WsfParseDefinitions spent parsing the WsfGrammar
   //! @return the latest grammar parsing time
   double GetLastGrammarParseTime() const noexcept { return mLastGrammarParseTime; }
   //! Get the time the WsfParser spent parsing the input files
   //! @return the latest input parsing time
   double GetLastInputParseTime() const noexcept { return mLastInputParseTime; }
   //! Get the time the WsfPProxyDeserialize spent deserializing the parse tree
   //! @return the latest proxy deserializing time
   double GetLastProxyDeserializeTime() const noexcept { return mLastProxyDeserializeTime; }
   //! Get the time the ProxyMerge spent merging the old and new WsfPProxys/Wizard spent updating its GUIs
   //! @return the latest proxy merging/GUI updating time
   double               GetLastProxyMergeGUI_Time() const noexcept { return mLastProxyMergeGUI_Time; }
   QString              GetCommandLineArgs() const { return mCmdLineArgs; }
   ParseResults*        GetParseResults() { return mParseResultsPtr; }
   ParseResultsTiny*    TinyParseResults() { return mCurrentTinyResultsPtr; }
   ParseResults*        WaitForParseResults() const;
   void                 ToXML(XML_Context& aContext) const;
   bool                 FromXML(XML_Context& aContext);
   bool                 FromXML_StartupFile(XML_Context& aContext);
   bool                 SaveAs();
   bool                 SaveProject();
   bool                 ProjectFilePath(UtPath& aFilePath) const;
   bool                 CloseProject(bool aForce = false);
   CacheSourceProvider* SourceProvider() const { return mSourceProviderPtr; }
   WsfPProxy*           Proxy() const;
   void                 HandleProxyNodeMoved(const WsfPProxyNode&, const WsfPProxyNode&);
   void                 AddProjectDirectoryBrowse();
   void                 ShowRevisionManager();
   bool                 OpenFile(const std::string& aFilePath);
   bool                 OpenFileInTextEditor(const std::string& aFilePath);
   QString              GetCmdLineArgs() const { return mCmdLineArgs; }
   void                 OpenAction();

   void BuildUpContextMenu(QMenu& aMenu);

   void       UpdateProxyEpoch(const wizard::ProxyChange& aProxyChange);
   void       UpdateProxyEpoch();
   UtCalendar GetStartDateTime() const { return mEpoch; }

signals:
   void ParseUpdatedFull();
   void EventPipeRunConfigurationChanged();
   void Closed();

public slots:
   void ShowStartupFileDialog();

private:
   std::string GetXmlText() const;
   void        UndoCommand();
   void        RedoCommand();
   void        WriteStartDocument(UtQtXmlWriter& aWriter) const;
   void        WriteEndDocument(UtQtXmlWriter& aWriter) const;
   //! Records the grammar parsing time
   void P_CompleteGrammarParsing() noexcept;
   //! Resets the parse timer to record grammar parsing time
   void P_GrammarParsingStarted() noexcept;
   //! Records the input parsing time
   void P_CompleteInputParsing() noexcept;
   //! Resets the parse timer to record input parsing time
   void P_InputParsingStarted() noexcept;
   //! Records the proxy deserializing time
   void P_CompleteProxyDeserialize() noexcept;
   //! Notifies ParseUpdated() subscribers about the completed input parse
   void P_NotifyParseUpdated() noexcept;
   //! Resets the parse timer to record proxy deserializing time
   void P_ProxyDeserializeStarted() noexcept;
   //! Records the proxy merging/GUI updating time
   void P_CompleteProxyMergeGUI() noexcept;
   //! Resets the parse timer to record proxy merge/GUI update time
   void P_ProxyMergeGUI_Started() noexcept;
   //! Wizard ran into an error during the scenario-loading procedure, so set the mParsingAllComplete flag to abort the
   //! procedure.
   //! @pre mParsingAllComplete is "false"
   //! @post mParsingAllComplete is "true" because Wizard failed to load the scenario.
   void P_ParsingAborted() noexcept;
   //! Wizard completed the scenario-loading procedure without error, so set the mParsingAllComplete flag denoting a
   //! successful load.
   //! @pre mParsingAllComplete is "false"
   //! @post mParsingAllComplete is "true" because Wizard succeeded to load the scenario.
   void     P_CompleteAllParsing() noexcept;
   bool     RequestClose(bool aForce = false);
   QAction* GoToLocationAction(const UtTextDocumentRange& aRange);
   void     DeferUpdate(bool aFullyReparsed);
   void     FullUpdate();
   void     CleanUpFileList();
   void     Reparse();
   QString  ConstructTitle();

   friend class ProjectParseThread;

   //! The root-directory for the project browser
   UtPath              mProjectDirectory;
   std::vector<UtPath> mProjectDirectories;
   std::vector<UtPath> mStartupFiles;

   //! Source cache for the project
   CacheSourceProvider* mSourceProviderPtr;
   ProjectWorkspace*    mWorkspacePtr;
   UtCallbackHolder     mUtCallbacks;

   //! 'true' if the file list is dirty and needs updated
   bool mQuitting;

   UtPath      mProjectFilePath;
   std::string mLoadedXML;
   // This holds the XML configuration of the map related items
   // This is updated when the map window is closed because those
   // visual items are destroyed at that time.  Opening the map window
   // requires re-parsing this XML data
   std::string mMapItemsXML;

   UtCallbackListN<void()>       OneTimeParseComplete;
   UtPath                        mWorkingDirectory;
   std::string                   mExePath;
   QString                       mCmdLineArgs;
   std::unique_ptr<AttributeSet> mProxyAttributesRoot;
   RevisionStore*                mRevisionStore;

   using ComponentMap = std::map<std::string, std::unique_ptr<ProjectComponent>>;
   ComponentMap mComponents;

   ////////////////////////////////////////////////////////////////////
   // Parse results management --
   //    Because the parsing is done in another thread, and in multiple phases,
   //    managing the parse results isn't as simple as copying the results.
   //
   //    When parsing begins, a limited set of data is available for access,
   //     called the Tiny parse results.  When the parse phase is complete,
   //     the full parse results structure is available.
   //    The proxy data is computed in a subsequent parse phase, and
   //    added to the parse results when that phase is complete.
   ////////////////////////////////////////////////////////////////////

   //! When non-null, this is the parse results which are up to date
   ParseResults* mParseResultsPtr;

   //! A single parse results structure is allocated initially, although access to it is not always allowed.
   ParseResults* mParseResultsPrivatePtr;

   //! A single 'tiny' parse results structure allocated here
   ParseResultsTiny* mParseResultsTinyPtr;

   // Points to the latest 'tiny' parse results
   ParseResultsTiny* mCurrentTinyResultsPtr;

   ParseWorker* mParseWorker;
   //! Denotes whether Wizard completed all steps in loading a scenario.
   //! @par details
   //!   Wizard's loading procedure for a scenario goes as followed:
   //!   * Load and parse the grammar (Wizard only loads the grammar when the user loads a scenario file for the first
   //!   time after the application starts.  Otherwise, it just uses the existing grammar.)
   //!   * Load and parse the scenario
   //!   * Deserialize the parse tree into the proxy
   //!   * Merge the old and new proxies and update the GUIs
   //!   After and only after these steps is Wizard considered fully-loaded.
   //!   If this flag is set, then it may mean one of two things:
   //!   1) Wizard ran into an error during the loading procedure, thus aborted the parse.
   //!   2) Wizard completed the loading procedure without error.
   volatile bool mParsingAllComplete;

   std::unique_ptr<TerrainMonitor> mTerrainMonitor{nullptr};

   bool       mEpochListenerSetup{false};
   UtCalendar mEpoch;

   //! The time the WsfParseDefinitions spent parsing the WsfGrammar
   //! @note This time matters only when the user loads a scenario file for the first time after the application starts.
   //!       Otherwise, it is a negligible value, as Wizard does not reparse the grammar.
   double mLastGrammarParseTime{0.0};
   //! Denotes whether the grammar parsing is complete
   bool mGrammarParsingComplete{false};
   //! The time the WsfParser spent parsing the input files
   double mLastInputParseTime{0.0};
   //! Denotes whether the input parsing is complete
   bool mInputParsingComplete{false};
   //! Denotes whether the proxy deserialization is complete
   bool mProxyDeserializeComplete{false};
   //! The time the WsfPProxyDeserialize spent deserializing the parse tree
   double mLastProxyDeserializeTime{0.0};
   //! Denotes whether the ProxyMerge / GUI update is complete
   bool mProxyMergeGUI_Complete{false};
   //! The time the ProxyMerge spent merging the old and new WsfPProxys/Wizard spent updating its GUIs
   double mLastProxyMergeGUI_Time{0.0};
   //! The timer used to record the above, constituent parser/proxy times
   UtWallClock mParseTimer;
};
} // namespace wizard

Q_DECLARE_METATYPE(wizard::Project*)

#endif
