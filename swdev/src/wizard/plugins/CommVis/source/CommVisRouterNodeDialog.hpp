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

#ifndef COMMVISROUTERNODEDIALOG_HPP
#define COMMVISROUTERNODEDIALOG_HPP

#include "CommVisEditDialog.hpp"
#include "WsfPM_Platform.hpp"
#include "WsfPProxyNode.hpp"
#include "comm_vis/WkfCommVisRouterNodeDialog.hpp"
class UtTextDocumentRange;

namespace WizCommVis
{
class CommVisRouterNodeDialog : public wkf::CommVisRouterNodeDialog, public CommVisEditDialog
{
   Q_OBJECT

public:
   CommVisRouterNodeDialog(WsfPM_Router                    aRouterProxyNode,
                           wkf::CommVisRouterGraphicsItem* aRouterPtr,
                           QWidget*                        aParent = nullptr,
                           Qt::WindowFlags                 aFlags  = Qt::WindowFlags());
   ~CommVisRouterNodeDialog() = default;

private:
   void    Initialize();
   void    UpdateRouterBlock();
   QString GenerateRouterString(const UtTextDocumentRange& aDocRange, const WsfPM_Platform& aPlatform);

   WsfPM_Router mRouterProxyNode;
};
} // namespace WizCommVis

#endif
