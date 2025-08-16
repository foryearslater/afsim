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

#ifndef ATTACHMENTZONESPHERE_HPP
#define ATTACHMENTZONESPHERE_HPP

#include "ViExport.hpp"
#include "zone/WkfAttachmentZoneSphere.hpp"

namespace vespa
{
class VaEntity;
class VaViewer;
} // namespace vespa

namespace wizard
{
class VI_EXPORT AttachmentZoneSphere : public wkf::AttachmentZoneSphere
{
public:
   AttachmentZoneSphere(vespa::VaEntity& aParent, vespa::VaViewer* aViewerPtr);

   ~AttachmentZoneSphere() = default;

   unsigned int GetPickMode() const override { return cPICK_ATTACHMENT; }

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
