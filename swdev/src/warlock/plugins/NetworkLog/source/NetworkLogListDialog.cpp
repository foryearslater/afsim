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

#include "NetworkLogListDialog.hpp"

#include <QPushButton>
#include <QTreeWidget>

#include "NetworkLogFilterRules.hpp"
#include "WkAppEnvironment.hpp"
#include "WkNetwork.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"

NetworkLog::ListDialog::ListDialog(QWidget* aParent, Model& aModel)
   : QDialog(aParent)
   , mModel(aModel)
{
   SetupUi();
   QTableView* table = mListWidget->GetTableView();

   connect(&mModel, &Model::RowsAppended, this, &ListDialog::OnRowsAppended);
   connect(&mModel, &Model::ColumnsAppended, this, &ListDialog::OnColumnsAppended);
   connect(table, &QTableView::doubleClicked, this, &ListDialog::OnDoubleClicked);

   mListWidget->RegisterRule<PlatformRuleSide>();
   mListWidget->RegisterRule<PlatformRuleType>();
   mListWidget->RegisterRule<PlatformRuleCategory>();

   mListWidget->RegisterRule<TimeBeforeRule>();
   mListWidget->RegisterRule<TimeAfterRule>();

   resize(800, 600);
}

void NetworkLog::ListDialog::SetupUi()
{
   setWindowFlag(Qt::WindowContextHelpButtonHint, false);
   setWindowTitle("Network Log");
   mListWidget = new WkfFilter::FilterWidget(this, "NetworkLog", &mModel);

   auto layout = new QVBoxLayout;
   layout->addLayout(SetupHeader());
   layout->addWidget(mListWidget);
   setLayout(layout);
}

QLayout* NetworkLog::ListDialog::SetupHeader()
{
   const auto& network = wkEnv.GetNetwork();
   connect(&network, &warlock::net::Network::Reconnected, this, &ListDialog::OnReconnect);

   QHBoxLayout* retval = new QHBoxLayout();

   mAddressLabel = new QLabel;
   mPortLabel    = new QLabel;
   mPIDLabel     = new QLabel;

   OnReconnect(network.GetSenderId());

   retval->addWidget(mAddressLabel);
   retval->addWidget(mPortLabel);
   retval->addWidget(mPIDLabel);
   retval->addWidget(MakePingButton());
   retval->addWidget(MakeClearButton());
   retval->addWidget(MakePreferencesButton());

   return retval;
}

QWidget* NetworkLog::ListDialog::MakePingButton()
{
   auto pingBtn = new QPushButton("Ping", this);
   pingBtn->setAutoDefault(false);

   connect(pingBtn, &QPushButton::clicked, this, &ListDialog::PingClicked);

   return pingBtn;
}

QWidget* NetworkLog::ListDialog::MakeClearButton()
{
   auto clearBtn = new QPushButton("Clear", this);
   clearBtn->setAutoDefault(false);

   connect(clearBtn, &QPushButton::clicked, &mModel, &Model::Clear);

   return clearBtn;
}

QWidget* NetworkLog::ListDialog::MakePreferencesButton()
{
   auto prefBtn = new QPushButton("Preferences...", this);
   prefBtn->setAutoDefault(false);

   connect(prefBtn, &QPushButton::clicked, []() { wkfEnv.GetMainWindow()->ShowPreferencePage("Network"); });

   return prefBtn;
}

void NetworkLog::ListDialog::OnRowsAppended()
{
   mListWidget->RowsAppended(1);
}

void NetworkLog::ListDialog::OnColumnsAppended()
{
   mListWidget->RunFilter();
}

//! Appends QTreeWidgetItems to aParent for each element of aData.
static void FillTreeRecurse(QTreeWidgetItem* aParent, const QList<NetworkLog::DataEntry>& aData)
{
   for (const auto& data : aData)
   {
      auto item = new QTreeWidgetItem({data.mName, data.mData.toString()});
      FillTreeRecurse(item, data.mListData);
      aParent->addChild(item);
   }
}

//! Creates and fills a QTreeWidget for aPacket.
static QTreeWidget* InitializeTreeWidget(const NetworkLog::PacketEntry& aPacket, int aSelected)
{
   auto retval = new QTreeWidget;
   retval->setHeaderLabels({"Name", "Value"});
   retval->addTopLevelItem(new QTreeWidgetItem({"Status", aPacket.mStatus}));
   retval->addTopLevelItem(new QTreeWidgetItem({"Time", aPacket.mTime.toString()}));
   retval->addTopLevelItem(new QTreeWidgetItem({"Type", aPacket.mPacketType}));
   retval->addTopLevelItem(new QTreeWidgetItem({"Address", aPacket.mAddress}));
   retval->addTopLevelItem(new QTreeWidgetItem({"Port", QString::number(aPacket.mPort)}));
   retval->addTopLevelItem(new QTreeWidgetItem({"PID", QString::number(aPacket.mPID)}));

   //! Make the metadata visually distinctive from the packet contents.
   for (int i = 0; i < retval->topLevelItemCount(); i++)
   {
      retval->topLevelItem(i)->setBackgroundColor(0, QColor(128, 128, 128, 64));
      retval->topLevelItem(i)->setBackgroundColor(1, QColor(128, 128, 128, 64));
   }

   FillTreeRecurse(retval->invisibleRootItem(), aPacket.mData);

   auto* selected = retval->topLevelItem(aSelected);
   if (selected)
   {
      selected->setSelected(true);
      selected->setExpanded(true);
   }

   return retval;
}

void NetworkLog::ListDialog::OnDoubleClicked(const QModelIndex& aIndex)
{
   const PacketEntry* packet = mModel.GetPacketEntry(mListWidget->MapToSourceRow(aIndex.row()));
   if (packet)
   {
      auto dialog = new QDialog(this);
      dialog->setAttribute(Qt::WA_DeleteOnClose);
      dialog->setWindowFlag(Qt::WindowContextHelpButtonHint, false);
      if (QStringList{"sent", "received"}.contains(packet->mStatus))
      {
         dialog->setWindowTitle(packet->mStatus + ": " + packet->mPacketType);
      }
      else
      {
         dialog->setWindowTitle(packet->mStatus);
      }

      auto layout = new QVBoxLayout;
      auto tree   = ::InitializeTreeWidget(*packet, aIndex.column());
      layout->addWidget(tree);
      dialog->setLayout(layout);
      dialog->show();
   }
}

void NetworkLog::ListDialog::OnReconnect(const warlock::net::SenderInfo& aId)
{
   mAddressLabel->setText("Interface: " + aId.mAddress.toString());
   mPortLabel->setText("Port: " + QString::number(aId.mPort));
   mPIDLabel->setText("PID: " + QString::number(aId.mPID));
}
