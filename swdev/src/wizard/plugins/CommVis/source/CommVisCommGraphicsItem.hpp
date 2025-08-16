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

#ifndef COMMVISCOMMGRAPHICSITEM_HPP
#define COMMVISCOMMGRAPHICSITEM_HPP

#include <QPointer>

#include "CommVisCommNodeDialog.hpp"
#include "WsfPProxyNode.hpp"
#include "comm_vis/WkfCommVisCommGraphicsItem.hpp"

namespace WizCommVis
{
class CommVisCommGraphicsItem : public wkf::CommVisCommGraphicsItem
{
public:
   CommVisCommGraphicsItem(const QString& aPlatformName,
                           const QString& aCommName,
                           const QString& aCommAddress,
                           const QString& aCommType);

   ~CommVisCommGraphicsItem() = default;

   void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* aEvent) override;

private:
   WsfPProxyNode                               mCommProxyNode;
   QPointer<WizCommVis::CommVisCommNodeDialog> mDialogWindowPtr;
};
} // namespace WizCommVis

#endif
