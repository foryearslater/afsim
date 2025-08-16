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

#ifndef NETWORK_LOG_MODEL_HPP
#define NETWORK_LOG_MODEL_HPP

#include <QAbstractTableModel>
#include <QTime>

#include "NetworkLogVisitor.hpp"

namespace NetworkLog
{
//! A struct containing the metadata for a single packet.
//! Can also hold data about errors and reconnects.
struct PacketEntry
{
   QString          mStatus;
   QTime            mTime;
   QString          mPacketType;
   int              mBytes = 0;
   QString          mAddress;
   int              mPort = 0;
   qint64           mPID  = 0;
   QList<DataEntry> mData;
};

//! Model contains a list of all network activity, including
//!   sent packets, received packets, network errors, and reconnects.
//! Activity is stored chronologically.
class Model final : public QAbstractTableModel
{
   Q_OBJECT

public:
   Model() = default;

   enum : int
   {
      cSTATUS_COLUMN,
      cTIME_COLUMN,
      cTYPE_COLUMN,
      cBYTES_COLUMN,
      cADDRESS_COLUMN,
      cPORT_COLUMN,
      cPID_COLUMN,
      cFIXED_COLUMNS,

      cPLATFORM_INDEX_ROLE = Qt::UserRole
   };

   //! Called when a packet is sent/received.
   //! Adds the packet to the list and emits the correct signals.
   //@{
   void ReceivedPacket(const warlock::net::Packet& aPacket);
   void SentPacket(const warlock::net::Packet& aPacket);
   //@}

   //! Clears the data in the model.
   void Clear();

   //! Necessary base class overrides.
   //@{
   int      rowCount(const QModelIndex& aParent = {}) const override;
   int      columnCount(const QModelIndex& aParent = {}) const override;
   QVariant data(const QModelIndex& aIndex, int aRole = Qt::DisplayRole) const override;
   QVariant headerData(int aSection, Qt::Orientation aOrientation, int aRole = Qt::DisplayRole) const override;
   //@}

   //! Returns the PacketEntry at row aRow.
   const PacketEntry* GetPacketEntry(int aRow) const;

   //! Returns the DataEntry, if any, at the given index.
   //! Data in fixed columns will not have a DataEntry.
   const DataEntry* GetDataEntry(const QModelIndex& aIndex) const;

signals:
   //! Emitted when a row is appended.
   void RowsAppended();
   //! Emitted when a column is appended.
   void ColumnsAppended();

public slots:
   void OnNetworkError(const QString& aMessage, const QString& aDetails);
   void OnReconnect(const warlock::net::SenderInfo& aId);

private:
   //! Fills out the data portion of aEntry and adds it to the model.
   void AddPacket(const warlock::net::Packet& aPacket, const QString& aStatus);

   //! Appends a row to the model and emits the correct signals.
   //! If columns need to be added, does that as well.
   void AppendRow(PacketEntry&& aEntry);

   //! Helpers for data()
   //@{
   QVariant               DisplayData(const QModelIndex& aIndex) const;
   QString                ToolTipData(const QModelIndex& aIndex) const;
   ut::optional<QColor>   TextColorData(const QModelIndex& aIndex) const;
   ut::optional<QColor>   BackgroundColorData(const QModelIndex& aIndex) const;
   ut::optional<unsigned> PlatformIndexData(const QModelIndex& aIndex) const;
   //@}

   QList<PacketEntry> mPackets;
   PacketVisitor      mVisitor;
   int                mDataColumns = 0;
};
} // namespace NetworkLog

#endif
