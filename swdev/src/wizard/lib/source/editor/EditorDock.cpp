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

#include "EditorDock.hpp"

#include <QMainWindow>
#include <QMenu>
#include <QStyleOptionFrame>
#include <QStylePainter>
#include <QTabBar>

#include "Editor.hpp"
#include "EditorManager.hpp"
#include "Environment.hpp"
#include "ProjectWorkspace.hpp"
#include "Signals.hpp"
#include "UtQtMemory.hpp"
#include "WkfDebugPrefObject.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"

wizard::EditorDock::EditorDock(QMainWindow* aMainWindow, Editor* aEditor)
   : wkf::DockWidget(aEditor->windowTitle(), "wizeditor", aMainWindow, Qt::WindowFlags(), true)
   , mEditor(aEditor)
{
   QWidget* baseWidgetPtr = new QWidget(this);
   baseWidgetPtr->setLayout(new QVBoxLayout(baseWidgetPtr));
   baseWidgetPtr->layout()->setSpacing(0);
   baseWidgetPtr->layout()->setContentsMargins(QMargins(0, 0, 0, 0));
   baseWidgetPtr->layout()->addWidget(aEditor);

   mStatusBar = new QStatusBar;
   BuildStatusBar();
   baseWidgetPtr->layout()->addWidget(mStatusBar);

   setWidget(baseWidgetPtr);
   setWindowTitle(aEditor->GetShortName());
   setToolTip(aEditor->windowTitle());
   aEditor->setToolTip("");

   setTitleBarWidget(
      nullptr); // this and the override of OverrideTitleBar will prevent the base class from hiding our title bar
   show();
   raise();
   setAttribute(Qt::WA_DeleteOnClose);

   connect(this, &QDockWidget::visibilityChanged, this, &EditorDock::VisibilityChanged);
   connect(wizSignals, &Signals::SourceModifiedStateChange, this, &EditorDock::ModificationChanged);
   connect(aEditor, &QPlainTextEdit::cursorPositionChanged, this, &EditorDock::CursorPositionChanged);
   connect(aEditor,
           &QObject::destroyed,
           [this]()
           {
              mEditor = nullptr;
              deleteLater();
           });
   connect(wizard::ProjectWorkspace::Instance(), &wizard::ProjectWorkspace::ChangeDebug, this, &EditorDock::OnChangeDebug);

   wkfEnv.GetMainWindow()->AddDockWidgetToViewMenu(this, "Editors");
   Initialize();
   ModifyTabBars();

   // Parsing timer
   mStatusUpdateTimer.setInterval(300);
   connect(&mStatusUpdateTimer, &QTimer::timeout, this, &EditorDock::StatusUpdate);
   mStatusUpdateTimer.start();
}

wizard::EditorDock::~EditorDock()
{
   if (mEditor)
   {
      mEditor->close();
   }
}

void wizard::EditorDock::OverrideTitleBar(QWidget* aWidget)
{
   // if an editor is docked this will be called (this may be called in a lot of other situations)
   // so this is an opportunity to change any QTabBars that may need changing
   ModifyTabBars();
}

void wizard::EditorDock::VisibilityChanged(bool aVisible)
{
   if (aVisible)
   {
      widget()->setFocus();
   }
   wizard::ProjectWorkspace::Instance()->GetEditorManager()->VisibilityChanged(mEditor, aVisible);
}

void wizard::EditorDock::ModificationChanged(wizard::TextSource* aSourcePtr)
{
   if (aSourcePtr == mEditor->GetSource())
   {
      if (aSourcePtr->IsModified())
      {
         setWindowTitle(mEditor->GetShortName() + " *");
      }
      else
      {
         setWindowTitle(mEditor->GetShortName());
      }
      emit wizSignals->ActionsStateChange();
   }
}

void wizard::EditorDock::contextMenuEvent(QContextMenuEvent* aEvent)
{
   QMenu menu(this);
   mEditor->BuildTitleBarMenu(menu);

   menu.exec(mapToGlobal(aEvent->pos()));
}

void wizard::EditorDock::closeEvent(QCloseEvent* event)
{
   if (mEditor->AcceptsClose())
   {
      event->accept();
      QDockWidget::closeEvent(event);
      deleteLater();
   }
   else
   {
      event->ignore();
   }
}

void wizard::EditorDock::Initialize()
{
   bool usableEditor = false;
   setFloating(false);

   // Tabify the new editor dock on top of the most recent editor dock, if one exists.
   QList<EditorDock*> editors =
      wkfEnv.GetMainWindow()->centralWidget()->findChildren<EditorDock*>(QString(), Qt::FindDirectChildrenOnly);
   for (const auto& iter : editors)
   {
      if (!iter->isFloating() && iter != this) // Don't tabify this editor onto a floating widget or onto itself.
      {
         wkfEnv.GetMainWindow()->centralWidget()->tabifyDockWidget(iter, this);
         usableEditor = true;
         break;
      }
   }

   // If there isn't a pre-existing editor to use, this new editor should share space evenly with
   // whatever central widget was originally occupying that space.
   if (!usableEditor)
   {
      wkfEnv.GetMainWindow()->centralWidget()->addDockWidget(Qt::TopDockWidgetArea, this);
      QList<wkf::DockWidget*> centralWidgets =
         wkfEnv.GetMainWindow()->centralWidget()->findChildren<wkf::DockWidget*>(QString(), Qt::FindDirectChildrenOnly);

      for (const auto& iter : centralWidgets)
      {
         if (!iter->isFloating() && iter != this) // Don't resize the central widget if it's floating.
         {
            wkfEnv.GetMainWindow()->centralWidget()->resizeDocks({iter, this}, {1, 1}, Qt::Horizontal);
            break;
         }
      }
   }
}

void wizard::EditorDock::ModifyTabBars()
{
   QMainWindow* mainwin = GetMainWindow();
   if (mainwin)
   {
      QList<QTabBar*> tabList = mainwin->findChildren<QTabBar*>();
      if (!tabList.empty())
      {
         for (auto&& tabbar : tabList)
         {
            QTabBar* tabBarPtr(nullptr);
            for (int tabidx = 0; tabidx < tabbar->count(); ++tabidx)
            {
               if (tabbar->tabData(tabidx) == Key())
               {
                  tabbar->setElideMode(Qt::ElideNone);
                  tabbar->setUsesScrollButtons(true);
                  tabbar->setTabsClosable(true);
                  tabBarPtr = tabbar;
               }
            }

            if (tabBarPtr)
            {
               // If this tabbar is the QTabBar that this EditorDock is part of,
               // make sure all tabs are updated on the TabBar with the correct button size
               for (int tabidx = 0; tabidx < tabbar->count(); ++tabidx)
               {
                  // This tabButton() will not return a widget until setTabsClosable(true) is called.
                  auto button = tabbar->tabButton(tabidx, QTabBar::RightSide);
                  if (button)
                  {
                     button->setMaximumHeight(16);
                     button->setMaximumWidth(16);
                  }
               }

               UpdateConnections(tabBarPtr);
            }
         }
      }
   }
}

void wizard::EditorDock::TabCloseRequested(int aTabIndex)
{
   QTabBar* tabbar = dynamic_cast<QTabBar*>(QObject::sender());
   if (tabbar)
   {
      QVariant tabData = tabbar->tabData(aTabIndex);
      if (tabData.isValid() && (tabData == Key()))
      {
         mEditor->RequestCloseAction();
      }
   }
}

void wizard::EditorDock::paintEvent(QPaintEvent* aEventPtr)
{
   QStylePainter painter(this);
   if (isFloating())
   {
      QStyleOptionFrame options;
      options.initFrom(this);
      painter.drawPrimitive(QStyle::PE_FrameDockWidget, options);
   }
   else
   {
      if (mEditor)
      {
         QStyleOptionDockWidget options;
         initStyleOption(&options);
         UpdateDockTitle(options);
         painter.drawControl(QStyle::CE_DockWidgetTitle, options);
      }
   }
}

void wizard::EditorDock::BuildStatusBar()
{
   // Far left

   // Status
   mStatusLabelPtr = new QLabel;
   mStatusLabelPtr->setText("Ready");
   mStatusBar->addWidget(mStatusLabelPtr);

   // Spacer
   QWidget* spacerPtr = new QWidget;
   mStatusBar->addWidget(spacerPtr, 1);

   // Center

   // Line and column information
   mLineLabelPtr = new QLabel;
   mStatusBar->addWidget(mLineLabelPtr);

   mColumnLabelPtr = new QLabel;
   mStatusBar->addWidget(mColumnLabelPtr);

   QWidget* spacerPtr2 = new QWidget;
   mStatusBar->addWidget(spacerPtr2, 1);
}

void wizard::EditorDock::CursorPositionChanged()
{
   size_t offset, line, column;
   mEditor->GetCurrentPosition(offset, line, column);
   mLineLabelPtr->setText("Ln " + QString::number(line + 1));
   mColumnLabelPtr->setText("Col " + QString::number(column + 1));
}

void wizard::EditorDock::StatusUpdate()
{
   if (!ProjectWorkspace::Instance()->ParseIsUpToDate())
   {
      mStatusLabelPtr->setText("Parsing...");
   }
   else
   {
      mStatusLabelPtr->setText("Ready:");
   }
}

void wizard::EditorDock::UpdateConnections(QTabBar* aTabBarPtr)
{
   if (aTabBarPtr)
   {
      auto               mapDisplay = wkfEnv.GetMainWindow()->findChild<wkf::DockWidget*>("WkMapDisplay");
      QList<EditorDock*> editors =
         wkfEnv.GetMainWindow()->centralWidget()->findChildren<EditorDock*>(QString(), Qt::FindDirectChildrenOnly);
      for (int tabidx = 0; tabidx < aTabBarPtr->count(); ++tabidx)
      {
         if (aTabBarPtr->tabData(tabidx) == mapDisplay->Key())
         {
            // Reset the Map::DockWidget connection
            auto connection = wizEnv.GetMapCloseConnection();
            if (connection)
            {
               QObject::disconnect(connection);
            }
            connection = QObject::connect(aTabBarPtr,
                                          &QTabBar::tabCloseRequested,
                                          [aTabBarPtr, mapDisplay](int aIndex)
                                          {
                                             if (aTabBarPtr->tabData(aIndex) == mapDisplay->Key())
                                             {
                                                mapDisplay->close();
                                             }
                                          });
            wizEnv.SetMapCloseConnection(connection);
         }
         for (const auto& iter : editors)
         {
            if (aTabBarPtr->tabData(tabidx) == iter->Key())
            {
               if (iter->mTabCloseConnection)
               {
                  QObject::disconnect(iter->mTabCloseConnection);
               }
               iter->mTabCloseConnection =
                  QObject::connect(aTabBarPtr, &QTabBar::tabCloseRequested, iter, &EditorDock::TabCloseRequested);
            }
         }

         // Editors exists for the tab bar
         if (!editors.empty())
         {
            // may not the be place to install the filter but wanted to capture mouse clicks on editor tabs in one spot
            aTabBarPtr->installEventFilter(wizard::ProjectWorkspace::Instance()->GetEditorManager());
         }
      }
   }
}

void wizard::EditorDock::UpdateDockTitle(QStyleOptionDockWidget& aOptions) noexcept
{
   if (mEditor->GetSource())
   {
      auto source = mEditor->GetSource();
      if (source->IsModified())
      {
         // Add the filename to the dock title bar plus an asterisk
         aOptions.title = mEditor->GetShortName(false) + " *";
      }
      else
      {
         // Add the filename to the dock title bar
         aOptions.title = mEditor->GetShortName(false);
      }
   }
}

const QMainWindow* wizard::EditorDock::GetMainWindow() const noexcept
{
   return dynamic_cast<QMainWindow*>(parent());
}

QMainWindow* wizard::EditorDock::GetMainWindow() noexcept
{
   return dynamic_cast<QMainWindow*>(parent());
}

void wizard::EditorDock::DisableFullscreen()
{
   mFullScreenActionPtr->setEnabled(false);
}

void wizard::EditorDock::EnableFullscreen()
{
   mFullScreenActionPtr->setEnabled(true);
}

void wizard::EditorDock::OnChangeDebug(bool aIsActive)
{
   if (aIsActive)
   {
      DisableFullscreen();
   }
   else
   {
      EnableFullscreen();
   }
}
