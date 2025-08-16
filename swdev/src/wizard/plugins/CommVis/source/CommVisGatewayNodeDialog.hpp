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

#ifndef COMMVISGATEWAYNODEDIALOG_HPP
#define COMMVISGATEWAYNODEDIALOG_HPP

#include "CommVisEditDialog.hpp"
#include "WsfPM_Comm.hpp"
#include "comm_vis/WkfCommVisGatewayNodeDialog.hpp"
class UtTextDocumentRange;

namespace WizCommVis
{
class CommVisGatewayNodeDialog : public wkf::CommVisGatewayNodeDialog, public CommVisEditDialog
{
   Q_OBJECT

public:
   CommVisGatewayNodeDialog(WsfPM_Comm                       aCommProxyNode,
                            wkf::CommVisGatewayGraphicsItem* aGatewayObjPtr,
                            QWidget*                         aParent = nullptr,
                            Qt::WindowFlags                  aFlags  = Qt::WindowFlags());
   ~CommVisGatewayNodeDialog() = default;

private:
   void Initialize();

   void       UpdateRemoteInterfaceAttributes();
   WsfPM_Comm mCommProxyNode;
};
} // namespace WizCommVis

#endif
