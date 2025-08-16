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

#include "CommVisCommGraphicsItem.hpp"

#include <QGraphicsScene>
#include <QGraphicsView>

#include "ProxyWatcher.hpp"
#include "UtQtMemory.hpp"
#include "WsfPM_Root.hpp"

WizCommVis::CommVisCommGraphicsItem::CommVisCommGraphicsItem(const QString& aPlatformName,
                                                             const QString& aCommName,
                                                             const QString& aCommAddress,
                                                             const QString& aCommType)
   : wkf::CommVisCommGraphicsItem(aCommName, aCommAddress, aCommType)
{
   WsfPM_Root     proxyRoot(wizard::ProxyWatcher::GetActiveProxy());
   WsfPM_Platform platformNode = proxyRoot.platforms() + aPlatformName.toStdString();
   WsfPM_Comm     curComm      = platformNode.Comms() + aCommName.toStdString();
   if (curComm.IsValid())
   {
      mCommProxyNode = curComm;
   }
}

void WizCommVis::CommVisCommGraphicsItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* aEvent)
{
   ToggleSelected();

   QWidget* parentPtr = nullptr;
   if (scene() && !scene()->views().empty())
   {
      parentPtr = scene()->views()[0]->parentWidget();
   }

   if (!mDialogWindowPtr)
   {
      mDialogWindowPtr = QPointer<WizCommVis::CommVisCommNodeDialog>(
         ut::qt::make_qt_ptr<WizCommVis::CommVisCommNodeDialog>(mCommProxyNode, this, parentPtr));
   }

   mDialogWindowPtr->show();
}
