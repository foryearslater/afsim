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

#include "ChangeWrapper.hpp"

#include "ProjectWorkspace.hpp"
#include "zone/WkfAttachmentZone.hpp"
#include "zone/WkfZonePoint.hpp"

namespace wizard
{

ChangeWrapper::ChangeWrapper(WsfPProxyNode aProxyNode)
   : mNode(std::move(aProxyNode))
{
   SetRangeFromNode();
   SetEditorFromRange();
}

ChangeWrapper::ChangeWrapper(wizard::Platform* aPlatformPtr, WsfPProxyNode aProxyNode)
   : mPlatformPtr(aPlatformPtr)
   , mNode(std::move(aProxyNode))
{
   SetRangeFromNode();
   SetEditorFromRange();
}

ChangeWrapper::ChangeWrapper(wizard::Platform* aPlatformPtr, vespa::VaAttachment* aAttachmentPtr, WsfPProxyNode aProxyNode)
   : mPlatformPtr(aPlatformPtr)
   , mAttachmentPtr(aAttachmentPtr)
   , mNode(std::move(aProxyNode))
{
   SetRangeFromNode();
   SetEditorFromRange();
}

ChangeWrapper::ChangeWrapper(wizard::Platform* aPlatformPtr, wkf::ZonePoint* aZonePointPtr, WsfPProxyNode aProxyNode)
   : mPlatformPtr(aPlatformPtr)
   , mZonePointPtr(aZonePointPtr)
   , mNode(std::move(aProxyNode))
{
   SetRangeFromNode();
   SetEditorFromRange();
}

ChangeWrapper::ChangeWrapper(wizard::Platform* aPlatformPtr, wkf::Waypoint* aWaypointPtr, WsfPProxyNode aProxyNode)
   : mPlatformPtr(aPlatformPtr)
   , mWaypointPtr(aWaypointPtr)
   , mNode(std::move(aProxyNode))
{
   SetRangeFromNode();
   SetEditorFromRange();
}

ChangeWrapper::ChangeWrapper(wkf::PointOfInterest* aPointOfInterestPtr, WsfPProxyNode aProxyNode)
   : mPointOfInterestPtr(aPointOfInterestPtr)
   , mNode(std::move(aProxyNode))
{
   SetRangeFromNode();
   SetEditorFromRange();
}

wkf::AttachmentRoute* ChangeWrapper::GetRoute() const noexcept
{
   return dynamic_cast<wkf::AttachmentRoute*>(mAttachmentPtr);
}

wkf::Waypoint* ChangeWrapper::GetWaypoint() const noexcept
{
   return mWaypointPtr;
}

wkf::AttachmentZone* ChangeWrapper::GetZone() const noexcept
{
   return dynamic_cast<wkf::AttachmentZone*>(mAttachmentPtr);
}

wkf::ZonePoint* ChangeWrapper::GetZonePoint() const noexcept
{
   return mZonePointPtr;
}

wizard::Platform* ChangeWrapper::GetPlatform() const noexcept
{
   return mPlatformPtr;
}

wkf::PointOfInterest* ChangeWrapper::GetPointOfInterest() const noexcept
{
   return mPointOfInterestPtr;
}

void ChangeWrapper::SetRangeFromNode() noexcept
{
   mRange = mNode.GetSourceDocumentRange();
   // if there was no valid range for this attribute, default to the platform's document range if it's attached to one
   if (!mRange.Valid())
   {
      if (mPlatformPtr)
      {
         mRange = mPlatformPtr->GetPlatform().GetSourceDocumentRange();
      }
   }
}

void ChangeWrapper::SetEditorFromRange() noexcept
{
   if (mRange.Valid())
   {
      std::string fileName{mRange.mSource->GetFilePath().GetNormalizedPath()};
      mEditorPtr = wizard::ProjectWorkspace::Instance()->GotoFile(fileName);
   }
}

} // namespace wizard
