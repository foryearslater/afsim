// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
/*******************************************************************************
*
*   File: GuiUtilities.hpp
*
*   Classification: UNCLASSIFIED
*
*   Details:
*
*   Contract Number:
*   Contractor Name: Radiance Technologies
*   Address:         350 Wynn Drive
*                    Huntsville, AL 35805
*
*   Abstract:
*
*******************************************************************************/

#ifndef SCENARIO_ANALYZER_GUI_UTILITIES_HPP
#define SCENARIO_ANALYZER_GUI_UTILITIES_HPP

#include <vector>

#include <QString>
#include <vector>

namespace ScenarioAnalyzer
{
// Absolute path to a directory. This is a lightweight marker type.
// Constructing it does not involve checking for the validity or existence of a
// path. All path separators should be set to '/' in this path. Additionally,
// the path should always end with a '/'. Including a trailing '/' simplifies
// of path manipulation.
struct AbsoluteDir
{
   QString d;
};

// Absolute path to a file. This is a lightweight marker type. Constructing it
// does not involve checking for the validity or existence of a path. All path
// separators should be set to '/' in this path.
struct AbsoluteFile
{
   QString f;
};

// NOTE: Since tasks emit a signal to inform the application that they are
// done, they require the app to remain alive while they are executing. All
// tasks contain a QEventLoopLocker field to prevent the app from quitting
// until the task is done.

AbsoluteDir createAbsoluteDir(const QString& dir);
// Creates an AbsoluteFile from an absolute or relative path to a file. This
// function does not check for the existence of the file.
AbsoluteFile createAbsoluteFile(const QString& file);
// Appends a directory name to a directory
inline QString appendDir(const AbsoluteDir& dir, const QString& s) { return dir.d + s + '/'; }
// Appends a file name to a directory
inline QString appendFile(const AbsoluteDir& dir, const QString& s) { return dir.d + s; }
AbsoluteDir parentDir(const AbsoluteFile& file);

// converts a directory to use the native separator character
QString toNativePath(const AbsoluteDir& dir);
// converts a file to use the native separator character
QString toNativePath(const AbsoluteFile& file);
QString fileNameFromAbsoluteFile(const AbsoluteFile& path);

void appendCString(std::vector<char>& buffer, const char* str);
void appendQString(std::vector<char>& buffer, QString str);

// Copies a QVector and all of its elements. This function is useful for
// exchanging QVector's of reentrant types between threads.
template <typename T>
QVector<T> deepCopy(const QVector<T>& v)
{
   QVector<T> vCopy;
   vCopy.reserve(v.size());
   for (int i = 0; i < v.size(); ++i)
   {
      vCopy.push_back(v[i]);
   }
   return vCopy;
}

struct DependencyName
{
   int offset, length;
};

struct ScriptFunctionName
{
   int offset, length;
};

struct ScriptTitle
{
   int offset, length;
};

struct SuiteFileLocation
{
   char* cursor, *lineBegin;
   size_t lineNumber;
};

enum struct SuiteFileParseError
{
   None,
   UnknownCommand,
   UnclosedBlockComment,
   UnclosedStringLiteral,
   UnclosedScriptBlock,
   InvalidScriptReturnType,
   MissingScriptFunctionName,
};

struct SuiteParseErrorWithContext
{
   SuiteFileParseError type;
   QString fileName;
   size_t lineNumber;
   QString context;
   int offset, length;
};

struct SuiteFileParseResult
{
   SuiteFileParseError error;
   SuiteFileLocation errorBegin;
   const char* errorEnd;
};

SuiteFileParseResult parseSuiteFile(
   size_t bufferLength,
   char* buffer,
   QVector<DependencyName>& requiredPlugins,
   QVector<char>& scriptFunctionText,
   QVector<ScriptFunctionName>& scriptFunctionNames);

struct StringSlice
{
   bool operator==(StringSlice rhs);
   bool operator!=(StringSlice rhs);

   const char* begin;
   const char* end;
};

struct Parser
{
   char* cursor, *end, *lineBegin;
   size_t lineNumber;
};

enum struct Token
{
   Include,
   Script,
   ScriptReturnType,
   ScriptName,
   DependencyName,
   EndScript,
};

struct ReadCommentResult
{
   // false if a block comment is not closed before the end of input
   bool closed;

   // the text of the comment, excluding the '#', '//', '/*', and '*/' marking
   // the begin and end of the comment
   StringSlice text;
};

struct ReadTokenResult
{
   // false if the token is a string literal and the closing quote was not
   // found before the end of input
   bool stringClosed;
   StringSlice token;
};

SuiteFileParseResult parseSuiteFile(
   size_t bufferLength,
   char* buffer,
   QVector<char>& scriptFunctionText,
   QVector<ScriptFunctionName>& scriptFunctionNames);
}
#endif
