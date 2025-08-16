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


#include "CommVisGatewayGraphicsItem.hpp"

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QPointer>

#include "CommVisGatewayNodeDialog.hpp"
#include "ProxyWatcher.hpp"
#include "UtQtMemory.hpp"
#include "WsfPM_Root.hpp"

WizCommVis::CommVisGatewayGraphicsItem::CommVisGatewayGraphicsItem(const wkf::CommEvent::GatewayInfo& aGatewayInfo)
   : wkf::CommVisGatewayGraphicsItem(aGatewayInfo)
{
   WsfPM_Root     proxyRoot(wizard::ProxyWatcher::GetActiveProxy());
   WsfPM_Platform platformNode = proxyRoot.platforms() + aGatewayInfo.mPlatformName;
   WsfPM_Comm     curComm      = platformNode.Comms() + aGatewayInfo.mGatewayName;
   if (curComm.IsValid())
   {
      mCommProxyNode = curComm;
   }
}


void WizCommVis::CommVisGatewayGraphicsItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* aEvent)
{
   ToggleSelected();
   QWidget* parentPtr = nullptr;
   if (scene() && !scene()->views().empty())
   {
      parentPtr = scene()->views()[0]->parentWidget();
   }

   if (mDialogWindowPtr == nullptr)
   {
      mDialogWindowPtr = QPointer<WizCommVis::CommVisGatewayNodeDialog>(
         ut::qt::make_qt_ptr<WizCommVis::CommVisGatewayNodeDialog>(mCommProxyNode, this, parentPtr));
   }

   mDialogWindowPtr->show();
}
