// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2016-2017 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#include "PlatformHistoryPlugin.hpp"

#include <QAction>
#include <QMenu>
#include <osg/Node>
#include <osg/Vec3>

#include "UtMemory.hpp"
#include "UtQt.hpp"
#include "UtoEntity.hpp"
#include "VaAttachmentTraceLine.hpp"
#include "VaAttachmentWingRibbon.hpp"
#include "VaCamera.hpp"
#include "VaCameraMotionTethered.hpp"
#include "VaEntity.hpp"
#include "VaEnvironment.hpp"
#include "VaModelDatabase.hpp"
#include "VaViewer.hpp"
#include "VaViewerManager.hpp"
#include "WkPlatform.hpp"
#include "WkfEnvironment.hpp"
#include "WkfObserver.hpp"
#include "WkfPlatform.hpp"
#include "WkfTeamVisibilityPrefObject.hpp"
#include "WkfUtils.hpp"
#include "WkfViewer.hpp"
#include "WkfVtkEnvironment.hpp"
#include "platform_history/WkfPlatformHistoryPrefObject.hpp"
#include "platform_history/WkfPlatformHistoryPrefWidget.hpp"

namespace
{
const int cDEFAULT_STATE  = 0;
const int cDETECTED_STATE = 1;
const int cTRACKED_STATE  = 2;
const int cATTACKED_STATE = 3;
} // namespace

WKF_PLUGIN_DEFINE_SYMBOLS(WkPlatformHistory::Plugin,
                          "Platform History",
                          "The Platform History plugin provides the ability to visualize a platform's state history by "
                          "drawing trace lines and wing ribbons on the main map display.",
                          "warlock")

WkPlatformHistory::Plugin::Plugin(const QString& aPluginName, const size_t aUniqueId)
   : warlock::PluginT<SimInterface>(aPluginName, aUniqueId)
   , mPrefWidgetPtr(new wkf::PlatformHistoryPrefWidget())
{
   mCallbacks.Add(wkf::Observer::EntityRemoved.Connect(&Plugin::EntityRemovedCB, this));
   mCallbacks.Add(wkf::Observer::ViewerDestroyed.Connect(&Plugin::ViewerDestroyedCB, this));

   mPrefObjectPtr = mPrefWidgetPtr->GetPreferenceObject();

   connect(mPrefObjectPtr,
           &wkf::PlatformHistoryPrefObject::TraceLineColorationChanged,
           this,
           &Plugin::TraceLineColorationHandler);
   connect(mPrefObjectPtr, &wkf::PlatformHistoryPrefObject::TraceLineLengthChanged, this, &Plugin::TraceLineLengthHandler);
   connect(mPrefObjectPtr, &wkf::PlatformHistoryPrefObject::TraceLineWidthChanged, this, &Plugin::TraceLineWidthHandler);
   connect(mPrefObjectPtr, &wkf::PlatformHistoryPrefObject::WingRibbonsLengthChanged, this, &Plugin::WingRibbonLengthHandler);
   connect(mPrefObjectPtr,
           &wkf::PlatformHistoryPrefObject::WingRibbonWidthScaleFieldChanged,
           this,
           &Plugin::WingRibbonWidthScaleHandler);
   connect(mPrefObjectPtr,
           &wkf::PlatformHistoryPrefObject::DeathTransparencySliderChanged,
           this,
           &Plugin::WingRibbonDeathTransparencyPercentHandler);
   connect(mPrefObjectPtr, &wkf::PlatformHistoryPrefObject::ColorSchemeChanged, this, &Plugin::WingRibbonColorSchemeHandler);

   QTreeWidgetItem* parent = RegisterOptionGroup(nullptr, "History");
   mTraceLineType          = RegisterOption(parent, "Trace lines")->type();
   mWingRibbonType         = RegisterOption(parent, "Wing ribbons")->type();
}

QList<wkf::PrefWidget*> WkPlatformHistory::Plugin::GetPreferencesWidgets() const
{
   QList<wkf::PrefWidget*> list;
   list.push_back(mPrefWidgetPtr);
   return list;
}

void WkPlatformHistory::Plugin::BuildViewerContextMenu(QMenu* aMenu, vespa::VaViewer* aViewerPtr)
{
   UtVariant v = aViewerPtr->GetData("type");
   if (!v.IsUnset())
   {
      std::string type = v.GetString();
      if (type == "immersive")
      {
         vespa::VaCameraMotionTethered* tether =
            dynamic_cast<vespa::VaCameraMotionTethered*>(aViewerPtr->GetCamera()->GetCameraMotion());
         if (tether)
         {
            warlock::Platform* plat = dynamic_cast<warlock::Platform*>(tether->GetEntity());

            if (plat)
            {
               if (mImmersiveTraceLines.find(aViewerPtr->GetUniqueId()) != mImmersiveTraceLines.end())
               {
                  QAction* remTraceline = new QAction("Remove Traceline", aMenu);
                  aMenu->addAction(remTraceline);
                  connect(remTraceline,
                          &QAction::triggered,
                          [aViewerPtr, this]() { RemoveImmersiveTraceLine(aViewerPtr); });
               }
               else
               {
                  QAction* addTraceline = new QAction("Add Traceline", aMenu);
                  aMenu->addAction(addTraceline);
                  connect(addTraceline,
                          &QAction::triggered,
                          [aViewerPtr, plat, this]() { AddImmersiveTraceLine(*plat, aViewerPtr); });
               }
               if (mImmersiveWingRibbons.find(aViewerPtr->GetUniqueId()) != mImmersiveWingRibbons.end())
               {
                  QAction* remWingRibbon = new QAction("Remove Wing Ribbon", aMenu);
                  aMenu->addAction(remWingRibbon);
                  connect(remWingRibbon,
                          &QAction::triggered,
                          [aViewerPtr, this]() { RemoveImmersiveWingRibbon(aViewerPtr); });
               }
               else
               {
                  QAction* addWingRibbon = new QAction("Add Wing Ribbon", aMenu);
                  aMenu->addAction(addWingRibbon);
                  connect(addWingRibbon,
                          &QAction::triggered,
                          [aViewerPtr, plat, this]() { AddImmersiveWingRibbon(*plat, aViewerPtr); });
               }
            }
         }
      }
   }
}

/*************************************************************************************************
 * @brief   Updates the trace line and wing ribbon attachments with new datapoints
 * @date 7/14/2017
 **************************************************************************************************/
void WkPlatformHistory::Plugin::GuiUpdate()
{
   mInterfacePtr->ProcessEvents(*this);

   for (auto& iter : mTraceLineMap)
   {
      iter.second->Update();
   }

   for (auto& iter : mImmersiveTraceLines)
   {
      iter.second->Update();
   }

   for (auto& iter : mWingRibbonMap)
   {
      iter.second->ResetCurrentScenarioLimitsIfNeeded();
   }

   for (auto& iter : mWingRibbonMap)
   {
      iter.second->Update();
   }

   for (auto& iter : mWingRibbonMap)
   {
      iter.second->SetCurrentScenarioLimitsIfNeeded();
   }

   for (auto& iter : mImmersiveWingRibbons)
   {
      iter.second->ResetCurrentScenarioLimitsIfNeeded();
   }

   for (auto& iter : mImmersiveWingRibbons)
   {
      iter.second->Update();
   }

   for (auto& iter : mImmersiveWingRibbons)
   {
      iter.second->SetCurrentScenarioLimitsIfNeeded();
   }
}

/*************************************************************************************************
 * @brief   Handler, called when the trace line coloration is changed by the gui.
 * @param   aColoration  The mode of coloration, either state or team color.
 * @date 8/2/2017
 **************************************************************************************************/
void WkPlatformHistory::Plugin::TraceLineColorationHandler(wkf::TraceStateValue       aColoration,
                                                           const std::vector<QColor>& aColorList)
{
   HandleTraceLinePrefChange();
}

/*************************************************************************************************
 * @brief   Handler, called when the trace line length is changed by the gui.
 * @param   aLength  The length, in seconds, that the tracelines will trail behind for.
 * @date 7/14/2017
 **************************************************************************************************/
void WkPlatformHistory::Plugin::TraceLineLengthHandler(double aLength)
{
   HandleTraceLinePrefChange();
}

void WkPlatformHistory::Plugin::TraceLineWidthHandler(unsigned int aWidth)
{
   HandleTraceLinePrefChange();
}

/*************************************************************************************************
 * @brief   Handler, called when the wing ribbon length is changed by the gui. Changes the length
 *          that wingribbons will trail behind their entity for.
 * @param   aLength  The length, in seconds, that the wingribbon will trail behind for.
 * @date 7/14/2017
 **************************************************************************************************/
void WkPlatformHistory::Plugin::WingRibbonLengthHandler(double aLength)
{
   for (auto& iter : mWingRibbonMap)
   {
      iter.second->SetWingRibbonLength(aLength);
   }
   for (auto& iter : mImmersiveWingRibbons)
   {
      iter.second->SetWingRibbonLength(aLength);
   }
}

void WkPlatformHistory::Plugin::WingRibbonMainMaxColorHandler(QColor aMainMaxColor)
{
   for (auto& iter : mWingRibbonMap)
   {
      iter.second->SetMainMaxColor(UtQtColor(aMainMaxColor));
   }
   for (auto& iter : mImmersiveWingRibbons)
   {
      iter.second->SetMainMaxColor(UtQtColor(aMainMaxColor));
   }
}
void WkPlatformHistory::Plugin::WingRibbonMainMinColorHandler(QColor aMainMinColor)
{
   for (auto& iter : mWingRibbonMap)
   {
      iter.second->SetMainMinColor(UtQtColor(aMainMinColor));
   }
   for (auto& iter : mImmersiveWingRibbons)
   {
      iter.second->SetMainMinColor(UtQtColor(aMainMinColor));
   }
}
void WkPlatformHistory::Plugin::WingRibbonEdgeMaxColorHandler(QColor aEdgeMaxColor)
{
   for (auto& iter : mWingRibbonMap)
   {
      iter.second->SetEdgeMaxColor(UtQtColor(aEdgeMaxColor));
   }
   for (auto& iter : mImmersiveWingRibbons)
   {
      iter.second->SetEdgeMaxColor(UtQtColor(aEdgeMaxColor));
   }
}
void WkPlatformHistory::Plugin::WingRibbonEdgeMinColorHandler(QColor aEdgeMinColor)
{
   for (auto& iter : mWingRibbonMap)
   {
      iter.second->SetEdgeMinColor(UtQtColor(aEdgeMinColor));
   }
   for (auto& iter : mImmersiveWingRibbons)
   {
      iter.second->SetEdgeMinColor(UtQtColor(aEdgeMinColor));
   }
}
void WkPlatformHistory::Plugin::WingRibbonMainMaxFlippedColorHandler(QColor aMainMaxFlippedColor)
{
   for (auto& iter : mWingRibbonMap)
   {
      iter.second->SetMainMaxFlippedColor(UtQtColor(aMainMaxFlippedColor));
   }
   for (auto& iter : mImmersiveWingRibbons)
   {
      iter.second->SetMainMaxFlippedColor(UtQtColor(aMainMaxFlippedColor));
   }
}
void WkPlatformHistory::Plugin::WingRibbonMainMinFlippedColorHandler(QColor aMainMinFlippedColor)
{
   for (auto& iter : mWingRibbonMap)
   {
      iter.second->SetMainMinFlippedColor(UtQtColor(aMainMinFlippedColor));
   }
   for (auto& iter : mImmersiveWingRibbons)
   {
      iter.second->SetMainMinFlippedColor(UtQtColor(aMainMinFlippedColor));
   }
}
void WkPlatformHistory::Plugin::WingRibbonEdgeMaxFlippedColorHandler(QColor aEdgeMaxFlippedColor)
{
   for (auto& iter : mWingRibbonMap)
   {
      iter.second->SetEdgeMaxFlippedColor(UtQtColor(aEdgeMaxFlippedColor));
   }
   for (auto& iter : mImmersiveWingRibbons)
   {
      iter.second->SetEdgeMaxFlippedColor(UtQtColor(aEdgeMaxFlippedColor));
   }
}
void WkPlatformHistory::Plugin::WingRibbonEdgeMinFlippedColorHandler(QColor aEdgeMinFlippedColor)
{
   for (auto& iter : mWingRibbonMap)
   {
      iter.second->SetEdgeMinFlippedColor(UtQtColor(aEdgeMinFlippedColor));
   }
   for (auto& iter : mImmersiveWingRibbons)
   {
      iter.second->SetEdgeMinFlippedColor(UtQtColor(aEdgeMinFlippedColor));
   }
}

/*************************************************************************************************
 * @brief   Handler, called when the wing ribbon main state being tracked is changed in gui.
 * @param   stateToTrack   The state to track, that affects what values will be recorded for the
 *                         main of the wing ribbons.
 * @date 7/14/2017
 **************************************************************************************************/
void WkPlatformHistory::Plugin::WingRibbonMainStateBeingTrackedHandler(wkf::TrackableStateValue stateToTrack)
{
   WingRibbonData::ResetCurrentScenarioMainLimits();
   for (auto& iter : mWingRibbonMap)
   {
      iter.second->SetMainStateBeingTracked(stateToTrack);
   }
   for (auto& iter : mImmersiveWingRibbons)
   {
      iter.second->SetMainStateBeingTracked(stateToTrack);
   }
}

/*************************************************************************************************
 * @brief   Handler, called when the wing ribbon edge state being tracked is changed in gui.
 * @param   stateToTrack   The state to track, that affects what values will be recorded for the
 *                         edge of the wing ribbons.
 * @date 7/14/2017
 **************************************************************************************************/
void WkPlatformHistory::Plugin::WingRibbonEdgeStateBeingTrackedHandler(wkf::TrackableStateValue stateToTrack)
{
   WingRibbonData::ResetCurrentScenarioEdgeLimits();
   for (auto& iter : mWingRibbonMap)
   {
      iter.second->SetEdgeStateBeingTracked(stateToTrack);
   }
   for (auto& iter : mImmersiveWingRibbons)
   {
      iter.second->SetEdgeStateBeingTracked(stateToTrack);
   }
}

/*************************************************************************************************
 * @brief   Handler, called when the wing ribbon edge limit type is changed by the gui. Changes
 *          how the colors of the wing ribbon's edge area is calculated in response to the recorded state
 *          values.
 * @param   aEdgeLimitType Type of the edge limit that determines how the colors of the wing
 *                         ribbon's edge is calculated in response to the recorded state values.
 * @date 7/14/2017
 **************************************************************************************************/
void WkPlatformHistory::Plugin::WingRibbonEdgeLimitTypeHandler(wkf::InterpolationLimitsMethod aEdgeLimitType)
{
   // if (aEdgeLimitType == eCURRENT_SCENARIO_LIMITS){
   WingRibbonData::ResetCurrentScenarioEdgeLimits();
   //}
   for (auto& iter : mWingRibbonMap)
   {
      iter.second->SetEdgeLimitType(aEdgeLimitType);
   }
   for (auto& iter : mImmersiveWingRibbons)
   {
      iter.second->SetEdgeLimitType(aEdgeLimitType);
   }
}

/*************************************************************************************************
 * @brief   Handler, called when the wing ribbon main limit type is changed by the gui. Changes
 *          how the colors of the wing ribbon's main area is calculated in response to the recorded state
 *          values.
 * @param   aMainLimitType Type of the main limit that determines how the colors of the wing
 *                         ribbon's main is calculated in response to the recorded state values.
 * @date 7/14/2017
 **************************************************************************************************/
void WkPlatformHistory::Plugin::WingRibbonMainLimitTypeHandler(wkf::InterpolationLimitsMethod aMainLimitType)
{
   // if (aMainLimitType == eCURRENT_SCENARIO_LIMITS){
   WingRibbonData::ResetCurrentScenarioMainLimits();
   //}
   for (auto& iter : mWingRibbonMap)
   {
      iter.second->SetMainLimitType(aMainLimitType);
   }
   for (auto& iter : mImmersiveWingRibbons)
   {
      iter.second->SetMainLimitType(aMainLimitType);
   }
}

/*************************************************************************************************
 * @brief   Handler, called when the wing ribbon hard maximum main limit is change by the gui.
 *          Used in determining what color to display in the wing ribbon's main/center area when the
 *          main limits type is set  to eHARD_LIMITS.
 * @note this is will only take effect when the main InterpolationLimitsMethod is set to
 *       eHARD_LIMITS.
 * @param   aHardMaxMainLimit The hard maximum main limit.
 * @date 7/14/2017
 **************************************************************************************************/
void WkPlatformHistory::Plugin::WingRibbonHardMaxMainLimitHandler(float aHardMaxMainLimit)
{
   for (auto& iter : mWingRibbonMap)
   {
      iter.second->SetHardMaxMainLimit(aHardMaxMainLimit);
   }
   for (auto& iter : mImmersiveWingRibbons)
   {
      iter.second->SetHardMaxMainLimit(aHardMaxMainLimit);
   }
}

/*************************************************************************************************
 * @brief   Handler, called when the wing ribbon hard minimum main limit is changed by the gui.
 *          Used in determining what color to display in the wing ribbon's main/center area when the
 *          main limits type is set  to eHARD_LIMITS.
 * @note Only used when the main InterpolationLimitsMethod is set to eHARD_LIMITS.
 * @param   aHardMinMainLimit The hard minimum main limit.
 * @date 7/14/2017
 **************************************************************************************************/
void WkPlatformHistory::Plugin::WingRibbonHardMinMainLimitHandler(float aHardMinMainLimit)
{
   for (auto& iter : mWingRibbonMap)
   {
      iter.second->SetHardMinMainLimit(aHardMinMainLimit);
   }
   for (auto& iter : mImmersiveWingRibbons)
   {
      iter.second->SetHardMinMainLimit(aHardMinMainLimit);
   }
}

/*************************************************************************************************
 * @brief   Handler, called when the wing ribbon hard maximum edge limit is changed by the gui.
 *          Used in determining what color to display in the wing ribbon's edge area when the main
 *          limits type is set  to eHARD_LIMITS.
 * @note Only used when the edge InterpolationLimitsMethod is set to eHARD_LIMITS.
 * @param   aHardMaxEdgeLimit The hard maximum edge limit.
 * @date 7/14/2017
 **************************************************************************************************/
void WkPlatformHistory::Plugin::WingRibbonHardMaxEdgeLimitHandler(float aHardMaxEdgeLimit)
{
   for (auto& iter : mWingRibbonMap)
   {
      iter.second->SetHardMaxEdgeLimit(aHardMaxEdgeLimit);
   }
   for (auto& iter : mImmersiveWingRibbons)
   {
      iter.second->SetHardMaxEdgeLimit(aHardMaxEdgeLimit);
   }
}

/*************************************************************************************************
 * @brief   Handler, called when the wing ribbon hard minimum edge limit is changed by the gui.
 *          Used in determining what color to display in the wing ribbon's edge area when the main
 *          limits type is set  to eHARD_LIMITS.
 * @note Only used when the edge InterpolationLimitsMethod is set to eHARD_LIMITS.
 * @param   aHardMinEdgeLimit The hard minimum edge limit.
 * @date 7/14/2017
 **************************************************************************************************/
void WkPlatformHistory::Plugin::WingRibbonHardMinEdgeLimitHandler(float aHardMinEdgeLimit)
{
   for (auto& iter : mWingRibbonMap)
   {
      iter.second->SetHardMinEdgeLimit(aHardMinEdgeLimit);
   }
   for (auto& iter : mImmersiveWingRibbons)
   {
      iter.second->SetHardMinEdgeLimit(aHardMinEdgeLimit);
   }
}

/*************************************************************************************************
 * @brief   Handler, called when the wing ribbon death transparency percent is changed by the
 *          gui. Used in determining how transparent a wing ribbon is at states where the platform
 *          is dead.
 * @param   aDeathTransparencyPercent  The death transparency as a percent between 0-100 for how
 *                                     transparent the wing ribbon becomes when a platform is
 *                                     dead.
 * @date 7/24/2017
 **************************************************************************************************/
void WkPlatformHistory::Plugin::WingRibbonDeathTransparencyPercentHandler(int aDeathTransparencyPercent)
{
   // convert to float from 0-1 and invert as alpha at 1 is fully opaque, not fully transparent.
   vespa::VaAttachmentWingRibbon::SetTransparencyOnDeath(1.0f - aDeathTransparencyPercent / 100.0f);
}

void WkPlatformHistory::Plugin::WingRibbonColorSchemeHandler()
{
   WingRibbonMainMaxColorHandler(mPrefObjectPtr->GetWingRibbonMainMaxColor());
   WingRibbonMainMinColorHandler(mPrefObjectPtr->GetWingRibbonMainMinColor());
   WingRibbonEdgeMaxColorHandler(mPrefObjectPtr->GetWingRibbonEdgeMaxColor());
   WingRibbonEdgeMinColorHandler(mPrefObjectPtr->GetWingRibbonEdgeMinColor());
   WingRibbonMainMaxFlippedColorHandler(mPrefObjectPtr->GetWingRibbonMainMaxFlippedColor());
   WingRibbonMainMinFlippedColorHandler(mPrefObjectPtr->GetWingRibbonMainMinFlippedColor());
   WingRibbonEdgeMaxFlippedColorHandler(mPrefObjectPtr->GetWingRibbonEdgeMaxFlippedColor());
   WingRibbonEdgeMinFlippedColorHandler(mPrefObjectPtr->GetWingRibbonEdgeMinFlippedColor());
   WingRibbonHardMaxEdgeLimitHandler(mPrefObjectPtr->GetWingRibbonHardMaxEdgeLimit());
   WingRibbonHardMinEdgeLimitHandler(mPrefObjectPtr->GetWingRibbonHardMinEdgeLimit());
   WingRibbonHardMaxMainLimitHandler(mPrefObjectPtr->GetWingRibbonHardMaxMainLimit());
   WingRibbonHardMinMainLimitHandler(mPrefObjectPtr->GetWingRibbonHardMinMainLimit());

   WingRibbonHardEdgeWidthHandler(mPrefObjectPtr->GetWingRibbonHardEdgeWidth());
   WingRibbonSoftEdgeWidthHandler(mPrefObjectPtr->GetWingRibbonSoftEdgeWidth());

   WingRibbonEdgeLimitTypeHandler(mPrefObjectPtr->GetWingRibbonEdgeLimitType());
   WingRibbonMainLimitTypeHandler(mPrefObjectPtr->GetWingRibbonMainLimitType());
   WingRibbonEdgeStateBeingTrackedHandler(mPrefObjectPtr->GetWingRibbonEdgeStateBeingTracked());
   WingRibbonMainStateBeingTrackedHandler(mPrefObjectPtr->GetWingRibbonMainStateBeingTracked());
}

void WkPlatformHistory::Plugin::UpdateTraceLineDetectionData(unsigned int aTargetId, unsigned int aSourceId, bool aInitiated)
{
   UpdateTraceLineStateDataP(aTargetId, aSourceId, aInitiated, mTraceLineStateData[aTargetId].mDetections);
}

void WkPlatformHistory::Plugin::UpdateTraceLineTrackData(unsigned int aTargetId, unsigned int aSourceId, bool aInitiated)
{
   UpdateTraceLineStateDataP(aTargetId, aSourceId, aInitiated, mTraceLineStateData[aTargetId].mTracks);
}

void WkPlatformHistory::Plugin::UpdateTraceLineAttackData(unsigned int aTargetId, unsigned int aSourceId, bool aInitiated)
{
   UpdateTraceLineStateDataP(aTargetId, aSourceId, aInitiated, mTraceLineStateData[aTargetId].mAttacks);
}

// Stop keeping track of platforms that are deleted prior to getting a detection change, track drop, or attack completed
// signal, then update all trace lines
void WkPlatformHistory::Plugin::PlatformDeletedHandler(unsigned int aId)
{
   for (auto& it : mTraceLineStateData)
   {
      it.second.mDetections.erase(aId);
      it.second.mTracks.erase(aId);
      it.second.mAttacks.erase(aId);
      SetStandardTraceLineState(it.first);
   }
   for (auto& it : mImmersiveTraceLines)
   {
      unsigned int platformIndex = it.second->GetPlatform()->GetIndex();
      if (platformIndex != aId)
      {
         SetImmersiveTraceLineState(it.first, platformIndex);
      }
   }
   mTraceLineStateData.erase(aId);
}

void WkPlatformHistory::Plugin::UpdateTraceLineStateDataP(unsigned int                 aTargetId,
                                                          unsigned int                 aSourceId,
                                                          bool                         aInitiated,
                                                          std::multiset<unsigned int>& aSet)
{
   if (aInitiated)
   {
      aSet.emplace(aSourceId);
   }
   else
   {
      // In cases where the entity is being targed by the same platform more than once, only remove a single ID
      auto setIter = aSet.find(aSourceId);
      if (setIter != aSet.end())
      {
         aSet.erase(setIter);
      }
   }
   SetStandardTraceLineState(aTargetId);

   for (auto& it : mImmersiveTraceLines)
   {
      if (it.second->GetPlatform()->GetIndex() == aTargetId)
      {
         SetImmersiveTraceLineState(it.first, aTargetId);
      }
   }
}

// Called when preferences are updated. Overwrites any existing trace lines.
void WkPlatformHistory::Plugin::CreateTraceLine(wkf::Platform* aPlatform, bool aSetVisible)
{
   warlock::Platform* platform = dynamic_cast<warlock::Platform*>(aPlatform);
   if (platform)
   {
      unsigned int index = platform->GetIndex();
      // Remove the existing trace line
      if (mTraceLineMap.find(index) != mTraceLineMap.end())
      {
         mTraceLineMap[index]->RemoveAttachment();
      }
      mTraceLineMap[index] = ut::make_unique<TracelineData>(*platform, vaEnv.GetStandardViewer()); // create it
      mTraceLineMap[index]->SetVisible(Qt::Checked == GetPlatformOptionState(mTraceLineType, platform));
      mTraceLineMap[index]->SetTraceLineLength(mPrefObjectPtr->GetTraceLineLength());
      mTraceLineMap[index]->SetLineWidth(mPrefObjectPtr->GetTraceLineWidth());
      const std::vector<QColor>& cl = mPrefObjectPtr->GetTraceLineColorationList();
      std::vector<UtColor>       ucl;
      for (auto qc : cl)
      {
         ucl.emplace_back(UtColor(qc.redF(), qc.greenF(), qc.blueF(), qc.alphaF()));
      }
      mTraceLineMap[index]->SetColoration(mPrefObjectPtr->GetTraceLineColoration(), ucl);
      SetStandardTraceLineState(index);
      mTraceLineMap[index]->SetVisible(aSetVisible);
   }
}

void WkPlatformHistory::Plugin::SetStandardTraceLineState(unsigned int aId)
{
   if (mTraceLineMap.find(aId) != mTraceLineMap.end() && mPrefObjectPtr->GetTraceLineColoration() == wkf::eTRACE_STATE)
   {
      mTraceLineMap[aId]->SetState(GetTraceLineState(aId));
   }
}

void WkPlatformHistory::Plugin::SetImmersiveTraceLineState(unsigned int aViewerId, unsigned int aPlatformId)
{
   if (mImmersiveTraceLines.find(aViewerId) != mImmersiveTraceLines.end() &&
       mPrefObjectPtr->GetTraceLineColoration() == wkf::eTRACE_STATE)
   {
      mImmersiveTraceLines[aViewerId]->SetState(GetTraceLineState(aPlatformId));
   }
}

int WkPlatformHistory::Plugin::GetTraceLineState(unsigned int aId)
{
   auto stateMap = mTraceLineStateData.find(aId);
   if (stateMap != mTraceLineStateData.end())
   {
      if (!stateMap->second.mAttacks.empty())
      {
         return cATTACKED_STATE;
      }
      else if (!stateMap->second.mTracks.empty())
      {
         return cTRACKED_STATE;
      }
      else if (!stateMap->second.mDetections.empty())
      {
         return cDETECTED_STATE;
      }
   }
   return cDEFAULT_STATE;
}

void WkPlatformHistory::Plugin::HandleTraceLinePrefChange()
{
   // Overwrite all existing lines with new lines that start from the time of change
   for (auto& iter : mTraceLineMap)
   {
      CreateTraceLine(iter.second->GetPlatform(), iter.second->IsVisible());
   }

   auto viewerList = vaEnv.GetViewerManager()->GetViewers();
   for (auto& iter : mImmersiveTraceLines)
   {
      // Find the viewer associated with the unique map id
      unsigned int viewerId = iter.first;
      auto         viewerIt = std::find_if(viewerList.begin(),
                                   viewerList.end(),
                                   [viewerId](vespa::VaViewer* aViewer)
                                   { return (aViewer) && (aViewer->GetUniqueId() == viewerId); });
      if (viewerIt != viewerList.end())
      {
         AddImmersiveTraceLine(*iter.second->GetPlatform(), *viewerIt);
      }
   }
}

/*************************************************************************************************
 * @brief   Handler, called when the wing ribbon width scale is changed by the gui. Modifies the
 *          width of the wingribbons, with the reference scale of 1.0f being aligned with the
 *          wingtips.
 * @param   aWidthScale The width scale.
 * @date 7/14/2017
 **************************************************************************************************/
void WkPlatformHistory::Plugin::WingRibbonWidthScaleHandler(float aWidthScale)
{
   for (auto& iter : mWingRibbonMap)
   {
      iter.second->SetWidthScale(aWidthScale);
   }
   for (auto& iter : mImmersiveWingRibbons)
   {
      iter.second->SetWidthScale(aWidthScale);
   }
}

/*************************************************************************************************
 * @brief   Handler, called when the wing ribbon hard edge width is changed by the gui. Changes
 *          the percent of the wing ribbon's area that is taken up by the edge color.
 * @param   aHardEdgeWidth Width of the hard edge, as a percent in the domain [0.0, 100.0].
 * @date 7/14/2017
 **************************************************************************************************/
void WkPlatformHistory::Plugin::WingRibbonHardEdgeWidthHandler(float aHardEdgeWidth)
{
   for (auto& iter : mWingRibbonMap)
   {
      iter.second->SetHardEdgeWidth(aHardEdgeWidth / 100);
   }
   for (auto& iter : mImmersiveWingRibbons)
   {
      iter.second->SetHardEdgeWidth(aHardEdgeWidth / 100);
   }
}

/*************************************************************************************************
 * @brief   Handler, called when the wing ribbon soft edge width. Changes the percent of the wing
 *          ribbon's area that is used to interpolate between the main and edge colors (as a form of anti-
 *          aliasing).
 * @param   aSoftEdgeWidth Width of the soft edge, as a percent in the domain [0.0, 100.0].
 * @date 7/14/2017
 **************************************************************************************************/
void WkPlatformHistory::Plugin::WingRibbonSoftEdgeWidthHandler(float aSoftEdgeWidth)
{
   for (auto& iter : mWingRibbonMap)
   {
      iter.second->SetSoftEdgeWidth(aSoftEdgeWidth / 100);
   }
   for (auto& iter : mImmersiveWingRibbons)
   {
      iter.second->SetSoftEdgeWidth(aSoftEdgeWidth / 100);
   }
}

/*************************************************************************************************
 * @brief   Entity removed callback. Cleans up private memory of the entity's wingribbon/traceline attachments
 * @param   aTgt        The target.
 * @param   aEntityPtr  Pointer to the entity being removed.
 * @date 7/14/2017
 **************************************************************************************************/
void WkPlatformHistory::Plugin::EntityRemovedCB(vespa::VaEntity* aEntityPtr)
{
   wkf::Platform* platform = dynamic_cast<wkf::Platform*>(aEntityPtr);
   if (platform != nullptr)
   {
      auto index = platform->GetIndex();
      mTraceLineMap.erase(index);
      mWingRibbonMap.erase(index);
      std::set<unsigned int> remset;
      for (auto& it : mImmersiveTraceLines)
      {
         if (it.second->GetPlatform()->GetIndex() == index)
         {
            remset.insert(it.first);
         }
      }
      for (auto& rem : remset)
      {
         mImmersiveTraceLines.erase(rem);
      }
      remset.clear();
      for (auto& it : mImmersiveWingRibbons)
      {
         if (it.second->mPlatformPtr->GetIndex() == index)
         {
            remset.insert(it.first);
         }
      }
      for (auto& rem : remset)
      {
         mImmersiveWingRibbons.erase(rem);
      }
   }
}

void WkPlatformHistory::Plugin::ViewerDestroyedCB(vespa::VaViewer* aViewerPtr)
{
   const auto& tl = mImmersiveTraceLines.find(aViewerPtr->GetUniqueId());
   if (tl != mImmersiveTraceLines.end())
   {
      mImmersiveTraceLines.erase(tl);
   }
   const auto& wr = mImmersiveWingRibbons.find(aViewerPtr->GetUniqueId());
   if (wr != mImmersiveWingRibbons.end())
   {
      mImmersiveWingRibbons.erase(wr);
   }
}

void WkPlatformHistory::Plugin::AddImmersiveTraceLine(warlock::Platform& aPlatform, vespa::VaViewer* aViewerPtr)
{
   unsigned int viewerId = aViewerPtr->GetUniqueId();
   // Remove the existing trace line
   if (mImmersiveTraceLines.find(viewerId) != mImmersiveTraceLines.end())
   {
      mImmersiveTraceLines[viewerId]->RemoveAttachment();
   }
   mImmersiveTraceLines[viewerId] = ut::make_unique<TracelineData>(aPlatform, aViewerPtr);
   auto& ptr                      = mImmersiveTraceLines[viewerId];
   ptr->SetVisible(true);
   ptr->SetLineWidth(mPrefObjectPtr->GetTraceLineWidth());
   const std::vector<QColor>& cl = mPrefObjectPtr->GetTraceLineColorationList();
   std::vector<UtColor>       ucl;
   for (auto qc : cl)
   {
      ucl.emplace_back(UtColor(qc.redF(), qc.greenF(), qc.blueF(), qc.alphaF()));
   }
   ptr->SetColoration(mPrefObjectPtr->GetTraceLineColoration(), ucl);
   SetImmersiveTraceLineState(viewerId, aPlatform.GetIndex());
}

void WkPlatformHistory::Plugin::RemoveImmersiveTraceLine(vespa::VaViewer* aViewerPtr)
{
   const auto& tl = mImmersiveTraceLines.find(aViewerPtr->GetUniqueId());
   if (tl != mImmersiveTraceLines.end())
   {
      tl->second->RemoveAttachment();
      mImmersiveTraceLines.erase(aViewerPtr->GetUniqueId());
   }
}

void WkPlatformHistory::Plugin::AddImmersiveWingRibbon(warlock::Platform& aPlatform, vespa::VaViewer* aViewerPtr)
{
   mImmersiveWingRibbons[aViewerPtr->GetUniqueId()] = ut::make_unique<WingRibbonData>(aPlatform, aViewerPtr);
   auto& ptr                                        = mImmersiveWingRibbons[aViewerPtr->GetUniqueId()];

   ptr->SetVisible(true);
   ptr->SetMainMaxColor(UtQtColor(mPrefObjectPtr->GetWingRibbonMainMaxColor()));
   ptr->SetMainMinColor(UtQtColor(mPrefObjectPtr->GetWingRibbonMainMinColor()));
   ptr->SetEdgeMaxColor(UtQtColor(mPrefObjectPtr->GetWingRibbonEdgeMaxColor()));
   ptr->SetEdgeMinColor(UtQtColor(mPrefObjectPtr->GetWingRibbonEdgeMinColor()));
   ptr->SetMainMaxFlippedColor(UtQtColor(mPrefObjectPtr->GetWingRibbonMainMaxFlippedColor()));
   ptr->SetMainMinFlippedColor(UtQtColor(mPrefObjectPtr->GetWingRibbonMainMinFlippedColor()));
   ptr->SetEdgeMaxFlippedColor(UtQtColor(mPrefObjectPtr->GetWingRibbonEdgeMaxFlippedColor()));
   ptr->SetEdgeMinFlippedColor(UtQtColor(mPrefObjectPtr->GetWingRibbonEdgeMinFlippedColor()));

   ptr->SetHardMaxEdgeLimit(mPrefObjectPtr->GetWingRibbonHardMaxEdgeLimit());
   ptr->SetHardMinEdgeLimit(mPrefObjectPtr->GetWingRibbonHardMinEdgeLimit());
   ptr->SetHardMaxMainLimit(mPrefObjectPtr->GetWingRibbonHardMaxMainLimit());
   ptr->SetHardMinMainLimit(mPrefObjectPtr->GetWingRibbonHardMinMainLimit());

   ptr->SetWidthScale(mPrefObjectPtr->GetWingRibbonWidthScale());
   ptr->SetHardEdgeWidth(mPrefObjectPtr->GetWingRibbonHardEdgeWidth() / 100);
   ptr->SetSoftEdgeWidth(mPrefObjectPtr->GetWingRibbonSoftEdgeWidth() / 100);

   ptr->SetEdgeLimitType(mPrefObjectPtr->GetWingRibbonEdgeLimitType());
   ptr->SetMainLimitType(mPrefObjectPtr->GetWingRibbonMainLimitType());
   ptr->SetEdgeStateBeingTracked(mPrefObjectPtr->GetWingRibbonEdgeStateBeingTracked());
   ptr->SetMainStateBeingTracked(mPrefObjectPtr->GetWingRibbonMainStateBeingTracked());
}

void WkPlatformHistory::Plugin::RemoveImmersiveWingRibbon(vespa::VaViewer* aViewerPtr)
{
   const auto& wr = mImmersiveWingRibbons.find(aViewerPtr->GetUniqueId());
   if (wr != mImmersiveWingRibbons.end())
   {
      wr->second->RemoveAttachment();
      mImmersiveWingRibbons.erase(aViewerPtr->GetUniqueId());
   }
}

/*************************************************************************************************
 * @brief   Constructor. Creates a Traceline attachment and adds it to aPlatform
 * @param   aPlatform   The platform to attach the traceline to.
 * @date 7/14/2017
 **************************************************************************************************/
WkPlatformHistory::Plugin::TracelineData::TracelineData(warlock::Platform& aPlatform, vespa::VaViewer* aViewerPtr)
   : mPlatformPtr(&aPlatform)
   , mTracelineLengthSec(30)
   , mLastUpdateTime(0)
   , mColoration(wkf::eTRACE_STATE)
   , mLineWidth(2)
{
   auto* traceline = vespa::make_attachment<vespa::VaAttachmentTraceLine>(aPlatform, aViewerPtr);
   vespa::VaAttachment::LoadAttachment(*traceline);
   mTraceLineUniqueId = traceline->GetUniqueId();
   traceline->AddStateBack(0.0f, 0.0f);
}

void WkPlatformHistory::Plugin::TracelineData::RemoveAttachment()
{
   mPlatformPtr->RemoveAttachment(mTraceLineUniqueId);
}

/*************************************************************************************************
 * @brief   Sets the traceline attachment's visibility.
 * @param   aVisible True to show, false to hide.
 * @date 7/14/2017
 **************************************************************************************************/
void WkPlatformHistory::Plugin::TracelineData::SetVisible(bool aVisible)
{
   vespa::VaAttachment* attachment = mPlatformPtr->FindAttachment(mTraceLineUniqueId);
   if (attachment != nullptr)
   {
      attachment->SetStateVisibility(aVisible);
   }
}

/*************************************************************************************************
 * @brief   Query if the traceline attachment is visible.
 * @return  True if visible, false if not.
 * @date 7/14/2017
 **************************************************************************************************/
bool WkPlatformHistory::Plugin::TracelineData::IsVisible()
{
   vespa::VaAttachment* attachment = mPlatformPtr->FindAttachment(mTraceLineUniqueId);
   if (attachment != nullptr)
   {
      return attachment->GetStateVisibility();
   }
   return false;
}

/*************************************************************************************************
 * @brief   Set the coloration mode on the traceline
 * @param   aColoration eTRACE_TEAM_COLOR - use team color, eTRACE_STATE - use engagement state
 * @date 8/3/2017
 **************************************************************************************************/
void WkPlatformHistory::Plugin::TracelineData::SetColoration(const wkf::TraceStateValue& aColoration,
                                                             const std::vector<UtColor>& aColorArray)
{
   mColoration = aColoration;
   vespa::VaAttachmentTraceLine* tl =
      dynamic_cast<vespa::VaAttachmentTraceLine*>(mPlatformPtr->FindAttachment(mTraceLineUniqueId));
   if (tl != nullptr)
   {
      if (aColoration == wkf::eTRACE_TEAM_COLOR)
      {
         auto teamColor =
            wkfEnv.GetPreferenceObject<wkf::TeamVisibilityPrefObject>()->GetTeamUtColor(mPlatformPtr->GetSide());
         tl->SetStaticColor(teamColor);
      }
      else if (aColoration == wkf::eTRACE_STATE)
      {
         tl->SetStateColoration();
         tl->SetColorArray(aColorArray);
      }
      else if (aColoration == wkf::eTRACE_NAME_HASH)
      {
         auto color = wkf::util::GetHashUtColor(mPlatformPtr->GetName());
         tl->SetStaticColor(color);
      }
   }
}

void WkPlatformHistory::Plugin::TracelineData::SetLineWidth(unsigned int aWidth)
{
   mLineWidth = aWidth;
   vespa::VaAttachmentTraceLine* tl =
      dynamic_cast<vespa::VaAttachmentTraceLine*>(mPlatformPtr->FindAttachment(mTraceLineUniqueId));
   if (tl != nullptr)
   {
      tl->SetLineWidth(aWidth);
   }
}

/*************************************************************************************************
 * @brief   Updates the traceline to create a new data point where the platform has moved to.
 *          Also erases the data points the older than the desired length of the wingribbon.
 * @note This will query for new data points even if the traceline attachment isn't visible. this
 *       behavior is different from wing ribbons wherein datapoints are only genereated when the
 *       attachment is visible.  This allows behavior thus allows the user to see where the
 *       platform was before the traceline was made visible.
 * @date 7/14/2017
 **************************************************************************************************/
void WkPlatformHistory::Plugin::TracelineData::Update()
{
   double curTime = mPlatformPtr->GetUpdateTime();
   if (mLastUpdateTime != curTime)
   {
      mLastUpdateTime = curTime;

      vespa::VaAttachmentTraceLine* traceLine =
         dynamic_cast<vespa::VaAttachmentTraceLine*>(mPlatformPtr->FindAttachment(mTraceLineUniqueId));
      if (traceLine != nullptr)
      {
         traceLine->PruneBefore(curTime - mTracelineLengthSec);

         double ecef[3];
         mPlatformPtr->GetPosition().GetECEF(ecef);
         traceLine->AddPointBack(curTime, ecef);

         if (mColoration == wkf::eTRACE_STATE)
         {
            traceLine->AddStateBack(curTime, mState);
         }
      }
   }
}

void WkPlatformHistory::Plugin::ResetOptionStates()
{
   for (auto& it : mTraceLineMap)
   {
      it.second->RemoveAttachment();
   }
   mTraceLineMap.clear();
   for (auto& jt : mWingRibbonMap)
   {
      jt.second->RemoveAttachment();
   }
   mWingRibbonMap.clear();
}

/*************************************************************************************************
 * @brief   Creates an attachment for the entity of type aId if one does not already exist for it, and sets visibility
 *of the attachments of type aId that are attached to that entity to aSetVisibe
 * @param   aId         The attachment type whose visibility is being changed.
 * @param   aSetVisible True to show, false to hide the attachments.
 * @param   aPlatform   Pointer to the platform to create/find the attachments on.
 * @date 7/14/2017
 **************************************************************************************************/
void WkPlatformHistory::Plugin::SetPlatformOptionState(int aId, bool aSetVisible, wkf::Platform* aPlatform)
{
   warlock::Platform* platform = dynamic_cast<warlock::Platform*>(aPlatform);
   if (platform != nullptr)
   {
      unsigned int index = platform->GetIndex();

      if (aId == mTraceLineType)
      {
         std::map<unsigned int, std::unique_ptr<TracelineData>>::iterator jt = mTraceLineMap.find(index);
         if (aSetVisible)
         {
            if (jt != mTraceLineMap.end())
            {
               jt->second->SetVisible(true);
               // show it
            }
            else
            {
               mTraceLineMap[index] = ut::make_unique<TracelineData>(*platform, vaEnv.GetStandardViewer()); // create it
               mTraceLineMap[index]->SetVisible(Qt::Checked == GetPlatformOptionState(mTraceLineType, platform));
               mTraceLineMap[index]->SetTraceLineLength(mPrefObjectPtr->GetTraceLineLength());
               mTraceLineMap[index]->SetLineWidth(mPrefObjectPtr->GetTraceLineWidth());
               const std::vector<QColor>& cl = mPrefObjectPtr->GetTraceLineColorationList();
               std::vector<UtColor>       ucl;
               for (auto qc : cl)
               {
                  ucl.emplace_back(UtColor(qc.redF(), qc.greenF(), qc.blueF(), qc.alphaF()));
               }
               mTraceLineMap[index]->SetColoration(mPrefObjectPtr->GetTraceLineColoration(), ucl);
            }
         }
         else
         {
            if (jt != mTraceLineMap.end())
            {
               jt->second->SetVisible(false);
            }
         }
         // turn on trace lines for the selected entity
      }
      else if (aId == mWingRibbonType)
      {
         if (aSetVisible)
         {
            // Get the modelDefinitionPtr so that we can check the wingtip locations
            const auto modelDefinitionPtr =
               vespa::VaEnvironment::Instance().GetModelDatabase()->GetModelDefinition(platform->GetIcon());
            if (modelDefinitionPtr)
            {
               // If a wing tip position exist, then we will draw wing ribbons, otherwise we don't draw wing ribbons.
               if (modelDefinitionPtr->GetWingTip(vespa::VaEnvironment::Instance().GetModelDatabase()->GetViewerModelSet(
                      *vaEnv.GetStandardViewer())) != osg::Vec3{0, 0, 0})
               {
                  auto jt = mWingRibbonMap.find(index);
                  if (jt != mWingRibbonMap.end())
                  {
                     jt->second->SetVisible(true);
                     // show it
                  }
                  else
                  {
                     mWingRibbonMap[index] = ut::make_unique<WingRibbonData>(*platform, vaEnv.GetStandardViewer());
                     auto& ptr             = mWingRibbonMap[index];

                     ptr->SetVisible(Qt::Checked == GetPlatformOptionState(mWingRibbonType, platform));

                     ptr->SetEdgeLimitType(mPrefObjectPtr->GetWingRibbonEdgeLimitType());
                     ptr->SetMainLimitType(mPrefObjectPtr->GetWingRibbonMainLimitType());
                     ptr->SetEdgeStateBeingTracked(mPrefObjectPtr->GetWingRibbonEdgeStateBeingTracked());
                     ptr->SetMainStateBeingTracked(mPrefObjectPtr->GetWingRibbonMainStateBeingTracked());

                     ptr->SetWingRibbonLength(mPrefObjectPtr->GetWingRibbonsLength());
                     ptr->SetMainMaxColor(UtQtColor(mPrefObjectPtr->GetWingRibbonMainMaxColor()));
                     ptr->SetMainMinColor(UtQtColor(mPrefObjectPtr->GetWingRibbonMainMinColor()));
                     ptr->SetEdgeMaxColor(UtQtColor(mPrefObjectPtr->GetWingRibbonEdgeMaxColor()));
                     ptr->SetEdgeMinColor(UtQtColor(mPrefObjectPtr->GetWingRibbonEdgeMinColor()));
                     ptr->SetMainMaxFlippedColor(UtQtColor(mPrefObjectPtr->GetWingRibbonMainMaxFlippedColor()));
                     ptr->SetMainMinFlippedColor(UtQtColor(mPrefObjectPtr->GetWingRibbonMainMinFlippedColor()));
                     ptr->SetEdgeMaxFlippedColor(UtQtColor(mPrefObjectPtr->GetWingRibbonEdgeMaxFlippedColor()));
                     ptr->SetEdgeMinFlippedColor(UtQtColor(mPrefObjectPtr->GetWingRibbonEdgeMinFlippedColor()));

                     ptr->SetHardMaxEdgeLimit(mPrefObjectPtr->GetWingRibbonHardMaxEdgeLimit());
                     ptr->SetHardMinEdgeLimit(mPrefObjectPtr->GetWingRibbonHardMinEdgeLimit());
                     ptr->SetHardMaxMainLimit(mPrefObjectPtr->GetWingRibbonHardMaxMainLimit());
                     ptr->SetHardMinMainLimit(mPrefObjectPtr->GetWingRibbonHardMinMainLimit());

                     ptr->SetWidthScale(mPrefObjectPtr->GetWingRibbonWidthScale());
                     ptr->SetHardEdgeWidth(mPrefObjectPtr->GetWingRibbonHardEdgeWidth() / 100);
                     ptr->SetSoftEdgeWidth(mPrefObjectPtr->GetWingRibbonSoftEdgeWidth() / 100);
                  }
               }
            }
         }
         else
         {
            auto jt = mWingRibbonMap.find(index);
            if (jt != mWingRibbonMap.end())
            {
               jt->second->SetVisible(false);
            }
         }
      }
   }
}

/*************************************************************************************************
 * @brief   Determines whether or not the attachment type is set to be visible on the entity
 * @param   aId          The type of attachment to determine visibility status on.
 * @param   aPlatformPtr Pointer to the platform that has the attachments we are looking for.
 * @return  True if the attachment type is visible, false if it fails.
 * @date 7/14/2017
 **************************************************************************************************/
Qt::CheckState WkPlatformHistory::Plugin::GetPlatformOptionState(int aId, const wkf::Platform* aPlatformPtr) const
{
   if (aPlatformPtr)
   {
      if (aId == mTraceLineType)
      {
         if (mTraceLineMap.find(aPlatformPtr->GetIndex()) != mTraceLineMap.end())
         {
            return (mTraceLineMap.at(aPlatformPtr->GetIndex())->IsVisible()) ? Qt::Checked : Qt::Unchecked;
         }
      }
      else if (aId == mWingRibbonType)
      {
         if (mWingRibbonMap.find(aPlatformPtr->GetIndex()) != mWingRibbonMap.end())
         {
            return (mWingRibbonMap.at(aPlatformPtr->GetIndex())->IsVisible()) ? Qt::Checked : Qt::Unchecked;
         }
      }
   }
   return Qt::Unchecked;
}

/*************************************************************************************************
 * @brief   Constructor. Fetches all the necessary information to create a wing ribbon attachment, and attaches it the
 *platform.
 * @param   aPlatform   The platform to attach the wing ribbon to.
 * @param   aViewer     The viewer that the attachment will be placed in.
 * @date 7/14/2017
 **************************************************************************************************/
WkPlatformHistory::Plugin::WingRibbonData::WingRibbonData(warlock::Platform& aPlatform, vespa::VaViewer* aViewerPtr)
   : mPlatformPtr(&aPlatform)
   , mLastUpdateTime(std::numeric_limits<decltype(mLastUpdateTime)>::max())
   , mWingRibbonLengthSec(30)
   , mWingRibbonMainMaxColor{1.0f, 0.0f, 1.0f, 0.5f} // rgba
   , mWingRibbonMainMinColor{0.0f, 1.0f, 1.0f, 0.5f}
   , mWingRibbonEdgeMaxColor{1.0f, 1.0f, 0.0f, 1.0f}
   , mWingRibbonEdgeMinColor{0.0f, 0.0f, 0.0f, 1.0f}
   , mWingRibbonMainMaxFlippedColor{1.0f, 0.0f, 1.0f, 0.4f} // rgba
   , mWingRibbonMainMinFlippedColor{0.0f, 1.0f, 1.0f, 0.4f}
   , mWingRibbonEdgeMaxFlippedColor{1.0f, 1.0f, 0.0f, 0.8f}
   , mWingRibbonEdgeMinFlippedColor{0.0f, 0.0f, 0.0f, 0.8f}
   , mWingRibbonWidthScale(1.0f)
   , mWingRibbonHardEdgeWidth(.1f)
   , mWingRibbonSoftEdgeWidth(.12f)
   , mEdgeStateBeingTracked(wkf::eALTITUDE)
   , mMainStateBeingTracked(wkf::eSPEED)
   , mEdgeLimitType(wkf::eINDEFINITE_INDIVIDUAL_LIMITS)
   , mMainLimitType(wkf::eCURRENT_INDIVIDUAL_LIMITS)
   , mHardMaxMainLimit(1)
   , mHardMinMainLimit(0)
   , mHardMaxEdgeLimit(1)
   , mHardMinEdgeLimit(0)
   , mWidthScale(1)
   , mHardEdgeWidth(0.09f)
   , mSoftEdgeWidth(0.12f)
{
   const auto modelDefinitionPtr =
      vespa::VaEnvironment::Instance().GetModelDatabase()->GetModelDefinition(mPlatformPtr->GetIcon());

   float      VA_ScreenScale = 0;
   osg::Node* platformNode   = nullptr;
   if (modelDefinitionPtr != nullptr)
   {
      // modelDefinitionPtr->mSharedResource.get()->getStateSet()->getUniform("VA_ScreenScale")->get(VA_ScreenScale);
      platformNode =
         vespa::VaEnvironment::Instance().GetModelDatabase()->RequestSharedModel(mPlatformPtr->GetIcon(), aViewerPtr);

      if (platformNode != nullptr)
      {
         auto ss = platformNode->getStateSet();
         if (ss)
         {
            auto uni = ss->getUniform("VA_ScreenScale");
            if (uni)
            {
               uni->get(VA_ScreenScale);
            }
         }
      }
   }
   if (VA_ScreenScale == 0)
   {
      VA_ScreenScale = 63.159f; // default of F-18
   }

   mWingTipA = {0, 0, 0};
   if (modelDefinitionPtr != nullptr)
   {
      mWingTipA = modelDefinitionPtr->GetWingTip("");
   }
   if (mWingTipA == osg::Vec3{0, 0, 0})
   {
      if (modelDefinitionPtr == nullptr)
      {
         if (mPlatformPtr->GetIcon() != "" && mPlatformPtr->GetIcon() != "__waypoint__")
         {
            std::cout << "unable to find icon \"" << mPlatformPtr->GetIcon() << "\" for platform \""
                      << mPlatformPtr->GetName() << "\"\n";
         }
      }
      else
      {
         // std::cout << "wing_tip data for \"" << mPlatformPtr->GetIcon() << "\" from platform \"" << mPlatformPtr->GetName()
         // << "\" has not been set\n";// , defaulting wing_tip for wing_ribbons to{ -26,35,0 }\n";
      }
      mWingTipA = osg::Vec3f{-26, 35, 0};

      // try to get a rough value for where the wingtip is based on bounding sphere
      if (platformNode != nullptr && platformNode->getBound().radius() > 0.0001)
      {
         // only do this heuristic if bounding radius is greater than 0
         if (mWingTipA.y() > platformNode->getBound().radius() * 0.9 ||
             -mWingTipA.x() > platformNode->getBound().radius() * 0.7)
         {
            mWingTipA.normalize();
            mWingTipA *= platformNode->getBound().radius() * 0.9;
         }
      }
   }
   mWingTipB     = mWingTipA;
   mWingTipB.y() = -mWingTipB.y(); // flip other wingtip across xz plane

   auto* wingribbon = vespa::make_attachment<vespa::VaAttachmentWingRibbon>(aPlatform, aViewerPtr);
   vespa::VaAttachment::LoadAttachment(*wingribbon);
   wingribbon->SetVA_ScreenScale(VA_ScreenScale);
   wingribbon->SetMainMaxColor(mWingRibbonMainMaxColor);
   wingribbon->SetMainMinColor(mWingRibbonMainMinColor);
   wingribbon->SetEdgeMaxColor(mWingRibbonEdgeMaxColor);
   wingribbon->SetEdgeMinColor(mWingRibbonEdgeMinColor);
   wingribbon->SetMainMaxFlippedColor(mWingRibbonMainMaxFlippedColor);
   wingribbon->SetMainMinFlippedColor(mWingRibbonMainMinFlippedColor);
   wingribbon->SetEdgeMaxFlippedColor(mWingRibbonEdgeMaxFlippedColor);
   wingribbon->SetEdgeMinFlippedColor(mWingRibbonEdgeMinFlippedColor);
   wingribbon->SetEdgeLimitType(static_cast<vespa::VaAttachmentWingRibbon::InterpolationLimitsMethod>(mEdgeLimitType));
   wingribbon->SetMainLimitType(static_cast<vespa::VaAttachmentWingRibbon::InterpolationLimitsMethod>(mMainLimitType));
   wingribbon->SetWidthScale(mWidthScale);
   wingribbon->SetHardEdgeWidth(mHardEdgeWidth);
   wingribbon->SetSoftEdgeWidth(mSoftEdgeWidth);
   if (mMainLimitType == wkf::eHARD_LIMITS)
   {
      wingribbon->SetHardMaxMainLimit(mHardMaxMainLimit);
      wingribbon->SetHardMinMainLimit(mHardMinMainLimit);
   }
   if (mEdgeLimitType == wkf::eHARD_LIMITS)
   {
      wingribbon->SetHardMaxEdgeLimit(mHardMaxEdgeLimit);
      wingribbon->SetHardMinEdgeLimit(mHardMinEdgeLimit);
   }
   if (mMainStateBeingTracked == wkf::eTEAM_COLOR)
   {
      auto teamColor =
         wkfEnv.GetPreferenceObject<wkf::TeamVisibilityPrefObject>()->GetTeamUtColor(mPlatformPtr->GetSide());
      teamColor[3] = .5; // apply an alpha to the team color
      wingribbon->SetMainMinColor(teamColor);
      wingribbon->SetMainMaxColor(teamColor);
      for (int x = 0; x < 3; x++)
      {
         teamColor[x] *= 0.7f;
      } // make flip side 30% darker, leave transparency the same
      wingribbon->SetMainMinFlippedColor(teamColor);
      wingribbon->SetMainMaxFlippedColor(teamColor);
   }
   if (mEdgeStateBeingTracked == wkf::eTEAM_COLOR)
   {
      auto teamColor =
         wkfEnv.GetPreferenceObject<wkf::TeamVisibilityPrefObject>()->GetTeamUtColor(mPlatformPtr->GetSide());
      teamColor[3] = .5; // apply an alpha to the team color
      wingribbon->SetEdgeMinColor(teamColor);
      wingribbon->SetEdgeMaxColor(teamColor);
      for (int x = 0; x < 3; x++)
      {
         teamColor[x] *= 0.7f;
      } // make flip side 30% darker, leave transparency the same
      wingribbon->SetEdgeMinFlippedColor(teamColor);
      wingribbon->SetEdgeMaxFlippedColor(teamColor);
   }

   mWingRibbonUniqueId = wingribbon->GetUniqueId();
}

void WkPlatformHistory::Plugin::WingRibbonData::RemoveAttachment()
{
   mPlatformPtr->RemoveAttachment(mWingRibbonUniqueId);
}

/*************************************************************************************************
 * @brief   Sets main maximum color of the wing ribbon, which is used in modifying how the wing
 *          ribbon looks in response to its recorded state values.
 * @note if mMainStateBeingTracked is set to eTEAM_COLOR, this color change will not take effect
 *       until the mainStateBeingTracked is set to something else.
 * @param   aMainMaxColor  The main maximum color.
 * @date 7/14/2017
 **************************************************************************************************/
void WkPlatformHistory::Plugin::WingRibbonData::SetMainMaxColor(const UtColor& aMainMaxColor)
{
   if (mWingRibbonMainMaxColor == aMainMaxColor)
   {
      return;
   }

   mWingRibbonMainMaxColor = aMainMaxColor;
   if (mMainStateBeingTracked == wkf::eTEAM_COLOR)
   {
      return;
   }

   vespa::VaAttachmentWingRibbon* wingribbon =
      dynamic_cast<vespa::VaAttachmentWingRibbon*>(mPlatformPtr->FindAttachment(mWingRibbonUniqueId));
   if (wingribbon != nullptr)
   {
      wingribbon->SetMainMaxColor(mWingRibbonMainMaxColor);
   }
}

/*************************************************************************************************
 * @brief   Sets main minimum color of the wing ribbon, which is used in modifying how the wing
 *          ribbon looks in response to its recorded state values.
 * @note if mMainStateBeingTracked is set to eTEAM_COLOR, this color change will not take effect
 *       until the mainStateBeingTracked is set to something else.
 * @param   aMainMinColor  The main minimum color.
 * @date 7/14/2017
 **************************************************************************************************/
void WkPlatformHistory::Plugin::WingRibbonData::SetMainMinColor(const UtColor& aMainMinColor)
{
   if (mWingRibbonMainMinColor == aMainMinColor)
   {
      return;
   }
   mWingRibbonMainMinColor = aMainMinColor;
   if (mMainStateBeingTracked == wkf::eTEAM_COLOR)
   {
      return;
   }
   vespa::VaAttachmentWingRibbon* wingribbon =
      dynamic_cast<vespa::VaAttachmentWingRibbon*>(mPlatformPtr->FindAttachment(mWingRibbonUniqueId));
   if (wingribbon != nullptr)
   {
      wingribbon->SetMainMinColor(mWingRibbonMainMinColor);
   }
}

/*************************************************************************************************
 * @brief   Sets edge maximum color of the wing ribbon, which is used in modifying how the wing
 *          ribbon looks in response to its recorded state values.
 * @note if mEdgeStateBeingTracked is set to eTEAM_COLOR, this color change will not take effect
 *       until the edgeStateBeingTracked is set to something else.
 * @param   aEdgeMaxColor  The edge maximum color.
 * @date 7/14/2017
 **************************************************************************************************/
void WkPlatformHistory::Plugin::WingRibbonData::SetEdgeMaxColor(const UtColor& aEdgeMaxColor)
{
   if (mWingRibbonEdgeMaxColor == aEdgeMaxColor)
   {
      return;
   }
   mWingRibbonEdgeMaxColor = aEdgeMaxColor;
   if (mEdgeStateBeingTracked == wkf::eTEAM_COLOR)
   {
      return;
   }
   vespa::VaAttachmentWingRibbon* wingribbon =
      dynamic_cast<vespa::VaAttachmentWingRibbon*>(mPlatformPtr->FindAttachment(mWingRibbonUniqueId));
   if (wingribbon != nullptr)
   {
      wingribbon->SetEdgeMaxColor(mWingRibbonEdgeMaxColor);
   }
}

/*************************************************************************************************
 * @brief   Sets edge minimum color of the wing ribbon, which is used in modifying how the wing
 *          ribbon looks in response to its recorded state values.
 * @note if mEdgeStateBeingTracked is set to eTEAM_COLOR, this color change will not take effect
 *       until the edgeStateBeingTracked is set to something else.
 * @param   aEdgeMinColor  The edge minimum color.
 * @date 7/14/2017
 **************************************************************************************************/
void WkPlatformHistory::Plugin::WingRibbonData::SetEdgeMinColor(const UtColor& aEdgeMinColor)
{
   if (mWingRibbonEdgeMinColor == aEdgeMinColor)
   {
      return;
   }
   mWingRibbonEdgeMinColor = aEdgeMinColor;
   if (mEdgeStateBeingTracked == wkf::eTEAM_COLOR)
   {
      return;
   }
   vespa::VaAttachmentWingRibbon* wingribbon =
      dynamic_cast<vespa::VaAttachmentWingRibbon*>(mPlatformPtr->FindAttachment(mWingRibbonUniqueId));
   if (wingribbon != nullptr)
   {
      wingribbon->SetEdgeMinColor(mWingRibbonEdgeMinColor);
   }
}

/*************************************************************************************************
 * @brief   Sets main maximum flipped color of the wing ribbon, which is used in modifying how the wing
 *          ribbon looks in response to its recorded state values.
 * @note if mMainStateBeingTracked is set to eTEAM_COLOR, this color change will not take effect
 *       until the mainStateBeingTracked is set to something else.
 * @param   aMainMaxFlippedColor The main maximum flipped color.
 * @date 7/14/2017
 **************************************************************************************************/
void WkPlatformHistory::Plugin::WingRibbonData::SetMainMaxFlippedColor(const UtColor& aMainMaxFlippedColor)
{
   if (mWingRibbonMainMaxFlippedColor == aMainMaxFlippedColor)
   {
      return;
   }
   mWingRibbonMainMaxFlippedColor = aMainMaxFlippedColor;
   if (mMainStateBeingTracked == wkf::eTEAM_COLOR)
   {
      return;
   }
   vespa::VaAttachmentWingRibbon* wingribbon =
      dynamic_cast<vespa::VaAttachmentWingRibbon*>(mPlatformPtr->FindAttachment(mWingRibbonUniqueId));
   if (wingribbon != nullptr)
   {
      wingribbon->SetMainMaxFlippedColor(mWingRibbonMainMaxFlippedColor);
   }
}

/*************************************************************************************************
 * @brief   Sets main minimum flipped color of the wing ribbon, which is used in modifying how the wing
 *          ribbon looks in response to its recorded state values.
 * @note if mMainStateBeingTracked is set to eTEAM_COLOR, this color change will not take effect
 *       until the mainStateBeingTracked is set to something else.
 * @param   aMainMinFlippedColor The main minimum flipped color.
 * @date 7/14/2017
 **************************************************************************************************/
void WkPlatformHistory::Plugin::WingRibbonData::SetMainMinFlippedColor(const UtColor& aMainMinFlippedColor)
{
   if (mWingRibbonMainMinFlippedColor == aMainMinFlippedColor)
   {
      return;
   }
   mWingRibbonMainMinFlippedColor = aMainMinFlippedColor;
   if (mMainStateBeingTracked == wkf::eTEAM_COLOR)
   {
      return;
   }
   vespa::VaAttachmentWingRibbon* wingribbon =
      dynamic_cast<vespa::VaAttachmentWingRibbon*>(mPlatformPtr->FindAttachment(mWingRibbonUniqueId));
   if (wingribbon != nullptr)
   {
      wingribbon->SetMainMinFlippedColor(mWingRibbonMainMinFlippedColor);
   }
}

/*************************************************************************************************
 * @brief   Sets edge maximum flipped color of the wing ribbon, which is used in modifying how the wing
 *          ribbon looks in response to its recorded state values.
 * @note if mEdgeStateBeingTracked is set to eTEAM_COLOR, this color change will not take effect
 *       until the edgeStateBeingTracked is set to something else.
 * @param   aEdgeMaxFlippedColor The edge maximum flipped color.
 * @date 7/14/2017
 **************************************************************************************************/
void WkPlatformHistory::Plugin::WingRibbonData::SetEdgeMaxFlippedColor(const UtColor& aEdgeMaxFlippedColor)
{
   if (mWingRibbonEdgeMaxFlippedColor == aEdgeMaxFlippedColor)
   {
      return;
   }
   mWingRibbonEdgeMaxFlippedColor = aEdgeMaxFlippedColor;
   if (mEdgeStateBeingTracked == wkf::eTEAM_COLOR)
   {
      return;
   }
   vespa::VaAttachmentWingRibbon* wingribbon =
      dynamic_cast<vespa::VaAttachmentWingRibbon*>(mPlatformPtr->FindAttachment(mWingRibbonUniqueId));
   if (wingribbon != nullptr)
   {
      wingribbon->SetEdgeMaxFlippedColor(mWingRibbonEdgeMaxFlippedColor);
   }
}

/*************************************************************************************************
 * @brief   Sets edge minimum flipped color of the wing ribbon, which is used in modifying how the wing
 *          ribbon looks in response to its recorded state values.
 * @note if mEdgeStateBeingTracked is set to eTEAM_COLOR, this color change will not take effect
 *       until the edgeStateBeingTracked is set to something else.
 * @param   aEdgeMinFlippedColor The edge minimum flipped color.
 * @date 7/14/2017
 **************************************************************************************************/
void WkPlatformHistory::Plugin::WingRibbonData::SetEdgeMinFlippedColor(const UtColor& aEdgeMinFlippedColor)
{
   if (mWingRibbonEdgeMinFlippedColor == aEdgeMinFlippedColor)
   {
      return;
   }
   mWingRibbonEdgeMinFlippedColor = aEdgeMinFlippedColor;
   if (mEdgeStateBeingTracked == wkf::eTEAM_COLOR)
   {
      return;
   }
   vespa::VaAttachmentWingRibbon* wingribbon =
      dynamic_cast<vespa::VaAttachmentWingRibbon*>(mPlatformPtr->FindAttachment(mWingRibbonUniqueId));
   if (wingribbon != nullptr)
   {
      wingribbon->SetEdgeMinFlippedColor(mWingRibbonEdgeMinFlippedColor);
   }
}

/*************************************************************************************************
 * @brief   Sets which state value from the platform will be recorded by this wing ribbon for the
 *          purpose of being displayed by the center/main area of the wing ribbon.
 * @note Unless the state to track is changed to eTEAM_COLOR, the history of the wing ribbon will
 *       be erased, and start over again from length zero.
 * @param   stateToTrack   The type of state to track.
 * @date 7/14/2017
 **************************************************************************************************/
void WkPlatformHistory::Plugin::WingRibbonData::SetMainStateBeingTracked(wkf::TrackableStateValue stateToTrack)
{
   if (mMainStateBeingTracked == stateToTrack)
   {
      vespa::VaAttachmentWingRibbon* wingribbon =
         dynamic_cast<vespa::VaAttachmentWingRibbon*>(mPlatformPtr->FindAttachment(mWingRibbonUniqueId));
      if (wingribbon != nullptr)
      {
         // undo reset of scenario wide min/max values
         mCurrentScenarioMainMax = std::max(mCurrentScenarioMainMax, wingribbon->GetCurrentMainMaxStateVal());
         mCurrentScenarioMainMin = std::min(mCurrentScenarioMainMin, wingribbon->GetCurrentMainMinStateVal());
      }
      return; // nothing changed
   }
   if (mMainStateBeingTracked == wkf::eTEAM_COLOR)
   {
      // changing away from team color setting, reset the min/max colors to what what they should be
      vespa::VaAttachmentWingRibbon* wingribbon =
         dynamic_cast<vespa::VaAttachmentWingRibbon*>(mPlatformPtr->FindAttachment(mWingRibbonUniqueId));
      if (wingribbon != nullptr)
      {
         wingribbon->SetMainMinColor(mWingRibbonMainMinColor);
         wingribbon->SetMainMaxColor(mWingRibbonMainMaxColor);
         wingribbon->SetMainMinFlippedColor(mWingRibbonMainMinFlippedColor);
         wingribbon->SetMainMaxFlippedColor(mWingRibbonMainMaxFlippedColor);
      }
   }
   mMainStateBeingTracked = stateToTrack;
   if (stateToTrack == wkf::eTEAM_COLOR)
   {
      auto teamColor =
         wkfEnv.GetPreferenceObject<wkf::TeamVisibilityPrefObject>()->GetTeamUtColor(mPlatformPtr->GetSide());
      vespa::VaAttachmentWingRibbon* wingribbon =
         dynamic_cast<vespa::VaAttachmentWingRibbon*>(mPlatformPtr->FindAttachment(mWingRibbonUniqueId));
      if (wingribbon != nullptr)
      {
         teamColor[3] = .5; // apply an alpha to the team color
         wingribbon->SetMainMinColor(teamColor);
         wingribbon->SetMainMaxColor(teamColor);
         for (int x = 0; x < 3; x++)
         {
            teamColor[x] *= 0.7f;
         } // make flip side 30% darker, leave transparency the same
         wingribbon->SetMainMinFlippedColor(teamColor);
         wingribbon->SetMainMaxFlippedColor(teamColor);
      }
   }
}

/*************************************************************************************************
 * @brief   Sets which state value from the platform will be recorded by this wing ribbon for the
 *          purpose of being displayed by the edge area of the wing ribbon.
 * @note Unless the state to track is changed to eTEAM_COLOR, the history of the wing ribbon will
 *       be erased, and start over again from length zero.
 * @param   stateToTrack   The type of state to track.
 * @date 7/14/2017
 **************************************************************************************************/
void WkPlatformHistory::Plugin::WingRibbonData::SetEdgeStateBeingTracked(wkf::TrackableStateValue stateToTrack)
{
   if (mEdgeStateBeingTracked == stateToTrack)
   {
      vespa::VaAttachmentWingRibbon* wingribbon =
         dynamic_cast<vespa::VaAttachmentWingRibbon*>(mPlatformPtr->FindAttachment(mWingRibbonUniqueId));
      if (wingribbon != nullptr)
      {
         // undo reset of scenario wide min/max values
         mCurrentScenarioEdgeMax = std::max(mCurrentScenarioEdgeMax, wingribbon->GetCurrentEdgeMaxStateVal());
         mCurrentScenarioEdgeMin = std::min(mCurrentScenarioEdgeMin, wingribbon->GetCurrentEdgeMinStateVal());
      }
      return; // nothing changed
   }
   if (mEdgeStateBeingTracked == wkf::eTEAM_COLOR)
   {
      // changing away from team color setting, reset the min/max colors to what what they should be
      vespa::VaAttachmentWingRibbon* wingribbon =
         dynamic_cast<vespa::VaAttachmentWingRibbon*>(mPlatformPtr->FindAttachment(mWingRibbonUniqueId));
      if (wingribbon != nullptr)
      {
         wingribbon->SetEdgeMinColor(mWingRibbonEdgeMinColor);
         wingribbon->SetEdgeMaxColor(mWingRibbonEdgeMaxColor);
         wingribbon->SetEdgeMinFlippedColor(mWingRibbonEdgeMinFlippedColor);
         wingribbon->SetEdgeMaxFlippedColor(mWingRibbonEdgeMaxFlippedColor);
      }
   }
   mEdgeStateBeingTracked = stateToTrack;
   if (stateToTrack == wkf::eTEAM_COLOR)
   {
      auto teamColor =
         wkfEnv.GetPreferenceObject<wkf::TeamVisibilityPrefObject>()->GetTeamUtColor(mPlatformPtr->GetSide());
      vespa::VaAttachmentWingRibbon* wingribbon =
         dynamic_cast<vespa::VaAttachmentWingRibbon*>(mPlatformPtr->FindAttachment(mWingRibbonUniqueId));
      if (wingribbon != nullptr)
      {
         teamColor[3] = .5; // apply an alpha to the team color
         wingribbon->SetEdgeMinColor(teamColor);
         wingribbon->SetEdgeMaxColor(teamColor);
         for (int x = 0; x < 3; x++)
         {
            teamColor[x] *= 0.7f;
         } // make flip side 30% darker, leave transparency the same
         wingribbon->SetEdgeMinFlippedColor(teamColor);
         wingribbon->SetEdgeMaxFlippedColor(teamColor);
      }
   }
}

/*************************************************************************************************
 * @brief   Changes how the colors of the wing ribbon's edge area is calculated in response to the
 *          recorded state values.
 * @param   aEdgeLimitType Type of the edge limit that determines how the colors of the wing
 *                         ribbon's main is calculated in response to the recorded state values.
 * @date 7/14/2017
 **************************************************************************************************/
void WkPlatformHistory::Plugin::WingRibbonData::SetEdgeLimitType(wkf::InterpolationLimitsMethod aEdgeLimitType)
{
   if (mEdgeLimitType == aEdgeLimitType)
   {
      return; // nothing changed
   }

   mEdgeLimitType = aEdgeLimitType;

   vespa::VaAttachmentWingRibbon* wingribbon =
      dynamic_cast<vespa::VaAttachmentWingRibbon*>(mPlatformPtr->FindAttachment(mWingRibbonUniqueId));
   if (wingribbon != nullptr)
   {
      wingribbon->SetEdgeLimitType(static_cast<vespa::VaAttachmentWingRibbon::InterpolationLimitsMethod>(mEdgeLimitType));
      if (mEdgeLimitType == wkf::eHARD_LIMITS)
      {
         wingribbon->SetHardMaxEdgeLimit(mHardMaxEdgeLimit);
         wingribbon->SetHardMinEdgeLimit(mHardMinEdgeLimit);
      }
      else
      {
         if (mEdgeLimitType == wkf::eCURRENT_SCENARIO_LIMITS || mEdgeLimitType == wkf::eINDEFINITE_SCENARIO_LIMITS)
         {
            mCurrentScenarioEdgeMax = std::max(mCurrentScenarioEdgeMax, wingribbon->GetCurrentEdgeMaxStateVal());
            mCurrentScenarioEdgeMin = std::min(mCurrentScenarioEdgeMin, wingribbon->GetCurrentEdgeMinStateVal());
         }
      }
   }
}

/*************************************************************************************************
 * @brief   Changes how the colors of the wing ribbon's main/center area is calculated in response to the
 *          recorded state values.
 * @param   aMainLimitType Type of the main limit that determines how the colors of the wing
 *                         ribbon's main is calculated in response to the recorded state values.
 * @date 7/14/2017
 **************************************************************************************************/
void WkPlatformHistory::Plugin::WingRibbonData::SetMainLimitType(wkf::InterpolationLimitsMethod aMainLimitType)
{
   if (mMainLimitType == aMainLimitType)
   {
      return; // nothing changed
   }

   mMainLimitType = aMainLimitType;

   vespa::VaAttachmentWingRibbon* wingribbon =
      dynamic_cast<vespa::VaAttachmentWingRibbon*>(mPlatformPtr->FindAttachment(mWingRibbonUniqueId));
   if (wingribbon != nullptr)
   {
      wingribbon->SetMainLimitType(static_cast<vespa::VaAttachmentWingRibbon::InterpolationLimitsMethod>(mMainLimitType));
      if (mMainLimitType == wkf::eHARD_LIMITS)
      {
         wingribbon->SetHardMaxMainLimit(mHardMaxMainLimit);
         wingribbon->SetHardMinMainLimit(mHardMinMainLimit);
      }
      else
      {
         if (mMainLimitType == wkf::eCURRENT_SCENARIO_LIMITS || mMainLimitType == wkf::eINDEFINITE_SCENARIO_LIMITS)
         {
            mCurrentScenarioMainMax = std::max(mCurrentScenarioMainMax, wingribbon->GetCurrentMainMaxStateVal());
            mCurrentScenarioMainMin = std::min(mCurrentScenarioMainMin, wingribbon->GetCurrentMainMinStateVal());
         }
      }
   }
}

/// @see PlatformHistory::Plugin::WingRibbonHardMaxMainLimitHandler for documented behavior
void WkPlatformHistory::Plugin::WingRibbonData::SetHardMaxMainLimit(float aHardMaxMainLimit)
{
   mHardMaxMainLimit = aHardMaxMainLimit;
   if (mMainLimitType == wkf::eHARD_LIMITS)
   {
      vespa::VaAttachmentWingRibbon* wingribbon =
         dynamic_cast<vespa::VaAttachmentWingRibbon*>(mPlatformPtr->FindAttachment(mWingRibbonUniqueId));
      if (wingribbon != nullptr)
      {
         wingribbon->SetHardMaxMainLimit(mHardMaxMainLimit);
      }
   }
}
/// @see PlatformHistory::Plugin::WingRibbonHardMinMainLimitHandler for documented behavior
void WkPlatformHistory::Plugin::WingRibbonData::SetHardMinMainLimit(float aHardMinMainLimit)
{
   mHardMinMainLimit = aHardMinMainLimit;
   if (mMainLimitType == wkf::eHARD_LIMITS)
   {
      vespa::VaAttachmentWingRibbon* wingribbon =
         dynamic_cast<vespa::VaAttachmentWingRibbon*>(mPlatformPtr->FindAttachment(mWingRibbonUniqueId));
      if (wingribbon != nullptr)
      {
         wingribbon->SetHardMinMainLimit(aHardMinMainLimit);
      }
   }
}
/// @see PlatformHistory::Plugin::WingRibbonHardMaxEdgeLimitHandler for documented behavior
void WkPlatformHistory::Plugin::WingRibbonData::SetHardMaxEdgeLimit(float aHardMaxEdgeLimit)
{
   mHardMaxEdgeLimit = aHardMaxEdgeLimit;
   if (mEdgeLimitType == wkf::eHARD_LIMITS)
   {
      vespa::VaAttachmentWingRibbon* wingribbon =
         dynamic_cast<vespa::VaAttachmentWingRibbon*>(mPlatformPtr->FindAttachment(mWingRibbonUniqueId));
      if (wingribbon != nullptr)
      {
         wingribbon->SetHardMaxEdgeLimit(aHardMaxEdgeLimit);
      }
   }
}
/// @see PlatformHistory::Plugin::WingRibbonHardMinEdgeLimitHandler for documented behavior
void WkPlatformHistory::Plugin::WingRibbonData::SetHardMinEdgeLimit(float aHardMinEdgeLimit)
{
   mHardMinEdgeLimit = aHardMinEdgeLimit;
   if (mEdgeLimitType == wkf::eHARD_LIMITS)
   {
      vespa::VaAttachmentWingRibbon* wingribbon =
         dynamic_cast<vespa::VaAttachmentWingRibbon*>(mPlatformPtr->FindAttachment(mWingRibbonUniqueId));
      if (wingribbon != nullptr)
      {
         wingribbon->SetHardMinEdgeLimit(aHardMinEdgeLimit);
      }
   }
}
/// @see PlatformHistory::Plugin::WingRibbonWidthScaleHandler for documented behavior
void WkPlatformHistory::Plugin::WingRibbonData::SetWidthScale(float aWidthScale)
{
   mWidthScale = aWidthScale;
   vespa::VaAttachmentWingRibbon* wingribbon =
      dynamic_cast<vespa::VaAttachmentWingRibbon*>(mPlatformPtr->FindAttachment(mWingRibbonUniqueId));
   if (wingribbon != nullptr)
   {
      wingribbon->SetWidthScale(aWidthScale);
   }
}
/// @see PlatformHistory::Plugin::WingRibbonHardEdgeWidthHandler for documented behavior
void WkPlatformHistory::Plugin::WingRibbonData::SetHardEdgeWidth(float aHardEdgeWidth)
{
   mHardEdgeWidth = aHardEdgeWidth;
   vespa::VaAttachmentWingRibbon* wingribbon =
      dynamic_cast<vespa::VaAttachmentWingRibbon*>(mPlatformPtr->FindAttachment(mWingRibbonUniqueId));
   if (wingribbon != nullptr)
   {
      wingribbon->SetHardEdgeWidth(aHardEdgeWidth);
   }
}
/// @see PlatformHistory::Plugin::WingRibbonSoftEdgeWidthHandler for documented behavior
void WkPlatformHistory::Plugin::WingRibbonData::SetSoftEdgeWidth(float aSoftEdgeWidth)
{
   mSoftEdgeWidth = aSoftEdgeWidth;
   vespa::VaAttachmentWingRibbon* wingribbon =
      dynamic_cast<vespa::VaAttachmentWingRibbon*>(mPlatformPtr->FindAttachment(mWingRibbonUniqueId));
   if (wingribbon != nullptr)
   {
      wingribbon->SetSoftEdgeWidth(aSoftEdgeWidth);
   }
}

/*************************************************************************************************
 * @brief   Sets the wing ribbon attachment's visibility.
 * @note hiding a wing ribbon causes it to clear it's history and will have the wing ribbon start
 *       over with zero length when set to be visible again this is done for performance tuning
 *       to prevent unnecessary datapoint collecting when the ribbon is invisible. If you want to
 *       see past history of where a platform has been while an attachment wasn't visible, use
 *       tracelines instead.
 * @param   aVisible True to show, false to hide.
 * @date 7/14/2017
 **************************************************************************************************/
void WkPlatformHistory::Plugin::WingRibbonData::SetVisible(bool aVisible)
{
   vespa::VaAttachment* attachment = mPlatformPtr->FindAttachment(mWingRibbonUniqueId);
   if (attachment != nullptr)
   {
      if (aVisible == false && IsVisible())
      {
         // changing from visible to not visible
         auto wingribbon = dynamic_cast<vespa::VaAttachmentWingRibbon*>(attachment);
         if (wingribbon != nullptr)
         {
            // delete everything in the wingribbon buffer, so that when turned back on their isn't a giant line that
            // stretches from where it was turned off to where it was turned on
            wingribbon->PruneAfter(std::numeric_limits<float>::lowest());
            wingribbon->shrink_to_fit(); // clear the memory
         }
      }
      attachment->SetStateVisibility(aVisible);
   }
}

/*************************************************************************************************
 * @brief   Query if the wing ribbon attachment is visible.
 * @return  True if visible, false if not.
 * @date 7/14/2017
 **************************************************************************************************/
bool WkPlatformHistory::Plugin::WingRibbonData::IsVisible()
{
   vespa::VaAttachment* attachment = mPlatformPtr->FindAttachment(mWingRibbonUniqueId);
   if (attachment != nullptr)
   {
      return attachment->GetStateVisibility();
   }
   return false;
}

/*************************************************************************************************
 * @brief   Query if 'aPlatformPtr' is alive.
 * @param   aPlatformPtr   The platform pointer that we want to know the liveliness of.
 * @return  True if alive, false if not.
 * @date 7/28/2017
 **************************************************************************************************/
bool IsAlive(warlock::Platform* aPlatformPtr)
{
   return aPlatformPtr->IsAlive();
}
/*************************************************************************************************
 * @brief   Updates the wingribbon with a new datapoint, of where the platform's wing tips are
 *          and the values of the tracked platform states. Also erases datapoints that are older
 *          than the desired length of the wing ribbon.
 * @note Datapoints will only be generated if the wing ribbon is visible (for mild performance
 *       reasons), and if the platform time has advanced.  This means that a wingribbon has to be
 *       reset if a wing ribbon is turned off and then back on again, other the ribbon would
 *       stretch from the two points where it was turned off and back on again.
 * @note in this same call, the static values for min/max scenario value are collected from the
 *       min/max values currently recorded in the wingribbon.
 * @date 7/14/2017
 **************************************************************************************************/
void WkPlatformHistory::Plugin::WingRibbonData::Update()
{
   double curTime = mPlatformPtr->GetUpdateTime();
   if (!IsVisible() || curTime == 0)
   {
      // don't do all this math if it is not needed
      return;
   }

   if (mLastUpdateTime < curTime)
   {
      // mLastUpdateTime = curTime;

      double                         minTime = curTime - mWingRibbonLengthSec;
      vespa::VaAttachmentWingRibbon* wingribbon =
         dynamic_cast<vespa::VaAttachmentWingRibbon*>(mPlatformPtr->FindAttachment(mWingRibbonUniqueId));
      if (wingribbon != nullptr)
      {
         wingribbon->PruneBefore(minTime);

         double ecef[3];
         mPlatformPtr->GetPosition().GetECEF(ecef);

         double hpr[3];
         mPlatformPtr->GetUtoEntity()->GetOrientationWCS(hpr[0], hpr[1], hpr[2]);
         auto         q = osg::Quat(hpr[2], osg::Vec3(1, 0, 0), hpr[1], osg::Vec3(0, 1, 0), hpr[0], osg::Vec3(0, 0, 1));
         osg::Matrixf rotation;
         q.get(rotation);

         osg::Vec3f wingTipA = mWingTipA;
         osg::Vec3f wingTipB = mWingTipB;

         wingTipA = rotation.preMult(wingTipA);
         wingTipB = rotation.preMult(wingTipB);

         float stateValEdge = RetrieveStateValFrom(mPlatformPtr, mEdgeStateBeingTracked);
         float stateValMain = RetrieveStateValFrom(mPlatformPtr, mMainStateBeingTracked);

         bool isAlive = IsAlive(mPlatformPtr);
         wingribbon->AddPointBack(curTime, ecef, stateValMain, stateValEdge, wingTipA, wingTipB, isAlive);

         if (mMainLimitType == wkf::eCURRENT_SCENARIO_LIMITS || mMainLimitType == wkf::eINDEFINITE_SCENARIO_LIMITS)
         {
            mCurrentScenarioMainMax = std::max(mCurrentScenarioMainMax, wingribbon->GetCurrentMainMaxStateVal());
            mCurrentScenarioMainMin = std::min(mCurrentScenarioMainMin, wingribbon->GetCurrentMainMinStateVal());
         }
         if (mEdgeLimitType == wkf::eCURRENT_SCENARIO_LIMITS || mEdgeLimitType == wkf::eINDEFINITE_SCENARIO_LIMITS)
         {
            mCurrentScenarioEdgeMax = std::max(mCurrentScenarioEdgeMax, wingribbon->GetCurrentEdgeMaxStateVal());
            mCurrentScenarioEdgeMin = std::min(mCurrentScenarioEdgeMin, wingribbon->GetCurrentEdgeMinStateVal());
         }
      }
   }
}

float WkPlatformHistory::Plugin::WingRibbonData::mCurrentScenarioMainMax =
   std::numeric_limits<decltype(mCurrentScenarioMainMax)>::lowest();
float WkPlatformHistory::Plugin::WingRibbonData::mCurrentScenarioMainMin =
   std::numeric_limits<decltype(mCurrentScenarioMainMin)>::max();
float WkPlatformHistory::Plugin::WingRibbonData::mCurrentScenarioEdgeMax =
   std::numeric_limits<decltype(mCurrentScenarioEdgeMax)>::lowest();
float WkPlatformHistory::Plugin::WingRibbonData::mCurrentScenarioEdgeMin =
   std::numeric_limits<decltype(mCurrentScenarioEdgeMin)>::max();

/*************************************************************************************************
 * @brief   Resets the current scenario limits if needed. It is necessary to reset the scenario
 *          limits if the limit type is using current scenario limits instead of indefinite scenario
 *          limits, as not reseting would allow the highest/lowest recorded values to outlive how
 *          long they existed in the scenario's wingribbons.
 * @date 7/14/2017
 **************************************************************************************************/
void WkPlatformHistory::Plugin::WingRibbonData::ResetCurrentScenarioLimitsIfNeeded()
{
   if (mMainLimitType == wkf::eCURRENT_SCENARIO_LIMITS)
   {
      ResetCurrentScenarioMainLimits();
   }
   if (mEdgeLimitType == wkf::eCURRENT_SCENARIO_LIMITS)
   {
      ResetCurrentScenarioEdgeLimits();
   }
}

/*************************************************************************************************
 * @brief   Resets the current scenario main limits.
 * @see ResetCurrentScenarioLimitsIfNeeded for further documentation
 * @date 7/14/2017
 **************************************************************************************************/
void WkPlatformHistory::Plugin::WingRibbonData::ResetCurrentScenarioMainLimits()
{
   mCurrentScenarioMainMax = std::numeric_limits<decltype(mCurrentScenarioMainMax)>::lowest();
   mCurrentScenarioMainMin = std::numeric_limits<decltype(mCurrentScenarioMainMin)>::max();
}

/*************************************************************************************************
 * @brief   Resets the current scenario edge limits.
 * @see ResetCurrentScenarioLimitsIfNeeded for further documentation
 * @date 7/14/2017
 **************************************************************************************************/
void WkPlatformHistory::Plugin::WingRibbonData::ResetCurrentScenarioEdgeLimits()
{
   mCurrentScenarioEdgeMax = std::numeric_limits<decltype(mCurrentScenarioEdgeMax)>::lowest();
   mCurrentScenarioEdgeMin = std::numeric_limits<decltype(mCurrentScenarioEdgeMin)>::max();
}

/*************************************************************************************************
 * @brief   Sets the min/max limits that determine what color to use in response to the
 *          wingribbons recorded values, if using a scenario wide InterpolationLimitsMethod.
 *
 *          This is called on all the wingribbons after all the wingribbons have been updated with
 *          new data points, wherein there may potentially be a newly recorded max/min value which
 *          needs to be propagated to the rest of the wingribbons.
 * @see   Update for further details on how scenario wide values are collected.
 * @date 7/14/2017
 **************************************************************************************************/
void WkPlatformHistory::Plugin::WingRibbonData::SetCurrentScenarioLimitsIfNeeded()
{
   double curTime = mPlatformPtr->GetUpdateTime();
   if (mLastUpdateTime == curTime)
   {
      // if the scenario is paused, then the limits were reset if current_scenario_limits thanks
      // to a call to ResetCurrentScenarioLimitsIfNeeded, but they were never recalculated in Update() because
      // that only runs if time advanced and so if we continued, the scenario limits would be set to min/max infinity
      return;
   }
   mLastUpdateTime = curTime;

   if (mMainLimitType == wkf::eCURRENT_SCENARIO_LIMITS || mMainLimitType == wkf::eINDEFINITE_SCENARIO_LIMITS)
   {
      vespa::VaAttachmentWingRibbon* wingribbon =
         dynamic_cast<vespa::VaAttachmentWingRibbon*>(mPlatformPtr->FindAttachment(mWingRibbonUniqueId));
      if (wingribbon != nullptr)
      {
         if (mMainLimitType == wkf::eCURRENT_SCENARIO_LIMITS)
         {
            wingribbon->SetScenarioMaxMainLimit(mCurrentScenarioMainMax);
            wingribbon->SetScenarioMinMainLimit(mCurrentScenarioMainMin);
         }
         else
         {
            // this if/else isn't necessary, indefinite could setScenario instead of Hard too,
            //  but for efficiency reasons, by setting Hard there won't be as many somewhat expensive calls to
            //  recalculate min/max in the attachment
            wingribbon->SetHardMaxMainLimit(mCurrentScenarioMainMax);
            wingribbon->SetHardMinMainLimit(mCurrentScenarioMainMin);
         }
      }
   }
   if (mEdgeLimitType == wkf::eCURRENT_SCENARIO_LIMITS || mEdgeLimitType == wkf::eINDEFINITE_SCENARIO_LIMITS)
   {
      vespa::VaAttachmentWingRibbon* wingribbon =
         dynamic_cast<vespa::VaAttachmentWingRibbon*>(mPlatformPtr->FindAttachment(mWingRibbonUniqueId));
      if (wingribbon != nullptr)
      {
         if (mEdgeLimitType == wkf::eCURRENT_SCENARIO_LIMITS)
         {
            wingribbon->SetScenarioMaxEdgeLimit(mCurrentScenarioEdgeMax);
            wingribbon->SetScenarioMinEdgeLimit(mCurrentScenarioEdgeMin);
         }
         else
         {
            // this if/else isn't necessary, indefinite could setScenario instead of Hard too,
            //  but for efficiency reasons, by setting Hard there won't be as many calls to recalculate min/max in the attachment
            wingribbon->SetHardMaxEdgeLimit(mCurrentScenarioEdgeMax);
            wingribbon->SetHardMinEdgeLimit(mCurrentScenarioEdgeMin);
         }
      }
   }
}

/*************************************************************************************************
 * @brief   Retrieves that tracked state value from the platform.
 * @param   aPlatformPtr   Pointer to the platform we want to retrieve the value from.
 * @param   aStateTracked  The type of state being tracked that we want to retrieve from the
 *                         platform.
 * @return  The current value of the platform's state thats being tracked.
 * @date 7/14/2017
 **************************************************************************************************/
float WkPlatformHistory::Plugin::WingRibbonData::RetrieveStateValFrom(vespa::VaEntity*         aPlatformPtr,
                                                                      wkf::TrackableStateValue aStateTracked)
{
   switch (aStateTracked)
   {
   case wkf::eHEADING:
      return std::abs(sin(mPlatformPtr->GetHeading()));
   case wkf::ePITCH:
      return aPlatformPtr->GetPitch();
   case wkf::eROLL:
      return aPlatformPtr->GetRoll();
   case wkf::eSPEED:
      return aPlatformPtr->GetVelocityWCS().Magnitude();
   case wkf::eALTITUDE:
      return aPlatformPtr->GetPosition().GetAlt();
   case wkf::eTEAM_COLOR:
      return 0.5f; // team color is handled through setting min/max color to the team color, any state val will do
   case wkf::eTRACK_NONE:
      return 0.5f; // min/max color to the same color, any state val will do
   default:
      assert(false); // unreachable code
      return 0;
   }
}
