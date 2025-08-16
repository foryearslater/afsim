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

#ifndef OUTPUTWIDGET_HPP
#define OUTPUTWIDGET_HPP

#include "logging/WkfLogConsoleWidget.hpp"
#include "logging/WkfLogPatternUtils.hpp"

namespace SimulationManager
{
class FileLineColumnPattern final : public wkf::log::PatternBase
{
public:
   FileLineColumnPattern();

   wkf::log::PatternMatch FindInPlainText(int aStartPosition, const QString& aPlainText) const override;
   QString                PrefixTag(const QString& aRawText) const override;
   QString                PostfixTag() const override;

private:
   QRegularExpression mRegex;
};

class FileEofPattern final : public wkf::log::PatternBase
{
public:
   FileEofPattern();

   wkf::log::PatternMatch FindInPlainText(int aStartPosition, const QString& aPlainText) const override;
   QString                PrefixTag(const QString& aRawText) const override;
   QString                PostfixTag() const override;

private:
   QRegularExpression mRegex;
};

class OutputWidget : public wkf::log::ConsoleWidget
{
public:
   explicit OutputWidget(QWidget* parent = nullptr)
      : wkf::log::ConsoleWidget(parent)
   {
   }

protected:
   void keyPressEvent(QKeyEvent* aKeyEvent) override;

   wkf::log::PatternList GetCustomPatterns() const override;

private:
   FileLineColumnPattern mFileLineColumn;
   FileEofPattern        mFileEof;
};
} // namespace SimulationManager
#endif
