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
#ifndef BM_SIM_INTERFACE_HPP
#define BM_SIM_INTERFACE_HPP

#include <mutex>

#include "BM_SimEvent.hpp"
#include "WkSimInterface.hpp"

namespace WkBM
{
class SimInterface final : public warlock::SimInterfaceT<BMSimEvent>
{
public:
   explicit SimInterface(const QString& aPlugin) noexcept;

   //! Executed on sim thread
   //! {
   void PlatformAdded(double aSimTime, WsfPlatform& aPlatform) override;
   void PlatformDeleted(double aSimTime, const WsfPlatform& aPlatform) override;
   void SimulationInitializing(const WsfSimulation& aSimulation) override;
   void SimulationClockRead(const WsfSimulation& aSimulation) override;
   //! }

   void ClearAuxDataKeys();
   void AddAuxDataKey(const QString& aKey);

   double GetSimTime() const noexcept;

private:
   void SetSimTime(double aSimTime) noexcept;

   double                   mSimTime;
   std::mutex               mAuxDataMutex;
   std::vector<std::string> mAuxDataKeys;
   AuxDataMap               mAuxData;
};
} // namespace WkBM

#endif
