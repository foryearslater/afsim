// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2016-2017 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef PLATFORMPARTBROWSERDOCKWIDGET_HPP
#define PLATFORMPARTBROWSERDOCKWIDGET_HPP

#include <QDockWidget>

#include "PlatformPartBrowserDataContainer.hpp"
#include "PlatformPartBrowserSimInterface.hpp"
#include "ui_PlatformPartBrowserDockWidget.h"

namespace wkf
{
class Platform;
};

namespace WkPlatformPartBrowser
{
class DockWidget : public QDockWidget
{
   Q_OBJECT

public:
   DockWidget(SimInterface*   aSimInterface,
              DataContainer&  aData,
              QWidget*        aParent = nullptr,
              Qt::WindowFlags f       = Qt::WindowFlags());
   ~DockWidget() override = default;

private:
   void HandleCheckBoxChanged(const std::string& aPartName, PropertyName aPropertyName, bool aChecked);

   void PlatformOfInterestChanged(wkf::Platform* aPlatform);

   void Populate(const QString& aPlatformName, const PlatformPartList& aPartList);

   void ResizeColumns();

   void showEvent(QShowEvent* aEvent) override;
   void closeEvent(QCloseEvent* aEvent) override;

   PlatformPartList mPlatformParts;
   SimInterface*    mSimInterface;
   DataContainer&   mData;

   Ui::PlatformPartBrowserDockWidget mUi;
};
} // namespace WkPlatformPartBrowser

#endif // !PLATFORMPARTBROWSERDOCKWIDGET_HPP
