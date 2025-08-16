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

#ifndef ATTACHMENTZONEELLIPSE_HPP
#define ATTACHMENTZONEELLIPSE_HPP

#include "ViExport.hpp"
#include "zone/WkfAttachmentZoneEllipse.hpp"

namespace vespa
{
class VaEntity;
class VaViewer;
} // namespace vespa

namespace wizard
{
class VI_EXPORT AttachmentZoneEllipse : public wkf::AttachmentZoneEllipse
{
public:
   AttachmentZoneEllipse(vespa::VaEntity& aParent, vespa::VaViewer* aViewerPtr);

   ~AttachmentZoneEllipse() = default;

   unsigned int GetPickMode() const override { return cPICK_ATTACHMENT; }
   bool         IsHighlightable() override { return true; }

   bool Highlight(vespa::VaViewer*                aViewerPtr,
                  const vespa::VaAttachmentSubId& aSubpartId,
                  bool                            aHighlight,
                  const UtColor&                  aColor) override
   {
      return false;
   }

   void SetRelative(bool aRelative) { mRelative = aRelative; }
   bool IsRelative() const { return mRelative; }

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

private:
   bool mRelative = false;
};
} // namespace wizard

#endif
