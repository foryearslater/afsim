// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2018 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#ifndef ENVIRONMENT_HPP
#define ENVIRONMENT_HPP

class QFileInfo;

#include <QApplication>
#include <QPainter>
#include <QTextDocument>

#include "Signals.hpp"
#include "UtCallbackHolder.hpp"
#include "ViExport.hpp"
#include "VtkInterface.hpp"
#include "WkfEnvironment.hpp"

#if defined(wizEnv)
#undef wizEnv
#endif
#define wizEnv static_cast<wizard::Environment&>(wizard::Environment::GetInstance())

class WsfStandardApplication;

namespace wizard
{
class Project;
class ProjectWorkspace;
class TextSource;
struct ToolDef;

// Determine when the application loses and gains focus; send the ApplicationActivated() event
class ActivationFilter : public QObject
{
public:
   bool mAppHasFocus;

   explicit ActivationFilter(QObject* aParent = nullptr)
      : QObject(aParent)
      , mAppHasFocus(true)
   {
   }

   bool eventFilter(QObject* obj, QEvent* event) override
   {
      if (event->type() == QEvent::FocusOut || event->type() == QEvent::FocusIn)
      {
         QFocusEvent* evt = (QFocusEvent*)event;
         if (evt->reason() == Qt::ActiveWindowFocusReason)
         {
            bool hasFocusNow = nullptr != QApplication::activeWindow();
            if (hasFocusNow != mAppHasFocus)
            {
               mAppHasFocus = hasFocusNow;
               if (mAppHasFocus)
               {
                  static bool activating = false;
                  if (!activating) // avoid reentering
                  {
                     activating = true;
                     wizSignals->ApplicationActivated();
                     activating = false;
                  }
               }
            }
         }
      }
      return QObject::eventFilter(obj, event);
   }
};

class ShutdownFilter : public QObject
{
public:
   explicit ShutdownFilter(QObject* aParent = nullptr)
      : QObject(aParent)
   {
   }

   bool eventFilter(QObject* obj, QEvent* aEvent) override;
};

class VI_EXPORT Environment : public QObject
{
   Q_OBJECT

public:
   Environment();
   ~Environment() override;

   static Environment& GetInstance();

   static void Create();
   static bool Exists() { return mInstancePtr != nullptr; }

   void StartUp();
   void Shutdown();

   void UpdateMenus();

   void PopulateRecents();

   bool LoadScenario(const QStringList& aFiles);

   void FileOpen();

   void OpenUrl(const QUrl& aUrl);
   void ProjectSettingsOpen(Project& aProject);

   void MarginClickAction(Qt::MouseButton aButton, TextSource* aSource, size_t aLineNumber);
   void MarginDraw(const QString& aSourcePath, QPainter& aPainter, const QMap<size_t, QPoint>& aVisibleLineMap);
   void GetHighlightLines(const QString& aSourcePath, QList<size_t>& aLineList);
   void LoadProject(const QUrl& aUrl);
   void BuildDeveloperMenu();
   void CopyFocusedData();
   void UpdatePlatform(const QString& aPlatformName);
   void DeletePlatform(const QString& aPlatformName);
   void DeleteSelection();

   WsfStandardApplication& GetWsfStandardApplication();

   void Undo();
   void Redo();
   void ClearUndo();
   //! Notifies the EditorManager to start capturing many changes to consider as one change
   void BeginUndoCapture() noexcept;
   //! Query the EditorManager whether it is capturing many changes to consider as one change
   //! @return the status of the EditorManager's undo capture
   bool IsCapturingUndo() const noexcept;
   //! Notifies the EditorManager to stop capturing many changes to consider as one change
   void EndUndoCapture() noexcept;

   //! @note If we create a platform by adding it to text, it will be a 'while' before the
   //! VaEntity exists.  This lets the system know that when it does exist we want to
   //! select it.
   void RequestSelectPlatform(const QString& aPlatformName);

   //! Sets the MoveMode in the VtkInterface
   void SetVtkMoveMode(wizard::VtkInterface::MoveMode aMoveMode) noexcept;
   //! Gets the MoveMode in the VtkInterface
   wizard::VtkInterface::MoveMode GetVtkMoveMode() const noexcept;

   //! Set the connection responsible for closing the Map::DockWidget
   //! @param aConnection is the new connection responsible for closing the Map::DockWidget
   void SetMapCloseConnection(const QMetaObject::Connection& aConnection) noexcept;
   //! Get the connection responsible for closing the Map::DockWidget
   //! @return the current connection responsible for closing the Map::DockWidget
   QMetaObject::Connection GetMapCloseConnection() const noexcept;

signals:
   void MarginClick(Qt::MouseButton aButton, const QString& aSourcePtr, size_t aLineNumber);
   void MarginDrawRequest(const QString& aSourcePath, QPainter& aPainter, const QMap<size_t, QPoint>& aVisibleLineMap);
   void HighlightLinesRequest(const QString& aSourcePath, QList<size_t>& aLineList);
   void FocusedCopyRequest();
   void PlatformUpdated(const QString& aPlatformName);
   void PlatformDeleted(const QString& aPlatformName);
   void WsfStandardApplicationPendingDestruction();

private:
   void ActionsForFileRequested(const QFileInfo& aFileInfo, QMenu* aMenu);
   void Execute(const QFileInfo& aFile, const ToolDef& aTool);
   void ProcessTags(QString& aString, const QFileInfo& aFile) const;
   bool MakeExecutable(QString& aExe, QStringList& aArgs) const;

   static Environment* mInstancePtr;

   std::unique_ptr<ProjectWorkspace> mWorkspacePtr;
   std::unique_ptr<VtkInterface>     mVtkInterfacePtr;

   //! The current connection responsible for closing the Map::DockWidget
   QMetaObject::Connection mMapCloseConnection;

   WsfStandardApplication* mWsfApplicationPtr;
};
} // namespace wizard
#endif // ENVIRONMENT_HPP
