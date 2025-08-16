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


#include "CommVisRouterGraphicsItem.hpp"

#include <QGraphicsScene>
#include <QGraphicsView>

#include "CommVisRouterNodeDialog.hpp"
#include "ProxyWatcher.hpp"
#include "UtQtMemory.hpp"
#include "WsfPM_Root.hpp"

WizCommVis::CommVisRouterGraphicsItem::CommVisRouterGraphicsItem(const wkf::CommEvent::RouterInfo& aRouterInfo)
   : wkf::CommVisRouterGraphicsItem(aRouterInfo)
{
   WsfPM_Root     proxyRoot(wizard::ProxyWatcher::GetActiveProxy());
   WsfPM_Platform platformNode = proxyRoot.platforms() + aRouterInfo.mPlatformName;
   mRouterProxyNode            = platformNode.Routers() + aRouterInfo.mRouterName;
}

void WizCommVis::CommVisRouterGraphicsItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* aEvent)
{
   ToggleSelected();

   QWidget* parentPtr = nullptr;
   if (scene() && !scene()->views().empty())
   {
      parentPtr = scene()->views()[0]->parentWidget();
   }

   if (mDialogWindowPtr == nullptr)
   {
      mDialogWindowPtr = QPointer<WizCommVis::CommVisRouterNodeDialog>(
         ut::qt::make_qt_ptr<WizCommVis::CommVisRouterNodeDialog>(mRouterProxyNode, this, parentPtr));
   }

   mDialogWindowPtr->show();
}
