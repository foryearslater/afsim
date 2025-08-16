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

#ifndef CHANGEWRAPPER_HPP
#define CHANGEWRAPPER_HPP

namespace wkf
{
class AttachmentZone;
class ZonePoint;
class AttachmentRoute;
class Waypoint;
} // namespace wkf

#include <QString>

#include "Platform.hpp"
#include "WkfPointOfInterest.hpp"
#include "WsfPM_Waypoint.hpp"

namespace wizard
{
class Editor;

//! Wraps proxy object and visual objects to aid in making changes in the wizard::Editor triggered from the Map.
//! @par details
//!   Due to the current read-only nature of the WsfPProxy, the Map triggers updates to the wizard::Editor directly.
//!   Developers should use this class wherever there are multiple changes occurring in the wizard::Editor to avoid
//!   corrupting the text when applying the changes. Adding this class to ordered containers, such as set and map, sort
//!   in reverse, as to mitigate the need to update other ChangeWrappers when applying the changes.
//! @post After construction, the mEditorPtr and mNode are initialized.
class VI_EXPORT ChangeWrapper
{
public:
   ChangeWrapper(wizard::Editor* aEditorPtr, UtTextDocumentRange aRange)
      : mEditorPtr(aEditorPtr)
      , mRange(aRange)
   {
   }
   //! Constructs a ChangeWrapper
   //! @param aProxyNode is the proxy property to modify
   explicit ChangeWrapper(WsfPProxyNode aProxyNode);

   //! Constructs a ChangeWrapper
   //! @param aPlatformPtr is the wizard::Platform modified on the Map
   //! @param aProxyNode is the proxy property to modify
   ChangeWrapper(wizard::Platform* aPlatformPtr, WsfPProxyNode aProxyNode);

   ChangeWrapper(wizard::Editor* aEditorPtr, vespa::VaAttachment* aAttachmentPtr, UtTextDocumentRange aRange)
      : mEditorPtr(aEditorPtr)
      , mRange(aRange)
      , mAttachmentPtr(aAttachmentPtr)
   {
   }

   ChangeWrapper(wizard::Editor*      aEditorPtr,
                 wizard::Platform*    aPlatformPtr,
                 vespa::VaAttachment* aAttachmentPtr,
                 WsfPProxyNode        aProxyNode)
      : mEditorPtr(aEditorPtr)
      , mPlatformPtr(aPlatformPtr)
      , mAttachmentPtr(aAttachmentPtr)
      , mNode(aProxyNode)
   {
      SetRangeFromNode();
   }

   //! Constructs a ChangeWrapper
   //! @param aPlatformPtr is the wizard::Platform containing the attachment
   //! @param aAttachmentPtr is the vespa::VaAttachment modified on the Map
   //! @param aProxyNode is the proxy property to modify
   ChangeWrapper(wizard::Platform* aPlatformPtr, vespa::VaAttachment* aAttachmentPtr, WsfPProxyNode aProxyNode);

   //! Constructs a ChangeWrapper
   //! @param aPlatformPtr is the wizard::Platform containing the route
   //! @param aWaypointPtr is the wkf::Waypoint modified on the Map
   //! @param aProxyNode is the proxy property to modify
   ChangeWrapper(wizard::Platform* aPlatformPtr, wkf::Waypoint* aWaypointPtr, WsfPProxyNode aProxyNode);

   //! Constructs a ChangeWrapper
   //! @param aPlatformPtr is the wizard::Platform containing the zone
   //! @param aZonePointPtr is the wkf::ZonePoint modified on the Map
   //! @param aProxyNode is the proxy property to modify
   ChangeWrapper(wizard::Platform* aPlatformPtr, wkf::ZonePoint* aZonePointPtr, WsfPProxyNode aProxyNode);

   //! @param aPointOfInterestPtr is the wkf::PointOfInterest modified on the Map
   //! @param aProxyNode is the proxy property to modify
   ChangeWrapper(wkf::PointOfInterest* aPointOfInterestPtr, WsfPProxyNode aProxyNode);

   //! Allows sorting of ChangeWrapper in ordered containers.
   //! @note This sorts ChangeWrappers in descending order.
   bool operator<(const ChangeWrapper& aRHS) const noexcept
   {
      if (!mRange.Valid())
      {
         // if the range for this object is not valid bail out
         return false;
      }
      else if (!aRHS.mRange.Valid())
      {
         // if the range of the item being compared to is undefined, assume this is smaller and bail out
         return true;
      }
      if (mRange.mSource->GetFilePath() == aRHS.mRange.mSource->GetFilePath())
      {
         if (mRange.GetBegin() == aRHS.mRange.GetBegin())
         {
            return mRange.GetEnd() > aRHS.mRange.GetEnd();
         }
         else
         {
            return mRange.GetBegin() > aRHS.mRange.GetBegin();
         }
      }
      else
      {
         return mRange.mSource->GetFilePath() < aRHS.mRange.mSource->GetFilePath();
      }
   }

   //! @name Get access to the objects that this ChangeWrapper wraps
   //@{

   //! Get the wkf::AttachmentRoute that this ChangeWrapper wraps
   //! @return the wkf::AttachmentRoute that this ChangeWrapper wraps
   wkf::AttachmentRoute* GetRoute() const noexcept;

   //! Get the wkf::Waypoint that this ChangeWrapper wraps
   //! @return the wkf::Waypoint that this ChangeWrapper wraps
   wkf::Waypoint* GetWaypoint() const noexcept;

   //! Get the wkf::AttachmentZone that this ChangeWrapper wraps
   //! @return the wkf::AttachmentZone that this ChangeWrapper wraps
   wkf::AttachmentZone* GetZone() const noexcept;

   //! Get the wkf::ZonePoint that this ChangeWrapper wraps
   //! @return the wkf::ZonePoint that this ChangeWrapper wraps
   wkf::ZonePoint* GetZonePoint() const noexcept;

   //! Get the wizard::Platform that this ChangeWrapper wraps
   //! @return the wizard::Platform that this ChangeWrapper wraps
   wizard::Platform* GetPlatform() const noexcept;

   //! Get the wkf::PointOfInterest that this ChangeWrapper wraps
   //! @return the wkf::PointOfInterest that this ChangeWrapper wraps
   wkf::PointOfInterest* GetPointOfInterest() const noexcept;

   wizard::Editor*            GetEditor() const { return mEditorPtr; }
   const UtTextDocumentRange& GetRange() const { return mRange; }
   WsfPProxyNode              GetProxyNode() const { return mNode; }
   //@}

private:
   //! Sets the UtTextDocumentRange from the WsfPProxyNode
   //! @pre mRange is empty
   //! @post mRange is the UtTextDocumentRange from the WsfPProxyNode
   void SetRangeFromNode() noexcept;

   //! Sets the wizard::Editor from the UtTextDocumentRange
   //! @pre mEditorPtr is nullptr
   //! @post mEditorPtr is the wizard::Editor containing the UtTextDocumentRange
   void SetEditorFromRange() noexcept;

   wizard::Editor*       mEditorPtr{nullptr};
   UtTextDocumentRange   mRange;
   wizard::Platform*     mPlatformPtr{nullptr};
   wkf::PointOfInterest* mPointOfInterestPtr{nullptr};
   vespa::VaAttachment*  mAttachmentPtr{nullptr};
   wkf::Waypoint*        mWaypointPtr{nullptr};
   wkf::ZonePoint*       mZonePointPtr{nullptr};
   WsfPProxyNode         mNode;
};
} // namespace wizard

#endif
