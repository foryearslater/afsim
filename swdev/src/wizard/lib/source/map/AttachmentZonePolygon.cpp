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


#include "AttachmentZonePolygon.hpp"

#include "VaEntity.hpp"
#include "VaViewer.hpp"
#include "zone/WkfZonePoint.hpp"

wizard::AttachmentZonePolygon::AttachmentZonePolygon(vespa::VaEntity& aParent, vespa::VaViewer* aViewerPtr)
   : wkf::AttachmentZonePolygon(aParent, aViewerPtr)
{
}

bool wizard::AttachmentZonePolygon::Pick(vespa::VaViewer*                aViewerPtr,
                                         int                             aMouseX,
                                         int                             aMouseY,
                                         bool                            aToggle,
                                         bool                            aDoubleClick,
                                         const vespa::VaAttachmentSubId& aAttachmentSubPartId)
{
   if (!IsMyViewer(aViewerPtr))
   {
      return false;
   }
   bool selected = false;
   int  subpart;

   if (!GetZonePoints().empty())
   {
      auto zonePoints = GetZonePoints();
      if (aAttachmentSubPartId.mSubId == 0)
      {
         subpart = (*zonePoints.begin())->GetUniqueId();
      }
      else
      {
         subpart = aAttachmentSubPartId.mSubId;
      }

      if (aDoubleClick) // select all points if the user double-clicks the zone
      {
         if (!GetZonePointsLoaded())
         {
            LoadZonePoints();
         }
         for (auto& zonePointPtr : zonePoints)
         {
            zonePointPtr->SetVisible(true);
            zonePointPtr->SetSelected(aViewerPtr, true);
         }
         SetSelectionCleared(aViewerPtr->GetUniqueId(), false);
         selected = true;
      }
      else
      {
         ZonePoints::iterator iter = zonePoints.end();

         // If we have a valid zone point unique id, then find it.
         if (aAttachmentSubPartId.mSubId != 0)
         {
            iter = std::find_if(zonePoints.begin(), zonePoints.end(), wkf::ZonePoint::FindPred(subpart));
         }

         // If we found a zone point by unique id, use it. Otherwise, use the last zone point in the list.
         wkf::ZonePoint* zonePointPtr = (iter != zonePoints.end()) ? *iter : *zonePoints.rbegin();
         if (zonePointPtr != nullptr)
         {
            if (!GetZonePointsLoaded())
            {
               LoadZonePoints();
            }
            selected = aToggle ? !zonePointPtr->GetSelected(aViewerPtr) : true;
            zonePointPtr->SetVisible(selected);
            zonePointPtr->SetSelected(aViewerPtr, selected);
            SetSelectionCleared(aViewerPtr->GetUniqueId(), false);
            selected = true;
         }
      }

      SetHideZonePoints(!selected);
   }

   GetParent().SetSelected(aViewerPtr, selected);
   return selected;
}


std::string wizard::AttachmentZonePolygon::GetMenuSelectionString(const vespa::VaAttachmentSubId& aSubId) const
{
   std::size_t zoneNamePos = GetParent().GetName().find(GetName());
   std::string parentName  = GetParent().GetName().substr(0, zoneNamePos - 1);

   std::ostringstream oss;
   oss << parentName + " : " + GetName();
   return oss.str();
}


void wizard::AttachmentZonePolygon::MoveRelative(UtEntity&               aRefBefore,
                                                 UtEntity&               aRefAfter,
                                                 int                     aFeedbackFormat,
                                                 double                  aFeedbackPrecision,
                                                 vespa::MoveRelativeMask aMoveRelativeMask,
                                                 vespa::VaViewer*        aViewerPtr)
{
   if (!IsRelative())
   {
      wkf::AttachmentZonePolygon::MoveRelative(aRefBefore,
                                               aRefAfter,
                                               aFeedbackFormat,
                                               aFeedbackPrecision,
                                               aMoveRelativeMask,
                                               aViewerPtr);
   }
}
