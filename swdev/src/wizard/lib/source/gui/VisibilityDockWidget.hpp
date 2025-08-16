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
#ifndef VISIBILITYDOCKWIDGET_HPP
#define VISIBILITYDOCKWIDGET_HPP

#include "visibility/WkfVisibilityDockWidget.hpp"

namespace wizard
{
class VisibilityDockWidget : public wkf::VisibilityDockWidget
{
public:
   VisibilityDockWidget(QWidget* aParent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
};
} // namespace wizard
#endif
