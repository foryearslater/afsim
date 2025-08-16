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

#ifndef SESSION_HPP
#define SESSION_HPP

#include <memory>

#include <QMutex>
#include <QObject>
#include <QQuaternion>

#include "Angle.hpp"
#include "Camera.hpp"
#include "DataScale.hpp"
#include "DockWidget.hpp"
#include "GlobalPlotOptions.hpp"
#include "LocatorOptions.hpp"
#include "Pattern.hpp"
#include "PatternNameGenerator.hpp"
#include "PatternUpdateManager.hpp"
#include "PatternVisualizerWidget.hpp"
#include "PatternsTreeModel.hpp"
#include "PatternsTreeView.hpp"
#include "PolarPlotOptions.hpp"
#include "UtMemory.hpp"
#include "WsfScenario.hpp"
#include "WsfSimulation.hpp"

class WsfSignature;
class WsfAntennaPattern;

namespace PatternVisualizer
{
class DockWidget;
class TreeItem;
class Canvas;
class GlobalPlotSettings;

class Session : public QObject
{
   Q_OBJECT
public:
   ~Session() override = default;

   static std::unique_ptr<Session> Create() { return std::unique_ptr<Session>{new Session{}}; }

   void SetPatternTreeRoot(TreeItem* aRootPtr) { mTreeRootPtr = aRootPtr; }
   void SetCanvas(Canvas* aCanvasPtr);
   void SetDockWidget(DockWidget* aDockWidgetPtr) { mDockWidgetPtr = aDockWidgetPtr; }
   void SetWidget(PatternVisualizerWidget* aWidgetPtr) { mWidgetPtr = aWidgetPtr; }

   void LoadPatterns(const QString& aFileName, WsfScenario& aScenarioPtr, const QString& aSigViewFile = QString());
   void PopulatePatternsDock(const QString& aFileName);

   bool                  HasPatterns();
   void                  ShowHidePattern(Pattern* aPatternPtr, bool aShow, bool aUserInitiated);
   PatternUpdateManager* GetPatternUpdateManager() { return mPatternUpdateManagerPtr.get(); }

   GlobalPlotOptions* GetGlobalPlotOptions() const { return mGlobalPlotOptionsPtr.get(); }

   PolarPlotOptions* GetPolarPlotOptions() const { return mPolarPlotOptionsPtr.get(); }

   LocatorOptions* GetLocatorOptions() const { return mLocatorOptionsPtr.get(); }
   float           GetCurrentValueAt(Angle aAzimuth, Angle aElevation, bool& aValid) const;

   Camera*            GetCamera() const { return mCameraPtr.get(); }
   PatternsTreeModel* GetPatternsTreeModel() const { return mPatternsTreeModelPtr.get(); }

   void             UpdateScaleDBRange();
   const DataScale& GetScale() const { return mGlobalPlotOptionsPtr->GetDataScale(); }
   void             Clear();

public slots:
   void FinalizePatternUpdate(Pattern* aPatternPtr, bool aDataChanged);
   void OnManualScalingChanged(bool aIsManual);
   void OnFrequencyChanged(float aValue);
   void OnFrequencyUnitsChanged(FrequencyUnits aUnits);

private:
   Session();

   Session(const Session&) = delete;
   Session& operator=(const Session&) = delete;

   void AddAntPatternToTree(const QString& aFileName, WsfAntennaPattern* aWsfPatternPtr, TreeItem* aParentPtr);
   void AddSignatureToTree(const QString& aFileName, Pattern::Type aSigType, WsfSignature* aWsfSigPtr, TreeItem* aParentPtr);
   bool RemovePatternFromList(Pattern* aPatternPtr);
   void RequestPatternUpdate(bool aUserInitiated, Pattern* aPatternPtr = nullptr);
   void SetDisplayedPatternStatus(bool aIsUpdated, Pattern* aPatternPtr = nullptr);

   // Map of displayed patterns (key = pattern, value=true if pattern is updated)
   QMap<Pattern*, bool> mDisplayedPatterns;
   QMutex               mDisplayedPatternsMutex;

   TreeItem*                mTreeRootPtr         = nullptr;
   TreeItem*                mTreeItemToSelectPtr = nullptr;
   Canvas*                  mCanvasPtr           = nullptr;
   DockWidget*              mDockWidgetPtr       = nullptr;
   PatternVisualizerWidget* mWidgetPtr           = nullptr;
   bool                     mIsTreeComplete      = false;

   std::unique_ptr<PatternNameGenerator> mNameGeneratorPtr;
   std::unique_ptr<PatternsTreeModel>    mPatternsTreeModelPtr;
   std::unique_ptr<PatternUpdateManager> mPatternUpdateManagerPtr;
   std::unique_ptr<GlobalPlotOptions>    mGlobalPlotOptionsPtr;
   std::unique_ptr<PolarPlotOptions>     mPolarPlotOptionsPtr;
   std::unique_ptr<LocatorOptions>       mLocatorOptionsPtr;
   std::unique_ptr<Camera>               mCameraPtr;
   std::unique_ptr<WsfSimulation>        mSimulationPtr;
};
} // namespace PatternVisualizer

#endif
