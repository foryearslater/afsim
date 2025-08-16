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


#include "AttachmentZoneCircle.hpp"

#include "VaEntity.hpp"
#include "VaViewer.hpp"

wizard::AttachmentZoneCircle::AttachmentZoneCircle(vespa::VaEntity& aParent, vespa::VaViewer* aViewerPtr)
   : wkf::AttachmentZoneCircle(aParent, aViewerPtr)
{
}

bool wizard::AttachmentZoneCircle::Pick(vespa::VaViewer*                aViewerPtr,
                                        int                             aMouseX,
                                        int                             aMouseY,
                                        bool                            aToggle,
                                        bool                            aDoubleClick,
                                        const vespa::VaAttachmentSubId& aAttachmentSubPartId)
{
   if (wkf::AttachmentZoneCircle::Pick(aViewerPtr, aMouseX, aMouseY, aToggle, aDoubleClick, aAttachmentSubPartId))
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


std::string wizard::AttachmentZoneCircle::GetMenuSelectionString(const vespa::VaAttachmentSubId& aSubId) const
{
   std::size_t zoneNamePos = GetParent().GetName().find(GetName());
   std::string parentName  = GetParent().GetName().substr(0, zoneNamePos - 1);

   std::ostringstream oss;
   oss << parentName + " : " + GetName();
   return oss.str();
}


void wizard::AttachmentZoneCircle::MoveRelative(UtEntity&               aRefBefore,
                                                UtEntity&               aRefAfter,
                                                int                     aFeedbackFormat,
                                                double                  aFeedbackPrecision,
                                                vespa::MoveRelativeMask aMoveRelativeMask,
                                                vespa::VaViewer*        aViewerPtr)
{
   if (!IsRelative())
   {
      wkf::AttachmentZoneCircle::MoveRelative(aRefBefore,
                                              aRefAfter,
                                              aFeedbackFormat,
                                              aFeedbackPrecision,
                                              aMoveRelativeMask,
                                              aViewerPtr);
   }
}
