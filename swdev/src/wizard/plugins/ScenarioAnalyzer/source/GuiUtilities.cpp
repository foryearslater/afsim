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
*   File: GuiUtilities.cpp
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

#include <cassert>
#include <string>
#include <vector>

#include <QChar>
#include <QDir>
#include <QString>
#include <QVector>

#include "GuiUtilities.hpp"
#include "UtCast.hpp"

namespace ScenarioAnalyzer
{

// Checks if a path is absolute.
//
// On Windows, an absolute path is one beginning with `\\`, `\\?\`, or `X:\`,
// where `X` stands for any alphabetic character (a-z, A-Z). See
// `https://msdn.microsoft.com/en-us/library/windows/desktop/aa365247` for
// details on Windows paths.
//
// On other platforms, assume that absolute paths begin with '/'.
//
// This function expects that all `\` characters are replaced with `/`.
//
// This function does not check if a path is valid - it only performs the
// minimal work necessary to determine if a path is an absolute path.
//
// This function treats an empty path as a relative path.
//
// Qt has `QFileInfo` and `QDir` types for dealing with paths, but they query
// the file system to do path operations, which is unnecessary work.
// Additionally, when `QFileInfo` is constructed with a nonexistant relative
// path, it returns the current directory, which is problematic in some
// circumstances.
//
// `UtPath` has facilities for dealing with paths as well, but it is not worth
// including and linking that whole library for these few lines of simple code.

namespace // {anonymous}
{
bool isAbsolutePath(const QString& path)
{
   size_t pathLength = path.size();

#ifdef _WIN32
   // check for network path (begins with `\\`) or long path (begins with `\\?\`)
   if (pathLength >= 1 && path[0] == QChar('/'))
   {
      return true;
   }

   // check for drive path (begins with `X:\`, where 'X' is any alphabetic character)
   // NOTE: `X:` (without the slash) is *not* an absolue path
   if (pathLength >= 3)
   {
      bool isLower = path[0] >= QChar('a') && path[0] <= QChar('z');
      bool isUpper = path[0] >= QChar('A') && path[0] <= QChar('Z');
      bool alphaDriveLetter = isLower || isUpper;
      if (alphaDriveLetter && path[1] == QChar(':') && path[2] == QChar('/'))
      {
         return true;
      }
   }
   return false;
#else
   return pathLength >= 1 && path[0] == '/';
#endif
}

// implementation function for creating absolute paths to files and directories
QString createAbsolutePathImpl(const QString& path)
{
   QString normalizedPath = path;
   normalizedPath.replace(QChar('\\'), QChar('/'));
   if (!isAbsolutePath(normalizedPath))
   {
      return QDir::currentPath() + '/' + normalizedPath;
   }
   return normalizedPath;
}
} // {anonymous}

// Creates an AbsoluteDir from an absolute or relative path to a directory.
// This function does not check for the existence of the directory.
AbsoluteDir createAbsoluteDir(const QString& dir)
{
   QString dirAbsolute = createAbsolutePathImpl(dir);
   // append trailing `/` if it is missing
   if (dirAbsolute.size() == 0 || dirAbsolute[dirAbsolute.size() - 1] != '/')
   {
      dirAbsolute += QChar('/');
   }
   return AbsoluteDir{ dirAbsolute };
}

AbsoluteFile createAbsoluteFile(const QString& file)
{
   return AbsoluteFile{createAbsolutePathImpl(file)};
}

// Returns the parent directory of a file
AbsoluteDir parentDir(const AbsoluteFile& file)
{
   unsigned int length = file.f.size();
   unsigned int i = length;
   if (i == 0)
   {
      // Absolute paths to a file must contain at least one slash character.
      // If they do not, this file was given an invalid path.
      assert(false);
      return AbsoluteDir{ QString() };
   }

   for (;;)
   {
      --i;
      if (file.f[i] == '/')
      {
         return AbsoluteDir{ file.f.left(i + 1) };
      }
   }
}

namespace // {anonymous}
{
QString toNativePathImpl(QString path)
{
#ifdef _WIN32
   path.replace(QChar('/'), QChar('\\'));
   return path;
#else
   return path;
#endif
}
}

QString toNativePath(const AbsoluteDir& dir)
{
   return toNativePathImpl(dir.d);
}

QString toNativePath(const AbsoluteFile& file)
{
   return toNativePathImpl(file.f);
}

QString fileNameFromAbsoluteFile(const AbsoluteFile& path)
{
   const std::string filePath = path.f.toLocal8Bit().constData();
   size_t pos = filePath.find_last_of('/');
   size_t pos2 = filePath.find_last_of('.');
   const std::string suiteName = filePath.substr(pos + 1, pos2 - (pos + 1));

   return QString::fromLocal8Bit(suiteName.c_str());
}

namespace // {anonymous}
{
void appendString(std::vector<char>& buffer, std::size_t strLength, const char* str)
{
   std::size_t bufferOffset = buffer.size();
   buffer.resize(buffer.size() + strLength);
   memcpy(buffer.data() + bufferOffset, str, strLength);
}

size_t cStringLength(const char* s)
{
   const char* end = s;
   while (*end != '\0')
   {
      ++end;
   }
   return end - s;
}

StringSlice cStringToStringSlice(const char* s)
{
   return StringSlice{s, s + cStringLength(s)};
}
}

void appendCString(std::vector<char>& buffer, const char* str)
{
   appendString(buffer, cStringLength(str), str);
}

void appendQString(std::vector<char>& buffer, QString str)
{
   size_t charCount = str.size();
   size_t bufferOffset = buffer.size();
   buffer.resize(buffer.size() + charCount);
   const ushort* pStr = str.utf16();
   const ushort* end = pStr + charCount;
   char* pBuffer = buffer.data() + bufferOffset;
   ushort mask = 0xFF;
   while (pStr != end)
   {
      // NOTE: casting UTF-16 characters to chars only works if the characters
      // are ASCII.
      *pBuffer = (char)((*pStr) & mask);
      ++pStr;
      ++pBuffer;
   }
}

bool StringSlice::operator==(StringSlice rhs)
{
   size_t s1Length = end - begin;
   size_t s2Length = rhs.end - rhs.begin;
   if (s1Length != s2Length)
   {
      return false;
   }

   for (;;)
   {
      if (begin == end)
      {
         return true;
      }
      if (*begin != *rhs.begin)
      {
         return false;
      }
      ++begin;
      ++rhs.begin;
   }
}

bool StringSlice::operator!=(StringSlice rhs)
{
   return !operator==(rhs);
}

namespace // {anonymous}
{
void incrementLine(Parser& parser)
{
   ++parser.lineNumber;
   parser.lineBegin = parser.cursor;
}

SuiteFileLocation saveLocation(Parser& parser)
{
   SuiteFileLocation location = {};
   location.cursor = parser.cursor;
   location.lineNumber = parser.lineNumber;
   location.lineBegin = parser.lineBegin;
   return location;
}

// If the parser is pointing at a whitespace character, return true, and
// advance the cursor. If the character is a newline, also advance the line
// number and reset the line start pointer.
bool checkWhitespace(Parser& parser)
{
   switch (*parser.cursor)
   {
   case '\n':
      ++parser.cursor;
      incrementLine(parser);
      return true;
   case '\r':
   case ' ':
   case '\t':
      ++parser.cursor;
      return true;
   default:
      return false;
   }
}

// Skips whitespace in the input stream.
void skipWhitespace(Parser& parser)
{
   while (parser.cursor != parser.end && checkWhitespace(parser))
   {
   }
}

// Reads the next line comment or block comment from the input. A line comment
// is a block of text beginning with '//' or '#' and ending with a newline or
// the end of input. A block comment is a comment beginning with /* and ending
// with */. Block comments cannot be nested. This function assumes that the
// input is already pointing at the first comment character. This function
// returns true if it finds a comment, and places additional information about
// the comment in the result parameter. Otherwise, it returns false, and the
// result parameter is undefined.
bool readComment(Parser& parser, ReadCommentResult& result)
{
   size_t remainingLength = parser.end - parser.cursor;

   result.closed = true;
   result.text.end = parser.cursor;

   bool poundComment = remainingLength >= 1 && parser.cursor[0] == '#';
   bool slashSlashComment = remainingLength >= 2 && parser.cursor[0] == '/' && parser.cursor[1] == '/';
   bool slashStarComment = remainingLength >= 2 && parser.cursor[0] == '/' && parser.cursor[1] == '*';

   if (poundComment || slashSlashComment)
   {
      if (poundComment)
      {
         ++parser.cursor;
      }
      else
      {
         parser.cursor += 2;
      }
      result.text.begin = parser.cursor;

      result.closed = true;

      for (;;)
      {
         if (parser.cursor == parser.end)
         {
            result.text.end = parser.cursor;
            return true;
         }

         char c = *parser.cursor;
         ++parser.cursor;

         if (c == '\n')
         {
            // subtract 1 to exclude the newline from the comment
            result.text.end = parser.cursor - 1;
            incrementLine(parser);
            return true;
         }
      }
   }

   if (slashStarComment)
   {
      parser.cursor += 2;
      result.text.begin = parser.cursor;

      for (;;)
      {
         // handle whitespace specially to make sure line and column numbers
         // remain correct
         skipWhitespace(parser);

         // At least two characters are required to end a block comment. If
         // less than this are left, the comment is not closed.
         if (parser.end - parser.cursor < 2)
         {
            result.closed = false;
            result.text.end = parser.end;
            return true;
         }

         if (parser.cursor[0] == '*' && parser.cursor[1] == '/')
         {
            result.closed = true;
            result.text.end = parser.cursor;
            parser.cursor += 2;
            return true;
         }

         ++parser.cursor;
      }
   }

   return false;
}

// Reads the next token from the input. A token is defined as:
//
// * A string literal: a set of characters enclosed in double quotes
// * A word: set of characters preceding whitespace or a comment
//
// This function assumes that any leading whitespace has already been consumed.
// If the cursor is currently pointing at whitespace or a comment, this
// function will return an empty string.
//
// The token text is included as part of the result. If the token is a string
// literal, the text includes the enclosing quotes. If it is a word, the text
// does not include trailing whitespace characters.
ReadTokenResult readNextToken(Parser& parser)
{
   ReadTokenResult result = {};

   if (parser.cursor == parser.end)
   {
      return result;
   }

handleStringLiteral:

   result.token.begin = parser.cursor;

   if (*parser.cursor == '"')
   {
      ++parser.cursor;
      for (;;)
      {
         if (parser.cursor == parser.end)
         {
            result.stringClosed = false;
            result.token.end = parser.cursor;
            return result;
         }

         // handle whitespace specially to make sure line and column numbers
         // remain correct
         if (checkWhitespace(parser))
         {
            continue;
         }

         char c = *parser.cursor;
         ++parser.cursor;

         if (c == '"')
         {
            result.stringClosed = true;
            result.token.end = parser.cursor;
            return result;
         }
      }
   }
   else
   {
      result.stringClosed = true;

      for (;;)
      {
         size_t remainingLength = parser.end - parser.cursor;
         if (remainingLength == 0)
         {
            result.token.end = parser.cursor;
            return result;
         }

         if (checkWhitespace(parser))
         {
            // checkWhitespace advances the cursor if it find whitespace. Not subtracting
            // 1 here causes whitespace to be included in the command.
            result.token.end = parser.cursor - 1;
            return result;
         }

         if (*parser.cursor == '"')
         {
            goto handleStringLiteral;
         }

         bool poundComment = remainingLength >= 1 && parser.cursor[0] == '#';
         bool slashSlashComment = remainingLength >= 2 && parser.cursor[0] == '/' && parser.cursor[1] == '/';
         bool slashStarComment = remainingLength >= 2 && parser.cursor[0] == '/' && parser.cursor[1] == '*';
         bool commentStarted = poundComment || slashSlashComment || slashStarComment;
         if (commentStarted)
         {
            result.token.end = parser.cursor;
            return result;
         }

         ++parser.cursor;
      }
   }
}
} // {anonymous}

SuiteFileParseResult parseSuiteFile(size_t bufferLength,
                                    char* buffer,
                                    QVector<DependencyName>& dependencyNames,
                                    QVector<char>& scriptFunctionText,
                                    QVector<ScriptFunctionName>& scriptFunctionNames)
{
   StringSlice includeToken = cStringToStringSlice("include");
   StringSlice includeOnceToken = cStringToStringSlice("include_once");
   StringSlice scriptToken = cStringToStringSlice("script");
   StringSlice voidToken = cStringToStringSlice("void");
   StringSlice endScriptToken = cStringToStringSlice("end_script");

   Parser parser = {};
   parser.cursor = buffer;
   parser.end = buffer + bufferLength;
   parser.lineBegin = buffer;
   parser.lineNumber = 1;

   Token expectedToken = Token::Include;
   SuiteFileLocation scriptTokenLocation = {};
   const char* scriptTokenEnd = nullptr;
   for (;;)
   {
      for (;;)
      {
         skipWhitespace(parser);

         SuiteFileLocation commentLocation = saveLocation(parser);

         ReadCommentResult commentResult;
         if (!readComment(parser, commentResult))
         {
            break;
         }
         if (!commentResult.closed)
         {
            SuiteFileParseResult result = {};
            result.error = SuiteFileParseError::UnclosedBlockComment;
            result.errorBegin = commentLocation;
            result.errorEnd = parser.end;
            return result;
         }
      }

      if (parser.cursor == parser.end)
      {
         break;
      }

      SuiteFileLocation tokenLocation = saveLocation(parser);

      ReadTokenResult tokenResult = readNextToken(parser);
      if (!tokenResult.stringClosed)
      {
         SuiteFileParseResult result = {};
         result.error = SuiteFileParseError::UnclosedStringLiteral;
         result.errorBegin = tokenLocation;
         result.errorEnd = parser.end;
         return result;
      }
      StringSlice token = tokenResult.token;

      switch (expectedToken)
      {
         case Token::Include:
         {
            if (token == includeToken || token == includeOnceToken)
            {
               // consume rest of line and wait for other includes
               while (*parser.cursor != '\n')
               {
                  ++parser.cursor;
               }
               ++parser.cursor;
               expectedToken = Token::Include;
            }
            // Includes are optional. If we don't have one, "script" should be
            // the next token we encounter. Also, we'll let case Token::Script
            // handle any unknown commands.
            else
            {
               goto expectingScriptToken;
            }
            break;
         }
         case Token::Script:
         {
expectingScriptToken:
            if (token != scriptToken)
            {
               SuiteFileParseResult result = {};
               result.error = SuiteFileParseError::UnknownCommand;
               result.errorBegin = tokenLocation;
               result.errorEnd = token.end;
               return result;
            }
            scriptTokenLocation = tokenLocation;
            scriptTokenEnd = token.end;
            expectedToken = Token::ScriptReturnType;
            break;
         }
         case Token::ScriptReturnType:
         {
            if (token != voidToken)
            {
               SuiteFileParseResult result = {};
               result.error = SuiteFileParseError::InvalidScriptReturnType;
               result.errorBegin = tokenLocation;
               result.errorEnd = token.end;
               return result;
            }
            expectedToken = Token::ScriptName;
            break;
         }
         case Token::ScriptName:
         {
            const char* scriptNameBegin = token.begin;
            const char* scriptNameEnd = token.begin;

            for (;;)
            {
               if (scriptNameEnd == token.end)
               {
                  break;
               }

               char c = *scriptNameEnd;
               bool isIdentChar =
                  (c >= 'A' && c <= 'Z') ||
                  (c >= 'a' && c <= 'z') ||
                  (c >= '0' && c <= '9') ||
                  c == '_';
               if (!isIdentChar)
               {
                  break;
               }

               ++scriptNameEnd;
            }

            if (scriptNameBegin == scriptNameEnd)
            {
               SuiteFileParseResult result = {};
               result.error = SuiteFileParseError::MissingScriptFunctionName;
               result.errorBegin = scriptTokenLocation;
               result.errorEnd = scriptTokenEnd;
               return result;
            }

            ScriptFunctionName name;
            name.offset = scriptFunctionText.size();
            name.length = ut::cast_to_int( scriptNameEnd - scriptNameBegin );

            // determine if this is a check or a dependency list
            std::string nameStr(scriptNameBegin, scriptNameEnd - scriptNameBegin);
            if (nameStr.find("ScenarioAnalyzerDependencies") != std::string::npos)
            {
               expectedToken = Token::DependencyName;
            }
            else
            {
               scriptFunctionNames.push_back(name);
               scriptFunctionText.resize(name.offset + name.length);
               memcpy(
                  scriptFunctionText.data() + name.offset,
                  scriptNameBegin,
                  name.length);
               expectedToken = Token::EndScript;
            }

            break;
         }
         case Token::DependencyName:
         {
            if (token == endScriptToken)
            {
               expectedToken = Token::Script;
            }
            // we've found a string literal with at least one character
            else if (token.end - token.begin > 2 && *token.begin == '"' && *(token.end - 1) == '"')
            {
               DependencyName name;
               name.offset = scriptFunctionText.size();
               name.length = ut::cast_to_int( token.end - token.begin ) - 2; // strip off quotes

               dependencyNames.push_back(name);
               scriptFunctionText.resize(name.offset + name.length);
               memcpy(
                  scriptFunctionText.data() + name.offset,
                  token.begin + 1, // skip opening quotes
                  name.length);
            }

            break;
         }
         case Token::EndScript:
         {
            if (token == endScriptToken)
            {
               expectedToken = Token::Script;
            }
            break;
         }
         default:
            assert(false);
      }
   }

   if (expectedToken != Token::Script)
   {
      SuiteFileParseResult result = {};
      result.error = SuiteFileParseError::UnclosedScriptBlock;
      result.errorBegin = scriptTokenLocation;
      result.errorEnd = scriptTokenEnd;
      return result;
   }

   SuiteFileParseResult result = {};
   result.error = SuiteFileParseError::None;
   return result;
}

} // namespace ScenarioAnalyzer
