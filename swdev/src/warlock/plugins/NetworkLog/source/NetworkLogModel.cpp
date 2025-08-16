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

#include "NetworkLogModel.hpp"

#include "WkAppEnvironment.hpp"
#include "WkNetwork.hpp"

template<typename T>
static QVariant ToVariant(const ut::optional<T>& aOpt)
{
   if (aOpt)
   {
      return *aOpt;
   }
   return QVariant();
}

void NetworkLog::Model::ReceivedPacket(const warlock::net::Packet& aPacket)
{
   AddPacket(aPacket, "received");
}

void NetworkLog::Model::SentPacket(const warlock::net::Packet& aPacket)
{
   AddPacket(aPacket, "sent");
}

void NetworkLog::Model::Clear()
{
   emit beginResetModel();
   mPackets.clear();
   mDataColumns = 0;
   emit endResetModel();
}

int NetworkLog::Model::rowCount(const QModelIndex& aParent /*= {}*/) const
{
   return mPackets.size();
}

int NetworkLog::Model::columnCount(const QModelIndex& aParent /*= {}*/) const
{
   return mDataColumns + cFIXED_COLUMNS;
}

QVariant NetworkLog::Model::data(const QModelIndex& aIndex, int aRole /*= Qt::DisplayRole*/) const
{
   if (!aIndex.isValid())
   {
      return QVariant();
   }
   switch (aRole)
   {
   case Qt::DisplayRole:
      return DisplayData(aIndex);
   case Qt::ToolTipRole:
      return ToolTipData(aIndex);
   case Qt::ForegroundRole:
      return ::ToVariant(TextColorData(aIndex));
   case Qt::BackgroundRole:
      return ::ToVariant(BackgroundColorData(aIndex));
   case cPLATFORM_INDEX_ROLE:
      return ::ToVariant(PlatformIndexData(aIndex));
   default:
      return QVariant();
   }
}

QVariant NetworkLog::Model::headerData(int aSection, Qt::Orientation aOrientation, int aRole /*= Qt::DisplayRole*/) const
{
   if (aOrientation == Qt::Vertical || aRole != Qt::DisplayRole)
   {
      return QVariant();
   }
   else if (aRole == Qt::DisplayRole)
   {
      switch (aSection)
      {
      case cSTATUS_COLUMN:
         return "Status";
      case cTIME_COLUMN:
         return "Time";
      case cTYPE_COLUMN:
         return "Type";
      case cBYTES_COLUMN:
         return "Bytes";
      case cADDRESS_COLUMN:
         return "Address";
      case cPORT_COLUMN:
         return "Port";
      case cPID_COLUMN:
         return "PID";
      }
   }
   return QVariant();
}

const NetworkLog::PacketEntry* NetworkLog::Model::GetPacketEntry(int aRow) const
{
   if (aRow >= 0 && aRow < mPackets.size())
   {
      return &mPackets[aRow];
   }
   return nullptr;
}

const NetworkLog::DataEntry* NetworkLog::Model::GetDataEntry(const QModelIndex& aIndex) const
{
   auto packet = GetPacketEntry(aIndex.row());
   if (packet)
   {
      const int dataIndex = aIndex.column() - cFIXED_COLUMNS;
      if (dataIndex >= 0 && dataIndex < packet->mData.size())
      {
         return &packet->mData[dataIndex];
      }
   }
   return nullptr;
}

// private
void NetworkLog::Model::AddPacket(const warlock::net::Packet& aPacket, const QString& aStatus)
{
   PacketEntry entry;

   entry.mStatus     = aStatus;
   entry.mTime       = QTime::currentTime();
   entry.mPacketType = aPacket.PacketType();
   entry.mBytes      = aPacket.ByteCount();
   entry.mAddress    = aPacket.SenderAddress().toString();
   entry.mPort       = aPacket.SenderPort();
   entry.mPID        = aPacket.SenderPID();

   const int count = aPacket.FieldCount();
   for (int i = 0; i < count; i++)
   {
      entry.mData += mVisitor.Visit(aPacket.GetField(i));
   }
   AppendRow(std::move(entry));
}

// private
void NetworkLog::Model::AppendRow(PacketEntry&& aEntry)
{
   const int index       = mPackets.size();
   const int dataColumns = aEntry.mData.size();

   beginInsertRows(QModelIndex(), index, index);
   mPackets.append(std::move(aEntry));
   endInsertRows();

   emit RowsAppended();

   if (dataColumns > mDataColumns)
   {
      const int current = mDataColumns + cFIXED_COLUMNS;
      beginInsertColumns(QModelIndex(), current, current);
      mDataColumns = dataColumns;
      endInsertColumns();
      emit ColumnsAppended();
   }
}

// private
void NetworkLog::Model::OnNetworkError(const QString& aMessage, const QString& aDetails)
{
   const auto& network = wkEnv.GetNetwork();
   PacketEntry entry;

   entry.mStatus     = "error";
   entry.mTime       = QTime::currentTime();
   entry.mPacketType = "(" + aDetails + ")";
   entry.mAddress    = network.GetInterface().toString();
   entry.mPort       = network.GetPort();
   entry.mPID        = network.GetPID();

   DataEntry data;
   data.mName = "Message";
   data.mData = aMessage;
   entry.mData.append(data);

   data.mName = "Details";
   data.mData = aDetails;
   entry.mData.append(data);

   AppendRow(std::move(entry));
}

// private
void NetworkLog::Model::OnReconnect(const warlock::net::SenderInfo& aId)
{
   PacketEntry entry;

   entry.mStatus     = "reconnect";
   entry.mTime       = QTime::currentTime();
   entry.mPacketType = "(reconnect)";
   entry.mAddress    = aId.mAddress.toString();
   entry.mPort       = aId.mPort;
   entry.mPID        = aId.mPID;

   AppendRow(std::move(entry));
}

// private
QVariant NetworkLog::Model::DisplayData(const QModelIndex& aIndex) const
{
   const PacketEntry& entry = mPackets[aIndex.row()];

   switch (aIndex.column())
   {
   case cSTATUS_COLUMN:
      return entry.mStatus;
   case cTIME_COLUMN:
      return entry.mTime.toString(Qt::ISODateWithMs);
   case cTYPE_COLUMN:
      return entry.mPacketType;
   case cBYTES_COLUMN:
      return entry.mBytes;
   case cADDRESS_COLUMN:
      return entry.mAddress;
   case cPORT_COLUMN:
      return entry.mPort;
   case cPID_COLUMN:
      return entry.mPID;
   default:
      const int index = aIndex.column() - cFIXED_COLUMNS;
      if (index < entry.mData.size())
      {
         const auto& data = entry.mData[index];
         return data.mData;
      }
   }
   return QVariant();
}

// private
QString NetworkLog::Model::ToolTipData(const QModelIndex& aIndex) const
{
   switch (aIndex.column())
   {
   case cSTATUS_COLUMN:
      return "Status";
   case cTIME_COLUMN:
      return "Time";
   case cTYPE_COLUMN:
      return "Type";
   case cBYTES_COLUMN:
      return "Bytes";
   case cADDRESS_COLUMN:
      return "Address";
   case cPORT_COLUMN:
      return "Port";
   case cPID_COLUMN:
      return "PID";
   default:
      const auto& packet = mPackets[aIndex.row()];
      const int   index  = aIndex.column() - cFIXED_COLUMNS;
      if (index < packet.mData.size())
      {
         return packet.mData[index].mName;
      }
   }
   return QString();
}

// private
ut::optional<QColor> NetworkLog::Model::TextColorData(const QModelIndex& aIndex) const
{
   const PacketEntry& entry = mPackets[aIndex.row()];

   if (entry.mStatus == "error")
   {
      return QColor("red");
   }
   return ut::nullopt;
}

// private
ut::optional<QColor> NetworkLog::Model::BackgroundColorData(const QModelIndex& aIndex) const
{
   if (aIndex.column() < cFIXED_COLUMNS)
   {
      return QColor(128, 128, 128, 64);
   }
   return ut::nullopt;
}

// private
ut::optional<unsigned> NetworkLog::Model::PlatformIndexData(const QModelIndex& aIndex) const
{
   const PacketEntry& entry      = mPackets[aIndex.row()];
   const int          dataColumn = aIndex.column() - cFIXED_COLUMNS;

   if (dataColumn >= 0 && dataColumn < entry.mData.size())
   {
      return entry.mData[dataColumn].mPlatformIndex;
   }
   return ut::nullopt;
}
