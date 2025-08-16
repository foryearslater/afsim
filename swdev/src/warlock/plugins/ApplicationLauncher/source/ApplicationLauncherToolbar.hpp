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
#ifndef ApplicationLauncherToolbar_HPP
#define ApplicationLauncherToolbar_HPP

#include <QToolBar>

#include "ApplicationLauncherSimInterface.hpp"
#include "UtPath.hpp"

namespace wkf
{
class Action;
}

namespace WkApplicationLauncher
{
class Toolbar : public QToolBar
{
   Q_OBJECT
public:
   Toolbar(SimInterface* aSimInterface);
   ~Toolbar() override = default;

private:
   void HandleWizardAction();
   void HandleMysticAction();

   static bool IsExecutable(const UtPath& aExePath);
   void        StartApplication(const QString& aExecutableName, const QStringList& aArgumentList);

   wkf::Action*  mWizardActionPtr;
   wkf::Action*  mMysticActionPtr;
   SimInterface* mSimInterface;
};
} // namespace WkApplicationLauncher
#endif
