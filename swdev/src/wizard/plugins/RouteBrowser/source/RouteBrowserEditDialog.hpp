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

#ifndef ROUTEBROWSEREDITDIALOG_HPP
#define ROUTEBROWSEREDITDIALOG_HPP

#include "route/WkfRouteBrowserEditDialog.hpp"

namespace RouteBrowser
{
class RouteBrowserEditDialog : public wkf::RouteBrowserEditDialog
{
public:
   explicit RouteBrowserEditDialog(QWidget* aParent = nullptr, Qt::WindowFlags aFlags = Qt::WindowFlags());
   ~RouteBrowserEditDialog() = default;

   //! Do not convert the altitude in Wizard
   //! @return the altitude UNCHANGED
   UtLengthValue GetUI_Altitude(const UtLengthValue& aAlt) const override { return aAlt; }
   //! Do nothing in Wizard
   void MoveSelectionAndAttachmentComplete(const wkf::PlatformList&       aPlatformList,
                                           const wkf::EntityList&         aNonPlatformList,
                                           const vespa::VaAttachmentList& aAttachmentList) override
   {
      return;
   }

   void hideEvent(QHideEvent* aEvent) override;
   void showEvent(QShowEvent* aEvent) override;
};
} // namespace RouteBrowser
#endif
