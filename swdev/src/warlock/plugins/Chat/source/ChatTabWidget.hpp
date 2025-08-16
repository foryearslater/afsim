// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2020 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef CHATTABWIDGET_HPP
#define CHATTABWIDGET_HPP

#include <QMouseEvent>
#include <QTabWidget>

namespace Chat
{

//! TabWidget is a modified QTabWidget that lets users drag and drop tabs from one TabWidget to another.
class TabWidget : public QTabWidget
{
   Q_OBJECT
public:
   TabWidget(QWidget* aParentPtr, int aWindowId);

   int  GetId() const { return mWindowId; }
   void SetId(int aId) { mWindowId = aId; }

   bool eventFilter(QObject* aWatchedPtr, QEvent* aEventPtr) override;

   void dragEnterEvent(QDragEnterEvent* aEventPtr) override;

   void dropEvent(QDropEvent* aEventPtr) override;

signals:
   void MoveTab(TabWidget* aSourcePtr, int aIndex);

private:
   QPoint mDragStart;
   int    mWindowId;
};

} // namespace Chat

#endif
