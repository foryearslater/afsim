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
#ifndef PLATFORMDATADOCKWIDGET_HPP
#define PLATFORMDATADOCKWIDGET_HPP

#include "WkfPlatformDataDockWidget.hpp"

namespace wizard
{
class PlatformDataDockWidget : public wkf::PlatformDataDockWidget
{
public:
   PlatformDataDockWidget(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

   void ItemChanged(const wkf::ChangeInfo& aInfo) override;
};
} // namespace wizard
#endif
