// ****************************************************************************
// CUI//REL TO USA ONLY
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2018 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#include "EngagePlugin.hpp"

#include <QMenu>

#include "UtQtMemory.hpp"
#include "WkfAction.hpp"
#include "WkfMainWindow.hpp"

WKF_PLUGIN_DEFINE_SYMBOLS(Engage::Plugin, "Engage", "A tool for creating generic engage scenarios", "wizard")

Engage::Plugin::Plugin(const QString& aPluginName, const size_t aUniqueId)
   : wizard::Plugin(aPluginName, aUniqueId)
   , mDialogPtr(nullptr)
{
   mStartAction = new wkf::Action("Engage tool", wkfEnv.GetMainWindow());
   connect(mStartAction, &QAction::triggered, [this]() { Start(); });

   QMenu* tmenu = wkfEnv.GetMainWindow()->GetMenuByName("Tools");
   if (tmenu)
   {
      tmenu->addAction(mStartAction);
   }
}

QList<wkf::Action*> Engage::Plugin::GetActions() const
{
   QList<wkf::Action*> alist;
   alist << mStartAction; // << mPK_Action;
   return alist;
}

void Engage::Plugin::Start()
{
   if (!mDialogPtr)
   {
      mDialogPtr = ut::qt::make_qt_ptr<Dialog>(wkfEnv.GetMainWindow());
   }
   mDialogPtr->show();
   mDialogPtr->raise();
}
