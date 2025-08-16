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
#ifndef PluginPlatformHistory_HPP
#define PluginPlatformHistory_HPP

#include <memory>
#include <unordered_set>

#include <osg/Vec3>

#include "PlatformHistorySimInterface.hpp"
#include "UtColor.hpp"
#include "VaCallbackHolder.hpp"
#include "WkPlatform.hpp"
#include "WkPlugin.hpp"
#include "WkfPlatform.hpp"
#include "platform_history/WkfPlatformHistoryPrefWidget.hpp"

namespace wkf
{
class Platform;
class PlatformHistoryPrefObject;
enum TraceStateValue : int;
enum TrackableStateValue : int;
enum InterpolationLimitsMethod : int;
} // namespace wkf

namespace vespa
{
class VaAttachment;
class VaAttachmentTraceLine;
class VaEntity;
} // namespace vespa

namespace WkPlatformHistory
{
class Plugin : public warlock::PluginT<SimInterface>
{
   Q_OBJECT
public:
   Plugin(const QString& aPluginName, const size_t aUniqueId);
   ~Plugin() override = default;

   QList<wkf::PrefWidget*> GetPreferencesWidgets() const override;

   void           ResetOptionStates() override;
   void           SetPlatformOptionState(int aId, bool aSetVisible, wkf::Platform* aPlatformPtr) override;
   Qt::CheckState GetPlatformOptionState(int aId, const wkf::Platform* aPlatformPtr) const override;

   void UpdateTraceLineDetectionData(unsigned int aTargetId, unsigned int aSourceId, bool aInitiated);
   void UpdateTraceLineTrackData(unsigned int aTargetId, unsigned int aSourceId, bool aInitiated);
   void UpdateTraceLineAttackData(unsigned int aTargetId, unsigned int aSourceId, bool aInitiated);
   void PlatformDeletedHandler(unsigned int aId);

private:
   void BuildViewerContextMenu(QMenu* aMenu, vespa::VaViewer* aViewerPtr) override;

   void GuiUpdate() override;

   void TraceLineColorationHandler(wkf::TraceStateValue aColoration, const std::vector<QColor>& aColorList);
   void TraceLineLengthHandler(double aLength);
   void TraceLineWidthHandler(unsigned int aWidth);
   void WingRibbonLengthHandler(double aLength);
   void WingRibbonWidthScaleHandler(float aWidthScale);
   void WingRibbonDeathTransparencyPercentHandler(int aDeathTransparencyPercent);
   void WingRibbonColorSchemeHandler();

   void WingRibbonMainMaxColorHandler(QColor aMainMaxColor);
   void WingRibbonMainMinColorHandler(QColor aMainMinColor);
   void WingRibbonEdgeMaxColorHandler(QColor aEdgeMaxColor);
   void WingRibbonEdgeMinColorHandler(QColor aEdgeMinColor);
   void WingRibbonMainMaxFlippedColorHandler(QColor aMainMaxFlippedColor);
   void WingRibbonMainMinFlippedColorHandler(QColor aMainMinFlippedColor);
   void WingRibbonEdgeMaxFlippedColorHandler(QColor aEdgeMaxFlippedColor);
   void WingRibbonEdgeMinFlippedColorHandler(QColor aEdgeMinFlippedColor);

   void WingRibbonMainStateBeingTrackedHandler(wkf::TrackableStateValue stateToTrack);
   void WingRibbonEdgeStateBeingTrackedHandler(wkf::TrackableStateValue stateToTrack);

   void WingRibbonEdgeLimitTypeHandler(wkf::InterpolationLimitsMethod aEdgeLimitType);
   void WingRibbonMainLimitTypeHandler(wkf::InterpolationLimitsMethod aMainLimitType);

   void WingRibbonHardMaxMainLimitHandler(float aHardMaxMainLimit);
   void WingRibbonHardMinMainLimitHandler(float aHardMinMainLimit);
   void WingRibbonHardMaxEdgeLimitHandler(float aHardMaxEdgeLimit);
   void WingRibbonHardMinEdgeLimitHandler(float aHardMinEdgeLimit);

   void WingRibbonHardEdgeWidthHandler(float aHardEdgeWidth);
   void WingRibbonSoftEdgeWidthHandler(float aSoftEdgeWidth);

   void EntityRemovedCB(vespa::VaEntity* aEntityPtr);

   void ViewerDestroyedCB(vespa::VaViewer* aViewerPtr);

   struct TraceLineStateData
   {
      // Keep track of which platforms are currently triggering a state change. Some platforms can have multiple
      // sensors/weapons/etc. targeting another platform
      std::multiset<unsigned int> mDetections;
      std::multiset<unsigned int> mTracks;
      std::multiset<unsigned int> mAttacks;
   };

   class TracelineData
   {
   public:
      TracelineData(warlock::Platform& aPlatform, vespa::VaViewer* aViewerPtr);
      void RemoveAttachment();

      void SetVisible(bool aVisible);
      bool IsVisible();

      void SetColoration(const wkf::TraceStateValue& aColoration, const std::vector<UtColor>& aColorArray);

      void               SetTraceLineLength(double aLength_sec) { mTracelineLengthSec = aLength_sec; }
      void               SetLineWidth(unsigned int aWidth);
      void               SetState(int aState) { mState = aState; }
      void               Update();
      warlock::Platform* GetPlatform() { return mPlatformPtr; }

   private:
      warlock::Platform* mPlatformPtr;
      //! id of trace line attachment for fetching the attachment from the platform pointer
      unsigned int         mTraceLineUniqueId;
      double               mTracelineLengthSec;
      double               mLastUpdateTime;
      wkf::TraceStateValue mColoration;
      unsigned int         mLineWidth;
      int                  mState{0};
   };
   void AddImmersiveTraceLine(warlock::Platform& aPlatform, vespa::VaViewer* aViewerPtr);
   void RemoveImmersiveTraceLine(vespa::VaViewer* aViewerPtr);

   void CreateTraceLine(wkf::Platform* aPlatform, bool aSetVisible = false);
   void SetStandardTraceLineState(unsigned int aId);
   void SetImmersiveTraceLineState(unsigned int aViewerId, unsigned int aPlatformId);
   int  GetTraceLineState(unsigned int aId);
   void HandleTraceLinePrefChange();
   void UpdateTraceLineStateDataP(unsigned int                 aTargetId,
                                  unsigned int                 aSourceId,
                                  bool                         aInitiated,
                                  std::multiset<unsigned int>& aSet);

   // Data is maintained outside of TraceLineData to account for changing preferences when switching to a state coloration
   std::map<unsigned int, std::unique_ptr<TracelineData>> mTraceLineMap;
   std::map<unsigned int, std::unique_ptr<TracelineData>> mImmersiveTraceLines;
   std::map<unsigned int, TraceLineStateData>             mTraceLineStateData;

   class WingRibbonData
   {
   public:
      WingRibbonData(warlock::Platform& aPlatform, vespa::VaViewer* aViewer);
      void RemoveAttachment();

      void SetVisible(bool aVisible);
      bool IsVisible();
      void SetWingRibbonLength(double aLength_sec) { mWingRibbonLengthSec = aLength_sec; }

      void SetMainMaxColor(const UtColor& aMainMaxColor);
      void SetMainMinColor(const UtColor& aMainMinColor);
      void SetEdgeMaxColor(const UtColor& aEdgeMaxColor);
      void SetEdgeMinColor(const UtColor& aEdgeMinColor);
      void SetMainMaxFlippedColor(const UtColor& aMainMaxFlippedColor);
      void SetMainMinFlippedColor(const UtColor& aMainMinFlippedColor);
      void SetEdgeMaxFlippedColor(const UtColor& aEdgeMaxFlippedColor);
      void SetEdgeMinFlippedColor(const UtColor& aEdgeMinFlippedColor);

      void SetMainStateBeingTracked(wkf::TrackableStateValue stateToTrack);
      void SetEdgeStateBeingTracked(wkf::TrackableStateValue stateToTrack);
      void SetEdgeLimitType(wkf::InterpolationLimitsMethod aEdgeLimitType);
      void SetMainLimitType(wkf::InterpolationLimitsMethod aMainLimitType);
      void SetHardMaxMainLimit(float aHardMaxMainLimit);
      void SetHardMinMainLimit(float aHardMinMainLimit);
      void SetHardMaxEdgeLimit(float aHardMaxEdgeLimit);
      void SetHardMinEdgeLimit(float aHardMinEdgeLimit);

      void SetWidthScale(float aWidthScale);
      void SetHardEdgeWidth(float aHardEdgeWidth);
      void SetSoftEdgeWidth(float aSoftEdgeWidth);

      void        ResetCurrentScenarioLimitsIfNeeded();
      static void ResetCurrentScenarioMainLimits();
      static void ResetCurrentScenarioEdgeLimits();
      void        SetCurrentScenarioLimitsIfNeeded();

      void Update();

      warlock::Platform* mPlatformPtr;

      //! id of wing ribbon attachment for fetching the attachment from the platform pointer
      unsigned int mWingRibbonUniqueId;
      double       mLastUpdateTime;

   private:
      float RetrieveStateValFrom(vespa::VaEntity* aPlatformPtr, wkf::TrackableStateValue aStateTracked);

      double  mWingRibbonLengthSec;
      UtColor mWingRibbonMainMaxColor;
      UtColor mWingRibbonMainMinColor;
      UtColor mWingRibbonEdgeMaxColor;
      UtColor mWingRibbonEdgeMinColor;
      UtColor mWingRibbonMainMaxFlippedColor;
      UtColor mWingRibbonMainMinFlippedColor;
      UtColor mWingRibbonEdgeMaxFlippedColor;
      UtColor mWingRibbonEdgeMinFlippedColor;
      float   mWingRibbonWidthScale;
      float   mWingRibbonHardEdgeWidth;
      float   mWingRibbonSoftEdgeWidth;

      wkf::TrackableStateValue mEdgeStateBeingTracked;
      wkf::TrackableStateValue mMainStateBeingTracked;

      wkf::InterpolationLimitsMethod mEdgeLimitType;
      wkf::InterpolationLimitsMethod mMainLimitType;
      float                          mHardMaxMainLimit;
      float                          mHardMinMainLimit;
      float                          mHardMaxEdgeLimit;
      float                          mHardMinEdgeLimit;
      float                          mWidthScale;
      float                          mHardEdgeWidth;
      float                          mSoftEdgeWidth;

      static float mCurrentScenarioMainMax;
      static float mCurrentScenarioMainMin;
      static float mCurrentScenarioEdgeMax;
      static float mCurrentScenarioEdgeMin;

      osg::Vec3 mWingTipA;
      osg::Vec3 mWingTipB;
   };
   void AddImmersiveWingRibbon(warlock::Platform& aPlatform, vespa::VaViewer* aViewerPtr);
   void RemoveImmersiveWingRibbon(vespa::VaViewer* aViewerPtr);

   std::map<unsigned int, std::unique_ptr<WingRibbonData>> mWingRibbonMap;
   std::map<unsigned int, std::unique_ptr<WingRibbonData>> mImmersiveWingRibbons;

   vespa::VaCallbackHolder mCallbacks;

   const wkf::PlatformHistoryPrefObject* mPrefObjectPtr;
   // Use guarded pointers because objects will be owned by main window
   PluginUiPointer<wkf::PlatformHistoryPrefWidget> mPrefWidgetPtr;

   int mTraceLineType;
   int mWingRibbonType;
};
} // namespace WkPlatformHistory

#endif
