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

#ifndef EDITORDOCK_HPP
#define EDITORDOCK_HPP

#include <QTimer>

#include "ScriptDebugger.hpp"
#include "UtCallbackHolder.hpp"
#include "WkfDockWidget.hpp"

class QLabel;
class QStatusBar;
class QTabBar;

namespace wizard
{
class Editor;
class TextSource;

//! A wkf::DockWidget containing a wizard::Editor
class EditorDock : public wkf::DockWidget
{
   Q_OBJECT

public:
   EditorDock(QMainWindow* aMainWindow, Editor* aEditor);
   ~EditorDock() override;

   void OverrideTitleBar(QWidget* aWidget) override;
   void VisibilityChanged(bool aVisible);
   void ModificationChanged(wizard::TextSource* aSourcePtr);
   void contextMenuEvent(QContextMenuEvent* aEvent) override;
   void closeEvent(QCloseEvent* event) override;
   void TabCloseRequested(int aTabIndex);
   void CursorPositionChanged();
   //! Get the wizard::Editor contained within this EditorDock
   //! @return the contained wizard::Editor
   Editor* GetEditor() { return mEditor; }

   //! Get the QMainWindow widget associated with this EditorDock
   //! @return the QMainWindow widget associated with this EditorDock
   const QMainWindow* GetMainWindow() const noexcept;
   //! Get the QMainWindow widget associated with this EditorDock
   //! @return the QMainWindow widget associated with this EditorDock
   QMainWindow* GetMainWindow() noexcept;

   void OnChangeDebug(bool aIsActive);

   void DisableFullscreen();
   void EnableFullscreen();

protected:
   void             paintEvent(QPaintEvent* aEventPtr) override;
   UtCallbackHolder mCallbackHolder;

private:
   void Initialize();
   void ModifyTabBars();
   void BuildStatusBar();
   void StatusUpdate();
   void UpdateConnections(QTabBar* aTabBarPtr);
   //! Updates the dock title
   //! @param aOptions are the DockWidget options containing the dock title
   void UpdateDockTitle(QStyleOptionDockWidget& aOptions) noexcept;

   Editor* mEditor;

   QTimer      mStatusUpdateTimer;
   QStatusBar* mStatusBar;
   QLabel*     mLineLabelPtr;
   QLabel*     mColumnLabelPtr;
   QLabel*     mStatusLabelPtr;
   bool        mDebugActive;

   QMetaObject::Connection mTabCloseConnection;
};
} // namespace wizard

#endif
