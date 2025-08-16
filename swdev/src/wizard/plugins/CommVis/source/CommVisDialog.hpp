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

#ifndef WIZCOMMVISDIALOG_HPP
#define WIZCOMMVISDIALOG_HPP

#include "CommVisAddCommDialog.hpp"
#include "CommVisAddGatewayDialog.hpp"
#include "CommVisAddNetworkDialog.hpp"
#include "CommVisAddRouterDialog.hpp"
#include "UtQtUiPointer.hpp"
#include "comm_vis/WkfCommVisDataContainer.hpp"
#include "comm_vis/WkfCommVisDialog.hpp"
namespace WizCommVis
{
class CommVisDialog : public wkf::CommVisDialog
{
public:
   explicit CommVisDialog(wkf::DataContainer& aCommData,
                          QWidget*            aParent = nullptr,
                          Qt::WindowFlags     aFlags  = Qt::WindowFlags());
   void ReInitialize();
   void AddCommToNetwork(const QString&                aNetworkName,
                         const QString&                aNetworkAddress,
                         const QString&                aNetworkType,
                         wkf::CommVisCommGraphicsItem* aCommPtr) override;
   void AddNetworkToScene(const QString& aNetworkName, const QString& aNetworkAddress, const QString& aNetworkType) override;
   void AddCommToScene(const wkf::CommEvent& aCommEvent) override;
   void ConfigureSidePanel() override;
   void AddSidePanelButton(const QString& aButtonName) override;
   void AddNewDevice();

   wkf::CommVisLink* FindLink(const QString& aSourceCommName, const QString& aDestinationCommName) const;

private:
   void                                       AddDirectLinksToScene() override;
   ut::qt::UiPointer<CommVisAddCommDialog>    mAddCommDialogPtr;
   ut::qt::UiPointer<CommVisAddNetworkDialog> mAddNetworkDialogPtr;
   ut::qt::UiPointer<CommVisAddRouterDialog>  mAddRouterDialogPtr;
   ut::qt::UiPointer<CommVisAddGatewayDialog> mAddGatewayDialogPtr;
};
} // namespace WizCommVis

#endif
