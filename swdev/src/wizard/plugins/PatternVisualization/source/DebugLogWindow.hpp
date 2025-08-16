// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2016 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef DEBUGLOGWINDOW_HPP
#define DEBUGLOGWINDOW_HPP

#include <memory>

#include "ui_DebugLogWindow.h"

namespace PatternVisualizer
{
class DebugLogModel;

class DebugLogWindow : public QWidget
{
   Q_OBJECT
public:
   explicit DebugLogWindow(QWidget* aParent = nullptr);
   ~DebugLogWindow() override;

signals:
   void VisibilityChanged();

protected:
   void hideEvent(QHideEvent* aEvent) override;
   void showEvent(QShowEvent* aEvent) override;

private:
   Ui::DebugLogWindow             mUI;
   std::unique_ptr<DebugLogModel> mDebugLogModel;
};
} // namespace PatternVisualizer

#endif
