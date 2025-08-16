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


#include "AttachmentZoneEllipse.hpp"

#include "VaEntity.hpp"
#include "VaViewer.hpp"

wizard::AttachmentZoneEllipse::AttachmentZoneEllipse(vespa::VaEntity& aParent, vespa::VaViewer* aViewerPtr)
   : wkf::AttachmentZoneEllipse(aParent, aViewerPtr)
{
}


bool wizard::AttachmentZoneEllipse::Pick(vespa::VaViewer*                aViewerPtr,
                                         int                             aMouseX,
                                         int                             aMouseY,
                                         bool                            aToggle,
                                         bool                            aDoubleClick,
                                         const vespa::VaAttachmentSubId& aAttachmentSubPartId)
{
   if (wkf::AttachmentZoneEllipse::Pick(aViewerPtr, aMouseX, aMouseY, aToggle, aDoubleClick, aAttachmentSubPartId))
   {
      GetParent().SetSelected(aViewerPtr, true);
      return true;
   }
   else
   {
      GetParent().SetSelected(aViewerPtr, false);
      return false;
   }
}

std::string wizard::AttachmentZoneEllipse::GetMenuSelectionString(const vespa::VaAttachmentSubId& aSubId) const
{
   std::size_t zoneNamePos = GetParent().GetName().find(GetName());
   std::string parentName  = GetParent().GetName().substr(0, zoneNamePos - 1);

   std::ostringstream oss;
   oss << parentName + " : " + GetName();
   return oss.str();
}


void wizard::AttachmentZoneEllipse::MoveRelative(UtEntity&               aRefBefore,
                                                 UtEntity&               aRefAfter,
                                                 int                     aFeedbackFormat,
                                                 double                  aFeedbackPrecision,
                                                 vespa::MoveRelativeMask aMoveRelativeMask,
                                                 vespa::VaViewer*        aViewerPtr)
{
   if (!IsRelative())
   {
      wkf::AttachmentZoneEllipse::MoveRelative(aRefBefore,
                                               aRefAfter,
                                               aFeedbackFormat,
                                               aFeedbackPrecision,
                                               aMoveRelativeMask,
                                               aViewerPtr);
   }
}
