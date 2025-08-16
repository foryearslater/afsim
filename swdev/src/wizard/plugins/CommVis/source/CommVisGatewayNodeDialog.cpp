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

#include "CommVisGatewayNodeDialog.hpp"

#include <QTextBlock>
#include <QTextDocumentFragment>

#include "CommVisGatewayGraphicsItem.hpp"
#include "Editor.hpp"
#include "EditorPrefObject.hpp"
#include "Project.hpp"
#include "WkfEnvironment.hpp"

WizCommVis::CommVisGatewayNodeDialog::CommVisGatewayNodeDialog(WsfPM_Comm                       aCommProxyNode,
                                                               wkf::CommVisGatewayGraphicsItem* aGatewayObjPtr,
                                                               QWidget*                         aParent,
                                                               Qt::WindowFlags                  aFlags)
   : wkf::CommVisGatewayNodeDialog(aGatewayObjPtr, aParent, aFlags)
   , mCommProxyNode(aCommProxyNode)
{
   Initialize();
}

void WizCommVis::CommVisGatewayNodeDialog::Initialize()
{
   if (mGatewayObjPtr != nullptr)
   {
      mUI.mPlatformNameLineEdit->setText(mGatewayObjPtr->GetPlatformName());
      mUI.mPlatformNameLineEdit->setReadOnly(false);
      mUI.mGatewayNameLineEdit->setText(mGatewayObjPtr->GetName());
      mUI.mGatewayNameLineEdit->setReadOnly(false);
      mUI.mGatewayAddressLineEdit->setText(mGatewayObjPtr->GetAddress());
      mUI.mGatewayAddressLineEdit->setReadOnly(false);
      mUI.mGatewayRemotePlatformLineEdit->setText(mGatewayObjPtr->GetRemotePlatform());
      mUI.mGatewayRemotePlatformLineEdit->setReadOnly(false);
      mUI.mGatewayRemoteCommLineEdit->setText(mGatewayObjPtr->GetRemoteComm());
      mUI.mGatewayRemoteCommLineEdit->setReadOnly(false);
      mUI.mGatewayRemoteAddressLineEdit->setText(mGatewayObjPtr->GetGatewayRemoteAddress());
      mUI.mGatewayRemoteAddressLineEdit->setReadOnly(false);

      // Connections
      connect(mUI.buttonBox, &QDialogButtonBox::accepted, [this]() { UpdateRemoteInterfaceAttributes(); });
   }
}


void WizCommVis::CommVisGatewayNodeDialog::UpdateRemoteInterfaceAttributes()
{
   QString platformStr;
   QString commStr;

   // Make sure both gateway fields have text in them
   if (!mUI.mGatewayRemoteCommLineEdit->text().isEmpty() && !mUI.mGatewayRemotePlatformLineEdit->text().isEmpty())
   {
      platformStr = mUI.mGatewayRemotePlatformLineEdit->text();
      commStr     = mUI.mGatewayRemoteCommLineEdit->text();
   }

   if (!platformStr.isEmpty() && !commStr.isEmpty())
   {
      if (mCommProxyNode.IsValid())
      {
         WsfPProxyNode gatewayNode = mCommProxyNode + "gatewayPlatform";
         if (gatewayNode.IsValid())
         {
            UtTextDocumentRange docRange = gatewayNode.GetSourceDocumentRange();
            if (docRange.Valid())
            {
               EditAttribute("gateway " + platformStr + " " + commStr, docRange);
               mGatewayObjPtr->SetRemoteInterface(platformStr, commStr);
            }
            else
            {
               // This attribute isn't part of the definition yet, add it
               AddNewAttribute("gateway " + platformStr + " " + commStr, mCommProxyNode);
            }
         }
      }
   }
}
