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
#ifndef UTIL_HPP
#define UTIL_HPP

#include <algorithm>
#include <iosfwd>
#include <set>
#include <string>

#include <QMap>
#include <QRegExp>
#include <QString>
#include <QStringList>

#include "UtPath.hpp"
#include "UtTextDocument.hpp"
#include "ViExport.hpp"

class QDir;

namespace wizard
{

#ifdef _WIN32
struct UtilWindowsFileCharCompare
{
   bool operator()(char a, char b) const { return tolower(a) < tolower(b); }
   bool operator()(QChar a, QChar b) const { return tolower(a.toLatin1()) < tolower(b.toLatin1()); }
};
#endif
struct UtilFileStringCompare
{
   bool operator()(const std::string& lhs, const std::string& rhs) const
   {
#ifdef _WIN32
      return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(), UtilWindowsFileCharCompare());
#else
      return lhs < rhs;
#endif
   }
};

struct UtilFileStringCompareQt
{
   bool operator()(const QString& lhs, const QString& rhs) const
   {
#ifdef _WIN32
      return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(), UtilWindowsFileCharCompare());
#else
      return lhs < rhs;
#endif
   }
};

// Compare 2 null-terminated strings
struct UtilCStringCompare
{
   bool operator()(const char* aLhs, const char* aRhs) const
   {
      while ((*aLhs | *aRhs) != 0)
      {
         if (*aLhs < *aRhs)
            return true;
         if (*aRhs < *aLhs)
            return false;
         ++aLhs;
         ++aRhs;
      }
      return false;
   }
};

class VI_EXPORT Util
{
public:
   static bool        IsExecutable(const UtPath& aExePath);
   static QStringList SplitCommandLine(const QString& aCommandLine, const QMap<QString, QString>& aVariables);

   static bool IsNonTextFile(const QString& aPath);

   static int FilesInDirectory(QDir& aDir, int aStopAt = 10000);

   static std::string MakeTempFileName();

   static bool CreateNewFile(const UtPath& aName);

   static QString QuotePathWithSpaces(const QString& aPath);
   static QString StripHtml(const QString& htmlStr);

   static QRegExp SimpleRegex(const QString& aText);

   static void EscapeForCSV(std::string& aText);

   static QString BuildFileLineColURL(const QString& aFilePath, int aLine, int aColumn = 0, const QString& aLabel = QString());

   static QString BuildHRef(const QString& aUrl, const QString& aLabel);

   // RegExp to be used in creating QRegExpValidators. This one disallows
   // white space characters and comment tokens.
   static const QRegExp wsfValidInputRegexp;
};
} // namespace wizard

#endif
