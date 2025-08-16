// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2021 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef COMMVISVIEWLINKSDIALOG_HPP
#define COMMVISVIEWLINKSDIALOG_HPP

#include "comm_vis/WkfCommVisViewLinksDialog.hpp"
namespace WizCommVis
{
class CommVisViewLinksDialog : public wkf::CommVisViewLinksDialog
{
   Q_OBJECT

public:
   explicit CommVisViewLinksDialog(const std::vector<wkf::CommEvent::LinkInfo>& aLinkInfo,
                                   QWidget*                                     aParent = nullptr,
                                   Qt::WindowFlags                              aFlags  = Qt::WindowFlags());
   ~CommVisViewLinksDialog() override = default;

private:
   void AddRow();
   void RemoveRow();
};
} // namespace WizCommVis

#endif
