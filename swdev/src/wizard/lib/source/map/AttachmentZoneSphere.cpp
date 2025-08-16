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

#include "AttachmentZoneSphere.hpp"

#include "VaEntity.hpp"
#include "VaViewer.hpp"

wizard::AttachmentZoneSphere::AttachmentZoneSphere(vespa::VaEntity& aParent, vespa::VaViewer* aViewerPtr)
   : wkf::AttachmentZoneSphere(aParent, aViewerPtr)
{
}

bool wizard::AttachmentZoneSphere::Pick(vespa::VaViewer*                aViewerPtr,
                                        int                             aMouseX,
                                        int                             aMouseY,
                                        bool                            aToggle,
                                        bool                            aDoubleClick,
                                        const vespa::VaAttachmentSubId& aAttachmentSubPartId)
{
   bool selected =
      wkf::AttachmentZoneSphere::Pick(aViewerPtr, aMouseX, aMouseY, aToggle, aDoubleClick, aAttachmentSubPartId);
   GetParent().SetSelected(aViewerPtr, selected);
   return selected;
}

std::string wizard::AttachmentZoneSphere::GetMenuSelectionString(const vespa::VaAttachmentSubId& aSubId) const
{
   std::size_t zoneNamePos = GetParent().GetName().find(GetName());
   std::string parentName  = GetParent().GetName().substr(0, zoneNamePos - 1);

   std::ostringstream oss;
   oss << parentName + " : " + GetName();
   return oss.str();
}

void wizard::AttachmentZoneSphere::MoveRelative(UtEntity&               aRefBefore,
                                                UtEntity&               aRefAfter,
                                                int                     aFeedbackFormat,
                                                double                  aFeedbackPrecision,
                                                vespa::MoveRelativeMask aMoveRelativeMask,
                                                vespa::VaViewer*        aViewerPtr)
{
   if (!IsRelative())
   {
      wkf::AttachmentZoneSphere::MoveRelative(aRefBefore,
                                              aRefAfter,
                                              aFeedbackFormat,
                                              aFeedbackPrecision,
                                              aMoveRelativeMask,
                                              aViewerPtr);
   }
}
