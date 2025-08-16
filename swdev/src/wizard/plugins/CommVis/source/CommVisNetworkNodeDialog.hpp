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

#ifndef COMMVISNETWORKNODEDIALOG_HPP
#define COMMVISNETWORKNODEDIALOG_HPP

#include "CommVisEditDialog.hpp"
#include "WsfPProxyNode.hpp"
#include "comm_vis/WkfCommVisNetworkNodeDialog.hpp"
class UtTextDocumentRange;

namespace WizCommVis
{
class CommVisNetworkNodeDialog : public wkf::CommVisNetworkNodeDialog, public CommVisEditDialog
{
   Q_OBJECT

public:
   CommVisNetworkNodeDialog(WsfPProxyNode                    aNetworkProxyNode,
                            wkf::CommVisNetworkGraphicsItem* aNetworkPtr,
                            QWidget*                         aParent = nullptr,
                            Qt::WindowFlags                  aFlags  = Qt::WindowFlags());
   ~CommVisNetworkNodeDialog() = default;

private:
   void UpdateAttribute(const QString& aAttribute, const QString& aScriptKeyword, const QString& aValue);
   void UpdateAttributes();
   void Initialize();
   void AddNewNetworkBlock();

   bool          mNameEdited{false};
   WsfPProxyNode mNetworkProxyNode;
};
} // namespace WizCommVis

#endif
