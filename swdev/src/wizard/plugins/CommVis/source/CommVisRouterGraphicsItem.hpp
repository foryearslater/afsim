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

#ifndef COMMVISROUTERGRAPHICSITEM_HPP
#define COMMVISROUTERGRAPHICSITEM_HPP

#include <QPointer>

#include "CommVisRouterNodeDialog.hpp"
#include "WsfPM_Router.hpp"
#include "comm_vis/WkfCommVisDataContainer.hpp"
#include "comm_vis/WkfCommVisRouterGraphicsItem.hpp"

namespace WizCommVis
{
class CommVisRouterGraphicsItem : public wkf::CommVisRouterGraphicsItem
{
public:
   CommVisRouterGraphicsItem(const wkf::CommEvent::RouterInfo& aRouterInfo);
   ~CommVisRouterGraphicsItem() = default;

   void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* aEvent) override;

private:
   WsfPM_Router                                  mRouterProxyNode;
   QPointer<WizCommVis::CommVisRouterNodeDialog> mDialogWindowPtr;
};
} // namespace WizCommVis

#endif
