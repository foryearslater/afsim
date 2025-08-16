// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2022 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

// Qt Includes
#include <QDebug>
#include <QTime>

// WKF Includes
#include "WkfEnvironment.hpp"

// TaskList Includes
#include "TaskListPrefObject.hpp"
#include "TaskListTask.hpp"

namespace TaskList
{
PathFormat Task::mPathFormat{PathFormat::Absolute};
UtPath     Task::mWorkingDirectory{};
QString    Task::sDebugLog{};

Task Task::RangeToTask(const UtTextDocumentRange& aRange, const QString& aTag) noexcept
{
   Task t;

   bool ok{aRange.Valid()};
   if (!ok)
   {
      QDebug debugStream{&sDebugLog};
      debugStream << QTime::currentTime().toString()
                  << "WizTaskList"
                     "\n  aRange is invalid.";
      return t;
   }
   auto text = QString::fromStdString(aRange.Text());

   ok &= MatchCommentP(text);
   if (!ok)
   {
      QDebug debugStream{&sDebugLog};
      debugStream << QTime::currentTime().toString()
                  << "WizTaskList"
                     "\n  Task::MatchCommentP failed.";
      return t;
   }

   ok &= MatchCommentSymbolsAndWhitespaceP(text);
   if (!ok)
   {
      QDebug debugStream{&sDebugLog};
      debugStream << QTime::currentTime().toString()
                  << "WizTaskList"
                     "\n  Task::MatchCommentSymbolsAndWhitespaceP failed.";
      return t;
   }

   ok &= MatchIdentifierP(text, aTag);
   if (!ok)
   {
      QDebug debugStream{&sDebugLog};
      debugStream << QTime::currentTime().toString()
                  << "WizTaskList"
                     "\n  Task::MatchIdentifierP failed.";
      return t;
   }

   ok &= MatchNonWordP(text);
   if (!ok)
   {
      QDebug debugStream{&sDebugLog};
      debugStream << QTime::currentTime().toString()
                  << "WizTaskList"
                     "\n  Task::MatchNonWordP failed.";
      return t;
   }

   t.mTag         = aTag;
   t.mDescription = text.trimmed();
   UtPath path{aRange.mSource->GetFilePath()};
   t.mPath = path;
   t.mFile = QString::fromStdString(path.GetFileName(true));
   path.Up();

   switch (mPathFormat)
   {
   case PathFormat::Relative:
   {
      UtPath      relativePath{mWorkingDirectory.GetRelativePath(path)};
      std::string relativePathString{relativePath.GetNormalizedPath()};
      if (relativePathString.empty())
      {
         relativePathString += "./";
      }
      else if (relativePathString == "..")
      {
         relativePathString += '/';
      }
      t.mDirectory = QString::fromStdString(relativePathString);
      break;
   }
   case PathFormat::Absolute:
      t.mDirectory = QString::fromStdString(path.GetNormalizedPath());
   default:
      break;
   }

   aRange.mSource->PositionToLineColumn(aRange.GetBegin(), t.mLine, t.mColumn);

   return t;
}

Task Task::RowToTask(QTableWidget* aTable, int aRow) noexcept
{
   Task t;

   QTableWidgetItem* tagItem{aTable->item(aRow, static_cast<int>(ColumnIndex::Tag))};
   if (!tagItem)
   {
      QDebug debugStream{&sDebugLog};
      debugStream << QTime::currentTime().toString()
                  << "WizTaskList"
                     "\n  The Tag item for row "
                  << QString::number(aRow) << " is unavailable.";
      return t;
   }
   t.mTag = tagItem->text();

   QTableWidgetItem* descriptionItem{aTable->item(aRow, static_cast<int>(ColumnIndex::Description))};
   if (!descriptionItem)
   {
      QDebug debugStream{&sDebugLog};
      debugStream << QTime::currentTime().toString()
                  << "WizTaskList"
                     "\n  The Description item for row "
                  << QString::number(aRow) << " is unavailable.";
      return t;
   }
   t.mDescription = descriptionItem->text();

   QTableWidgetItem* directoryItem{aTable->item(aRow, static_cast<int>(ColumnIndex::Directory))};
   if (!directoryItem)
   {
      QDebug debugStream{&sDebugLog};
      debugStream << QTime::currentTime().toString()
                  << "WizTaskList"
                     "\n  The Directory item for row "
                  << QString::number(aRow) << " is unavailable.";
      return t;
   }
   t.mDirectory = directoryItem->text();

   QTableWidgetItem* fileItem{aTable->item(aRow, static_cast<int>(ColumnIndex::File))};
   if (!fileItem)
   {
      QDebug debugStream{&sDebugLog};
      debugStream << QTime::currentTime().toString()
                  << "WizTaskList"
                     "\n  The File item for row "
                  << QString::number(aRow) << " is unavailable.";
      return t;
   }
   t.mFile = fileItem->text();

   QTableWidgetItem* lineItem{aTable->item(aRow, static_cast<int>(ColumnIndex::Line))};
   if (!lineItem)
   {
      QDebug debugStream{&sDebugLog};
      debugStream << QTime::currentTime().toString()
                  << "WizTaskList"
                     "\n  The Line item for row "
                  << QString::number(aRow) << " is unavailable.";
      return t;
   }
   QString            lineText{lineItem->text()};
   bool               lineOk{false};
   unsigned long long lineNum{lineText.toULongLong(&lineOk)};
   if (!lineOk)
   {
      QDebug debugStream{&sDebugLog};
      debugStream << QTime::currentTime().toString()
                  << "WizTaskList"
                     "\n  Conversion of line number from QString to unsigned long long failed.";
      return t;
   }
   t.mLine = lineNum;

   QTableWidgetItem* columnItem{aTable->item(aRow, static_cast<int>(ColumnIndex::Column))};
   if (!columnItem)
   {
      QDebug debugStream{&sDebugLog};
      debugStream << QTime::currentTime().toString()
                  << "WizTaskList"
                     "\n  The Column item for row "
                  << QString::number(aRow) << " is unavailable.";
      return t;
   }
   QString            columnText{columnItem->text()};
   bool               columnOk{false};
   unsigned long long columnNum{columnText.toULongLong(&columnOk)};
   if (!columnOk)
   {
      QDebug debugStream{&sDebugLog};
      debugStream << QTime::currentTime().toString()
                  << "WizTaskList"
                     "\n  Conversion of column number from QString to unsigned long long failed.";
      return t;
   }
   t.mColumn = columnNum;

   return t;
}

bool Task::IsValid() const noexcept
{
   if (mTag.isEmpty())
   {
      return false;
   }

   auto* prefs = wkfEnv.GetPreferenceObject<PrefObject>();
   if (!prefs)
   {
      QDebug debugStream{&mDebugLog};
      debugStream << QTime::currentTime().toString()
                  << "WizTaskList"
                     "\n  TaskList::PrefObject is unavailable.";
      return false;
   }
   QStringList taskTags{prefs->GetTaskTags()};
   int         tagIndex{taskTags.indexOf(mTag)};
   if (tagIndex == -1)
   {
      return false;
   }

   if (mDirectory.isEmpty())
   {
      return false;
   }

   if (mFile.isEmpty())
   {
      return false;
   }

   if (!mPath.Exists())
   {
      return false;
   }

   if (mLine == ut::npos)
   {
      return false;
   }

   if (mColumn == ut::npos)
   {
      return false;
   }

   return true;
}

bool Task::MatchCommentP(QString& aText) noexcept
{
   // Match the (starting) comment delimiter
   const QRegularExpression cCOMMENT_START_REGEX{"(/\\*|#|//)"};
   QRegularExpressionMatch  commentStartMatch{cCOMMENT_START_REGEX.match(aText)};
   if (!commentStartMatch.hasMatch())
   {
      return false;
   }
   int commentStartMatchFirstCaptureEndPos{commentStartMatch.capturedEnd(0)};
   aText = aText.mid(commentStartMatchFirstCaptureEndPos);

   return true;
}

bool Task::MatchCommentSymbolsAndWhitespaceP(QString& aText) noexcept
{
   // Match zero or more subsequent comment symbols ('#', '*', '/') or whitespaces (' ', '\f', '\n', '\r', '\t', '\v')
   const QRegularExpression cCOMMENT_SYMBOLS_AND_WHITESPACE_REGEX{"(#|\\*|/|\\s)*"};
   QRegularExpressionMatch  commentSymbolsAndWhitespaceMatch{cCOMMENT_SYMBOLS_AND_WHITESPACE_REGEX.match(aText)};
   if (!commentSymbolsAndWhitespaceMatch.hasMatch())
   {
      return false;
   }
   int commentSymbolsAndWhitespaceMatchLastCaptureIndex{commentSymbolsAndWhitespaceMatch.lastCapturedIndex()};
   int commentSymbolsAndWhitespaceMatchLastCaptureEndPos{
      commentSymbolsAndWhitespaceMatch.capturedEnd(commentSymbolsAndWhitespaceMatchLastCaptureIndex)};
   aText = aText.mid(commentSymbolsAndWhitespaceMatchLastCaptureEndPos);

   return true;
}

bool Task::MatchIdentifierP(QString& aText, const QString& aIdentifier) noexcept
{
   // At this point in the routine, the resulting string should start with the identifier
   if (!aText.startsWith(aIdentifier, Qt::CaseInsensitive))
   {
      return false;
   }
   aText = aText.mid(aIdentifier.size());

   return true;
}

bool Task::MatchNonWordP(QString& aText) noexcept
{
   // Match one or more symbols or non-word characters
   const QRegularExpression cSYMBOL_REGEX{"(\\W+)"};
   QRegularExpressionMatch  symbolMatch{cSYMBOL_REGEX.match(aText)};
   if (!symbolMatch.hasMatch())
   {
      return false;
   }
   int symbolMatchLastCaptureIndex{symbolMatch.lastCapturedIndex()};
   int symbolMatchLastCaptureEndPos{symbolMatch.capturedEnd(symbolMatchLastCaptureIndex)};
   aText = aText.mid(symbolMatchLastCaptureEndPos);

   // The WsfParser treats '\0' as whitespace.
   // However, the text renders weird in a QWidget.
   // So, remove the '\0' if it exists.
   if (aText.indexOf(QChar{'\0'}) != -1)
   {
      aText = aText.left(aText.size() - 2);
   }

   return true;
}

bool Task::Compare::operator()(const Task& aLhs, const Task& aRhs) const
{
   switch (mIndex)
   {
   case ColumnIndex::Tag:
      return CompareImpl(aLhs.mTag, aRhs.mTag);
   case ColumnIndex::Description:
      return CompareImpl(aLhs.mDescription, aRhs.mDescription);
   case ColumnIndex::Directory:
   {
      bool directoryEqual{aLhs.mDirectory == aRhs.mDirectory};
      if (directoryEqual)
      {
         bool fileEqual{aLhs.mFile == aRhs.mFile};
         if (fileEqual)
         {
            return CompareImpl(aLhs.mLine, aRhs.mLine);
         }
         return CompareImpl(aLhs.mFile, aRhs.mFile);
      }
      return CompareImpl(aLhs.mDirectory, aRhs.mDirectory);
   }
   case ColumnIndex::File:
   {
      bool fileEqual{aLhs.mFile == aRhs.mFile};
      if (fileEqual)
      {
         return CompareImpl(aLhs.mLine, aRhs.mLine);
      }
      return CompareImpl(aLhs.mFile, aRhs.mFile);
   }
   case ColumnIndex::Line:
      return CompareImpl(aLhs.mLine, aRhs.mLine);
   case ColumnIndex::Column:
      return CompareImpl(aLhs.mLine, aRhs.mLine);
   default:
      return false;
   }
}

bool operator==(const Task& aLhs, const Task& aRhs) noexcept
{
   bool sameTags{aLhs.mTag == aRhs.mTag};
   bool sameDescription{aLhs.mDescription == aRhs.mDescription};
   bool samePath{aLhs.mPath == aRhs.mPath};
   bool sameLine{aLhs.mLine == aRhs.mLine};
   bool sameColumn{aLhs.mColumn == aRhs.mColumn};
   return sameTags && sameDescription && samePath && sameLine && sameColumn;
}

bool operator!=(const Task& aLhs, const Task& aRhs) noexcept
{
   return !(aLhs == aRhs);
}

} // end namespace TaskList
