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

#ifndef CREATEPLATFORMDOCKWIDGET_HPP
#define CREATEPLATFORMDOCKWIDGET_HPP

#include <QDockWidget>

#include "CreatePlatformDataContainer.hpp"
#include "CreatePlatformSimInterface.hpp"
#include "ui_CreatePlatformDockWidget.h"

namespace WkCreatePlatform
{
class DockWidget : public QDockWidget
{
public:
   DockWidget(SimInterface*   aInterfacePtr,
              DataContainer&  aCreatePlatformData,
              QWidget*        parent = nullptr,
              Qt::WindowFlags f      = Qt::WindowFlags());

private:
   void CreatePlatform();
   void UpdateTypeComboBox(const std::vector<std::string>& aTypeList);
   void LocationSelected(double aLatitude, double aLongitude, double aAltitude, bool aAltitudeValid);
   void LocationChanged(double aLatitude, double aLongitude);

   Ui::CreatePlatformDockWidget mUI;
   DataContainer&               mCreatePlatformData;
   SimInterface*                mSimInterfacePtr;
};
} // namespace WkCreatePlatform

#endif
