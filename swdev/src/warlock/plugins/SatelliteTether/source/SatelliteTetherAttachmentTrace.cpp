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

#include "SatelliteTetherAttachmentTrace.hpp"

#include "UtCalendar.hpp"
#include "UtOrbitalPropagatorBase.hpp"
#include "UtRIC_Frame.hpp"
#include "UtoMemory.hpp"
#include "VaEntity.hpp"

namespace SatelliteTether
{

AttachmentTrace::AttachmentTrace(vespa::VaEntity& aParentEntity,
                                 vespa::VaViewer& aViewer,
                                 vespa::VaEntity& aRelativeToEntity,
                                 const QColor&    aColor,
                                 double           aInitialSimTime)
   : wkf::AttachmentSatelliteTetherTrace(aParentEntity, aViewer, aRelativeToEntity, aColor)
   , mInitialSimTime{aInitialSimTime}
{
}

void AttachmentTrace::UpdateData()
{
   if (mRIC_Updates.empty())
   {
      return;
   }

   osg::ref_ptr<osg::DrawElementsUInt> pset;
   if (mGeometryPtr->getNumPrimitiveSets() == 0)
   {
      pset = ut::osg::make_ref<osg::DrawElementsUInt>(GL_LINE_STRIP);
      mGeometryPtr->addPrimitiveSet(pset);
   }
   else
   {
      pset = dynamic_cast<osg::DrawElementsUInt*>(mGeometryPtr->getPrimitiveSet(0));
   }

   size_t index = mVertexArray->size();
   for (const auto& ric : mRIC_Updates)
   {
      mVertexArray->push_back(osg::Vec3(ric[0], ric[1], ric[2]));
      mColorArray->push_back(osg::Vec4(mColor.redF(), mColor.greenF(), mColor.blueF(), 0.8f));
      pset->push_back(index);
      ++index;
   }
   mOldRic = mRIC_Updates.back();
   mRIC_Updates.clear();
   mGeometryPtr->dirtyDisplayList();
}

void AttachmentTrace::UpdateRIC_Data(UtOrbitalPropagatorBase* aSourcePropagatorPtr,
                                     UtOrbitalPropagatorBase* aTargetPropagatorPtr,
                                     const UtCalendar&        aUpdateEpoch,
                                     double                   aUpdateTime)
{
   static constexpr double cUPDATE_INTERVAL{3.0};

   if (mLastDataSimTime < 0.0 || aUpdateTime - mLastDataSimTime > cUPDATE_INTERVAL)
   {
      UtCalendar sourceEpoch = aSourcePropagatorPtr->GetCurrentTime();
      UtCalendar targetEpoch = aTargetPropagatorPtr->GetCurrentTime();

      double loopInitialTime = mLastDataSimTime > 0.0 ? -(aUpdateTime - mLastDataSimTime) + cUPDATE_INTERVAL :
                                                        -(aUpdateTime - mInitialSimTime);

      UtCalendar epoch = aUpdateEpoch;
      epoch.AdvanceTimeBy(loopInitialTime);
      for (; epoch < aUpdateEpoch; epoch.AdvanceTimeBy(cUPDATE_INTERVAL))
      {
         aSourcePropagatorPtr->Update(epoch);
         aTargetPropagatorPtr->Update(epoch);

         auto    osv = aSourcePropagatorPtr->GetOrbitalState().GetOrbitalStateVectorInertial();
         UtVec3d ric = ut::RIC_Frame::CalculateToRIC(
            osv.GetLocation(),
            osv.GetVelocity(),
            aTargetPropagatorPtr->GetOrbitalState().GetOrbitalStateVectorInertial().GetLocation());

         double time = aUpdateTime - aUpdateEpoch.GetTimeSince(epoch);
         AddRIC_Update(ric, time);
      }

      // Now also do the final time
      aSourcePropagatorPtr->Update(aUpdateEpoch);
      aTargetPropagatorPtr->Update(aUpdateEpoch);

      auto    osv = aSourcePropagatorPtr->GetOrbitalState().GetOrbitalStateVectorInertial();
      UtVec3d ric =
         ut::RIC_Frame::CalculateToRIC(osv.GetLocation(),
                                       osv.GetVelocity(),
                                       aTargetPropagatorPtr->GetOrbitalState().GetOrbitalStateVectorInertial().GetLocation());
      AddRIC_Update(ric, aUpdateTime);

      // Reset propagators
      aSourcePropagatorPtr->Update(sourceEpoch);
      aTargetPropagatorPtr->Update(targetEpoch);
   }
}

bool AttachmentTrace::PropagatorIsOfInterest(const std::string& aPlatformName) const
{
   return GetOriginPlatformName() == aPlatformName || GetParent().GetName() == aPlatformName;
}

void AttachmentTrace::AddRIC_Update(const UtVec3d& aRIC_Update, double aUpdateTime)
{
   UtVec3d delta;
   if (mRIC_Updates.empty())
   {
      delta = aRIC_Update - mOldRic;
   }
   else
   {
      delta = aRIC_Update - mRIC_Updates.back();
   }

   if (delta.MagnitudeSquared() > cTOLERANCE_SQUARED)
   {
      mRIC_Updates.push_back(aRIC_Update);
   }
   mLastDataSimTime = aUpdateTime;
}

} // namespace SatelliteTether
