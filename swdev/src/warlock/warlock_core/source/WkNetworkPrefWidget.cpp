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

#include "WkNetworkPrefWidget.hpp"

#include <QInputDialog>

#include "UtQtNetwork.hpp"

warlock::net::PrefWidget::PrefWidget(QWidget* aParent)
   : wkf::PrefWidgetT<PrefObject>(aParent)
{
   mUi.setupUi(this);

   connect(mUi.addInterface, &QToolButton::clicked, this, &PrefWidget::OnAddInterfaceClicked);
}

void warlock::net::PrefWidget::showEvent(QShowEvent* aEvent)
{
   PopulateInterfaces(mUi.interfaceList->currentText());
   wkf::PrefWidgetT<PrefObject>::showEvent(aEvent);
}

void warlock::net::PrefWidget::ReadPreferenceData(const PrefData& aPrefData)
{
   mUi.addressEdit->setText(aPrefData.mAddress.toString());
   mUi.portSpinBox->setValue(aPrefData.mPort);

   PopulateInterfaces(aPrefData.mInterface.toString());
}

void warlock::net::PrefWidget::WritePreferenceData(PrefData& aPrefData)
{
   aPrefData.mAddress   = mUi.addressEdit->text();
   aPrefData.mInterface = mUi.interfaceList->currentText();
   aPrefData.mPort      = mUi.portSpinBox->value();
}

void warlock::net::PrefWidget::OnAddInterfaceClicked()
{
   bool    isOk;
   QString userValue =
      QInputDialog::getText(this, "Add Interface", "Interface:", QLineEdit::Normal, "000.000.000.000", &isOk);
   if (isOk)
   {
      PopulateInterfaces(userValue);
   }
}

void warlock::net::PrefWidget::PopulateInterfaces(QString aInterface)
{
   mUi.interfaceList->clear();
   mUi.interfaceList->addItems(UtQtNetwork::GetMulticastIpv4Interfaces());
   if (mUi.interfaceList->findText(aInterface) < 0)
   {
      mUi.interfaceList->insertSeparator(mUi.interfaceList->count());
      mUi.interfaceList->addItem(aInterface);
   }
   mUi.interfaceList->setCurrentText(aInterface);
}
