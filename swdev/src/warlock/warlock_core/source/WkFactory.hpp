// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2020 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#ifndef WKFACTORY_HPP
#define WKFACTORY_HPP

#include "warlock_core_export.h"

#include "WkfFactory.hpp"

namespace warlock
{
class WARLOCK_CORE_EXPORT Factory : public wkf::Factory
{
public:
   Factory() = default;

protected:
#undef CreateDialog
   wkf::Dialog* CreateDialog(const std::string& aDialogType, wkf::MainWindow* aParent = nullptr) const override;
   QDockWidget* CreateDockWidget(const std::string& aDockWidgetType, QMainWindow* aParent = nullptr) const override;
   QWidget*     CreateWidget(const std::string& aWidgetType, QMainWindow* aParent = nullptr) const override;
   void         UserOpenFile(const QStringList& aFileNames) const override;
   vespa::VaScenario* UserCreateScenario(const std::string& aScenarioType) const override;
   vespa::VaOverlay* UserCreateOverlay(const std::string& aOverlayType, const std::string& aOverlayName = "") const override;
};
} // namespace warlock
#endif
