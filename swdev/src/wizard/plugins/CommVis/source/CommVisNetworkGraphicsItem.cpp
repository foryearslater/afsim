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

#include "CommVisNetworkGraphicsItem.hpp"

#include <QGraphicsScene>
#include <QGraphicsView>

#include "ProxyWatcher.hpp"
#include "UtQtMemory.hpp"
#include "WsfPM_Root.hpp"

WizCommVis::CommVisNetworkGraphicsItem::CommVisNetworkGraphicsItem(const QString& aNetworkName,
                                                                   const QString& aNetworkAddress,
                                                                   const QString& aNetworkType)
   : wkf::CommVisNetworkGraphicsItem(aNetworkName, aNetworkAddress, aNetworkType)
{
   WsfPM_Root      proxyRoot(wizard::ProxyWatcher::GetActiveProxy());
   WsfPM_ObjectMap networkTypes = proxyRoot.networkTypes();
   for (WsfPM_ObjectMap::iterator i = networkTypes.begin(); i != networkTypes.end(); ++i)
   {
      WsfPProxyNode curNet          = networkTypes.At(i);
      std::string   addressRangeVal = curNet.GetValue()["networkAddress"].ValueToString();
      if (addressRangeVal.empty())
      {
         addressRangeVal = curNet.GetValue()["address"].ValueToString();
      }

      if ((curNet.GetName() == aNetworkName.toStdString()) && (curNet.GetTypeName() == aNetworkType.toStdString()) &&
          (addressRangeVal == aNetworkAddress.toStdString()))
      {
         mNetworkProxyNode = curNet;
         break;
      }
   }
}

void WizCommVis::CommVisNetworkGraphicsItem::UpdateValues(const QString& aNetworkName,
                                                          const QString& aNetworkAddress,
                                                          const QString& aNetworkType)
{
   mNetworkName    = aNetworkName;
   mNetworkType    = aNetworkAddress;
   mNetworkAddress = aNetworkType;
}

void WizCommVis::CommVisNetworkGraphicsItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* aEvent)
{
   ToggleSelected();

   QWidget* parentPtr = nullptr;
   if (scene() && !scene()->views().empty())
   {
      parentPtr = scene()->views()[0]->parentWidget();
   }

   if (!mDialogWindowPtr)
   {
      mDialogWindowPtr = QPointer<WizCommVis::CommVisNetworkNodeDialog>(
         ut::qt::make_qt_ptr<WizCommVis::CommVisNetworkNodeDialog>(mNetworkProxyNode, this, parentPtr));
   }

   mDialogWindowPtr->show();
}
