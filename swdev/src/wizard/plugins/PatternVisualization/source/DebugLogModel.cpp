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

#include "DebugLogModel.hpp"

#include <vector>

#include <QDebug>
#include <QDir>
#include <QString>

namespace
{
PatternVisualizer::DebugLogModel* gDebugLogModelInstance{};

void GlobalOnDebugMessage(QtMsgType aType, const QMessageLogContext& aContext, const QString& aMsg)
{
   Q_ASSERT(gDebugLogModelInstance);
   gDebugLogModelInstance->OnDebugMessage(aType, aContext, aMsg);
}

QString ConvertCategoryToString(QtMsgType aCategory)
{
   switch (aCategory)
   {
   case QtDebugMsg:
      return "Debug";
   case QtWarningMsg:
      return "Warning";
   case QtCriticalMsg:
      return "Critical";
   case QtFatalMsg:
      return "Fatal";
   case QtInfoMsg:
      return "Info";
   default:
      break;
   }

   return "(unknown)";
}

QString ConvertLogTimeToString(QDateTime aStartTime, QDateTime aCurrentTime)
{
   qint64 msecs = aStartTime.msecsTo(aCurrentTime);
   return QString::number(msecs / 1000.0, 'f', 3);
}

QString ConvertLogFileToString(const QString& aFileName)
{
   static QDir sourceDir = []()
   {
      // This file is located at debug/DebugLogModel.cpp in the source tree, so we can use
      // __FILE__ to get the base directory of the source tree. This bit of code truncates
      // everything after the second-to-last directory separator.
      QString x = QString(__FILE__);
      x.remove(x.lastIndexOf(QDir::separator(), x.lastIndexOf(QDir::separator()) - 1), x.length());
      return QDir(x);
   }();

   return sourceDir.relativeFilePath(aFileName);
}
} // namespace

namespace PatternVisualizer
{
namespace
{
const std::vector<QString> defaultColumnNames = {QString{"Time"},
                                                 QString{"Category"},
                                                 QString{"Message"},
                                                 QString{"File"},
                                                 QString{"Line"}};
}

DebugLogModel::DebugLogModel(QObject* aParent)
   : QAbstractTableModel{aParent}
   , mColumnNames{defaultColumnNames}
{
   Q_ASSERT(!gDebugLogModelInstance);
   gDebugLogModelInstance = this;

   mPreviousMessageHandler = qInstallMessageHandler(GlobalOnDebugMessage);

   qDebug() << "Log started at" << mLogStartTime.toString(Qt::RFC2822Date);
}

DebugLogModel::~DebugLogModel()
{
   qInstallMessageHandler(mPreviousMessageHandler);
   gDebugLogModelInstance = nullptr;
}

int DebugLogModel::rowCount(const QModelIndex& aParent) const
{
   return static_cast<int>(mMessages.size());
}

int DebugLogModel::columnCount(const QModelIndex& aParent) const
{
   return static_cast<int>(mColumnNames.size());
}

QVariant DebugLogModel::data(const QModelIndex& aIndex, int aRole) const
{
   int row    = aIndex.row();
   int column = aIndex.column();

   if (aRole == Qt::DisplayRole)
   {
      const MessageItem& message = mMessages[row];

      switch (column)
      {
      case 0:
         return ConvertLogTimeToString(mLogStartTime, message.mTime);
      case 1:
         return ConvertCategoryToString(message.mCategory);
      case 2:
         return message.mMessage;
      case 3:
         return ConvertLogFileToString(message.mFile);
      case 4:
         return message.mLine;
      default:
         break;
      }
   }
   else if (aRole == Qt::ToolTipRole)
   {
      const MessageItem& message = mMessages[row];

      // Display the absolute time for the time column.
      if (column == 0)
      {
         return message.mTime.toString(Qt::RFC2822Date);
      }

      // The file field also shows the absolute filename.
      if (column == 3)
      {
         return message.mFile;
      }

      // All other columns have their data as tooltips.
      return data(aIndex, Qt::DisplayRole);
   }

   return QVariant{};
}

QVariant DebugLogModel::headerData(int aSection, Qt::Orientation aOrientation, int aRole) const
{
   if (aRole == Qt::DisplayRole)
   {
      if (aOrientation == Qt::Horizontal)
      {
         return mColumnNames[aSection];
      }

      // Orientation must be vertical, so the heading will be the row number.
      if (aSection < static_cast<int>(mMessages.size()))
      {
         return aSection + 1;
      }
   }

   return QVariant{};
}

void DebugLogModel::OnDebugMessage(QtMsgType aType, const QMessageLogContext& aContext, const QString& aMsg)
{
   QDateTime currentTime = QDateTime::currentDateTime();

   QModelIndex parent;
   int         lastRow = static_cast<int>(mMessages.size());
   emit        beginInsertRows(parent, lastRow, lastRow);

   mMessages.emplace_back(currentTime, aType, aMsg, aContext.file, aContext.line);

   emit endInsertRows();

   mPreviousMessageHandler(aType, aContext, aMsg);
}
} // namespace PatternVisualizer
