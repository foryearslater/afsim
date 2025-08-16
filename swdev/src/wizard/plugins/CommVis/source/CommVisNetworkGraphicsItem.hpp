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

#ifndef COMMVISNETWORKGRAPHICSITEM_HPP
#define COMMVISNETWORKGRAPHICSITEM_HPP

#include <QPointer>

#include "CommVisNetworkNodeDialog.hpp"
#include "WsfPProxyNode.hpp"
#include "comm_vis/WkfCommVisNetworkGraphicsItem.hpp"

namespace WizCommVis
{
class CommVisNetworkGraphicsItem : public wkf::CommVisNetworkGraphicsItem
{
public:
   CommVisNetworkGraphicsItem(const QString& aNetworkName, const QString& aNetworkAddress, const QString& aNetworkType);

   ~CommVisNetworkGraphicsItem() = default;

   void UpdateValues(const QString& aNetworkName, const QString& aNetworkAddress, const QString& aNetworkType);
   void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* aEvent) override;

private:
   WsfPProxyNode                                  mNetworkProxyNode;
   QPointer<WizCommVis::CommVisNetworkNodeDialog> mDialogWindowPtr;
};
} // namespace WizCommVis

#endif
