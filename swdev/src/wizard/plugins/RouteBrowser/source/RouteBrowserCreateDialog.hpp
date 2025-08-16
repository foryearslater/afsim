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

#ifndef ROUTEBROWSERCREATEDIALOG_HPP
#define ROUTEBROWSERCREATEDIALOG_HPP

#include "route/WkfRouteBrowserCreateDialog.hpp"

namespace RouteBrowser
{
class RouteBrowserCreateDialog : public wkf::RouteBrowserCreateDialog
{
   Q_OBJECT
public:
   explicit RouteBrowserCreateDialog(QWidget* aParent = nullptr, Qt::WindowFlags aFlags = Qt::WindowFlags());
   ~RouteBrowserCreateDialog() = default;

   void PopulateDialog() override;
   void AppendWaypoint(double aLatitude, double aLongitude, UtLengthValue aAltitude);
   //! Do not convert the altitude in Wizard
   //! @return the altitude UNCHANGED
   UtLengthValue GetUI_Altitude(const UtLengthValue& aAlt) const override { return aAlt; }

   void hideEvent(QHideEvent* aEvent) override;
   void showEvent(QShowEvent* aEvent) override;

signals:
   void TempRouteEdited(const wkf::RouteBrowserInterface::RouteInfo& aRouteInfo, wkf::AttachmentRoute* aRouteAttachmentPtr);

private:
   void MoveSelectionAndAttachmentComplete(const wkf::PlatformList&       aPlatformList,
                                           const wkf::EntityList&         aNonPlatformList,
                                           const vespa::VaAttachmentList& aAttachmentList);
};
} // namespace RouteBrowser
#endif
