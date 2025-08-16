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

#ifndef WKSATELLITETETHERDOCKWIDGET_HPP
#define WKSATELLITETETHERDOCKWIDGET_HPP

#include <set>
#include <string>

#include "SatelliteTetherPropagationManager.hpp"
#include "SatelliteTetherSimInterface.hpp"
#include "satellite_tether/WkfSatelliteTetherDockWidget.hpp"

namespace SatelliteTether
{
class AttachmentTrace;

class DockWidget : public wkf::SatelliteTetherDockWidget
{
public:
   DockWidget(const QString&                 aObjectName,
              const QString&                 aPlatformName,
              unsigned int                   aId,
              SatelliteTether::SimInterface* aInterface,
              PropagationManager&            aPropagationManager,
              QMainWindow*                   aParent = nullptr,
              Qt::WindowFlags                aFlags  = Qt::WindowFlags());
   ~DockWidget() override;

   PropagationManager& GetPropagationManager() const { return mPropagationManager; }

   bool PropagatorIsOfInterest(const std::string& aPlatformName) const;

   void UpdateTraceDataForPlatform(const std::string& aPlatformName, double aSimTime) const;

   void SetupOwnship();

private:
   void UpdateTrace(wkf::AttachmentSatelliteTetherTrace* aTracePtr) override;
   void UpdateTraceRIC_Data(AttachmentTrace* aTracePtr, double aSimTime) const;
   wkf::AttachmentSatelliteTetherTrace*
        MakeTrace(wkf::Platform& aSrc, vespa::VaViewer& aViewer, wkf::Platform& aTgt, const QColor& aColor) override;
   void AddTrack(wkf::Platform* aPlatformPtr) override;
   void ClearTracks() override;
   void PlatformToBeDeletedPrivate(wkf::Platform* aPlatformPtr) override;

   SatelliteTether::SimInterface* mInterfacePtr;
   PropagationManager&            mPropagationManager;
   std::set<std::string>          mTargetList{};
};
} // namespace SatelliteTether

#endif
