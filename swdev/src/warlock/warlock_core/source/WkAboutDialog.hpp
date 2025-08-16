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

#ifndef WKABOUTDIALOG_HPP
#define WKABOUTDIALOG_HPP

#include "WkfAboutDialog.hpp"

namespace warlock
{
class AboutDialog : public wkf::AboutDialog
{
public:
   AboutDialog(wkf::MainWindow* parent, Qt::WindowFlags f = Qt::WindowFlags());
};
} // namespace warlock
#endif