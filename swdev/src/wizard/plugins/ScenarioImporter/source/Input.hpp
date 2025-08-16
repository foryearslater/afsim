// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#if 0
TODO list

* What about empty fields(e.g. the input ",,,,")
* What about trailing delimiters(e.g. the input "a,b,")



WHITESPACE HANDLING:

Consider the string "  a  b  ". Should we:

* Strip leading spaces ?
* Strip trailing spaces ?
* Keep all spaces ?
* Keep only middle spaces ?
* Report an error ?

There may not be a one - size - fits - all option, so whitespace behavior should
be a configurable option, per column.
#endif

#define ArrayLength(a) (sizeof(a) / sizeof((a)[0]))

#include <assert.h>

typedef int8_t i8;
typedef uint8_t u8;
typedef int16_t i16;
typedef uint16_t u16;
typedef int32_t i32;
typedef uint32_t u32;
typedef int64_t i64;
typedef uint64_t u64;

namespace ScenarioImporter
{
enum struct preProcessorErrorType : int
{
   fileNameError,
   argumentError,
   processError,
};

struct GenericError
{
   bool isWarning;
   std::string explanation;
   GenericError(bool _isWarning, std::string _explanation) :
      isWarning(_isWarning), explanation(_explanation) {}
};

// Errors in recognizing selected command line arguments for the Pre-Processor
struct PreProcessorError : GenericError
{
   preProcessorErrorType type;
   int argumentIdx;
   PreProcessorError(bool _isWarning, std::string _explanation, preProcessorErrorType _type, int _argumentIdx) :
      GenericError(_isWarning, _explanation),
      type(_type),
      argumentIdx(_argumentIdx) {}
};

// TODO This is a dummy struct to match the argumentTypeList, flags for the pre processor would go here
enum struct ArgumentFlag : int
{
   CommandLineFlag1,
   CommandLineFlag2,
   CommandLineFlag3,
};

struct Parser
{
   // pointer to current character
   const char* cursor;
   // pointer to one character past the last character
   const char* end;
   // current line number
   u32 lineNumber;
   // pointer to first character in current line
   const char* lineBegin;
   // set after a delimiter is neither discarded or consumed
   bool takeNextDelimiter;
};

// This needs to match parserColumnDataTypeList
enum struct ParserColumnDatatype : int
{
   String,
   Identifier,
   SignedInteger,
   UnsignedInteger,
   FractionalNumber,
   Lat,
   Long
};

// Do we not want some explicit connection between these columns and records (platforms)?
struct ParserColumn
{
   // Key under which the data will be stored in a record object
   std::string attributeKey;

   // Character delimiting the end of the token
   char delimiter;

   // The expected data type for the column
   ParserColumnDatatype datatype;

   // Delimiter handling options
   bool discardDelimiters;
   bool consumeDelimiters;

   ParserColumn(std::string aKey, char aDelimiter, ParserColumnDatatype aType, bool aDiscard, bool aConsume) :
      attributeKey(aKey), delimiter(aDelimiter), datatype(aType), discardDelimiters(aDiscard), consumeDelimiters(aConsume)
   {}
};

struct ParserError : GenericError
{
   // the line number containing the error
   int row;
   int col;

   ParserError(bool _isWarning, std::string _explanation, int _row, int _col) :
      GenericError(_isWarning, _explanation),
      row(_row),
      col(_col) {}
};

inline static void parserIncrementLineNumber(Parser& parser)
{
   ++parser.lineNumber;
   parser.lineBegin = parser.cursor;
}

static StringSlice parserNextToken(Parser& parser, char delimiter, bool discardDelimiters, bool consumeDelimiters)
{
   StringSlice slice;

   slice.begin = parser.cursor;
   for (;;)
   {
      if (parser.cursor == parser.end)
      {
         slice.end = parser.cursor;
         break;
      }

      char c = *parser.cursor;

      // check for end of line - EOL is always a delimiter
      if (c == '\r' || c == '\n')
      {
         slice.end = parser.cursor;
         ++parser.cursor;
         // check for Windows-style line ending
         if (
            c == '\r' &&
            parser.cursor != parser.end &&
            *parser.cursor == '\n')
         {
            ++parser.cursor;
         }
         parserIncrementLineNumber(parser);
         return slice;
      }

      if (c == delimiter)
      {
         if (parser.takeNextDelimiter)
         {
            ++parser.cursor;
            parser.takeNextDelimiter = false;
         }
         else if (discardDelimiters == true)
         {
            slice.end = parser.cursor;
            ++parser.cursor;
            return slice;
         }
         else if (consumeDelimiters == true) // don't discard, and keep the delimeter in this token
         {
            ++parser.cursor;
            slice.end = parser.cursor;
            return slice;
         }
         else // don't discard, and leave the delimiter for the next token. Requires the parser to handle the next token differently
         {
            slice.end = parser.cursor;
            parser.takeNextDelimiter = true;
            return slice;
         }

      }

      ++parser.cursor;
   }
   return slice;
}

static void parserConsumeLine(Parser& parser)
{
   for (;;)
   {
      if (parser.cursor == parser.end)
      {
         break;
      }

      char c = *parser.cursor;
      ++parser.cursor;

      if (c == '\r' || c == '\n')
      {
         // check for Windows-style line ending
         if (
            c == '\r' &&
            parser.cursor != parser.end &&
            *parser.cursor == '\n')
         {
            ++parser.cursor;
         }
         parserIncrementLineNumber(parser);
         break;
      }
   }
}

// Checks that a string is a valid identifier (e.g. the name of a platform,
// or the type of a sensor)
static bool validateIdentifier(StringSlice s)
{
   // empty identifiers are not allowed
   if (isEmpty(s))
   {
      return false;
   }

   // Check for characters that could be a problem. These checks are only a
   // heuristic. AFSIM accepts some of these characters in some circumstances,
   // but they cause problems in others.
   while (s.begin != s.end)
   {
      // this is not an exhaustive check, but identifies some characters that
      // are problematic
      switch (*s.begin)
      {
         // whitespace
         case ' ':
         case '\t':
         case '\r':
         case '\n':

         // commments
         case '#':
         case '/':

         // string literal
         case '"':
            return false;
      }
      ++s.begin;
   }
   return true;
}

inline static bool isDigit(char c)
{
   return c >= '0' && c <= '9';
}

static bool validateUnsignedInteger(StringSlice s)
{
   if (isEmpty(s))
   {
      return false;
   }

   // check for non-digit characters
   while (s.begin != s.end)
   {
      if (!isDigit(*s.begin))
      {
         return false;
      }
      ++s.begin;
   }
   return true;
}

static bool validateSignedInteger(StringSlice s)
{
   // consume leading sign
   char first = *s.begin;
   if (first == '-' || first == '+')
   {
      ++s.begin;
   }

   // this case happens when the string is "", "-", "+"
   if (isEmpty(s))
   {
      return false;
   }

   // check for non-digit characters
   while (s.begin != s.end)
   {
      if (!isDigit(*s.begin))
      {
         return false;
      }
      ++s.begin;
   }
   return true;
}

static bool validateFractionalNumber(StringSlice s)
{
   const char* p = s.begin;

   // consume leading sign
   if (*p == '-' || *p == '+')
   {
      ++p;
   }

   // consume the integer part (the part preceding the decimal point)
   const char* integerPartBegin = p;
   while (p != s.end && isDigit(*p))
   {
      ++p;
   }
   const char* integerPartEnd = p;

   // if a decimal point is present, consume the fractional part (the part
   // following the decimal point)
   const char* fractionPartBegin = p;
   const char* fractionPartEnd = p;
   if (*p == '.')
   {
      ++p;

      fractionPartBegin = p;
      while (p != s.end && isDigit(*p))
      {
         ++p;
      }
      fractionPartEnd = p;
   }

   // this case happens if the number contains no digits (e.g. "-", ".", "-.", ".e5")
   if (
      integerPartBegin == integerPartEnd &&
      fractionPartBegin == fractionPartEnd)
   {
      return false;
   }

   // consume the exponent
   if (*p == 'e' || *p == 'E')
   {
      ++p;

      // consume exponent sign
      if (*p == '-' || *p == '+')
      {
         ++p;
      }

      const char* exponentPartBegin = p;
      while (p != s.end && isDigit(*p))
      {
         ++p;
      }
      const char* exponentPartEnd = p;

      // this case happens if there are no digits (or invalid characters)
      // following the exponent (e.g. "1e", "5e-")
      if (exponentPartBegin == exponentPartEnd)
      {
         return false;
      }
   }

   // some characters are leftover - this means the number contained an invalid
   // character somewhere
   if (p != s.end)
   {
      return false;
   }

   return true;
}


static bool validateLatDD(StringSlice s)
{
   const char* p = s.begin;

   bool hasFraction = false;
   bool maxValue = false;

   // consume the degrees
   const char* degPartBegin = p;
   while (p != s.end && isDigit(*p))
   {
      ++p;
   }
   const char* degPartEnd = p;

   const char* fractionPartEnd = degPartEnd;
   const char* fractionPartBegin = fractionPartEnd;
   if (*p == '.')
   {
      ++p;
      hasFraction = true;

      fractionPartBegin = p;
      while (p != s.end && isDigit(*p))
      {
         ++p;
      }
      fractionPartEnd = p;
   }

   // consume latitude direction
   if (*p == 'n' || *p == 'N' || *p == 's' || *p == 'S')
   {
      ++p;

      auto degLen = degPartEnd - degPartBegin;

      // if degrees magnitude is greater than 90, error
      if (
         (degLen > 3) ||
         (degLen == 3 && (*degPartBegin != '0' || (*(degPartBegin + 1) == '9' && *(degPartBegin + 2) != '0'))) ||
         (degLen == 2 && *degPartBegin == '9' && *(degPartBegin + 1) != '0')
      )
      {
         return false;
      }

      if (
         (degLen == 2 && *degPartBegin == '9') ||
         (degLen == 3 && *(degPartBegin + 1) == '9')
      )
      {
         maxValue = true;
      }
   }
   // unrecognized direction (probably a longitude, eE/wW
   else
   {
      return false;
   }

   if (
      (degPartEnd == degPartBegin) ||
      (hasFraction && fractionPartEnd == fractionPartBegin)
   )
   {
      return false;
   }

   // if deg = 90, ensure that all fractional part is 0
   if (maxValue)
   {
      for (auto c = fractionPartBegin; c < fractionPartEnd; ++c)
      {
         if (*c != '0')
         {
            return false;
         }
      }
   }
   // some characters are leftover - this means the slice contained an invalid
   // character somewhere
   if (p != s.end)
   {
      return false;
   }

   return true;
}

static bool validateLongDD(StringSlice s)
{
   const char* p = s.begin;

   bool hasFraction = false;
   bool maxValue = false;

   // consume the degrees
   const char* degPartBegin = p;
   while (p != s.end && isDigit(*p))
   {
      ++p;
   }
   const char* degPartEnd = p;

   const char* fractionPartEnd = degPartEnd;
   const char* fractionPartBegin = fractionPartEnd;
   if (*p == '.')
   {
      ++p;
      hasFraction = true;

      fractionPartBegin = p;
      while (p != s.end && isDigit(*p))
      {
         ++p;
      }
      fractionPartEnd = p;
   }

   // consume longitude direction
   if (*p == 'e' || *p == 'E' || *p == 'w' || *p == 'W')
   {
      ++p;
   }
   else    // unrecognized direction character, probably latitude nN/sS
   {
      return false;
   }
   // if degrees magnitude is greater than 180, error
   if (
      (degPartEnd - degPartBegin > 3) ||
      (degPartEnd - degPartBegin == 3 && (*degPartBegin > '1' || (*degPartBegin == '1' && (*(degPartBegin + 1) > '8' || (*(degPartBegin + 1) == '8' && *(degPartBegin + 2) > '0')))))
   )
   {
      return false;
   }

   if (degPartEnd - degPartBegin == 3 && *degPartBegin == '1' && *(degPartBegin + 1) == '8')
   {
      maxValue = true;
   }

   if (
      (degPartEnd == degPartBegin) ||
      (hasFraction && fractionPartEnd == fractionPartBegin)
   )
   {
      return false;
   }

   // if deg = 90, ensure that all fractional part is 0
   if (maxValue)
   {
      for (auto c = fractionPartBegin; c < fractionPartEnd; ++c)
      {
         if (*c != '0')
         {
            return false;
         }
      }
   }
   // some characters are leftover - this means the slice contained an invalid
   // character somewhere
   if (p != s.end)
   {
      return false;
   }

   return true;
}


static bool validateLatitudeDMS(StringSlice s)
{
   const char* p = s.begin;

   bool hasMinutes = false;
   bool hasSeconds = false;
   bool hasFraction = false;
   bool maxValue = false;

   // consume the degrees
   const char* degPartBegin = p;
   while (p != s.end && isDigit(*p))
   {
      ++p;
   }
   const char* degPartEnd = p;

   // if a colon is present, consume the minutes
   const char* minPartBegin = p;
   const char* minPartEnd = p;
   if (*p == ':')
   {
      ++p;
      hasMinutes = true;

      minPartBegin = p;
      while (p != s.end && isDigit(*p))
      {
         ++p;
      }
      minPartEnd = p;
   }

   // if a colon is present, consume the seconds
   const char* secPartBegin = p;
   const char* secPartEnd = p;
   if (*p == ':')
   {
      ++p;
      hasSeconds = true;

      secPartBegin = p;
      while (p != s.end && isDigit(*p))
      {
         ++p;
      }
      secPartEnd = p;
   }

   // if a decimal is present, consume the fraction
   const char* fractionPartBegin = p;
   const char* fractionPartEnd = p;
   if (hasSeconds && *p == '.')
   {
      ++p;
      hasFraction = true;

      fractionPartBegin = p;
      while (p != s.end && isDigit(*p))
      {
         ++p;
      }
      fractionPartEnd = p;
   }

   // consume latitude direction
   if (*p == 'n' || *p == 'N' || *p == 's' || *p == 'S')
   {
      ++p;

      auto degLen = degPartEnd - degPartBegin;

      // if degrees magnitude is greater than 90, error
      if (
         (degLen > 3) ||
         (degLen == 3 && (*degPartBegin != '0' || (*(degPartBegin + 1) == '9' && *(degPartBegin + 2) != '0'))) ||
         (degLen == 2 && *degPartBegin == '9' && *(degPartBegin + 1) != '0')
      )
      {
         return false;
      }

      if (
         (degLen == 2 && *degPartBegin == '9') ||
         (degLen == 3 && *(degPartBegin + 1) == '9')
      )
      {
         maxValue = true;
      }
   }
   // unrecognized direction (probably a longitude, eE/wW
   else
   {
      return validateLatDD(s);
   }

   // this case happens if a portion of the number contains no digits
   if (
      (degPartEnd == degPartBegin) ||
      (hasMinutes && minPartEnd == minPartBegin) ||
      (hasSeconds && secPartEnd == secPartBegin) ||
      (hasFraction && fractionPartEnd == fractionPartBegin)
   )
   {
      return false;
   }

   // if minutes magnitude is greater than 59, error
   if (
      (minPartEnd - minPartBegin > 2) ||
      (minPartEnd - minPartBegin == 2 && *minPartBegin > '5')
   )
   {
      return false;
   }

   // if seconds magnitude is greater than 59, error
   if (
      (secPartEnd - secPartBegin > 2) ||
      (secPartEnd - secPartBegin == 2 && *secPartBegin > '5')
   )
   {
      return false;
   }

   // if degrees are max value, ensure remaining values are 0
   if (maxValue)
   {
      for (auto c = minPartBegin; c < minPartEnd; ++c)
      {
         if (*c != '0')
         {
            return false;
         }
      }

      for (auto c = secPartBegin; c < secPartEnd; ++c)
      {
         if (*c != '0')
         {
            return false;
         }
      }

      for (auto c = fractionPartBegin; c < fractionPartEnd; ++c)
      {
         if (*c != '0')
         {
            return false;
         }
      }
   }

   // some characters are leftover - this means the slice contained an invalid
   // character somewhere
   if (p != s.end)
   {
      return false;
   }

   return true;
}

static bool validateLongitudeDMS(StringSlice s)
{
   const char* p = s.begin;

   bool hasMinutes = false;
   bool hasSeconds = false;
   bool hasFraction = false;
   bool maxValue = false;

   // consume the degrees
   const char* degPartBegin = p;
   while (p != s.end && isDigit(*p))
   {
      ++p;
   }
   const char* degPartEnd = p;

   // if a colon is present, consume the minutes
   const char* minPartBegin = p;
   const char* minPartEnd = p;
   if (*p == ':')
   {
      ++p;
      hasMinutes = true;

      minPartBegin = p;
      while (p != s.end && isDigit(*p))
      {
         ++p;
      }
      minPartEnd = p;
   }

   // if a colon is present, consume the seconds
   const char* secPartBegin = p;
   const char* secPartEnd = p;
   if (*p == ':')
   {
      ++p;
      hasSeconds = true;

      secPartBegin = p;
      while (p != s.end && isDigit(*p))
      {
         ++p;
      }
      secPartEnd = p;
   }

   // if a decimal is present, consume the fraction
   const char* fractionPartBegin = p;
   const char* fractionPartEnd = p;
   if (hasSeconds && *p == '.')
   {
      ++p;
      hasFraction = true;

      fractionPartBegin = p;
      while (p != s.end && isDigit(*p))
      {
         ++p;
      }
      fractionPartEnd = p;
   }

   // consume longitude direction
   if (*p == 'e' || *p == 'E' || *p == 'w' || *p == 'W')
   {
      ++p;
   }
   else    // unrecognized direction character, probably latitutde nN/sS
   {
      return validateLongDD(s);
   }
   // if degrees magnitude is greater than 180, error
   if (
      (degPartEnd - degPartBegin > 3) ||
      (degPartEnd - degPartBegin == 3 && (*degPartBegin > '1' || (*degPartBegin == '1' && (*(degPartBegin + 1) > '8' || (*(degPartBegin + 1) == '8' && *(degPartBegin + 2) > '0')))))
   )
   {
      return false;
   }

   if (degPartEnd - degPartBegin == 3 && *degPartBegin == '1' && *(degPartBegin + 1) == '8')
   {
      maxValue = true;
   }


   // this case happens if a portion of the number contains no digits
   if (
      (degPartEnd == degPartBegin) ||
      (hasMinutes && minPartEnd == minPartBegin) ||
      (hasSeconds && secPartEnd == secPartBegin) ||
      (hasFraction && fractionPartEnd == fractionPartBegin)
   )
   {
      return false;
   }

   // if minutes magnitude is greater than 59, error
   if (
      (minPartEnd - minPartBegin > 2) ||
      (minPartEnd - minPartBegin == 2 && *minPartBegin > '5')
   )
   {
      return false;
   }

   // if seconds magnitude is greater than 59, error
   if (
      (secPartEnd - secPartBegin > 2) ||
      (secPartEnd - secPartBegin == 2 && *secPartBegin > '5')
   )
   {
      return false;
   }

   // if degrees are max value, ensure remaining values are 0
   if (maxValue)
   {
      for (auto c = minPartBegin; c < minPartEnd; ++c)
      {
         if (*c != '0')
         {
            return false;
         }
      }

      for (auto c = secPartBegin; c < secPartEnd; ++c)
      {
         if (*c != '0')
         {
            return false;
         }
      }

      for (auto c = fractionPartBegin; c < fractionPartEnd; ++c)
      {
         if (*c != '0')
         {
            return false;
         }
      }
   }
   // some characters are leftover - this means the slice contained an invalid
   // character somewhere
   if (p != s.end)
   {
      return false;
   }

   return true;
}
// TODO handle + or -, degrees symbols, apostrophes, and spaces
inline static bool validateDegreesMinutesSeconds(StringSlice s)
{
   const char* p = s.begin;

   bool hasMinutes = false;
   bool hasSeconds = false;
   bool hasFraction = false;
   bool maxValue = false;

   // consume the degrees
   const char* degPartBegin = p;
   while (p != s.end && isDigit(*p))
   {
      ++p;
   }
   const char* degPartEnd = p;

   // if a colon is present, consume the minutes
   const char* minPartBegin = p;
   const char* minPartEnd = p;
   if (*p == ':')
   {
      ++p;
      hasMinutes = true;

      minPartBegin = p;
      while (p != s.end && isDigit(*p))
      {
         ++p;
      }
      minPartEnd = p;
   }

   // if a colon is present, consume the seconds
   const char* secPartBegin = p;
   const char* secPartEnd = p;
   if (*p == ':')
   {
      ++p;
      hasSeconds = true;

      secPartBegin = p;
      while (p != s.end && isDigit(*p))
      {
         ++p;
      }
      secPartEnd = p;
   }

   // if a decimal is present, consume the fraction
   const char* fractionPartBegin = p;
   const char* fractionPartEnd = p;
   if (hasSeconds && *p == '.')
   {
      ++p;
      hasFraction = true;

      fractionPartBegin = p;
      while (p != s.end && isDigit(*p))
      {
         ++p;
      }
      fractionPartEnd = p;
   }

   // consume latitude direction
   if (*p == 'n' || *p == 'N' || *p == 's' || *p == 'S')
   {
      ++p;

      auto degLen = degPartEnd - degPartBegin;

      // if degrees magnitude is greater than 90, error
      if (
         (degLen > 3) ||
         (degLen == 3 && (*degPartBegin != '0' || (*(degPartBegin + 1) == '9' && *(degPartBegin + 2) != '0'))) ||
         (degLen == 2 && *degPartBegin == '9' && *(degPartBegin + 1) != '0')
      )
      {
         return false;
      }

      if (
         (degLen == 2 && *degPartBegin == '9') ||
         (degLen == 3 && *(degPartBegin + 1) == '9')
      )
      {
         maxValue = true;
      }
   }
   // consume longitude direction
   else
   {
      if (*p == 'e' || *p == 'E' || *p == 'w' || *p == 'W')
      {
         ++p;
      }

      // if degrees magnitude is greater than 180, error
      if (
         (degPartEnd - degPartBegin > 3) ||
         (degPartEnd - degPartBegin == 3 && (*degPartBegin > '1' || (*degPartBegin == '1' && (*(degPartBegin + 1) > '8' || (*(degPartBegin + 1) == '8' && *(degPartBegin + 2) > '0')))))
      )
      {
         return false;
      }

      if (degPartEnd - degPartBegin == 3 && *degPartBegin == '1' && *(degPartBegin + 1) == '8')
      {
         maxValue = true;
      }
   }

   // this case happens if a portion of the number contains no digits
   if (
      (degPartEnd == degPartBegin) ||
      (hasMinutes && minPartEnd == minPartBegin) ||
      (hasSeconds && secPartEnd == secPartBegin) ||
      (hasFraction && fractionPartEnd == fractionPartBegin)
   )
   {
      return false;
   }

   // if minutes magnitude is greater than 59, error
   if (
      (minPartEnd - minPartBegin > 2) ||
      (minPartEnd - minPartBegin == 2 && *minPartBegin > '5')
   )
   {
      return false;
   }

   // if seconds magnitude is greater than 59, error
   if (
      (secPartEnd - secPartBegin > 2) ||
      (secPartEnd - secPartBegin == 2 && *secPartBegin > '5')
   )
   {
      return false;
   }

   // if degrees are max value, ensure remaining values are 0
   if (maxValue)
   {
      for (auto c = minPartBegin; c < minPartEnd; ++c)
      {
         if (*c != '0')
         {
            return false;
         }
      }

      for (auto c = secPartBegin; c < secPartEnd; ++c)
      {
         if (*c != '0')
         {
            return false;
         }
      }

      for (auto c = fractionPartBegin; c < fractionPartEnd; ++c)
      {
         if (*c != '0')
         {
            return false;
         }
      }
   }

   // some characters are leftover - this means the slice contained an invalid
   // character somewhere
   if (p != s.end)
   {
      return false;
   }

   return true;
}

inline static bool validateToken(const StringSlice token, const ParserColumnDatatype dataType)
{
   switch (dataType)
   {
      case ParserColumnDatatype::String:
         return true;
      case ParserColumnDatatype::Identifier:
         return validateIdentifier(token);
      case ParserColumnDatatype::SignedInteger:
         return validateSignedInteger(token);
      case ParserColumnDatatype::UnsignedInteger:
         return validateUnsignedInteger(token);
      case ParserColumnDatatype::FractionalNumber:
         return validateFractionalNumber(token);
      case ParserColumnDatatype::Lat:
         return validateLatitudeDMS(token);
      case ParserColumnDatatype::Long:
         return validateLongitudeDMS(token);
      default:
         assert(false);
         return false;
   }
}

inline static bool parseCsv(
   StringSlice input, size_t& tokenCount, StringSlice* tokens)
{
   Parser parser;
   parser.cursor = input.begin;
   parser.end = input.end;
   parser.lineNumber = 1;
   parser.lineBegin = input.begin;
   parser.takeNextDelimiter = false;

   size_t maxTokens = tokenCount;
   tokenCount = 0;
   while (tokenCount < maxTokens)
   {
      StringSlice token = parserNextToken(parser, ',', true, false);
      if (isEmpty(token))
      {
         return true;
      }

      tokens[tokenCount] = token;
      ++tokenCount;
   }

   return false;
}

inline static std::vector<StringSlice> splitIntoLines(const StringSlice text)
{
   std::vector<StringSlice> textLines;
   Parser parser = { text.begin, text.end, 0, text.begin };

   while (parser.cursor != parser.end)
   {
      const char* begin = parser.lineBegin;
      parserConsumeLine(parser);
      textLines.emplace_back(begin, parser.cursor);
   }

   return textLines;
}

static bool contains(const StringSlice str1, const StringSlice str2)
{
   size_t str2Len = length(str2);
   int searchLen = static_cast<int>(length(str1)) - static_cast<int>(str2Len);

   for (int index1 = 0; index1 < searchLen; ++index1)
   {
      size_t index2 = 0;

      for (; index2 < str2Len; ++index2)
      {
         if (*(str1.begin + index1 + index2) != *(str2.begin + index2))
         {
            break;
         }
      }

      if (index2 == str2Len)
      {
         return true;
      }
   }

   return false;
}

// This needs to match filterTypeList
enum struct FilterType : int
{
   ExclusiveLineNumberFilter,
   InclusiveLineNumberFilter,
   ExclusiveMatchingTextFilter,
   InclusiveMatchingTextFilter,
};

struct FilterError : GenericError
{
   StringSlice textToHighlight;
   int filterIdx;
   FilterError(bool _isWarning, std::string _explanation, StringSlice _textToHighlight, int _filterIdx) :
      GenericError(_isWarning, _explanation),
      textToHighlight(_textToHighlight),
      filterIdx(_filterIdx) {}
};

class Filter
{
   public:

      virtual ~Filter()
      {}

      Filter& operator=(const Filter&) = delete;

      bool GetExclusionary() const { return exclusionary; }
      std::string GetSpecString(bool saving) const
      {
         if (!saving)
         {
            return specString;
         }
         QStringList splitOnCommas = QString::fromStdString(specString).split(",");
         std::string resSpecString = splitOnCommas.join(" comma ").toStdString();
         return resSpecString;
      }

      //COMMENT (DRB): Making this a virtual function means a list of filters has to
      // be stored as pointers. Consider using a discriminated union, which would
      // allow a list of filters to be stored in a single contiguous memory block.
      //COMMENT (DRB): This filter interface requires making multiple passes over the
      // data to be filtered, which is not cache-efficient. A better interface may be:
      //
      // bool removeLine(StringSlice line, FilterError& error);
      //
      // This interface permits each line to be kept in cache while we test whether
      // or not to filter it. It also does not enforce that all lines be stored in a
      // std::vector, giving the caller more freedom in data structure choice.
      virtual void performFilter(std::vector<StringSlice>& textLines, std::vector<FilterError>& errors, int filterIdx) const = 0;

   protected:

      Filter(const bool exclusive, const std::string& spec)
         : exclusionary(exclusive)
         , specString(spec)
      {}

      const bool exclusionary;

      //COMMENT (DRB): Storing the filter specification as a string requires parsing
      // the string in the main filtering loop. This is a lot of redundant work, and
      // adds complexity to the filtering code. This field should be specialized for
      // each filter type. For a line filter, it should be two integers, a starting
      // and ending line. For a text filter, it should be some kind of text. The GUI
      // could be similarly specialized; for a line filter, there could be separate
      // widgets for specifying the starting and ending line.
      // TODO Eventually, create individual config widgets for different filter types
      std::string specString;
};

class LineNumberFilter : public Filter
{
   public:

      LineNumberFilter(const bool exclusive, const std::string& spec)
         : Filter(exclusive, spec)
      {}

      ~LineNumberFilter() override = default;

      LineNumberFilter& operator=(const LineNumberFilter&) = delete;

      // Performs the filter, removing the appropriate StringSlices from textLines
      void performFilter(std::vector<StringSlice>& textLines, std::vector<FilterError>& errors, int filterIdx) const override
      {
         //COMMENT (DRB): The way the line number filter is designed, the line number
         // changes meaning depending on which filters come before it. For example,
         // applying two exclusionary filters for line number 1 removes both line number
         // 1 and 2, rather than just removing line number 1. Is this a good behavior?
         const auto numLines = textLines.size();
         //COMMENT (DRB): As far as I can tell, this boolean array adds no benefit. Why
         // not remove a line directly once it is discovered that it matches the filter?
         // Even better, since line filters are defined as ranges already, why not
         // remove the whole line range all at once, rather than checking lines one at
         // time? For example, if the line filter is to exclude lines 15-20, the range
         // of indices 14-19 could be removed from the vector directly, in one shot.
         auto filterLines = ut::make_unique<bool[]>(numLines);
         bool noErrors = true;

         // Parse the specString
         for (size_t tokenBegin = 0; tokenBegin < specString.size();)
         {
            std::string token = specString.substr(tokenBegin, specString.find(',', tokenBegin) - tokenBegin);

            // Interpret the token, which can be of the form " -2, 4,6-8, 10-" signalling lines to be excluded/included would be 1,2,4,6,7,8,10,11...
            std::string firstNum, lastNum;
            bool hyphenated = false;
            for (size_t index = 0; index < token.size(); ++index)
            {
               const char c = token[index];

               // Digits should be accepted
               if (isDigit(c))
               {
                  (hyphenated ? lastNum : firstNum) += c;
               }
               // Hyphens indicate a range
               else if (c == '-')
               {
                  if (hyphenated)
                  {
                     errors.emplace_back(false, std::string("Duplicate hyphen."), stringSliceFromCString(token.c_str()), filterIdx);
                     noErrors = false;
                  }
                  else
                  {
                     hyphenated = true;
                  }
               }
               // Spaces are ignored; all other characters indicate an error
               else if (c != ' ')
               {
                  errors.emplace_back(false, std::string("Invalid character: " + std::string(token)), stringSliceFromCString(token.c_str()), filterIdx);
                  noErrors = false;
               }
            }

            size_t index, last;

            if (firstNum.empty())
            {
               if (hyphenated)
               {
                  index = 1;
               }
               else
               {
                  // TODO add warnings and make this a warning
                  errors.emplace_back(true, std::string("Empty token."), stringSliceFromCString(token.c_str()), filterIdx);
                  tokenBegin += token.size() + 1;
                  continue;
               }
            }
            else
            {
               index = std::stoull(firstNum);
            }

            if (hyphenated)
            {
               if (lastNum.empty())
               {
                  last = numLines;
               }
               else
               {
                  last = std::stoull(lastNum);
               }
            }
            else
            {
               last = index;
            }

            if (index < 1)
            {
               errors.emplace_back(false, "Invalid beginning index: " + std::string(token), stringSliceFromCString(token.c_str()), filterIdx);
               noErrors = false;
               index = 1;
            }
            else if (index > numLines)
            {
               errors.emplace_back(false, "Invalid beginning index, out of input range: " + std::string(token), stringSliceFromCString(token.c_str()), filterIdx);
               noErrors = false;
               index = numLines;
            }

            if (last < 1)
            {
               errors.emplace_back(false, "Invalid ending index: " + std::string(token), stringSliceFromCString(token.c_str()), filterIdx);
               noErrors = false;
               last = 1;
            }
            else if (last > numLines)
            {
               errors.emplace_back(false, "Invalid ending index, out of input range: " + std::string(token), stringSliceFromCString(token.c_str()), filterIdx);
               noErrors = false;
               last = numLines;
            }

            // Adjust index by 1 for array indexing, then iterate through indeces
            for (--index; index < last; ++index)
            {
               if (filterLines[index])
               {
                  // TODO add warnings and make this a warning
                  //COMMENT (DRB): std::string concatenation is very inefficient. It requires
                  // reallocating and copying memory for every concatenated element.
                  errors.emplace_back(true, "Line " + std::to_string(index + 1) + " already filtered.", stringSliceFromCString(token.c_str()), filterIdx);
               }
               else
               {
                  filterLines[index] = true;
               }
            }

            tokenBegin += token.size() + 1;
         }

         if (noErrors && !specString.empty())
         {
            // Erase the lines marked for exclusion
            size_t index = numLines;
            for (;;)
            {
               if (index == 0)
               {
                  break;
               }
               --index;

               if (filterLines[index] == exclusionary)
               {
                  textLines.erase(textLines.begin() + static_cast<int64_t>(index));
               }
            }
         }
      }
};

class MatchingTextFilter : public Filter
{
   public:

      MatchingTextFilter(const bool exclusive, const std::string& spec)
         : Filter(exclusive, spec)
      {}

      ~MatchingTextFilter() override
      {}

      MatchingTextFilter& operator=(const MatchingTextFilter&) = delete;

      // Performs the filter, removing the appropriate StringSlices from textLines
      void performFilter(std::vector<StringSlice>& textLines, std::vector<FilterError>&, int filterIdx) const override
      {
         //COMMENT (DRB): From the docs for std::string: "There are no guarantees that a
         // null character terminates the character sequence pointed by the value
         // returned by this function. See string::c_str for a function that provides
         // such guarantee."
         const StringSlice specSlice = StringSlice{ specString.data(), specString.data() + specString.length() };
         const auto numLines = textLines.size();
         auto filterLines = ut::make_unique<bool[]>(numLines);
         //bool noErrors = true;

         // Iterate through each line and mark each line which includes the specified text for filtering
         for (size_t i = 0; i < numLines; ++i)
         {
            filterLines[i] = contains(textLines[i], specSlice);
         }

         // TODO if no lines filtered or all lines filtered, print a warning

         if (!specString.empty())
         {
            // Erase the lines marked for exclusion
            size_t index = numLines;
            for (;;)
            {
               if (index == 0)
               {
                  break;
               }
               --index;

               if (filterLines[index] == exclusionary)
               {
                  textLines.erase(textLines.begin() + static_cast<int64_t>(index));
               }
            }
         }
      }
};
}
