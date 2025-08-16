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

#include "CommVisViewLinksDialog.hpp"

#include <QDialog>
#include <QLineEdit>
#include <QPointer>
#include <QPushButton>
#include <QTableWidgetItem>

#include "UtCast.hpp"

WizCommVis::CommVisViewLinksDialog::CommVisViewLinksDialog(const std::vector<wkf::CommEvent::LinkInfo>& aLinkInfo,
                                                           QWidget*                                     aParent,
                                                           Qt::WindowFlags                              aFlags)
   : wkf::CommVisViewLinksDialog(aParent, aFlags)
{
   mUI.mLinksTable->setRowCount(ut::cast_to_int(aLinkInfo.size()));
   mUI.mLinksTable->setColumnCount(4);

   const QStringList labels = {"Link Platform Name", "Link Comm Name", "Link Comm Address", "Delete?"};
   mUI.mLinksTable->setHorizontalHeaderLabels(labels);
   mUI.mLinksTable->setAlternatingRowColors(false);

   int row = 0;
   for (const auto& curLink : aLinkInfo)
   {
      QLineEdit* destPlatLineEdit = new QLineEdit(QString::fromStdString(curLink.mDestinationPlatformName), this);
      mUI.mLinksTable->setCellWidget(row, 0, destPlatLineEdit);

      QLineEdit* destCommLineEdit = new QLineEdit(QString::fromStdString(curLink.mDestinationCommName), this);
      mUI.mLinksTable->setCellWidget(row, 1, destCommLineEdit);

      QLineEdit* destAddressLineEdit = new QLineEdit(QString::fromStdString(curLink.mDestinationAddress), this);
      mUI.mLinksTable->setCellWidget(row, 2, destAddressLineEdit);

      QPushButton* deleteLinkPushButton = new QPushButton("Delete Link", this);
      mUI.mLinksTable->setCellWidget(row, 3, deleteLinkPushButton);
      deleteLinkPushButton->setProperty("row", row);
      connect(deleteLinkPushButton, &QPushButton::clicked, this, &CommVisViewLinksDialog::RemoveRow);

      ++row;
   }
   mUI.mAddLinkPushButton->show();
   connect(mUI.mAddLinkPushButton, &QPushButton::clicked, [this]() { AddRow(); });
}

void WizCommVis::CommVisViewLinksDialog::AddRow()
{
   int numRows = mUI.mLinksTable->rowCount();
   mUI.mLinksTable->setRowCount(numRows + 1);

   QLineEdit* destPlatLineEdit = new QLineEdit(this);
   mUI.mLinksTable->setCellWidget(numRows, 0, destPlatLineEdit);

   QLineEdit* destCommLineEdit = new QLineEdit(this);
   mUI.mLinksTable->setCellWidget(numRows, 1, destCommLineEdit);

   QLineEdit* destAddressLineEdit = new QLineEdit(this);
   mUI.mLinksTable->setCellWidget(numRows, 2, destAddressLineEdit);

   QPushButton* deleteLinkPushButton = new QPushButton("Delete Link", this);
   mUI.mLinksTable->setCellWidget(numRows, 3, deleteLinkPushButton);
   connect(deleteLinkPushButton, &QPushButton::clicked, this, &CommVisViewLinksDialog::RemoveRow);
}

void WizCommVis::CommVisViewLinksDialog::RemoveRow()
{
   auto* button = qobject_cast<QPushButton*>(sender());
   if (button)
   {
      int row = button->property("row").toInt();
      mUI.mLinksTable->removeRow(row);
   }
}
