// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2018 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "CreatePlatform.hpp"

#include "CreatePlatformDialog.hpp"
#include "WsfPM_Root.hpp"

wizard::CreatePlatform::CreatePlatform(const WsfPProxyNode& aNode, QWidget* aParentWidget, QObject* aParent)
   : QAction("Create Platform...", aParent)
   , mNode(aNode)
   , mParentWidgetPtr(aParentWidget)
{
   connect(this, &CreatePlatform::triggered, this, &CreatePlatform::OnTriggered);
}

void wizard::CreatePlatform::OnTriggered()
{
   CreatePlatformDialog* createPlatDlg = new CreatePlatformDialog(mNode.GetProxy(), mParentWidgetPtr);
   if (mNode.GetParent() == WsfPM_Root(mNode.Root()).platformTypes())
   {
      createPlatDlg->SelectPlatformType(mNode.GetName());
   }
   createPlatDlg->show();
}
