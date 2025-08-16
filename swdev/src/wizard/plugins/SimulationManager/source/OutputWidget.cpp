// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2003-2015 The Boeing Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "OutputWidget.hpp"

#include <QKeyEvent>
#include <QUrl>
#include <QUrlQuery>

#include "Project.hpp"

static QString GetFileSystemPath(const QString& aLocalPath)
{
   const UtPath&     cwd      = wizard::Project::Instance()->WorkingDirectory();
   const std::string filePath = (cwd + aLocalPath.toStdString()).GetSystemPath();
   return QString::fromStdString(filePath);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SimulationManager::FileLineColumnPattern::FileLineColumnPattern()
   : mRegex("'([^']*)',.line (\\d+),.near column.(\\d+)")
{
}

wkf::log::PatternMatch SimulationManager::FileLineColumnPattern::FindInPlainText(int            aStartPosition,
                                                                                 const QString& aPlainText) const
{
   return wkf::log::FindRegexPatternMatch(mRegex, *this, aStartPosition, aPlainText);
}

QString SimulationManager::FileLineColumnPattern::PrefixTag(const QString& aRawText) const
{
   const UtPath cwd     = wizard::Project::Instance()->WorkingDirectory();
   auto         match   = mRegex.match(aRawText);
   auto         matches = match.capturedTexts();

   QUrl      errorUrl = QString("ide://wsf_error");
   QUrlQuery query;
   query.addQueryItem("file", ::GetFileSystemPath(matches[1]));
   query.addQueryItem("line", matches[2]);
   query.addQueryItem("column", matches[3]);
   errorUrl.setQuery(query);
   const QString format = "<a href=\"%1\">";
   return format.arg(errorUrl.toString());
}

QString SimulationManager::FileLineColumnPattern::PostfixTag() const
{
   return "</a>";
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SimulationManager::FileEofPattern::FileEofPattern()
   : mRegex("'([^']*)'.at.end-of-file")
{
}

wkf::log::PatternMatch SimulationManager::FileEofPattern::FindInPlainText(int aStartPosition, const QString& aPlainText) const
{
   return wkf::log::FindRegexPatternMatch(mRegex, *this, aStartPosition, aPlainText);
}

QString SimulationManager::FileEofPattern::PrefixTag(const QString& aRawText) const
{
   auto match   = mRegex.match(aRawText);
   auto matches = match.capturedTexts();

   QUrl      errorUrl = QString("ide://wsf_error");
   QUrlQuery query;
   query.addQueryItem("file", ::GetFileSystemPath(matches[1]));
   query.addQueryItem("EOF", "Y");
   errorUrl.setQuery(query);
   const QString format = "<a href=\"%1\">";
   return format.arg(errorUrl.toString());
}

QString SimulationManager::FileEofPattern::PostfixTag() const
{
   return "</a>";
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SimulationManager::OutputWidget::keyPressEvent(QKeyEvent* aKeyEvent)
{
   // Move the cursor
   if (aKeyEvent->key() == Qt::Key_End)
   {
      moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
   }
   else if (aKeyEvent->key() == Qt::Key_Home)
   {
      moveCursor(QTextCursor::Start, QTextCursor::MoveAnchor);
   }
   else if (aKeyEvent->key() == Qt::Key_Up)
   {
      moveCursor(QTextCursor::Up, QTextCursor::MoveAnchor);
   }
   else if (aKeyEvent->key() == Qt::Key_Down)
   {
      moveCursor(QTextCursor::Down, QTextCursor::MoveAnchor);
   }
   else if ((aKeyEvent->modifiers() & Qt::ControlModifier) && aKeyEvent->key() == Qt::Key_C)
   {
      copy();
   }
   else if ((aKeyEvent->modifiers() & Qt::ControlModifier) && aKeyEvent->key() == Qt::Key_A)
   {
      selectAll();
   }
}

wkf::log::PatternList SimulationManager::OutputWidget::GetCustomPatterns() const
{
   wkf::log::PatternList retval;
   retval += &mFileLineColumn;
   retval += &mFileEof;
   return retval;
}
