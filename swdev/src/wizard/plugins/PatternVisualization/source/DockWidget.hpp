// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2019 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef DOCKWIDGET_HPP
#define DOCKWIDGET_HPP

#include <array>

#include <QByteArray>
#include <QDockWidget>
#include <QMainWindow>
#include <QSettings>
#include <QString>

#include "Canvas.hpp"
#include "DebugLogWindow.hpp"
#include "Pattern.hpp"
#include "PatternVisualizerWidget.hpp"
#include "UtCallbackHolder.hpp"
#include "WkfDockWidget.hpp"
#include "WsfStandardApplication.hpp"

namespace PatternVisualizer
{
class Canvas;
class PatternVisualizerWidget;
class Session;

class DockWidget : public wkf::DockWidget
{
   Q_OBJECT

public:
   DockWidget(const QString&  aName,
              QMainWindow*    aParent,
              Qt::WindowFlags aFlags              = Qt::WindowFlags(),
              bool            aDefaultStartDocked = false);
   ~DockWidget() override = default;

   void ReadPatternFile(const QString& aStartupFile, const QString& aSigViewFile = QString());
   void ClearAllPatterns();

private:
   void PrepareForDockStateChange(bool aFloating) override;

   PatternVisualizerWidget* mPatternVisualizerWidgetPtr;
   std::unique_ptr<Session> mSessionPtr;

   std::unique_ptr<WsfScenario> mScenarioPtr;

   QString mStartupFilename;
};
} // namespace PatternVisualizer

#endif