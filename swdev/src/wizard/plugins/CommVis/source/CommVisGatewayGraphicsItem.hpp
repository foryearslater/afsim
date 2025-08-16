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

#ifndef COMMVISGATEWAYGRAPHICSITEM_HPP
#define COMMVISGATEWAYGRAPHICSITEM_HPP

#include <QPointer>

#include "CommVisGatewayNodeDialog.hpp"
#include "comm_vis/WkfCommVisGatewayGraphicsItem.hpp"

namespace WizCommVis
{
class CommVisGatewayGraphicsItem : public wkf::CommVisGatewayGraphicsItem
{
public:
   CommVisGatewayGraphicsItem(const wkf::CommEvent::GatewayInfo& aGatewayInfo);
   ~CommVisGatewayGraphicsItem() = default;

   void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* aEvent) override;

private:
   QPointer<WizCommVis::CommVisGatewayNodeDialog> mDialogWindowPtr;
   WsfPM_Comm                                     mCommProxyNode;
};
} // namespace WizCommVis

#endif
