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

#ifndef ATTACHMENTZONEPOLYGON_HPP
#define ATTACHMENTZONEPOLYGON_HPP

#include "ViExport.hpp"
#include "zone/WkfAttachmentZonePolygon.hpp"

namespace vespa
{
class VaEntity;
class VaViewer;
} // namespace vespa

namespace wizard
{
class VI_EXPORT AttachmentZonePolygon : public wkf::AttachmentZonePolygon
{
public:
   AttachmentZonePolygon(vespa::VaEntity& aParent, vespa::VaViewer* aViewerPtr);

   ~AttachmentZonePolygon() = default;

   unsigned int GetPickMode() const override { return cPICK_ATTACHMENT; }
   bool         IsHighlightable() override { return true; }

   bool Highlight(vespa::VaViewer*                aViewerPtr,
                  const vespa::VaAttachmentSubId& aSubpartId,
                  bool                            aHighlight,
                  const UtColor&                  aColor) override
   {
      return false;
   }

   bool Pick(vespa::VaViewer*                aViewerPtr,
             int                             aMouseX,
             int                             aMouseY,
             bool                            aToggle,
             bool                            aDoubleClick,
             const vespa::VaAttachmentSubId& aAttachmentSubPartId) override;

   void MoveRelative(UtEntity&               aRefBefore,
                     UtEntity&               aRefAfter,
                     int                     aFeedbackFormat,
                     double                  aFeedbackPrecision,
                     vespa::MoveRelativeMask aMoveRelativeMask,
                     vespa::VaViewer*        aViewerPtr) override;

   std::string GetMenuSelectionString(const vespa::VaAttachmentSubId& aSubId) const override;
};
} // namespace wizard

#endif
