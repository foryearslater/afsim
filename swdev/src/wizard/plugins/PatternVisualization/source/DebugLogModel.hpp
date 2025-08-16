// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2016 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef DEBUGLOGMODEL_HPP
#define DEBUGLOGMODEL_HPP

#include <QAbstractTableModel>
#include <QDateTime>

namespace PatternVisualizer
{
class DebugLogModel : public QAbstractTableModel
{
   Q_OBJECT
public:
   explicit DebugLogModel(QObject* aParent = nullptr);
   ~DebugLogModel() override;

   int      rowCount(const QModelIndex& aParent = QModelIndex{}) const override;
   int      columnCount(const QModelIndex& aParent = QModelIndex{}) const override;
   QVariant data(const QModelIndex& aIndex, int aRole = Qt::DisplayRole) const override;
   QVariant headerData(int aSection, Qt::Orientation aOrientation, int aRole) const override;

   void OnDebugMessage(QtMsgType aType, const QMessageLogContext& aContext, const QString& aMsg);

private:
   struct MessageItem
   {
      MessageItem(QDateTime aTime, QtMsgType aCategory, QString aMessage, QString aFile, int aLine)
         : mTime{aTime}
         , mCategory{aCategory}
         , mMessage{aMessage}
         , mFile{aFile}
         , mLine{aLine}
      {
      }

      QDateTime mTime;
      QtMsgType mCategory;
      QString   mMessage;
      QString   mFile;
      int       mLine{};
   };

   QDateTime                mLogStartTime{QDateTime::currentDateTime()};
   QtMessageHandler         mPreviousMessageHandler{};
   std::vector<QString>     mColumnNames;
   std::vector<MessageItem> mMessages;
};
} // namespace PatternVisualizer

#endif
