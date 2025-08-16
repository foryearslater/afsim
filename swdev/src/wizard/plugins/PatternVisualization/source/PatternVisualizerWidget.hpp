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

#ifndef PATTERNVISUALIZERWIDGET_HPP
#define PATTERNVISUALIZERWIDGET_HPP

#include <QWidget>

#include "Canvas.hpp"
#include "Pattern.hpp"
#include "PatternsTreeView.hpp"
#include "ui_PatternVisualizer.h"

namespace PatternVisualizer
{
class Canvas;
class Pattern;

// A container widget that holds the patterns tree, the display options, and the canvas.
class PatternVisualizerWidget : public QWidget
{
public:
   PatternVisualizerWidget(Session& aSession, QWidget* aParentPtr = nullptr);
   ~PatternVisualizerWidget() override = default;

   void     ExpandAll();
   Pattern* GetCurrentPattern() const;
   void     ShowTreeAndSettings(bool aShow);
   void     UpdatePlotStyle(PlotStyle aPlotStyle);

private slots:
   void PatternDataChanged(const QModelIndex& aTopLeft, const QModelIndex& aBottomRight, const QVector<int>& aRoles);

private:
   Ui::PatternVisualizer mUi;

   Session* mSessionPtr;
   Canvas*  mCanvasPtr;
};
} // namespace PatternVisualizer

#endif // !PATTERNVISUALIZERWIDGET_HPP
