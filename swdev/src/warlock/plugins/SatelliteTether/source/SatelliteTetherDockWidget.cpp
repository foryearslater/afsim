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
#include "SatelliteTetherDockWidget.hpp"

#include "SatelliteTetherAttachmentTrace.hpp"
#include "SatelliteTetherSimCommands.hpp"
#include "VaUtils.hpp"
#include "WkSimEnvironment.hpp"
#include "WkfPlatform.hpp"
#include "satellite_tether/WkfSatelliteTetherDockWidget.hpp"

SatelliteTether::DockWidget::DockWidget(const QString&                 aObjectName,
                                        const QString&                 aPlatformName,
                                        unsigned int                   aId,
                                        SatelliteTether::SimInterface* aInterface,
                                        PropagationManager&            aPropagationManager,
                                        QMainWindow*                   aParent /*= nullptr*/,
                                        Qt::WindowFlags                aFlags /*= Qt::WindowFlags()*/)
   : wkf::SatelliteTetherDockWidget(aObjectName, aPlatformName, aId, aParent, aFlags)
   , mInterfacePtr(aInterface)
   , mPropagationManager(aPropagationManager)
{
   mTargetList.clear();
   SetupOwnship();
}

SatelliteTether::DockWidget::~DockWidget()
{
   ClearTracks();
   if (mPlatformPtr != nullptr)
   {
      mPropagationManager.RemovePlatformOfInterest(mPlatformPtr->GetName());
      mInterfacePtr->RemovePlatformOfInterest(mPlatformPtr->GetName());
   }
}

bool SatelliteTether::DockWidget::PropagatorIsOfInterest(const std::string& aPlatformName) const
{
   bool retval{false};
   if (aPlatformName == mPlatformPtr->GetName())
   {
      retval = true;
   }
   else if (mTargetList.find(aPlatformName) != mTargetList.end())
   {
      retval = true;
   }
   return retval;
}

void SatelliteTether::DockWidget::UpdateTraceDataForPlatform(const std::string& aPlatformName, double aSimTime) const
{
   auto op = [this, &aPlatformName, aSimTime](wkf::AttachmentSatelliteTetherTrace* aTracePtr)
   {
      auto tracePtr = dynamic_cast<AttachmentTrace*>(aTracePtr);
      if (tracePtr)
      {
         if (tracePtr->PropagatorIsOfInterest(aPlatformName))
         {
            UpdateTraceRIC_Data(tracePtr, aSimTime);
         }
      }
   };
   ApplyToTraces(op);
}

void SatelliteTether::DockWidget::SetupOwnship()
{
   mInterfacePtr->AddSimCommand(
      ut::make_unique<AddOwnshipCommand>(mInterfacePtr, GetConnectedPlatformName().toStdString()));
   mInterfacePtr->AddPlatformOfInterest(GetConnectedPlatformName().toStdString());
   mPropagationManager.AddPlatformOfInterest(GetConnectedPlatformName().toStdString());
}

void SatelliteTether::DockWidget::UpdateTrace(wkf::AttachmentSatelliteTetherTrace* aTracePtr)
{
   auto tracePtr = dynamic_cast<AttachmentTrace*>(aTracePtr);
   if (tracePtr)
   {
      double time = vaEnv.GetDisplayedTime();
      UpdateTraceRIC_Data(tracePtr, time);
      tracePtr->UpdateData();
   }
}

void SatelliteTether::DockWidget::UpdateTraceRIC_Data(AttachmentTrace* aTracePtr, double aSimTime) const
{
   auto       srcPropPtr = mPropagationManager.GetPropagator(aTracePtr->GetParent().GetName());
   auto       tgtPropPtr = mPropagationManager.GetPropagator(aTracePtr->GetOriginPlatformName());
   UtCalendar epoch      = mPropagationManager.GetSimulationStartEpoch();
   epoch.AdvanceTimeBy(aSimTime);
   aTracePtr->UpdateRIC_Data(srcPropPtr, tgtPropPtr, epoch, aSimTime);
}

wkf::AttachmentSatelliteTetherTrace* SatelliteTether::DockWidget::MakeTrace(wkf::Platform&   aSrc,
                                                                            vespa::VaViewer& aViewer,
                                                                            wkf::Platform&   aTgt,
                                                                            const QColor&    aColor)
{
   mTargetList.insert(aTgt.GetName());
   mInterfacePtr->AddPlatformOfInterest(aTgt.GetName());
   mPropagationManager.AddPlatformOfInterest(aTgt.GetName());
   return vespa::make_attachment<AttachmentTrace>(aSrc, aViewer, aTgt, aColor, vaEnv.GetDisplayedTime());
}

void SatelliteTether::DockWidget::AddTrack(wkf::Platform* aPlatformPtr)
{
   mInterfacePtr->AddSimCommand(ut::make_unique<AddTrackCommand>(mInterfacePtr,
                                                                 aPlatformPtr->GetName(),
                                                                 GetId(),
                                                                 GetTrackColor(aPlatformPtr->GetName())));
}

void SatelliteTether::DockWidget::ClearTracks()
{
   for (auto&& iter : mTargetList)
   {
      mPropagationManager.RemovePlatformOfInterest(iter);
      mInterfacePtr->RemovePlatformOfInterest(iter);
   }
   mTargetList.clear();
   wkf::SatelliteTetherDockWidget::ClearTracks();
}

void SatelliteTether::DockWidget::PlatformToBeDeletedPrivate(wkf::Platform* aPlatformPtr)
{
   if (aPlatformPtr == mPlatformPtr)
   {
      ClearTracks();
      ClearGrid();
   }
}
