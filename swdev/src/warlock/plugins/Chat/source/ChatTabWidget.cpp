// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2017 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "ChatTabWidget.hpp"

#include <QDrag>
#include <QMimeData>
#include <QTabBar>


Chat::TabWidget::TabWidget(QWidget* aParentPtr, int aWindowId)
   : QTabWidget(aParentPtr)
   , mWindowId(aWindowId)
{
   tabBar()->installEventFilter(this);
   tabBar()->setSelectionBehaviorOnRemove(QTabBar::SelectLeftTab);
   setAcceptDrops(true);
}

bool Chat::TabWidget::eventFilter(QObject* aWatchedPtr, QEvent* aEventPtr)
{
   if (aEventPtr->type() == QEvent::MouseButtonPress)
   {
      QMouseEvent* mouseEventPtr = dynamic_cast<QMouseEvent*>(aEventPtr);
      if (mouseEventPtr && mouseEventPtr->button() == Qt::LeftButton)
      {
         mDragStart = mouseEventPtr->pos();
      }
   }
   if (aEventPtr->type() == QEvent::MouseMove && tabText(currentIndex())[0] != QChar('+'))
   {
      QMouseEvent* moveEventPtr = dynamic_cast<QMouseEvent*>(aEventPtr);
      if (moveEventPtr)
      {
         if (std::abs(moveEventPtr->y() - mDragStart.y()) < 10)
         {
            return false;
         }
         auto dragPtr = new QDrag(this);
         dragPtr->setMimeData(new QMimeData);
         dragPtr->exec(Qt::CopyAction | Qt::MoveAction);

         mDragStart = QPoint();

         // Refresh tabs to prevent them from being stuck in incorrect positions when a drag is ignored
         setTabPosition(QTabWidget::South);
         setTabPosition(QTabWidget::North);

         return true;
      }
   }
   if (aEventPtr->type() == QEvent::MouseButtonRelease)
   {
      mDragStart = QPoint();
   }
   return false;
}

void Chat::TabWidget::dragEnterEvent(QDragEnterEvent* aEventPtr)
{
   QDragEnterEvent* dragEventPtr = dynamic_cast<QDragEnterEvent*>(aEventPtr);
   if (dragEventPtr)
   {
      dragEventPtr->acceptProposedAction();
   }
}

void Chat::TabWidget::dropEvent(QDropEvent* aEventPtr)
{
   QDropEvent* dropEventPtr = dynamic_cast<QDropEvent*>(aEventPtr);
   TabWidget*  sourcePtr    = dynamic_cast<TabWidget*>(aEventPtr->source());
   if (sourcePtr == this)
   {
      dropEventPtr->acceptProposedAction();
   }
   else if (dropEventPtr && sourcePtr)
   {
      dropEventPtr->acceptProposedAction();
      emit MoveTab(sourcePtr, sourcePtr->currentIndex());
   }
}
