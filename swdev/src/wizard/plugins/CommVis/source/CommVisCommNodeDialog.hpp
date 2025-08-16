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

#ifndef COMMVISCOMMNODEDIALOG_HPP
#define COMMVISCOMMNODEDIALOG_HPP

#include "CommVisEditDialog.hpp"
#include "WsfPProxyNode.hpp"
#include "comm_vis/WkfCommVisCommNodeDialog.hpp"

class UtTextDocumentRange;

namespace WizCommVis
{
class CommVisCommNodeDialog : public wkf::CommVisCommNodeDialog, public CommVisEditDialog
{
   Q_OBJECT

public:
   CommVisCommNodeDialog(WsfPProxyNode                 aCommProxyNode,
                         wkf::CommVisCommGraphicsItem* aCommPtr,
                         QWidget*                      aParent = nullptr,
                         Qt::WindowFlags               aFlags  = Qt::WindowFlags());
   ~CommVisCommNodeDialog() = default;

private:
   void          UpdateAttributes();
   void          Initialize();
   void          AddNewCommBlock();
   void          UpdateCommBlock();
   void          DeleteComm();
   QString       GetLinksString(const QString& aSpaces) const;
   const QString GetAttributeString(int aTabSize) const override { return ""; }

   WsfPProxyNode mCommProxyNode;
};
} // namespace WizCommVis

#endif
