// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2016 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "ViewTypeSummaryAction.hpp"

#include "EditAssist.hpp"
#include "Project.hpp"

wizard::ViewTypeSummaryAction::ViewTypeSummaryAction(const WsfPProxyNode& aNode, QObject* aParent)
   : QAction("View Type Info...", aParent)
   , mNode(aNode)
{
   mPos = QCursor::pos();
   connect(this, &ViewTypeSummaryAction::triggered, this, &ViewTypeSummaryAction::OnTriggered);
}

void wizard::ViewTypeSummaryAction::OnTriggered()
{
   Project* proj = Project::Instance();
   if (!proj || !proj->GetParseResults())
      return;
   wizard::EditAssist_Type2* typeAssist = new EditAssist_Type2();
   typeAssist->SetAssistantData(mPos);
   typeAssist->SetTypeData(*proj->GetParseResults(), mNode.GetPath());
   typeAssist->show();
}
