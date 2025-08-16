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

#include "PatternVisualizerWidget.hpp"

#include "Debug.hpp"
#include "PrefObject.hpp"
#include "WkfEnvironment.hpp"

PatternVisualizer::PatternVisualizerWidget::PatternVisualizerWidget(Session& aSession, QWidget* aParentPtr)
   : QWidget(aParentPtr)
   , mSessionPtr(&aSession)
{
   mUi.setupUi(this);

   mUi.mPatternVisLayout->setContentsMargins(0, 0, 0, 0);

   mUi.patternsTree->setModel(mSessionPtr->GetPatternsTreeModel());
   mUi.patternsTree->setHeaderHidden(true);

   mUi.generic3DPlotSettings->FinalizeUi(mSessionPtr);
   mUi.globalPlotSettingsWidget->FinalizeUi(mSessionPtr, mSessionPtr->GetGlobalPlotOptions());
   mUi.polarConstantAzimuthPlotSettings->FinalizeUi(mSessionPtr->GetPolarPlotOptions());
   mUi.polarConstantElevationPlotSettings->FinalizeUi(mSessionPtr->GetPolarPlotOptions());

   mUi.polarConstantAzimuthPlotSettings->SetPlaneType(PlaneType::AZ);
   mUi.polarConstantElevationPlotSettings->SetPlaneType(PlaneType::EL);

   // Add the canvas to the grid layout.
   mCanvasPtr = new Canvas(mSessionPtr, this);
   mCanvasPtr->setMinimumSize(200, 200);
   mUi.mPatternVisLayout->addWidget(mCanvasPtr, 0, 1, mUi.mPatternVisLayout->rowCount(), 1);
   mSessionPtr->SetCanvas(mCanvasPtr);

   // Set the session pointer in preferences.
   wkfEnv.GetPreferenceObject<PrefObject>()->SetSession(mSessionPtr);

   // Show/hide different settings when the plot style changes.
   connect(mSessionPtr->GetGlobalPlotOptions(),
           &GlobalPlotOptions::PlotStyleChanged,
           this,
           &PatternVisualizerWidget::UpdatePlotStyle);

   // Update the locator value when a pattern is selected (not checked) in the tree view.
   connect(mUi.patternsTree,
           &PatternsTreeView::CurrentIndexChanged,
           mSessionPtr->GetLocatorOptions(),
           &LocatorOptions::RecomputeDockValue);

   // Update the locator value when the tree model changes.
   connect(mSessionPtr->GetPatternsTreeModel(),
           &PatternsTreeModel::dataChanged,
           this,
           &PatternVisualizerWidget::PatternDataChanged);

   mSessionPtr->SetWidget(this);
   UpdatePlotStyle(mSessionPtr->GetGlobalPlotOptions()->GetPlotStyle());
}

// Expands each item in the pattern tree.
void PatternVisualizer::PatternVisualizerWidget::ExpandAll()
{
   mUi.patternsTree->expandAll();
}

// Returns a pointer to the currently selected pattern.
PatternVisualizer::Pattern* PatternVisualizer::PatternVisualizerWidget::GetCurrentPattern() const
{
   QModelIndex index = mUi.patternsTree->currentIndex();
   if (!index.isValid())
   {
      return nullptr;
   }

   return mSessionPtr->GetPatternsTreeModel()->GetPatternAtIndex(index);
}

// Shows/hides the pattern tree and plot settings.
void PatternVisualizer::PatternVisualizerWidget::ShowTreeAndSettings(bool aShow)
{
   mUi.patternsTree->setVisible(aShow);
   mUi.globalPlotSettingsWidget->setVisible(aShow);

   if (aShow)
   {
      UpdatePlotStyle(mSessionPtr->GetGlobalPlotOptions()->GetPlotStyle());
   }
   else
   {
      mUi.polarConstantAzimuthPlotSettings->setVisible(false);
      mUi.polarConstantElevationPlotSettings->setVisible(false);
      mUi.generic3DPlotSettings->setVisible(false);
   }
}

// Shows/hides various settings depending on the current plot style.
void PatternVisualizer::PatternVisualizerWidget::UpdatePlotStyle(PlotStyle aPlotStyle)
{
   mUi.polarConstantAzimuthPlotSettings->setVisible(false);
   mUi.polarConstantElevationPlotSettings->setVisible(false);
   mUi.generic3DPlotSettings->setVisible(false);

   switch (aPlotStyle)
   {
   case PlotStyle::PolarConstantAzimuth:
      mUi.polarConstantAzimuthPlotSettings->setVisible(true);
      break;

   case PlotStyle::PolarConstantElevation:
      mUi.polarConstantElevationPlotSettings->setVisible(true);
      break;

   case PlotStyle::Generic3D:
      mUi.generic3DPlotSettings->setVisible(true);
      break;

   default:
      Debug::ShowFatalError("Unexpected plot style enumeration in the PatternVisualizerWidget!");
      break;
   }
}

void PatternVisualizer::PatternVisualizerWidget::PatternDataChanged(const QModelIndex&  aTopLeft,
                                                                    const QModelIndex&  aBottomRight,
                                                                    const QVector<int>& aRoles)
{
   mSessionPtr->GetLocatorOptions()->RecomputeDockValue();
}