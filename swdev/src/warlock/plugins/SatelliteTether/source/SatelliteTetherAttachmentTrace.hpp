// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2021 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef SATELLITETETHERATTACHMENTTRACE_HPP
#define SATELLITETETHERATTACHMENTTRACE_HPP

#include <vector>

#include "satellite_tether/WkfAttachmentSatelliteTetherTrace.hpp"

class UtCalendar;
class UtOrbitalPropagatorBase;
#include "UtVec3.hpp"


namespace SatelliteTether
{

class AttachmentTrace : public wkf::AttachmentSatelliteTetherTrace
{
public:
   AttachmentTrace(vespa::VaEntity& aParentEntity,
                   vespa::VaViewer& aViewer,
                   vespa::VaEntity& aRelativeToEntity,
                   const QColor&    aColor,
                   double           aInitialSimTime);
   ~AttachmentTrace() override = default;

   void UpdateData() override;

   void UpdateRIC_Data(UtOrbitalPropagatorBase* aSourcePropagatorPtr,
                       UtOrbitalPropagatorBase* aTargetPropagatorPtr,
                       const UtCalendar&        aUpdateEpoch,
                       double                   aUpdateTime);

   bool PropagatorIsOfInterest(const std::string& aPlatformName) const;

private:
   void AddRIC_Update(const UtVec3d& aRIC_Update, double aUpdateTime);

   std::vector<UtVec3d> mRIC_Updates{};
   double               mInitialSimTime;
   double               mLastDataSimTime{-1.0};
};

} // namespace SatelliteTether

#endif // SATELLITETETHERATTACHMENTTRACE_HPP
