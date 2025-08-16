// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

namespace ScenarioImporter
{
// Writes slice to buffer at position of cursor and then sets cursor to the element after the end of the write.
// Returns the length of the written StringSlice.
inline static size_t write(StringSlice slice, char*& cursor)
{
   const size_t len = length(slice);
   std::memcpy(cursor, slice.begin, len);
   cursor += len;
   return len;
}

#ifndef NOQT
// Appends slice to buffer.
static void write(StringSlice slice, QByteArray& buffer)
{
   buffer.append(slice.begin, static_cast<int>(length(slice)));
}
#endif

// Struct for error output, allows re-display of only relevant error types
struct ErrorStrings
{
   QString filterErrorString;
   QString parserErrorString;
   QString templateErrorString;
   QString fileNameErrorString;
};

// A struct for information pertaining to output template errors
struct TemplateError : GenericError
{
   // QString indices to highlight when using error navigation
   int begin;
   int end;

   TemplateError(bool _isWarning, std::string _explanation, int _begin, int _end) :
      GenericError(_isWarning, _explanation),
      begin(_begin),
      end(_end) {}
};


// TODO discuss should this be a member function of Platform?
// Returns attribute value associated with key; if key is not found, returns empty StringSlice.
inline static StringSlice getAttribute(std::string key, Record record)
{
   auto value = record.find(key);
   if (value != record.end())
   {
      return value->second;
   }

   return stringSliceFromCString("");
}

enum struct dmsType : int
{
   lat = 0,
   lon = 1,
   heading = 2
};

// Takes a pre-validated DMS string of the form: dd:mm::ss:n/s/e/w
// and converts it to a 0 - 2pi angle with respect to north
static double radFromDMSSlice(StringSlice input, dmsType type)
{
   double degrees;
   double minutes;
   double seconds;
   double angleRes = 0.0;
   QStringList components = QString::fromStdString(stdStringFromStringSlice(input)).split(":");
   QStringList fractionComponents = QString::fromStdString(stdStringFromStringSlice(input)).split(".");
   if (components.length() == 3)
   {
      degrees = std::atof(components[0].toStdString().c_str());
      minutes = std::atof(components[1].toStdString().c_str());
      seconds = std::atof(components[2].toStdString().c_str()); // should ignore first non-decimal character n/e/s/w
      angleRes = degrees + minutes / 60.0 + seconds / 3600.0;
   }
   // 65.8543543n deg/frac
   else if (fractionComponents.length() == 2)
   {
      angleRes = std::atof(stdStringFromStringSlice(input).c_str());
   }
   char direction = ' ';
   for (const char* c = input.end; c > input.begin; c--) // find n/e/s/w manually
   {
      if (*c == ' ' || *c == '\t' || *c == '\n') // skip trailing spaces
      {
         continue;
      }
      if (isDigit(stdStringFromStringSlice(StringSlice{ c - 1, c })[0]))
      {
         direction = *c;
         break;
      }
   }
   if (type == dmsType::lat)
   {
      // Lats converted from -90 to 90 to take arcsin
      if (!(direction == QString("n") || direction == QString("N")))
      {
         angleRes = -angleRes;
      }
   }
   else if (type == dmsType::lon)
   {
      // Longs converted to -180 to 180
      if (!(direction == QString("e") || direction == QString("E")))
      {
         angleRes = -angleRes;
      }
   }
   else if (type == dmsType::heading)
   {
      // Headings converted to 0 to 360, 45:00:00w = 135deg
      if (direction == QString("e") || direction == QString("E"))
      {
         angleRes += 90.0;
      }
      else if (direction == QString("s") || direction == QString("S"))
      {
         angleRes += 180.0;
      }
      else if (direction == QString("w") || direction == QString("W"))
      {
         angleRes += 270.0;
      }
      // otherwise it was North, add nothing
   }
   return angleRes * M_PI / 180.0;
}

// Take an angle in radians and convert it to the proper
// DMS StringSlice, which can be written to a record
static std::string dmsSliceFromRad(double angle, dmsType type)
{
   double degAngle = angle * 180.0 / M_PI;
   char dir;
   if (type == dmsType::lat)
   {
      (degAngle >= 0) ? dir = 'n' : dir = 's';
   }
   else if (type == dmsType::lon)
   {
      (degAngle >= 0) ? dir = 'e' : dir = 'w';
   }
   else // shouldn't happen
   {
      dir = 'x';
   }
   // These round down, add some smudging factor (
   degAngle = std::abs(degAngle);
   int deg = static_cast<int>(degAngle);
   int min = static_cast<int>((degAngle - deg) * 60.0);
   int sec = static_cast<int>((degAngle - (deg + min / 60.0)) * 3600.0);
   std::string res;
   (deg < 10) ? res += "0" + std::to_string(deg) + ":" : res += std::to_string(deg) + ":";
   (min < 10) ? res += "0" + std::to_string(min) + ":" : res += std::to_string(min) + ":";
   (sec < 10) ? res += "0" + std::to_string(sec) + dir : res += std::to_string(sec) + dir;
   return res;
}

// Takes an already verified DMS heading, from 0 - 360
// A distance, of units "units" which refers to the enum stored in UtUnitLength class
// and two lat/long tokens which have yet to be verified
// and translates the lat/long along the great circle for distance.
inline static std::pair<std::string, std::string> getTranslatedLatLong(double heading, double distance, StringSlice latToken, StringSlice longToken, int units)
{
   double radHeading = heading * M_PI / 180.0;
   double radLatitude = radFromDMSSlice(latToken, dmsType::lat);
   double radLongitude = radFromDMSSlice(longToken, dmsType::lon);

   // Convert to meters then to nautical miles then to degrees with provided UtUnits functions
   double standardDistance = UtUnitLength::ConvertToStandard(distance, units);
   double nmDistance = UtUnitLength::ConvertFromStandard(standardDistance, UtUnitLength::mUnitStrings.find("nm")->second);
   double radDistance = (M_PI / (180.0 * 60.0)) * nmDistance;

   double latRes;
   double longRes;
   latRes = asin(sin(radLatitude) * cos(radDistance) + cos(radLatitude) * sin(radDistance) * cos(radHeading));
   double dLon = atan2(sin(radHeading) * sin(radDistance) * cos(radLatitude), cos(radDistance) - sin(radLatitude) * sin(latRes));
   if (radHeading < M_PI)
   {
      longRes = fmod(radLongitude + dLon + M_PI, 2 * M_PI) - M_PI;
   }
   else
   {
      longRes = fmod(radLongitude - std::abs(dLon) + M_PI, 2 * M_PI) - M_PI;
   }

   // right angle cases to prevent the 59:59:59 rounding occurences
   if (radHeading == 0.0 || radHeading == M_PI) // north/south
   {
      return std::make_pair(dmsSliceFromRad(latRes, dmsType::lat), stdStringFromStringSlice(longToken));
   }
   else if (radHeading == M_PI / 2.0 || radHeading == 3.0 * M_PI / 2.0) // east/west
   {
      return std::make_pair(stdStringFromStringSlice(latToken), dmsSliceFromRad(longRes, dmsType::lon));
   }
   return std::make_pair(dmsSliceFromRad(latRes, dmsType::lat), dmsSliceFromRad(longRes, dmsType::lon));
}


class OutputTemplate
{

      // Forward Declarations
      struct ConditionalExpression;
      struct ConditionalTerm;
      struct RelationalTerm;
      struct RelationalExpression;

      // A base class from which syntax tokens must inherit
      class SyntaxToken
      {
         protected:
            explicit SyntaxToken(const StringSlice syntaxToken)
               : token(syntaxToken)
            {}

            StringSlice token;

         public:

            virtual ~ SyntaxToken()
            {}

            //COMMENT (DRB): This structure requires a virtual function call for every CSV
            // field to be written. This could be avoided.
            virtual StringSlice getStringSlice(const Record& record) const = 0;
      };


      // A child class into which string literal tokens may be stored for output
      class StringToken : public SyntaxToken
      {
         public:
            explicit StringToken(const StringSlice syntaxToken)
               : SyntaxToken(syntaxToken)
            {}

            ~StringToken() override = default;

            inline StringSlice getStringSlice(const Record&) const override
            {
               return token;
            }
      };


      // A child class into which attribute key tokens may be stored for attribute values to be output
      class AttributeToken : public SyntaxToken
      {
         public:
            explicit AttributeToken(const StringSlice& syntaxToken)
               : SyntaxToken(syntaxToken)
            {}

            ~AttributeToken() override = default;

            inline StringSlice getStringSlice(const Record& record) const override
            {
               return getAttribute(std::string(token.begin, length(token)), record);
            }
      };

      // A child class into which conditional tokens may be stored which contain their own
      // child vectors of tokens of all types
      class ConditionalToken : public SyntaxToken
      {
         public:
            explicit ConditionalToken(const StringSlice syntaxToken)
               : SyntaxToken(syntaxToken)
            {
               parentToken = nullptr;
               isAnIfConditional = false;
               finalConditional = false;
               caseState = false;
               literalConditionalCount = 0;
            }

            ~ConditionalToken() override = default;

            ConditionalToken* parentToken; // allows specifySyntax to return to parent conditionals
            std::unique_ptr<RelationalTerm> rootNode; // the return from parseConditional() yields a tree structure to be evauluated, this is the root
            bool isAnIfConditional; // for ifs
            bool finalConditional; // for elses
            bool caseState; // allows specifySyntax to return to parent conditionals in the proper state
            int literalConditionalCount;
            std::vector<std::unique_ptr<SyntaxToken>> trueCaseTokens;
            std::vector<std::unique_ptr<SyntaxToken>> falseCaseTokens;
            inline StringSlice getStringSlice(const Record& record) const override // should not be used
            {
               return token;
            }



      };

      // A child class for token functions: [toUpper(AttributeToken)], [toUpper(AttributeToken, int count)] and toLower
      class ToCaseToken : public SyntaxToken
      {
         public:
            explicit ToCaseToken(const StringSlice syntaxToken)
               : SyntaxToken(syntaxToken)
            {
               count = 0;
               toUpper = true;
            }

            ~ToCaseToken() override = default;

            int count; // When parsing, should be set to the length of the token if no argument was given
            bool toUpper; // toUpper = true, toLower = false, effects writeRecord interpretation.
            inline StringSlice getStringSlice(const Record& record) const override
            {
               return getAttribute(std::string(token.begin, length(token)), record);
            }
      };

      // Child class for random function [random(start, end)]
      // Random generator is seeded when the token is called to write from writeRecord
      class RandomToken : public SyntaxToken
      {
         public:
            explicit RandomToken(const StringSlice syntaxToken)
               : SyntaxToken(syntaxToken)
            {
               start = 0.0;
               end = 1.0;
            }

            ~RandomToken() override = default;

            double start;
            double end;
            inline StringSlice getStringSlice(const Record& record) const override
            {
               return token;
            }
      };

      // A child class for token function: [translate(Heading, Distance, inputLat, inputLong)] It can be generated:
      // Using randoms                     [translate(random(0,360), random(0,20)km, [lat_dms], [long_dms])]
      // Using values                      [translate(265.54, 19km, [lat_dms], [long_dms])]
      // Its write/getStringSlice should print:
      // DD:MM:SS:n/s DD:MM:SS:e/w  <--- the translated latlong position
      class TranslateToken : public SyntaxToken
      {
         public:
            explicit TranslateToken(const StringSlice syntaxToken)
               : SyntaxToken(syntaxToken)
            {
               heading = 0.0;
               distance = 0.0;
               units = 0;
            }

            ~TranslateToken() override = default;

            double heading; // 0 - 360 from N
            double distance; // in units

            std::unique_ptr<RandomToken> randHeading;
            std::unique_ptr<RandomToken> randDistance;

            std::unique_ptr<AttributeToken> latToken;
            std::unique_ptr<AttributeToken> longToken;
            int units; // distance units specified when parsing, should default to 0:km

            inline StringSlice getStringSlice(const Record& record) const override
            {
               return token;
            }
      };

      ///////////////////////////////////
      // Boolean Expression Enum Types //
      ///////////////////////////////////

      enum struct TermTypeEnum : int
      {
         stringTerm = 0, // base case
         floatTerm = 1, // base case
         tokenTerm = 2, // base case
         conditionalTerm = 3, // children
         unknown = 4, // for parsing, should not be added to the tree
      };
      enum struct RelationTypeEnum
      {
         conditionalExpression,
         relationalExpression
      };
      enum struct cOperator : int
      {
         equal = 0,        // ==
         notEqual = 1,     // !=
         less = 2,         // <
         lessEqual = 3,    // <=
         greater = 4,      // >
         greaterEqual = 5, // >=
         exists = 6,       // [token]
         unassigned = 7, // for parsing, should not be added to the tree
      };

      enum struct cRelation
      {
         andRelation = 0,    // &&
         orRelation = 1,     // ||
      };

      ///////////////////////////////////
      // Boolean Expression Tree Types //
      ///////////////////////////////////

      struct ConditionalTerm
      {
         explicit ConditionalTerm(TermTypeEnum type) :
            termType(type), stringSliceTerm(StringSlice{ nullptr, nullptr }), floatTerm(StringSlice{ nullptr, nullptr })
         {}

         TermTypeEnum termType;

         StringSlice stringSliceTerm;                             // base case
         StringSlice floatTerm;                                   // base case
         std::unique_ptr<AttributeToken> tokenTerm;               // base case
      };

      struct ConditionalExpression
      {
         explicit ConditionalExpression(cOperator _op) : op(_op) {}
         std::unique_ptr<ConditionalTerm> leftTermPtr;
         std::unique_ptr<ConditionalTerm> rightTermPtr;
         cOperator op;                                         // ==, !=, <, >, . . . etc.
      };

      struct RelationalTerm // root node, can own more of itself through RelationalExpression
      {
         explicit RelationalTerm(RelationTypeEnum type) : relationType(type) {}
         RelationTypeEnum relationType;

         std::unique_ptr<ConditionalExpression> conditionalExpressionPtr;
         std::unique_ptr<RelationalExpression> relationalExpressionPtr;
      };

      struct RelationalExpression
      {
         explicit RelationalExpression(cRelation _op) : op(_op) {}
         std::vector<std::unique_ptr<RelationalTerm>> relationalTerms;
         cRelation op;                                         // &&, ||
      };

      ///////////////////////////////////
      //////// Parser State Enums ///////
      ///////////////////////////////////

      enum struct ParserState : int
      {
         open = 0,
         inToken = 1,
         postConditional = 4,
         inBlock = 5,
         postBlock = 6,
         postElseConditional = 7,
         inElseBlock = 8,
         grabbingFunction = 9
      };

      enum struct ConditionalParserState : int
      {
         waitingForOperator = 0,
         inToken = 1,
         postLeftToken = 2,
         waitingForRelation = 3,
         end = 4
      };

      enum struct ToUpperParserState : int
      {
         start = 0,
         inToken = 1,
         postToken = 2,
         inArg = 3,
         postArg = 4,
         end = 5,
         closing = 6
      };

      enum struct TranslateParserState : int
      {
         heading = 0,
         distance = 1,
         lat = 2,
         inLatToken = 3,
         lon = 4,
         inLongToken = 5,
         lookForComma = 6,
         end = 7,
         closing = 8
      };

      enum struct RandomParserState : int
      {
         start = 0,
         comma = 1,
         end = 2,
         closing = 3
      };

   public:

      // A vector of StringTokens and AttributeTokens generated from the syntaxSpecification
      //  passed into specifySyntax. When writeRecord is called for a passed in Record, these
      //  tokens will be iterated through in order: StringTokens will be directly written;
      //  AttributeTokens will be used as a key to obtain and write the associated value found
      //  in the Record.
      //  Kyle: I've made it public out of conveniance for access when writing records
      std::vector<std::unique_ptr<SyntaxToken>> tokens;

      // Recursive writeRecord for conditional token trees. Returns early if one of the conditions was found to have an error in it so we don't try and write them all
      // Writes the record to the end of the QByteArray according to the specified output template and conditionals
      bool writeRecord(const Record& record, QByteArray& buffer, std::vector<TemplateError>& errors, int& num, std::vector<std::unique_ptr<SyntaxToken>>& tokens, ut::Random& random, bool caseState)
      {
         for (auto const& token : tokens)
         {

            ConditionalToken* conditionalToken = dynamic_cast<ConditionalToken*>(token.get());
            ToCaseToken* toCaseToken = dynamic_cast<ToCaseToken*>(token.get());
            TranslateToken* translateToken = dynamic_cast<TranslateToken*>(token.get());
            RandomToken* randomToken = dynamic_cast<RandomToken*>(token.get());
            // If its a ConditionalToken, evaulate its condition and recurse
            if (conditionalToken != nullptr)
            {
               // indexing for error navigation
               if (conditionalToken->isAnIfConditional || conditionalToken->finalConditional == true)
               {
                  num += 4; // [if(     else
               }
               else
               {
                  num += 7; // else if
               }
               // else statements dont get a conditional expression tree, they are just true
               bool res;
               if (conditionalToken->finalConditional == true)
               {
                  res = true;
               }
               else
               {
                  res = evaluateConditional(conditionalToken->rootNode.get(), record, errors, num);
               }
               if (res)
               {
                  // if we evaluate true and are currently writing false case tokens, we want to ignore any additional
                  // false case tokens (skip additional else if statements)
                  if (caseState == true)
                  {
                     writeRecord(record, buffer, errors, num, conditionalToken->trueCaseTokens, random, true);
                  }
                  else // else if or else statments
                  {
                     writeRecord(record, buffer, errors, num, conditionalToken->trueCaseTokens, random, true);
                     return (errors.size() == 0);
                  }
               }
               else
               {
                  writeRecord(record, buffer, errors, num, conditionalToken->falseCaseTokens, random, false);
               }
            }

            else if (toCaseToken != nullptr)
            {
               auto initialStringSlice = toCaseToken->getStringSlice(record);
               std::string res = std::string(initialStringSlice.begin, length(initialStringSlice));
               if (toCaseToken->count != 0)
               {
                  if (toCaseToken->count > 0)
                  {
                     for (int i = 0; i < toCaseToken->count && ut::cast_to_size_t(i) < length(initialStringSlice); i++)
                     {
                        (toCaseToken->toUpper == true) ? res[i] = toupper(res[i]) : res[i] = tolower(res[i]);
                     }
                  }
                  else
                  {
                     for (size_t i = length(initialStringSlice) - 1; i >= length(initialStringSlice) - std::abs(toCaseToken->count); i--)
                     {
                        (toCaseToken->toUpper == true) ? res[i] = toupper(res[i]) : res[i] = tolower(res[i]);
                     }
                  }

               }
               else
               {
                  for (auto& c : res)
                  {
                     c = toupper(c);
                  }
               }
               write(stringSliceFromCString(res.c_str()), buffer);
            }
            else if (translateToken != nullptr)
            {
               // Replace distance and heading stringslices with random valued stringslices
               // if their respective random tokens are not null
               if (translateToken->randDistance.get() != nullptr)
               {
                  float randomMapped = random.Uniform(translateToken->randDistance.get()->start, translateToken->randDistance.get()->end);
                  translateToken->distance = randomMapped;
               }
               if (translateToken->randHeading.get() != nullptr)
               {
                  float randomMapped = random.Uniform(translateToken->randHeading.get()->start, translateToken->randHeading.get()->end);
                  translateToken->heading = randomMapped;
               }
               if (!validateLatitudeDMS(translateToken->latToken->getStringSlice(record)) || !validateLongitudeDMS(translateToken->longToken->getStringSlice(record)))
               {
                  errors.emplace_back(false, std::string("Improperly formatted lat/long from template input for function 'translate'"), num, num + 1);
                  return false;
               }
               auto initialStringSlice = token->getStringSlice(record); // get the stored "token" looks like (heading, dist, lat, long) for its start c* position
               auto latLongStringPair = getTranslatedLatLong(translateToken->heading, translateToken->distance, translateToken->latToken->getStringSlice(record), translateToken->longToken->getStringSlice(record), translateToken->units);
               std::string temp = latLongStringPair.first + " " + latLongStringPair.second;
               std::string res = std::string(initialStringSlice.begin, temp.length()); // Reserve a string whose c_str begins at the start of the token with the length of the resulting latLong string to be written
               for (size_t i = 0; i < res.length(); i++)
               {
                  res[i] = temp[i];
               }
               write(stringSliceFromCString(res.c_str()), buffer);
            }
            else if (randomToken != nullptr)
            {
               float randomMapped = random.Uniform(randomToken->start, randomToken->end);
               write(stringSliceFromCString(std::to_string(randomMapped).c_str()), buffer);
            }
            // Otherwise we write the token as usual
            else
            {
               write(token->getStringSlice(record), buffer);
            }
            num += static_cast<int>(length(token->getStringSlice(record)));
         }
         return (errors.size() == 0);
      }

      // In cases where we enter an if statement block, we want to ensure that any characters between
      // the end of the conditional ')' and the start of the block '{' are only spaces, tabs, newlines, etc.
      // and then skip the cursor over them if they are valid or throw an error if they are not.
      void validateSpaceSlice(StringSlice slice, std::vector<TemplateError>& errors, int textBegin)
      {
         if (slice.begin > slice.end) // cases like:    if( ){    without a newline or space
         {
            return;
         }
         for (const char* c = slice.begin; c != slice.end; ++c)
         {
            if (!(*c == ' ' || *c == '\n' || *c == '\t'))
            {
               errors.emplace_back(false, std::string("Invalid characters before expected opening: {"), textBegin - 1, textBegin);
            }
         }
      }

      cOperator validateAndGetOperator(const char*& c, std::vector<TemplateError>& errors)
      {
         if (*c == '=')
         {
            if (*(c + 1) == '=')
            {
               c = c + 1;
               return cOperator::equal;
            }
            else
            {
               return cOperator::unassigned;
            }
         }
         else if (*c == '<')
         {
            if (*(c + 1) == '=')
            {
               c = c + 1;
               return cOperator::lessEqual;
            }
            else
            {
               return cOperator::less;
            }
         }
         else if (*c == '>')
         {
            if (*(c + 1) == '=')
            {
               c = c + 1;
               return cOperator::greaterEqual;
            }
            else
            {
               return cOperator::greater;
            }
         }
         else if (*c == '!')
         {
            if (*(c + 1) == '=')
            {
               c = c + 1;
               return cOperator::notEqual;
            }
            else
            {
               c = c + 1;
               return cOperator::unassigned;
            }
         }
         else { return cOperator::unassigned; }
      }

      // Terms can be a string, a number, or an attributeToken
      // Defaults to string if number is invalid
      std::unique_ptr<ConditionalTerm> validateAndGetTerm(StringSlice& slice, const Record& exampleRecord, std::vector<TemplateError>& errors, int textBegin)
      {
         TermTypeEnum resType = TermTypeEnum::unknown;
         StringSlice resSlice;
         resSlice.begin = slice.begin;
         resSlice.end = nullptr;
         bool sliceCompleted = false;
         for (const char* c = slice.begin; c != slice.end; ++c)
         {
            if (resType == TermTypeEnum::unknown)
            {
               if (isDigit(stdStringFromStringSlice(StringSlice{ c, c + 1 })[0]))
               {
                  resSlice.begin = c;
                  resType = TermTypeEnum::floatTerm;
               }
               else if (*c == '[')
               {
                  resSlice.begin = c + 1;
                  resType = TermTypeEnum::tokenTerm;
               }
               else if ((*c == ' ' || *c == '\t' || *c == '\n'))
               {
                  resSlice.begin = c + 1;
               }
               else if (!isDigit(stdStringFromStringSlice(StringSlice{ c, c + 1 })[0]))
               {
                  resType = TermTypeEnum::stringTerm;
               }
            }
            else if (resType == TermTypeEnum::stringTerm)
            {
               // skip the last space char before the && in: ([token] == string && . . .)
               if ((*c == ' ' || *c == '\t' || *c == '\n'))
               {
                  resSlice.end = c;
                  sliceCompleted = true;
               }
            }
            else if (resType == TermTypeEnum::floatTerm)
            {
               // skip over negatives,
               if (*c == '-')
               {
                  continue;
               }
               if ((*c == ' ' || *c == '\t' || *c == '\n'))
               {
                  sliceCompleted = true;
                  resSlice.end = c;
                  break;
               }
               else if (!isDigit(stdStringFromStringSlice(StringSlice{ c, c + 1 })[0]) && *c != '.')
               {
                  resType = TermTypeEnum::stringTerm;
               }
               else if (c == slice.end - 1)
               {
                  sliceCompleted = true;
                  resSlice.end = c + 1;
                  break;
               }
            }
            else if (resType == TermTypeEnum::tokenTerm)
            {
               if (*c == ']')
               {
                  resSlice.end = c;
                  if (isEmpty(getAttribute(stdStringFromStringSlice(resSlice), exampleRecord)))
                  {
                     int errorEnd = static_cast<int>(resSlice.end - slice.begin + textBegin);
                     int errorBegin = static_cast<int>(resSlice.begin - slice.begin + textBegin);
                     errors.emplace_back(false, std::string("Invalid identifier, must match a name from template input "), errorBegin, errorEnd);
                     return ut::make_unique<ConditionalTerm>(TermTypeEnum::unknown);
                  }
                  sliceCompleted = true;
               }
               else if (sliceCompleted == true)
               {
                  if (!(*c == ' ' || *c == '\t' || *c == '\n'))
                  {
                     int errorBegin = static_cast<int>(c - slice.begin + textBegin);
                     errors.emplace_back(false, std::string("Unexpected characters after token "), errorBegin, errorBegin + 1);
                     return ut::make_unique<ConditionalTerm>(TermTypeEnum::unknown);
                  }
               }
            }
         }
         if (!sliceCompleted && resType == TermTypeEnum::stringTerm)
         {
            resSlice.end = slice.end;
         }
         auto resTerm = ut::make_unique<ConditionalTerm>(resType);
         if (resType == TermTypeEnum::floatTerm)
         {
            if (!sliceCompleted)
            {
               resSlice.end = slice.end;
            }
            resTerm->floatTerm  = resSlice;
            return resTerm;
         }
         else if (resType == TermTypeEnum::stringTerm)
         {
            resTerm->stringSliceTerm = resSlice;
            return resTerm;
         }
         else if (resType == TermTypeEnum::tokenTerm)
         {
            if (!sliceCompleted)
            {
               int errorIdx = static_cast<int>(resSlice.begin - slice.begin + textBegin);
               errors.emplace_back(false, std::string("Opening token delimiter never closed "), errorIdx - 1, errorIdx);
               return ut::make_unique<ConditionalTerm>(TermTypeEnum::unknown);
            }
            resTerm->tokenTerm = std::move(ut::make_unique<AttributeToken>(resSlice));
            return resTerm;
         }
         else
         {
            errors.emplace_back(false, std::string("Unrecognized conditional term in expression "), textBegin - 1, textBegin);
            return ut::make_unique<ConditionalTerm>(TermTypeEnum::unknown);
         }
      }

      // Parses a conditional statement found after an if( or an else if(
      // Creates a tree structure of RelationalExpressions and child ConditionalExpressions,
      // and returns the root node of this tree to be added to the current ConditionalToken
      // and to be evaluated during the call to writeRecord
      std::unique_ptr<RelationalTerm> parseConditional(const char*& c, StringSlice syntaxSpecification, const Record& exampleRecord, std::vector<TemplateError>& errors, int& num)
      {
         // The root node to return always contains a list of && statements divided by ||
         // The vector could only have on term, which could only have one conditional;
         auto rootRelationalOrTerm = ut::make_unique<RelationalTerm>(RelationTypeEnum::relationalExpression);
         rootRelationalOrTerm->relationalExpressionPtr = std::move(ut::make_unique<RelationalExpression>(cRelation::orRelation));

         auto relationalAndTerm = ut::make_unique<RelationalTerm>(RelationTypeEnum::relationalExpression);
         relationalAndTerm->relationalExpressionPtr = std::move(ut::make_unique<RelationalExpression>(cRelation::andRelation));
         rootRelationalOrTerm->relationalExpressionPtr->relationalTerms.emplace_back(std::move(relationalAndTerm));

         // the current && expression, owned by the root || expression, and w/ vector of size one containing current relational condition
         RelationalExpression* currentRelationalExpressionPtr = rootRelationalOrTerm->relationalExpressionPtr->relationalTerms.back()->relationalExpressionPtr.get();

         auto relationalConditionTerm = ut::make_unique<RelationalTerm>(RelationTypeEnum::conditionalExpression);
         relationalConditionTerm->conditionalExpressionPtr = std::move(ut::make_unique<ConditionalExpression>(cOperator::unassigned));
         relationalConditionTerm->conditionalExpressionPtr->leftTermPtr = nullptr;
         relationalConditionTerm->conditionalExpressionPtr->rightTermPtr = nullptr;
         currentRelationalExpressionPtr->relationalTerms.emplace_back(std::move(relationalConditionTerm));

         // the current condition term, owned by the current relationalExpression and owning a blank conditionalExpressionPtr
         RelationalTerm* currentRelationalConditionPtr = currentRelationalExpressionPtr->relationalTerms.back().get();

         int textBegin = num;
         StringSlice slice;
         slice.begin = c;
         ConditionalParserState state = ConditionalParserState::waitingForOperator;
         for (; c != syntaxSpecification.end; ++c, ++num)
         {
            switch (state)
            {
               case ConditionalParserState::waitingForOperator: // look for operators, add left hand children
                  if (*c == '(')
                  {
                     // (~~~~~~~~~) statements are created as their own RelationalExpression and trigger a
                     // recursive call, the result gets added to the current list of && conditions
                     c = c + 1;
                     num = num + 1;
                     currentRelationalExpressionPtr->relationalTerms.emplace_back(std::move(parseConditional(c, syntaxSpecification, exampleRecord, errors, num)));
                     state = ConditionalParserState::waitingForRelation; // Left: (A > B)* &&      Right: A && (B || C)*     looking for && or ||
                     break;
                  }
                  // Operator tells us to assign currentLeftTermPtr from the slice to the left
                  else if ((*c == '=' || *c == '<' || *c == '>' || *c == '!') && (c + 1 < syntaxSpecification.end))
                  {
                     slice.end = c;
                     currentRelationalConditionPtr->conditionalExpressionPtr->leftTermPtr = std::move(validateAndGetTerm(slice, exampleRecord, errors, textBegin)); //returns a ConditionalTerm* or nullptr if error

                     // moves the cursor
                     auto op = validateAndGetOperator(c, errors);
                     if (op == cOperator::unassigned)
                     {
                        int errorBegin = static_cast<int>(c - syntaxSpecification.begin + textBegin);
                        errors.emplace_back(false, std::string("Unrecognized operator term, expected <, >, ==, etc. "), errorBegin, errorBegin + 1);
                     }
                     currentRelationalConditionPtr->conditionalExpressionPtr->op = op;
                     slice.begin = c + 1;
                     state = ConditionalParserState::waitingForRelation; // && to add new conditions to this expresion, || to start a new expression
                     break;
                  }
                  // Could see && and || relationals before an operator, which would be valid
                  // only in the case that the captured term is a token (interpreted as [token].exists() )
                  else if (*c == '&')
                  {
                     if (*(c + 1) == '&')
                     {
                        slice.end = c;
                        currentRelationalConditionPtr->conditionalExpressionPtr->leftTermPtr = std::move(validateAndGetTerm(slice, exampleRecord, errors, textBegin));
                        if (currentRelationalConditionPtr->conditionalExpressionPtr->leftTermPtr->termType != TermTypeEnum::tokenTerm)
                        {
                           int errorBegin = static_cast<int>(slice.begin - syntaxSpecification.begin + textBegin);
                           int errorEnd = static_cast<int>(slice.end - syntaxSpecification.begin + textBegin);
                           errors.emplace_back(false, std::string("Expected a conditional statement, only '[token]' is interpreted as [token].exists"), errorBegin, errorEnd);
                        }
                        // add it to the current expression as a "[token].exists" condition.
                        currentRelationalConditionPtr->conditionalExpressionPtr->op = cOperator::exists;

                        // create and step into new relationalTerm->conditionalExpression:
                        auto newConditionalExpression = ut::make_unique<ConditionalExpression>(cOperator::unassigned);
                        auto newRelationalConditionTerm = ut::make_unique<RelationalTerm>(RelationTypeEnum::conditionalExpression);
                        newConditionalExpression->leftTermPtr = nullptr;
                        newConditionalExpression->rightTermPtr = nullptr;
                        newRelationalConditionTerm->conditionalExpressionPtr = std::move(newConditionalExpression);

                        currentRelationalExpressionPtr->relationalTerms.emplace_back(std::move(newRelationalConditionTerm));
                        currentRelationalConditionPtr = currentRelationalExpressionPtr->relationalTerms.back().get();

                        c = c + 2;
                        num = num + 2;
                        slice.begin = c;
                     }
                     else
                     {
                        int errorBegin = static_cast<int>(c - syntaxSpecification.begin + textBegin);
                        errors.emplace_back(false, std::string("Unrecognized relational term, expected &&, || "), errorBegin, errorBegin + 1);
                     }
                  }
                  else if (*c == '|')
                  {
                     // Similar to the && case, but we also create and step into a new relationalTerm->relationalExpression
                     if (*(c + 1) == '|')
                     {
                        slice.end = c;
                        currentRelationalConditionPtr->conditionalExpressionPtr->leftTermPtr = std::move(validateAndGetTerm(slice, exampleRecord, errors, textBegin));;
                        if (currentRelationalConditionPtr->conditionalExpressionPtr->leftTermPtr->termType != TermTypeEnum::tokenTerm)
                        {
                           int errorBegin = static_cast<int>(slice.begin - syntaxSpecification.begin + textBegin);
                           int errorEnd = static_cast<int>(slice.end - syntaxSpecification.begin + textBegin);
                           errors.emplace_back(false, std::string("Expected a conditional statement, only '[token]' is interpreted as [token].exists"), errorBegin, errorEnd);
                        }
                        currentRelationalConditionPtr->conditionalExpressionPtr->op = cOperator::exists;

                        // create and step into new relationalTerm->relationalExpression:
                        auto newRelationalAndTerm = ut::make_unique<RelationalTerm>(RelationTypeEnum::relationalExpression);
                        newRelationalAndTerm->relationalExpressionPtr = std::move(ut::make_unique<RelationalExpression>(cRelation::andRelation));
                        rootRelationalOrTerm->relationalExpressionPtr->relationalTerms.emplace_back(std::move(newRelationalAndTerm));
                        currentRelationalExpressionPtr = rootRelationalOrTerm->relationalExpressionPtr->relationalTerms.back()->relationalExpressionPtr.get();

                        // create and step into new relationalTerm->conditionalExpression:
                        auto newConditionalExpression = ut::make_unique<ConditionalExpression>(cOperator::unassigned);
                        auto newRelationalConditionTerm = ut::make_unique<RelationalTerm>(RelationTypeEnum::conditionalExpression);
                        newConditionalExpression->leftTermPtr = nullptr;
                        newConditionalExpression->rightTermPtr = nullptr;
                        newRelationalConditionTerm->conditionalExpressionPtr = std::move(newConditionalExpression);

                        currentRelationalExpressionPtr->relationalTerms.emplace_back(std::move(newRelationalConditionTerm));
                        currentRelationalConditionPtr = currentRelationalExpressionPtr->relationalTerms.back().get();

                        c = c + 2;
                        num = num + 2;
                        slice.begin = c;
                     }
                     else
                     {
                        int errorBegin = static_cast<int>(c - syntaxSpecification.begin + textBegin);
                        errors.emplace_back(false, std::string("Unrecognized relational term, expected &&, || "), errorBegin, errorBegin + 1);
                     }
                  }
                  // Closing the expression while waiting for an operator (directly after an &&/||)
                  // means there should be a lone child [token].exists condition, otherwise user closed too early
                  // We only need to assign and tag the term, the tree structure should already be filled and ready to return
                  else if (*c == ')')
                  {
                     slice.end = c;
                     currentRelationalConditionPtr->conditionalExpressionPtr->leftTermPtr = std::move(validateAndGetTerm(slice, exampleRecord, errors, textBegin));;
                     if (currentRelationalConditionPtr->conditionalExpressionPtr->leftTermPtr->termType != TermTypeEnum::tokenTerm)
                     {
                        int errorBegin = static_cast<int>(slice.begin - syntaxSpecification.begin + textBegin);
                        int errorEnd = static_cast<int>(slice.end - syntaxSpecification.begin + textBegin);
                        errors.emplace_back(false, std::string("Expected a conditional statement, only '[token]' is interpreted as [token].exists"), errorBegin, errorEnd);
                     }
                     currentRelationalConditionPtr->conditionalExpressionPtr->op = cOperator::exists;
                     return rootRelationalOrTerm;
                  }
                  break;

               case ConditionalParserState::waitingForRelation: // look for relational &&/||, add right children to conditionals, change current conditionals and "RelationalConditionals"
                  // If the parser is explicitely waiting for a relation, the conditional expression already
                  // has a left child and an operator, and is already pointed to by a parent relationalTerm and parent-parent relationalExpression
                  if (*c == '&')
                  {
                     if (*(c + 1) == '&')
                     {
                        slice.end = c;
                        currentRelationalConditionPtr->conditionalExpressionPtr->rightTermPtr = std::move(validateAndGetTerm(slice, exampleRecord, errors, textBegin)); //returns a unique_ptr ConditionalTerm or nullptr if error
                        if (currentRelationalConditionPtr->conditionalExpressionPtr->rightTermPtr == nullptr)
                        {
                           int errorBegin = static_cast<int>(slice.begin - syntaxSpecification.begin + textBegin);
                           int errorEnd = static_cast<int>(slice.end - syntaxSpecification.begin + textBegin);
                           errors.emplace_back(false, std::string("Expected a conditional statement, only '[token]' is interpreted as [token].exists"), errorBegin, errorEnd);
                        }

                        // create a new, unspecified, conditional expression and its parent relationTerm before parsing for the next child condition
                        auto newConditionalExpression = ut::make_unique<ConditionalExpression>(cOperator::unassigned);
                        auto newRelationalConditionTerm = ut::make_unique<RelationalTerm>(RelationTypeEnum::conditionalExpression);
                        newConditionalExpression->leftTermPtr = nullptr;
                        newConditionalExpression->rightTermPtr = nullptr;
                        newRelationalConditionTerm->conditionalExpressionPtr = std::move(newConditionalExpression);

                        // push it to the current && expression and switch to it
                        currentRelationalExpressionPtr->relationalTerms.emplace_back(std::move(newRelationalConditionTerm));
                        currentRelationalConditionPtr = currentRelationalExpressionPtr->relationalTerms.back().get();

                        // skip the relation and look the operator defining the next condition
                        c = c + 2;
                        num = num + 2;
                        slice.begin = c;
                        state = ConditionalParserState::waitingForOperator;
                     }
                     else
                     {
                        int errorBegin = static_cast<int>(c - syntaxSpecification.begin + textBegin);
                        errors.emplace_back(false, std::string("Unrecognized relational term, expected &&, || "), errorBegin, errorBegin + 1);
                     }
                  }
                  else if (*c == '|')
                  {
                     // an || statement here should add the remaining child, and then cycle to a new
                     // relationalTerm->relationalExpression with a new relationalTerm->conditionalExpression
                     // Yields the same "state" as the start of the method, but w/ existing && expressions
                     if (*(c + 1) == '|')
                     {
                        currentRelationalConditionPtr->conditionalExpressionPtr->rightTermPtr = std::move(validateAndGetTerm(slice, exampleRecord, errors, textBegin));;
                        if (currentRelationalConditionPtr->conditionalExpressionPtr->rightTermPtr->termType != TermTypeEnum::tokenTerm)
                        {
                           int errorBegin = static_cast<int>(slice.begin - syntaxSpecification.begin + textBegin);
                           int errorEnd = static_cast<int>(slice.end - syntaxSpecification.begin + textBegin);
                           errors.emplace_back(false, std::string("Expected a conditional statement, only '[token]' is interpreted as [token].exists"), errorBegin, errorEnd);
                        }
                        currentRelationalConditionPtr->conditionalExpressionPtr->op = cOperator::exists;

                        // create and step into new relationalTerm->relationalExpression:
                        auto newRelationalAndTerm = ut::make_unique<RelationalTerm>(RelationTypeEnum::relationalExpression);
                        newRelationalAndTerm->relationalExpressionPtr = std::move(ut::make_unique<RelationalExpression>(cRelation::andRelation));
                        rootRelationalOrTerm->relationalExpressionPtr->relationalTerms.emplace_back(std::move(newRelationalAndTerm));
                        currentRelationalExpressionPtr = rootRelationalOrTerm->relationalExpressionPtr->relationalTerms.back()->relationalExpressionPtr.get();

                        // create and step into new relationalTerm->conditionalExpression:
                        auto newConditionalExpression = ut::make_unique<ConditionalExpression>(cOperator::unassigned);
                        auto newRelationalConditionTerm = ut::make_unique<RelationalTerm>(RelationTypeEnum::conditionalExpression);
                        newConditionalExpression->leftTermPtr = nullptr;
                        newConditionalExpression->rightTermPtr = nullptr;
                        newRelationalConditionTerm->conditionalExpressionPtr = std::move(newConditionalExpression);

                        currentRelationalExpressionPtr->relationalTerms.emplace_back(std::move(newRelationalConditionTerm));
                        currentRelationalConditionPtr = currentRelationalExpressionPtr->relationalTerms.back().get();

                        c = c + 2;
                        num = num + 2;
                        slice.begin = c;
                        state = ConditionalParserState::waitingForOperator;
                     }
                     else
                     {
                        int errorBegin = static_cast<int>(c - syntaxSpecification.begin + textBegin);
                        errors.emplace_back(false, std::string("Unrecognized relational term, expected &&, || "), errorBegin, errorBegin + 1);
                     }
                  }
                  // If the expression closes directly after an operator: <, >, ==, etc...
                  else if (*c == ')')
                  {
                     slice.end = c;
                     if (slice.begin == slice.end)
                     {
                        int errorBegin = static_cast<int>(c - syntaxSpecification.begin + textBegin);
                        errors.emplace_back(false, std::string("Missing conditional term "), errorBegin, errorBegin + 1);
                     }
                     currentRelationalConditionPtr->conditionalExpressionPtr->rightTermPtr = std::move(validateAndGetTerm(slice, exampleRecord, errors, textBegin));
                     return rootRelationalOrTerm;
                  }

                  break;

               default:
                  break;
            }
         }
         // Proper returns do not reach this switch
         switch (state)
         {
               int errorIdx;
            case ConditionalParserState::waitingForOperator:
            case ConditionalParserState::waitingForRelation:
               errorIdx = textBegin;
               errors.emplace_back(false, std::string("Conditional expression never closed "), errorIdx - 1, errorIdx);
               break;
            default:
               break;
         }
         return std::unique_ptr<RelationalTerm>(nullptr);
      }

      // Recursive method to evaluate the boolean expression tree formed by parseConditional
      // The structure should be a list of statements separated by ||
      // which are themselves divided by &&'s, with some number of conditional expressions each
      bool evaluateConditional(RelationalTerm* rootNode, const Record& record, std::vector<TemplateError>& errors, int& num)
      {
         // we're evaluating an or of ands, result starts true
         // and flips if any and in an expression is found false
         bool result = true;
         if (rootNode->relationType == RelationTypeEnum::relationalExpression)
         {
            if (rootNode->relationalExpressionPtr->op == cRelation::orRelation)
            {
               // Call the method without assigning the result for each list of && statements (separated by ||), any one can return true
               // stops and returns the first time this happens
               for (size_t i = 0; i < rootNode->relationalExpressionPtr->relationalTerms.size(); i++)
               {
                  result = evaluateConditional(rootNode->relationalExpressionPtr->relationalTerms[i].get(), record, errors, num);
                  if (result == true)
                  {
                     return result;
                  }
               }
               return result;
            }
            else
            {
               // For each chain of && statements, return whether they all return true or not
               for (size_t i = 0; i < rootNode->relationalExpressionPtr->relationalTerms.size(); i++)
               {
                  result = result && evaluateConditional(rootNode->relationalExpressionPtr->relationalTerms[i].get(), record, errors, num);
               }
               return result;
            }
         }
         else if (rootNode->relationType == RelationTypeEnum::conditionalExpression)
         {
            // raw ptr to the conditional expression to evaluate, this destroys the tree as you evaluate, so put them back at the end
            auto conditionalExpression = std::move(rootNode->conditionalExpressionPtr);
            auto leftTermPtr = std::move(conditionalExpression->leftTermPtr);
            auto rightTermPtr = std::move(conditionalExpression->rightTermPtr);

            // Any time a token is called and the current record doesn't have that token, return false
            // This includes ([token] == float), giving all [token] conditional terms an implicit "exists" condition
            if (leftTermPtr->termType == TermTypeEnum::tokenTerm)
            {
               if (isEmpty(leftTermPtr->tokenTerm->getStringSlice(record)))
               {
                  rootNode->conditionalExpressionPtr = std::move(conditionalExpression);
                  rootNode->conditionalExpressionPtr->leftTermPtr = std::move(leftTermPtr);
                  rootNode->conditionalExpressionPtr->rightTermPtr = std::move(rightTermPtr);
                  return false;
               }
            }
            if (conditionalExpression->op != cOperator::exists)
            {
               if (rightTermPtr->termType == TermTypeEnum::tokenTerm)
               {
                  if (isEmpty(rightTermPtr->tokenTerm->getStringSlice(record)))
                  {
                     rootNode->conditionalExpressionPtr = std::move(conditionalExpression);
                     rootNode->conditionalExpressionPtr->leftTermPtr = std::move(leftTermPtr);
                     rootNode->conditionalExpressionPtr->rightTermPtr = std::move(rightTermPtr);
                     return false;
                  }
               }
            }
            // For now, operators are restricted to the left/right types as follows:
            // >, <, >=, <=  . . . Float - Float || Float - AttributeToken (must be numeric, either order)
            // ==, != . . . Float - Float || float - AttributeToken (numeric) || StringSlice - AttributeToken (assumed to be a string, interprets a number)
            // exists . . . AttributeToken (should be an only left child)
            if (conditionalExpression->op == cOperator::greater || conditionalExpression->op == cOperator::greaterEqual ||
                conditionalExpression->op == cOperator::less || conditionalExpression->op == cOperator::lessEqual)
            {
               if (leftTermPtr->termType == TermTypeEnum::floatTerm && rightTermPtr->termType == TermTypeEnum::floatTerm)
               {
                  switch (conditionalExpression->op)
                  {
                     case cOperator::greater:
                        result = atof(stdStringFromStringSlice(leftTermPtr->floatTerm).c_str()) > atof(stdStringFromStringSlice(rightTermPtr->floatTerm).c_str());
                        break;
                     case cOperator::greaterEqual:
                        result = atof(stdStringFromStringSlice(leftTermPtr->floatTerm).c_str()) >= atof(stdStringFromStringSlice(rightTermPtr->floatTerm).c_str());
                        break;
                     case cOperator::less:
                        result = atof(stdStringFromStringSlice(leftTermPtr->floatTerm).c_str()) < atof(stdStringFromStringSlice(rightTermPtr->floatTerm).c_str());
                        break;
                     case cOperator::lessEqual:
                        result = atof(stdStringFromStringSlice(leftTermPtr->floatTerm).c_str()) <= atof(stdStringFromStringSlice(rightTermPtr->floatTerm).c_str());
                        break;
                     default:
                        break;
                  }
               }
               else if (leftTermPtr->termType == TermTypeEnum::floatTerm && rightTermPtr->termType == TermTypeEnum::tokenTerm)
               {
                  if (!validateFractionalNumber(rightTermPtr->tokenTerm->getStringSlice(record)))
                  {
                     errors.emplace_back(false, std::string("Comparison between unlike types, expected a token with a numeric value "), num - 1, num);
                     rootNode->conditionalExpressionPtr = std::move(conditionalExpression);
                     rootNode->conditionalExpressionPtr->leftTermPtr = std::move(leftTermPtr);
                     rootNode->conditionalExpressionPtr->rightTermPtr = std::move(rightTermPtr);
                     return false;
                  }
                  switch (conditionalExpression->op)
                  {
                     case cOperator::greater:
                        result = atof(stdStringFromStringSlice(leftTermPtr->floatTerm).c_str()) > atof(stdStringFromStringSlice(rightTermPtr->tokenTerm->getStringSlice(record)).c_str());
                        break;
                     case cOperator::greaterEqual:
                        result = atof(stdStringFromStringSlice(leftTermPtr->floatTerm).c_str()) >= atof(stdStringFromStringSlice(rightTermPtr->tokenTerm->getStringSlice(record)).c_str());
                        break;
                     case cOperator::less:
                        result = atof(stdStringFromStringSlice(leftTermPtr->floatTerm).c_str()) < atof(stdStringFromStringSlice(rightTermPtr->tokenTerm->getStringSlice(record)).c_str());
                        break;
                     case cOperator::lessEqual:
                        result = atof(stdStringFromStringSlice(leftTermPtr->floatTerm).c_str()) <= atof(stdStringFromStringSlice(rightTermPtr->tokenTerm->getStringSlice(record)).c_str());
                        break;
                     default:
                        break;
                  }
               }
               else if (leftTermPtr->termType == TermTypeEnum::tokenTerm && rightTermPtr->termType == TermTypeEnum::floatTerm)
               {
                  if (!validateFractionalNumber(leftTermPtr->tokenTerm->getStringSlice(record)))
                  {
                     errors.emplace_back(false, std::string("Comparison between unlike types, expected a token with a numeric value "), num - 1, num);
                     rootNode->conditionalExpressionPtr = std::move(conditionalExpression);
                     rootNode->conditionalExpressionPtr->leftTermPtr = std::move(leftTermPtr);
                     rootNode->conditionalExpressionPtr->rightTermPtr = std::move(rightTermPtr);
                     return false;
                  }
                  switch (conditionalExpression->op)
                  {
                     case cOperator::greater:
                        result = atof(stdStringFromStringSlice(leftTermPtr->tokenTerm->getStringSlice(record)).c_str()) > atof(stdStringFromStringSlice(rightTermPtr->floatTerm).c_str());
                        break;
                     case cOperator::greaterEqual:
                        result = atof(stdStringFromStringSlice(leftTermPtr->tokenTerm->getStringSlice(record)).c_str()) >= atof(stdStringFromStringSlice(rightTermPtr->floatTerm).c_str());
                        break;
                     case cOperator::less:
                        result = atof(stdStringFromStringSlice(leftTermPtr->tokenTerm->getStringSlice(record)).c_str()) < atof(stdStringFromStringSlice(rightTermPtr->floatTerm).c_str());
                        break;
                     case cOperator::lessEqual:
                        result = atof(stdStringFromStringSlice(leftTermPtr->tokenTerm->getStringSlice(record)).c_str()) <= atof(stdStringFromStringSlice(rightTermPtr->floatTerm).c_str());
                        break;
                     default:
                        break;
                  }
               }
               else if (leftTermPtr->termType == TermTypeEnum::tokenTerm && rightTermPtr->termType == TermTypeEnum::tokenTerm)
               {
                  if (!(validateFractionalNumber(leftTermPtr->tokenTerm->getStringSlice(record)) && validateFractionalNumber(rightTermPtr->tokenTerm->getStringSlice(record))))
                  {
                     errors.emplace_back(false, std::string("Comparison between unlike types, expected a token with a numeric value "), num - 1, num);
                     rootNode->conditionalExpressionPtr = std::move(conditionalExpression);
                     rootNode->conditionalExpressionPtr->leftTermPtr = std::move(leftTermPtr);
                     rootNode->conditionalExpressionPtr->rightTermPtr = std::move(rightTermPtr);
                     return false;
                  }
                  switch (conditionalExpression->op)
                  {
                     case cOperator::greater:
                        result = atof(stdStringFromStringSlice(leftTermPtr->stringSliceTerm).c_str()) > atof(stdStringFromStringSlice(rightTermPtr->stringSliceTerm).c_str());
                        break;
                     case cOperator::greaterEqual:
                        result = atof(stdStringFromStringSlice(leftTermPtr->stringSliceTerm).c_str()) >= atof(stdStringFromStringSlice(rightTermPtr->stringSliceTerm).c_str());
                        break;
                     case cOperator::less:
                        result = atof(stdStringFromStringSlice(leftTermPtr->stringSliceTerm).c_str()) < atof(stdStringFromStringSlice(rightTermPtr->stringSliceTerm).c_str());
                        break;
                     case cOperator::lessEqual:
                        result = atof(stdStringFromStringSlice(leftTermPtr->stringSliceTerm).c_str()) <= atof(stdStringFromStringSlice(rightTermPtr->stringSliceTerm).c_str());
                        break;
                     default:
                        break;
                  }
               }
               else
               {
                  errors.emplace_back(false, std::string("Comparison between unlike types "), num - 1, num);
                  rootNode->conditionalExpressionPtr = std::move(conditionalExpression);
                  rootNode->conditionalExpressionPtr->leftTermPtr = std::move(leftTermPtr);
                  rootNode->conditionalExpressionPtr->rightTermPtr = std::move(rightTermPtr);
                  return false;
               }
            }
            else if (conditionalExpression->op == cOperator::equal || conditionalExpression->op == cOperator::notEqual)
            {
               if (leftTermPtr->termType == TermTypeEnum::floatTerm && rightTermPtr->termType == TermTypeEnum::floatTerm)
               {
                  switch (conditionalExpression->op)
                  {
                     case cOperator::equal:
                        result = equal(leftTermPtr->floatTerm, rightTermPtr->floatTerm);
                        break;
                     case cOperator::notEqual:
                        result = !equal(leftTermPtr->floatTerm, rightTermPtr->floatTerm);
                        break;
                     default:
                        break;
                  }
               }
               if (leftTermPtr->termType == TermTypeEnum::floatTerm && rightTermPtr->termType == TermTypeEnum::tokenTerm)
               {
                  if (!validateFractionalNumber(rightTermPtr->tokenTerm->getStringSlice(record)))
                  {
                     errors.emplace_back(false, std::string("Comparison between unlike types, expected a token with a numeric value "), num - 1, num);
                     rootNode->conditionalExpressionPtr = std::move(conditionalExpression);
                     rootNode->conditionalExpressionPtr->leftTermPtr = std::move(leftTermPtr);
                     rootNode->conditionalExpressionPtr->rightTermPtr = std::move(rightTermPtr);
                     return false;
                  }
                  switch (conditionalExpression->op)
                  {
                     case cOperator::equal:
                        result = equal(leftTermPtr->floatTerm, rightTermPtr->tokenTerm->getStringSlice(record));
                        break;
                     case cOperator::notEqual:
                        result = !equal(leftTermPtr->floatTerm, rightTermPtr->tokenTerm->getStringSlice(record));
                        break;
                     default:
                        break;
                  }
               }
               else if (leftTermPtr->termType == TermTypeEnum::tokenTerm && rightTermPtr->termType == TermTypeEnum::floatTerm)
               {
                  if (!validateFractionalNumber(leftTermPtr->tokenTerm->getStringSlice(record)))
                  {
                     errors.emplace_back(false, std::string("Comparison between unlike types, expected a token with a numeric value "), num - 1, num);
                     rootNode->conditionalExpressionPtr = std::move(conditionalExpression);
                     rootNode->conditionalExpressionPtr->leftTermPtr = std::move(leftTermPtr);
                     rootNode->conditionalExpressionPtr->rightTermPtr = std::move(rightTermPtr);
                     return false;
                  }
                  switch (conditionalExpression->op)
                  {
                     case cOperator::equal:
                        result = equal(leftTermPtr->tokenTerm->getStringSlice(record), rightTermPtr->floatTerm);
                        break;
                     case cOperator::notEqual:
                        result = equal(leftTermPtr->tokenTerm->getStringSlice(record), rightTermPtr->floatTerm);
                        break;
                     default:
                        break;
                  }
               }
               // If either term is a stringslice ([token] == some_characters) the comparison is assumed to be a string comparison
               // even if the token contains a number
               else if (leftTermPtr->termType == TermTypeEnum::stringTerm && rightTermPtr->termType == TermTypeEnum::tokenTerm)
               {
                  switch (conditionalExpression->op)
                  {
                     case cOperator::equal:
                        result = equal(leftTermPtr->stringSliceTerm, rightTermPtr->tokenTerm->getStringSlice(record));
                        break;
                     case cOperator::notEqual:
                        result = !equal(leftTermPtr->stringSliceTerm, rightTermPtr->tokenTerm->getStringSlice(record));
                        break;
                     default:
                        break;
                  }
               }
               else if (leftTermPtr->termType == TermTypeEnum::tokenTerm && rightTermPtr->termType == TermTypeEnum::stringTerm)
               {
                  switch (conditionalExpression->op)
                  {
                     case cOperator::equal:
                        result = equal(leftTermPtr->tokenTerm->getStringSlice(record), rightTermPtr->stringSliceTerm);
                        break;
                     case cOperator::notEqual:
                        result = !equal(leftTermPtr->tokenTerm->getStringSlice(record), rightTermPtr->stringSliceTerm);
                        break;
                     default:
                        break;
                  }
               }
               else if (leftTermPtr->termType == TermTypeEnum::stringTerm && rightTermPtr->termType == TermTypeEnum::stringTerm)
               {
                  switch (conditionalExpression->op)
                  {
                     case cOperator::equal:
                        result = equal(leftTermPtr->stringSliceTerm, rightTermPtr->stringSliceTerm);
                        break;
                     case cOperator::notEqual:
                        result = !equal(leftTermPtr->stringSliceTerm, rightTermPtr->stringSliceTerm);
                        break;
                     default:
                        break;
                  }
               }
               else
               {
                  errors.emplace_back(false, std::string("Comparison between unlike types "), num - 1, num);
                  rootNode->conditionalExpressionPtr = std::move(conditionalExpression);
                  rootNode->conditionalExpressionPtr->leftTermPtr = std::move(leftTermPtr);
                  rootNode->conditionalExpressionPtr->rightTermPtr = std::move(rightTermPtr);
                  return false;
               }
            }
            // The only time the operator was set to "exists" was when the parser
            // caught a lone [AttributeToken], added as the left child
            else if (conditionalExpression->op == cOperator::exists)
            {
               result = !isEmpty(leftTermPtr->tokenTerm->getStringSlice(record));
            }
            // Shouldn't happen, poorly formed condition
            else
            {
               errors.emplace_back(false, std::string("Poorly formed condition in boolean expression "), num, num + 1);
               result = false;
            }

            // return the conditionalExpressions to the tree so the next record can use them
            rootNode->conditionalExpressionPtr = std::move(conditionalExpression);
            rootNode->conditionalExpressionPtr->leftTermPtr = std::move(leftTermPtr);
            rootNode->conditionalExpressionPtr->rightTermPtr = std::move(rightTermPtr);
            return result;
         }
         // If, somehow, an undefined expression didn't throw an error and we tried to evaluate it:
         errors.emplace_back(false, std::string("Poorly formed condition in boolean expression "), num, num + 1);
         return false;
      }

      void parseToCase(const char*& c, StringSlice syntaxSpecification, const Record& exampleRecord, std::vector<TemplateError>& errors, int& num, ConditionalToken*& currentConditionalRawPtr, bool toUpper)
      {
         int count = 0;
         int textBegin = num;
         StringSlice tokenSlice;
         StringSlice slice;
         slice.begin = c;
         bool isNegative = false;
         ToUpperParserState state = ToUpperParserState::start;
         for (; c != syntaxSpecification.end; ++c, ++num)
         {
            switch (state)
            {
               case ToUpperParserState::start:
                  if (*c == '[')
                  {
                     tokenSlice.begin = c + 1;
                     state = ToUpperParserState::inToken;
                     break;
                  }
                  if (!(*c == ' ' || *c == '\n' || *c == '\t'))
                  {
                     int errorIdx = static_cast<int>(slice.begin - syntaxSpecification.begin + textBegin);
                     errors.emplace_back(false, std::string("Expected a token: [token] as the first argument of toUpper "), errorIdx, errorIdx + 1);
                     return;
                  }
                  break;
               case ToUpperParserState::inToken:
                  if (*c == ']')
                  {
                     tokenSlice.end = c;
                     if (isEmpty(getAttribute(stdStringFromStringSlice(tokenSlice), exampleRecord)))
                     {
                        int errorEnd = static_cast<int>(tokenSlice.end - syntaxSpecification.begin + textBegin);
                        int errorBegin = static_cast<int>(tokenSlice.begin - syntaxSpecification.begin + textBegin);
                        errors.emplace_back(false, std::string("Invalid identifier, must match a name from template input "), errorBegin, errorEnd);
                        return;
                     }
                     slice.begin = c + 1;
                     state = ToUpperParserState::postToken;
                  }
                  else if (*c == ')' || *c == '(' || *c == '{' || *c == '}')
                  {
                     int errorBegin = static_cast<int>(tokenSlice.begin - syntaxSpecification.begin + textBegin);
                     errors.emplace_back(false, std::string("Expected closing token delimiter: ] "), errorBegin - 1, errorBegin);
                     return;
                  }
                  break;
               case ToUpperParserState::postToken:
                  if (*c == ',')
                  {
                     slice.begin = c + 1;
                     state = ToUpperParserState::inArg;
                     break;
                  }
                  else if (*c == ')')
                  {
                     state = ToUpperParserState::end;
                     break;
                  }
                  else if (!(*c == ' ' || *c == '\n' || *c == '\t'))
                  {
                     slice.begin = c;
                     int errorIdx = static_cast<int>(slice.begin - syntaxSpecification.begin + textBegin);
                     errors.emplace_back(false, std::string("Expected toUpper([token]) or toUpper([token], int count) "), errorIdx, errorIdx + 1);
                     return;
                  }
                  break;
               case ToUpperParserState::inArg:
                  if (*c == ')')
                  {
                     slice.end = c;
                     count = atoi(slice.begin);
                     if (isNegative)
                     {
                        count = -count;
                     }
                     state = ToUpperParserState::end;
                     break;
                  }
                  else if (*c == '-') // skip a dash and set the count to read as negative
                  {
                     slice.begin = c + 1;
                     isNegative = true;
                  }
                  else if (!(*c == ' ' || *c == '\n' || *c == '\t' || isDigit(stdStringFromStringSlice(StringSlice{ c, c + 1 })[0])))
                  {
                     slice.begin = c;
                     int errorIdx = static_cast<int>(slice.begin - syntaxSpecification.begin + textBegin);
                     errors.emplace_back(false, std::string("Expected integer as second optional argument for toUpper() "), errorIdx, errorIdx + 1);
                     return;
                  }
                  break;
               case ToUpperParserState::end: //the function call end )
                  // If we found a token we already validated its existance in the record, now we
                  // just want to edit it slightly before pushing it to the tokens vector
                  if (*c == ']')
                  {
                     auto tokenInput = ut::make_unique<ToCaseToken>(tokenSlice);
                     tokenInput->count = count; // zero means full string
                     tokenInput->toUpper = toUpper;
                     if (currentConditionalRawPtr != nullptr)
                     {
                        if (currentConditionalRawPtr->caseState == true)
                        {
                           currentConditionalRawPtr->trueCaseTokens.emplace_back(std::move(tokenInput));
                        }
                        else
                        {
                           currentConditionalRawPtr->falseCaseTokens.emplace_back(std::move(tokenInput));
                        }
                        return;
                     }
                     tokens.emplace_back(std::move(tokenInput));
                     return;
                  }
                  else
                  {
                     int errorIdx = static_cast<int>(c - syntaxSpecification.begin + textBegin);
                     errors.emplace_back(false, std::string("Expected closing token delimiter for function: ] "), errorIdx, errorIdx + 1);
                     return;
                  }
               default:
                  break;
            }
         }
         switch (state)
         {
               int errorIdx;
            case ToUpperParserState::start:
               errorIdx = static_cast<int>(slice.begin - syntaxSpecification.begin + textBegin);
               errors.emplace_back(false, std::string("Expected a token: [token] as the first argument of toUpper "), errorIdx, errorIdx + 1);
               break;
            case ToUpperParserState::inToken:
               errorIdx = static_cast<int>(tokenSlice.begin - syntaxSpecification.begin + textBegin);
               errors.emplace_back(false, std::string("Opening token delimiter [ never closed "), errorIdx - 1, errorIdx);
               break;
            case ToUpperParserState::postToken:
               errorIdx = static_cast<int>(slice.begin - syntaxSpecification.begin + textBegin);
               errors.emplace_back(false, std::string("Expected toUpper([token]) or toUpper([token], int count) "), errorIdx, errorIdx + 1);
               break;
            case ToUpperParserState::inArg:
               errorIdx = static_cast<int>(slice.begin - syntaxSpecification.begin + textBegin);
               errors.emplace_back(false, std::string("Expected second argumnet and closing ) "), errorIdx, errorIdx + 1);
               return;
            case ToUpperParserState::end:
               errorIdx = static_cast<int>(c - syntaxSpecification.begin + textBegin);
               errors.emplace_back(false, std::string("Expected closing token delimiter for function: ] "), errorIdx, errorIdx + 1);
               return;
            default:
               break;
         }
      }

      // Parser for the translate function, pushes a TranslateToken to the current location in the token tree
      void parseTranslate(const char*& c, StringSlice syntaxSpecification, const Record& exampleRecord, std::vector<TemplateError>& errors, int& num, ConditionalToken*& currentConditionalRawPtr)
      {
         int textBegin = num;

         StringSlice headingSlice{ nullptr, nullptr };
         StringSlice distSlice{ nullptr, nullptr };
         StringSlice unitSlice{ nullptr, nullptr };
         StringSlice latSlice;   // Get from a [Lat_DMS] token
         StringSlice longSlice;  // Get from a [Long_DMS] token
         int unitInt = 0; // the integer representation of the base units enum, for use by any UtUnitLength conversion methods
         headingSlice.begin = c;

         std::unique_ptr<RandomToken> randomHeading = nullptr;
         std::unique_ptr<RandomToken> randomDistance = nullptr;

         StringSlice slice;
         slice.begin = c;
         TranslateParserState state = TranslateParserState::heading;
         for (; c != syntaxSpecification.end; ++c, ++num)
         {
            switch (state)
            {
               case TranslateParserState::heading:
                  // found a comma before anything else, indicating a real valued heading angle
                  if (*c == ',')
                  {
                     if (!randomHeading)
                     {
                        headingSlice.end = c;
                        if (!validateFractionalNumber(headingSlice))
                        {
                           int errorBegin = (headingSlice.begin - syntaxSpecification.begin + textBegin);
                           int errorEnd = (headingSlice.end - syntaxSpecification.begin + textBegin);
                           errors.emplace_back(false, std::string("Improper input, a 0-360 degree heading angle, or a random()"), errorBegin, errorEnd);
                        }
                        distSlice.begin = c + 1;
                        state = TranslateParserState::distance;
                     }
                     // if we already found a random token
                     else
                     {
                        distSlice.begin = c + 1;
                        state = TranslateParserState::distance;
                     }

                  }
                  // Found an opening token, in this argument only a value or a random token is allowed, so check for the random
                  else if (*c == '[' && c + 8 < syntaxSpecification.end)
                  {
                     if (equal(StringSlice{ c, c + 8 }, stringSliceFromCString("[random(")))
                     {
                        // Skip ahead, run the random parser on the syntaxSpecification, and check that it didn't pick up any errors
                        slice.begin = c + 8;
                        c = c + 8;

                        auto errorCount = errors.size();
                        randomHeading = std::move(parseRandom(c, syntaxSpecification, exampleRecord, errors, num, currentConditionalRawPtr));
                        if (errorCount != errors.size())
                        {
                           return;
                        }
                        break;
                     }
                  }
                  else if ((*c == ' ' || *c == '\n' || *c == '\t')) // skip white space
                  {
                     headingSlice.begin = c + 1;
                  }
                  break;
               case TranslateParserState::distance:
                  // found a comma before anything else, indicating a real valued distance
                  if (*c == ',')
                  {
                     if (!randomDistance)
                     {
                        // if we found units before the comma, try to parse and add them from UtUnitLengths
                        if (unitSlice.begin != nullptr)
                        {
                           unitSlice.end = c;
                           auto value = UtUnitLength::mUnitStrings.find(stdStringFromStringSlice(unitSlice));
                           if (value != UtUnitLength::mUnitStrings.end())
                           {
                              unitInt = value->second;
                           }
                           else
                           {
                              unitInt = UtUnitLength::mUnitStrings.find("km")->second;
                              int errorBegin = (unitSlice.begin - syntaxSpecification.begin + textBegin);
                              int errorEnd = (unitSlice.end - syntaxSpecification.begin + textBegin);
                              errors.emplace_back(false, std::string("Unrecognized units, expected length (nm, mi, km, etc.)"), errorBegin, errorEnd);
                              state = TranslateParserState::lat;
                           }
                        }
                        // Otherwise assume km and capture the distSlice
                        else
                        {
                           distSlice.end = c;
                           unitSlice.begin = c + 1;
                           unitInt = UtUnitLength::mUnitStrings.find("km")->second;
                        }
                        state = TranslateParserState::lat;
                     }
                     // Otherwise assume km and do not capture the distSlice to indicate a random token
                     else
                     {
                        if (unitSlice.begin != nullptr)
                        {
                           unitSlice.end = c;
                           auto value = UtUnitLength::mUnitStrings.find(stdStringFromStringSlice(unitSlice));
                           if (value != UtUnitLength::mUnitStrings.end())
                           {
                              unitInt = value->second;
                           }
                           else
                           {
                              unitInt = UtUnitLength::mUnitStrings.find("km")->second;
                              int errorBegin = (unitSlice.begin - syntaxSpecification.begin + textBegin);
                              int errorEnd = (unitSlice.end - syntaxSpecification.begin + textBegin);
                              errors.emplace_back(false, std::string("Unrecognized units, expected length (nm, mi, km, etc.)"), errorBegin, errorEnd);
                              state = TranslateParserState::lat;
                           }
                        }
                        else
                        {
                           unitInt = UtUnitLength::mUnitStrings.find("km")->second;
                        }
                     }
                     state = TranslateParserState::lat;
                  }
                  // Found an opening token, in this argument only a value or a random token is allowed, so check for the random
                  else if (*c == '[' && c + 8 < syntaxSpecification.end)
                  {
                     if (equal(StringSlice{ c, c + 8 }, stringSliceFromCString("[random(")))
                     {
                        // Skip ahead, run the random parser on the syntaxSpecification, and check that it didn't pick up any errors
                        slice.begin = c + 8;
                        c = c + 8;

                        int errorCount = static_cast<int>(errors.size());
                        randomDistance = std::move(parseRandom(c, syntaxSpecification, exampleRecord, errors, num, currentConditionalRawPtr));
                        if (errorCount != static_cast<int>(errors.size()))
                        {
                           return;
                        }
                        // keep parsing for units if there are any
                        break;
                     }
                  }
                  else if (*c == ' ' || *c == '\n' || *c == '\t')
                  {
                     distSlice.begin = c + 1;
                  }
                  // Capture the distance ending on the first non-space && non-digit (the units or the comma): |1234.5|nm,
                  if (!(*c == ' ' || *c == '\n' || *c == '\t') && !isDigit(stdStringFromStringSlice(StringSlice{ c, c + 1 })[0]) && unitSlice.begin == nullptr)
                  {
                     if (!randomDistance)
                     {
                        distSlice.end = c;
                     }
                     unitSlice.begin = c;
                  }
                  break;
               case TranslateParserState::lat:
                  if (*c == '[')
                  {
                     latSlice.begin = c + 1;
                     state = TranslateParserState::inLatToken;
                     break;
                  }
                  else if (!(*c == ' ' || *c == '\n' || *c == '\t'))
                  {
                     int errorEnd = (c - syntaxSpecification.begin + textBegin);
                     errors.emplace_back(false, std::string("Expected [Lat_DMS]or [Lat_DD] token"), errorEnd, errorEnd + 1);
                     return;
                  }
                  break;
               case TranslateParserState::inLatToken:
                  if (*c == ']')
                  {
                     latSlice.end = c;
                     int errorEnd = static_cast<int>(latSlice.end - syntaxSpecification.begin + textBegin);
                     int errorBegin = static_cast<int>(latSlice.begin - syntaxSpecification.begin + textBegin);
                     if (isEmpty(getAttribute(std::string(latSlice.begin, length(latSlice)), exampleRecord)))
                     {
                        errors.emplace_back(false, std::string("Invalid identifier, must match a name from template input "), errorBegin, errorEnd);
                     }
                     state = TranslateParserState::lookForComma;
                  }
                  break;
               case TranslateParserState::lookForComma:
                  if (*c == ',')
                  {
                     longSlice.begin = c + 1;
                     state = TranslateParserState::lon;
                  }
                  else if (!(*c == ' ' || *c == '\n' || *c == '\t'))
                  {
                     int errorEnd = (c - syntaxSpecification.begin + textBegin);
                     errors.emplace_back(false, std::string("Improper translate() format, translate(Heading, Distance, [Lat_DMS/DD], [Long_DMS/DD])"), errorEnd, errorEnd + 1);
                     state = TranslateParserState::lon;
                  }
                  break;
               case TranslateParserState::lon:
                  if (*c == '[')
                  {
                     longSlice.begin = c + 1;
                     state = TranslateParserState::inLongToken;
                     break;
                  }
                  else if (!(*c == ' ' || *c == '\n' || *c == '\t'))
                  {
                     int errorEnd = (c - syntaxSpecification.begin + textBegin);
                     errors.push_back(TemplateError{ false, std::string("Expected [Long_DMS] or [Long_DD] token"), errorEnd, errorEnd + 1 });
                     return;
                  }
                  break;
               case TranslateParserState::inLongToken:
                  if (*c == ']')
                  {
                     longSlice.end = c;
                     int errorEnd = static_cast<int>(longSlice.end - syntaxSpecification.begin + textBegin);
                     int errorBegin = static_cast<int>(longSlice.begin - syntaxSpecification.begin + textBegin);
                     if (isEmpty(getAttribute(std::string(longSlice.begin, length(longSlice)), exampleRecord)))
                     {
                        errors.emplace_back(false, std::string("Invalid identifier, must match a name from template input "), errorBegin, errorEnd);
                     }
                     state = TranslateParserState::closing;
                  }
                  break;
               case TranslateParserState::closing:
                  if (*c == ')')
                  {
                     state = TranslateParserState::end;
                  }
                  else if (!(*c == ' ' || *c == '\n' || *c == '\t'))
                  {
                     int errorEnd = (c - syntaxSpecification.begin + textBegin);
                     errors.emplace_back(false, std::string("Expected closing paren: ) "), errorEnd, errorEnd + 1);
                     return;
                  }
                  break;
               case TranslateParserState::end: //the function call end )
                  if (*c == ']')
                  {
                     slice.end = c; // I store the SS capturing all parameters because something has to go there, but it should never be referenced when writing the record
                     auto translateToken = ut::make_unique<TranslateToken>(slice);
                     // Store either a random token or a stringslice, depending on what was found for the first two parameters.

                     // If dist slice is null we should have acquired a randomToken instead:
                     if (distSlice.end == nullptr)
                     {
                        translateToken->randDistance = std::move(randomDistance);
                     }
                     else
                     {
                        if (validateFractionalNumber(distSlice))
                        {
                           translateToken->distance = std::stof(stdStringFromStringSlice(distSlice).c_str());
                        }
                        else
                        {
                           int errorEnd = static_cast<int>(distSlice.end - syntaxSpecification.begin + textBegin);
                           int errorBegin = static_cast<int>(distSlice.begin - syntaxSpecification.begin + textBegin);
                           errors.emplace_back(false, std::string("Invalid distance, expected numeric value or random token "), errorBegin, errorEnd);
                        }
                     }
                     // if headingSlice is null we should have acquired a randomToken instead;
                     if (headingSlice.end == nullptr)
                     {
                        translateToken->randHeading = std::move(randomHeading);
                     }
                     else
                     {
                        if (validateFractionalNumber(headingSlice))
                        {
                           translateToken->heading = std::stof(stdStringFromStringSlice(headingSlice).c_str());
                        }
                        else
                        {
                           int errorEnd = static_cast<int>(headingSlice.end - syntaxSpecification.begin + textBegin);
                           int errorBegin = static_cast<int>(headingSlice.begin - syntaxSpecification.begin + textBegin);
                           errors.emplace_back(false, std::string("Invalid heading, expected numeric value or random token in degrees from north "), errorBegin, errorEnd);
                        }
                     }
                     translateToken->latToken = std::move(ut::make_unique<AttributeToken>(latSlice));
                     translateToken->longToken = std::move(ut::make_unique<AttributeToken>(longSlice));
                     translateToken->units = unitInt;
                     if (currentConditionalRawPtr != nullptr)
                     {
                        if (currentConditionalRawPtr->caseState == true)
                        {
                           currentConditionalRawPtr->trueCaseTokens.emplace_back(std::move(translateToken));
                        }
                        else
                        {
                           currentConditionalRawPtr->falseCaseTokens.emplace_back(std::move(translateToken));
                        }
                        return;
                     }
                     tokens.emplace_back(std::move(translateToken));
                     return;
                  }
                  else
                  {
                     int errorIdx = static_cast<int>(c - syntaxSpecification.begin + textBegin);
                     errors.emplace_back(false, std::string("Expected closing token delimiter for function: ] "), errorIdx, errorIdx + 1);
                     return;
                  }
            }
         }
         switch (state)
         {
               int errorIdx;
            case TranslateParserState::heading:
            case TranslateParserState::lookForComma:
            case TranslateParserState::distance:
            case TranslateParserState::lat:
            case TranslateParserState::lon:
               errorIdx = static_cast<int>(slice.begin - syntaxSpecification.begin + textBegin);
               errors.emplace_back(false, std::string("Expected argumnets: translate( Heading (DMS), Distance (nm), [Lat_DMS], [Long_DMS]) "), errorIdx, errorIdx + 1);
               break;
            case TranslateParserState::inLatToken:
               errorIdx = static_cast<int>(latSlice.begin - syntaxSpecification.begin + textBegin);
               errors.emplace_back(false, std::string("Opening token delimiter [ never closed "), errorIdx - 1, errorIdx);
               break;
            case TranslateParserState::inLongToken:
               errorIdx = static_cast<int>(longSlice.begin - syntaxSpecification.begin + textBegin);
               errors.emplace_back(false, std::string("Opening token delimiter [ never closed "), errorIdx - 1, errorIdx);
               break;
            case TranslateParserState::end:
               errorIdx = static_cast<int>(c - syntaxSpecification.begin + textBegin);
               errors.emplace_back(false, std::string("Expected closing token delimiter for function: ] "), errorIdx, errorIdx + 1);
               break;
            case TranslateParserState::closing:
               errorIdx = static_cast<int>(c - syntaxSpecification.begin + textBegin);
               errors.emplace_back(false, std::string("Expected closing delimiter for function: ) "), errorIdx, errorIdx + 1);
               break;
         }
      }

      // Parse through a call to [random(float,float)], validating the form and creating a RandomToken
      // the conditional and populate pointer references determine where the function stores this token:
      // Either as a RandomToken parameter to a function like translate(random(), random(), lat, long)
      // Or as a standalone RandomToken in the proper conditional scope.
      std::unique_ptr<RandomToken> parseRandom(const char*& c, StringSlice syntaxSpecification, const Record& exampleRecord, std::vector<TemplateError>& errors, int& num, ConditionalToken*& currentConditionalRawPtr)
      {
         int textBegin = num;

         // Range Arguments
         bool isNegative = false;
         StringSlice startSlice{ nullptr, nullptr };
         StringSlice endSlice{ nullptr, nullptr };
         double start = 0.0;
         double end = 0.0;
         startSlice.begin = c;
         RandomParserState state = RandomParserState::start;
         for (; c != syntaxSpecification.end; ++c, ++num)
         {
            switch (state)
            {
               case RandomParserState::start:
                  if (*c == ',')
                  {
                     startSlice.end = c;
                     if (validateFractionalNumber(startSlice))
                     {

                        start = stod(stdStringFromStringSlice(startSlice));
                        if (isNegative)
                        {
                           start = -start;
                           isNegative = false;
                        }
                        endSlice.begin = c + 1;
                        state = RandomParserState::end;
                     }
                     else
                     {
                        int errorIdx = static_cast<int>(c - syntaxSpecification.begin + textBegin);
                        errors.emplace_back(false, std::string("First argument to random() was missing or invalid "), errorIdx, errorIdx + 1);
                        return nullptr;
                     }
                  }
                  else if (*c == '-')
                  {
                     isNegative = true;
                     startSlice.begin = c + 1;
                  }
                  else if (!(*c == ' ' || *c == '\n' || *c == '\t' || isDigit(stdStringFromStringSlice(StringSlice{ c, c + 1 })[0])))
                  {
                     int errorIdx = static_cast<int>(c - syntaxSpecification.begin + textBegin);
                     errors.emplace_back(false, std::string("Expected arguments: Random(float start, float end) "), errorIdx, errorIdx + 1);
                     return nullptr;
                  }
                  break;
               case RandomParserState::end:
                  if (*c == ')')
                  {
                     endSlice.end = c;
                     if (validateFractionalNumber(endSlice))
                     {
                        end = stod(stdStringFromStringSlice(endSlice));
                        if (isNegative)
                        {
                           end = -end;
                           isNegative = false;
                        }
                        state = RandomParserState::closing;
                     }
                     else
                     {
                        int errorIdx = static_cast<int>(c - syntaxSpecification.begin + textBegin);
                        errors.emplace_back(false, std::string("Second argument to random() was missing or invalid "), errorIdx, errorIdx + 1);
                        return nullptr;
                     }
                  }
                  else if (*c == '-')
                  {
                     isNegative = true;
                     endSlice.begin = c + 1;
                  }
                  // skips spaces for validatefractionalnumber
                  else if (*c == ' ' || *c == '\n' || *c == '\t')
                  {
                     endSlice.begin = c + 1;
                  }
                  else if (!(*c == ' ' || *c == '\n' || *c == '\t' || isDigit(stdStringFromStringSlice(StringSlice{ c, c + 1 })[0])))
                  {
                     int errorIdx = static_cast<int>(c - syntaxSpecification.begin + textBegin);
                     errors.emplace_back(false, std::string("Expected arguments: Random(float start, float end) "), errorIdx, errorIdx + 1);
                     return nullptr;
                  }
                  break;
               case RandomParserState::closing:
                  if (*c == ']')
                  {
                     auto randomToken = ut::make_unique<RandomToken>(startSlice); // created with startSlice as its token, but this is not what its getSS function will return
                     randomToken->start = start;
                     randomToken->end = end;
                     return randomToken;
                  }
                  else
                  {
                     int errorIdx = static_cast<int>(c - syntaxSpecification.begin + textBegin);
                     errors.emplace_back(false, std::string("Expected closing token delimiter for function: ] "), errorIdx, errorIdx + 1);
                     return nullptr;
                  }
               case RandomParserState::comma:
                  break; 
            }
         }
         switch (state)
         {
               int errorIdx;
            case RandomParserState::end:
            case RandomParserState::start:
               errorIdx = static_cast<int>(startSlice.begin - syntaxSpecification.begin + textBegin);
               errors.emplace_back(false, std::string("Expected function parameters: random(float start, float end) "), errorIdx, errorIdx + 1);
               break;
            case RandomParserState::closing:
               errorIdx = static_cast<int>(c - syntaxSpecification.begin + textBegin);
               errors.emplace_back(false, std::string("Expected closing token delimiter for function: ] "), errorIdx, errorIdx + 1);
               break;
            case RandomParserState::comma:
               break;
         }
         return nullptr;
      }

      bool specifySyntax(const StringSlice syntaxSpecification, const Record& exampleRecord, std::vector<TemplateError>& errors)
      {
         tokens.clear();

         // Create a slice to keep track of the current slice of the syntax specification
         StringSlice slice;
         slice.begin = syntaxSpecification.begin;

         ConditionalToken* currentConditionalRawPtr = nullptr;

         ParserState state = ParserState::open;
         int literalConditionalCount = 0; // to differentiate between literal if(){} <-- and interpreted [if(){} <-- closing braces

         for (const char* c = slice.begin; c != syntaxSpecification.end; ++c)
         {
            switch (state)
            {
               case ParserState::open: // looking for any kind of token
                  if (*c == '[')
                  {
                     slice.end = c;
                     // If slice preceding opening delimiter is not empty, add it as a StringToken. Either to the current conditional or the parent list
                     if (!isEmpty(slice))
                     {
                        // We're in a conditional, push the token to its current case tokens
                        if (currentConditionalRawPtr != nullptr)
                        {
                           if (currentConditionalRawPtr->caseState == true)
                           {
                              currentConditionalRawPtr->trueCaseTokens.emplace_back(ut::make_unique<StringToken>(slice));
                           }
                           else
                           {
                              currentConditionalRawPtr->falseCaseTokens.emplace_back(ut::make_unique<StringToken>(slice));
                           }
                        }
                        // We're outside of any conditional, push the token to the top level vector
                        else
                        {
                           tokens.emplace_back((ut::make_unique<StringToken>(slice)));
                        }
                     }
                     // Check if its an if statement to enter a conditional
                     if (c + 5 < syntaxSpecification.end)
                     {
                        if (equal(StringSlice{ c, c + 4 }, stringSliceFromCString("[if(")) || equal(StringSlice{ c, c + 5 }, stringSliceFromCString("[if (")))
                        {
                           if (equal(StringSlice{ c, c + 4 }, stringSliceFromCString("[if(")))
                           {
                              slice.begin = c + 4;
                              c = c + 4;
                           }
                           else
                           {
                              slice.begin = c + 5;
                              c = c + 5;
                           }

                           int currentIdx = static_cast<int>(c - syntaxSpecification.begin);
                           auto errorCount = errors.size();


                           auto rootNode = std::move(parseConditional(c, StringSlice{ c, syntaxSpecification.end }, exampleRecord, errors, currentIdx));
                           if (errorCount != errors.size())
                           {
                              return false;
                           }
                           slice.end = c;
                           auto newConditional = ut::make_unique<ConditionalToken>(slice);
                           newConditional->rootNode = std::move(rootNode);
                           newConditional->finalConditional = false; // it's explicitely not an else statment
                           newConditional->caseState = true; // we add to its true cases first, regardless of if/elseif
                           newConditional->isAnIfConditional = false; // If statements are marked to return to upon final closing }] delimiters
                           newConditional->literalConditionalCount = 0;
                           if (equal(StringSlice{ slice.begin - 4, slice.begin }, stringSliceFromCString("[if(")) || equal(StringSlice{ slice.begin - 5, slice.begin }, stringSliceFromCString("[if (")))
                           {
                              newConditional->isAnIfConditional = true;
                           }

                           // If we are in a conditional, step into the new one and add the current as a parent
                           if (currentConditionalRawPtr != nullptr)
                           {
                              newConditional->parentToken = currentConditionalRawPtr;
                              if (currentConditionalRawPtr->caseState == true)
                              {
                                 currentConditionalRawPtr->trueCaseTokens.emplace_back(std::move(newConditional));
                                 currentConditionalRawPtr = static_cast<ConditionalToken*>(currentConditionalRawPtr->trueCaseTokens.back().get());
                              }
                              else
                              {
                                 currentConditionalRawPtr->falseCaseTokens.emplace_back(std::move(newConditional));
                                 currentConditionalRawPtr = static_cast<ConditionalToken*>(currentConditionalRawPtr->falseCaseTokens.back().get());
                              }
                           }
                           else
                           {
                              newConditional->parentToken = nullptr; // unsure if this is explicitely necessary
                              tokens.emplace_back(std::move(newConditional));
                              currentConditionalRawPtr = static_cast<ConditionalToken*>(tokens.back().get());
                           }
                           state = ParserState::postConditional;
                           slice.begin = c + 1;
                           break;
                        }
                     }
                     // Check if its a call to [toUpper() or [toLower()
                     if (c + 9 < syntaxSpecification.end)
                     {
                        if (equal(StringSlice{ c, c + 9 }, stringSliceFromCString("[toUpper(")) || equal(StringSlice{ c, c + 9 }, stringSliceFromCString("[toLower(")))
                        {
                           // Skip ahead, run the toCase parser on the syntaxSpecification, and check that it didn't pick up any errors
                           slice.begin = c + 9;
                           c = c + 9;
                           int currentIdx = static_cast<int>(c - syntaxSpecification.begin);
                           int errorCount = static_cast<int>(errors.size());

                           bool toUpper = equal(StringSlice{ c - 9, c }, stringSliceFromCString("[toUpper("));
                           parseToCase(c, syntaxSpecification, exampleRecord, errors, currentIdx, currentConditionalRawPtr, toUpper);
                           if (errorCount != static_cast<int>(errors.size()))
                           {
                              return false;
                           }
                           slice.begin = c + 1;
                           break;
                        }
                     }
                     // Check if its a call to [translate()
                     if (c + 11 < syntaxSpecification.end)
                     {
                        if (equal(StringSlice{ c, c + 11 }, stringSliceFromCString("[translate(")))
                        {
                           // Skip ahead, run the translate parser on the syntaxSpecification, and check that it didn't pick up any errors
                           slice.begin = c + 11;
                           c = c + 11;
                           int currentIdx = static_cast<int>(c - syntaxSpecification.begin);
                           int errorCount = static_cast<int>(errors.size());
                           parseTranslate(c, StringSlice{ slice.begin, syntaxSpecification.end }, exampleRecord, errors, currentIdx, currentConditionalRawPtr);
                           if (errorCount != static_cast<int>(errors.size()))
                           {
                              return false;
                           }
                           slice.begin = c + 1;
                           break;
                        }
                     }
                     // Check if its a call to [random()
                     if (c + 8 < syntaxSpecification.end)
                     {
                        if (equal(StringSlice{ c, c + 8 }, stringSliceFromCString("[random(")))
                        {
                           // Skip ahead, run the random parser on the syntaxSpecification, and check that it didn't pick up any errors
                           slice.begin = c + 8;
                           c = c + 8;
                           int currentIdx = static_cast<int>(c - syntaxSpecification.begin);
                           int errorCount = static_cast<int>(errors.size());

                           auto uniqueRandomtoken = parseRandom(c, StringSlice{ slice.begin, syntaxSpecification.end }, exampleRecord, errors, currentIdx, currentConditionalRawPtr);
                           if (errorCount != static_cast<int>(errors.size()))
                           {
                              return false;
                           }
                           if (currentConditionalRawPtr != nullptr)
                           {
                              if (currentConditionalRawPtr->caseState == true)
                              {
                                 currentConditionalRawPtr->trueCaseTokens.emplace_back(std::move(uniqueRandomtoken));
                              }
                              else
                              {
                                 currentConditionalRawPtr->falseCaseTokens.emplace_back(std::move(uniqueRandomtoken));
                              }
                           }
                           else
                           {
                              tokens.emplace_back(std::move(uniqueRandomtoken));
                           }
                           slice.begin = c + 1;
                           break;
                        }
                     }

                     // Start the next slice
                     slice.begin = c + 1;
                     state = ParserState::inToken;
                  }


                  else if (equal(StringSlice{ c - 5, c }, stringSliceFromCString(" else")) && !equal(StringSlice{ c - 4, c + 3 }, stringSliceFromCString("else if")))
                  {
                     // If we are in a conditional we want to add the proceding tokens to it's false case vector, beging in state::postBlock we are already in the false case setting
                     slice.end = c;
                     int errorBegin = static_cast<int>((c - 4) - syntaxSpecification.begin);
                     int errorEnd = static_cast<int>((c) - syntaxSpecification.begin);
                     errors.emplace_back(false, std::string("Else without previous if "), errorBegin, errorEnd);
                     state = ParserState::open; // Continue parsing as if we properly entered the else, to error check
                     slice.begin = c + 1;
                  }
                  else if (*c == '{')
                  {
                     if (currentConditionalRawPtr != nullptr)
                     {
                        currentConditionalRawPtr->literalConditionalCount++;
                     }
                     else
                     {
                        literalConditionalCount++;
                     }
                  }
                  else if (*c == '}')
                  {
                     slice.end = c;
                     // If we're in a conditional move through it and look for false cases
                     if (currentConditionalRawPtr != nullptr)
                     {
                        // Attempt to close an interpreted conditional statement, or skip the delimiter if we are in some level of literal conditional statement
                        if (currentConditionalRawPtr->literalConditionalCount == 0)
                        {
                           if (!isEmpty(slice))
                           {
                              if (currentConditionalRawPtr->caseState == true)
                              {
                                 currentConditionalRawPtr->trueCaseTokens.emplace_back(ut::make_unique<StringToken>(slice));
                              }
                              else
                              {
                                 currentConditionalRawPtr->falseCaseTokens.emplace_back(ut::make_unique<StringToken>(slice));
                              }
                           }
                           if (currentConditionalRawPtr->caseState == true)
                           {
                              currentConditionalRawPtr->caseState = false;
                           }
                           // return to adding further else ifs to the parent if in the case that we were in an else-if
                           if (!currentConditionalRawPtr->isAnIfConditional)
                           {
                              currentConditionalRawPtr = currentConditionalRawPtr->parentToken;
                           }
                           state = ParserState::postBlock; // postBlock allows entry into further else/elseifs, or will step out of the current conditional
                           slice.begin = c + 1;
                        }
                        else
                        {
                           currentConditionalRawPtr->literalConditionalCount--;
                        }
                     }
                     // Check the base level literal count as well
                     else if (literalConditionalCount == 0)
                     {
                        int errorEnd = static_cast<int>((c) - syntaxSpecification.begin);
                        errors.emplace_back(false, std::string("Closing block delimiter } outside of conditional statement "), errorEnd, errorEnd + 1);
                     }
                     else
                     {
                        literalConditionalCount--;
                     }
                  }
                  // in the state::open case, if we find ] it is meant to signal the (final) end of a conditional
                  // Therefore check that we are in a conditional and step out of it
                  else if (*c == ']')
                  {
                     if (currentConditionalRawPtr != nullptr)
                     {
                        if (currentConditionalRawPtr->parentToken != nullptr)
                        {
                           currentConditionalRawPtr = currentConditionalRawPtr->parentToken;
                        }
                        else
                        {
                           currentConditionalRawPtr = nullptr; // prompts token addition to the top level vector
                        }
                        slice.begin = c + 1;
                     }
                     else
                     {
                        int errorEnd = static_cast<int>((c) - syntaxSpecification.begin);
                        errors.emplace_back(false, std::string("Closing delimiter: ] without opening "), errorEnd, errorEnd + 1);
                     }
                  }
                  break;
               case ParserState::inToken:
                  if (*c == ']')
                  {
                     slice.end = c;
                     if (isEmpty(getAttribute(std::string(slice.begin, length(slice)), exampleRecord)))
                     {
                        int errorEnd = static_cast<int>(slice.end - syntaxSpecification.begin);
                        int errorBegin = static_cast<int>(slice.begin - syntaxSpecification.begin);
                        errors.emplace_back(false, std::string("Invalid identifier, must match a name from template input "), errorBegin, errorEnd);
                     }
                     else
                     {
                        if (currentConditionalRawPtr != nullptr)
                        {
                           if (!isEmpty(slice))
                           {
                              if (currentConditionalRawPtr->caseState == true)
                              {
                                 currentConditionalRawPtr->trueCaseTokens.emplace_back(ut::make_unique<AttributeToken>(slice));
                              }
                              else
                              {
                                 currentConditionalRawPtr->falseCaseTokens.emplace_back(ut::make_unique<AttributeToken>(slice));
                              }
                           }
                        }
                        else
                        {
                           tokens.emplace_back(ut::make_unique<AttributeToken>(slice));
                        }
                     }
                     slice.begin = c + 1;
                     state = ParserState::open;
                  }
                  break;
               case ParserState::postConditional: // We found a conditional and are looking to enter it's block when we find {
                  if (*c == '{')
                  {
                     slice.end = c;
                     validateSpaceSlice(slice, errors, static_cast<int>(slice.begin - syntaxSpecification.begin));
                     state = ParserState::open;
                     slice.begin = c + 1;
                  }
                  else if (!(*c == ' ' || *c == '\n' || *c == '\t'))
                  {
                     int errorIdx = static_cast<int>(slice.begin - syntaxSpecification.begin);
                     errors.emplace_back(false, std::string("Expected opening block delimiter: { "), errorIdx, errorIdx + 1);
                     c--; //Allow open to continue parsing for errors
                     state = ParserState::open;
                  }
                  break;
               case ParserState::postBlock: // We are in a conditional and want to determine if we step out of it or add to its false conditionals
                  // If we see the final close delimiter we step out of the current conditional,
                  // travelling up its parents until we are one level above the parent "if" conditional or root
                  if (*c == ']')
                  {
                     while (currentConditionalRawPtr->parentToken != nullptr && !(currentConditionalRawPtr->isAnIfConditional))
                     {
                        currentConditionalRawPtr = currentConditionalRawPtr->parentToken;
                     }
                     // Once we are in the parent if we want to move to its parent, a conditional or the root vector
                     if (currentConditionalRawPtr->parentToken != nullptr)
                     {
                        currentConditionalRawPtr = currentConditionalRawPtr->parentToken;
                     }
                     else
                     {
                        currentConditionalRawPtr = nullptr;
                     }
                     state = ParserState::open;
                     slice.begin = c + 1;
                     break;
                  }
                  if (*c == '}')
                  {
                     int errorIdx = static_cast<int>(slice.begin - syntaxSpecification.begin);
                     errors.emplace_back(false, std::string("Expected conditional or closing: ] "), errorIdx, errorIdx + 1);
                     slice.begin = c + 1;
                     break;
                  }
                  if (equal(StringSlice{ c - 4, c }, stringSliceFromCString("else")) && !equal(StringSlice{ c - 4, c + 3 }, stringSliceFromCString("else if")) && (*(c - 5) == '\n' || (*c - 5) == '\t' || *(c - 5) == ' '))
                  {
                     // Else conditionals are given a "true" token to be always executed
                     slice.end = c;
                     validateSpaceSlice(StringSlice{ slice.begin, c - 4 }, errors, static_cast<int>(slice.begin - syntaxSpecification.begin));
                     auto newConditional = ut::make_unique<ConditionalToken>(stringSliceFromCString("true"));
                     newConditional->finalConditional = true; // it's an else statment
                     newConditional->caseState = true; // we add to its true cases first, regardless of if/elseif
                     newConditional->isAnIfConditional = false; // If statements are marked to return to upon final closing }] delimiters
                     newConditional->parentToken = currentConditionalRawPtr;
                     newConditional->literalConditionalCount = 0;
                     if (currentConditionalRawPtr->caseState == true)
                     {
                        currentConditionalRawPtr->trueCaseTokens.emplace_back(std::move(newConditional));
                        currentConditionalRawPtr = static_cast<ConditionalToken*>(currentConditionalRawPtr->trueCaseTokens.back().get());
                     }
                     else
                     {
                        currentConditionalRawPtr->falseCaseTokens.emplace_back(std::move(newConditional));
                        currentConditionalRawPtr = static_cast<ConditionalToken*>(currentConditionalRawPtr->falseCaseTokens.back().get());
                     }
                     state = ParserState::postConditional;
                     if (c + 1 < syntaxSpecification.end)
                     {
                        slice.begin = c + 1;
                     }
                  }
                  else if (*c == '(')
                  {
                     // An else if remains in the current conditional and gets added (its caseState should already be false)
                     // This is heavily repeated code from entering the [if( statemetn in the open case!
                     if ((equal(StringSlice{ c - 7, c }, stringSliceFromCString("else if")) || equal(StringSlice{ c - 8, c }, stringSliceFromCString("else if "))) && c + 1 != syntaxSpecification.end)
                     {
                        slice.end = c - 8;
                        validateSpaceSlice(slice, errors, static_cast<int>(slice.begin - syntaxSpecification.begin));

                        int currentIdx = static_cast<int>(c - syntaxSpecification.begin);
                        auto errorCount = errors.size();

                        // skip the '(' which is skipped for the same reason
                        // in [if( statements
                        c = c + 1;

                        auto rootRelationalTerm = std::move(parseConditional(c, StringSlice{ c, syntaxSpecification.end }, exampleRecord, errors, currentIdx));
                        if (errorCount != errors.size())
                        {
                           return false;
                        }
                        auto newConditional = ut::make_unique<ConditionalToken>(slice);
                        newConditional->rootNode = std::move(rootRelationalTerm);
                        newConditional->finalConditional = false; // it's explicitely not an else statment
                        newConditional->caseState = true; // we add to its true cases first, regardless of if/elseif
                        newConditional->isAnIfConditional = false; // If statements are marked to return to upon final closing }] delimiters
                        newConditional->literalConditionalCount = 0;
                        newConditional->isAnIfConditional = false;

                        // currentConditional has to exist to have reached this case:
                        newConditional->parentToken = currentConditionalRawPtr;
                        if (currentConditionalRawPtr->caseState == true)
                        {
                           currentConditionalRawPtr->trueCaseTokens.emplace_back(std::move(newConditional));
                           currentConditionalRawPtr = static_cast<ConditionalToken*>(currentConditionalRawPtr->trueCaseTokens.back().get());
                        }
                        else
                        {
                           currentConditionalRawPtr->falseCaseTokens.emplace_back(std::move(newConditional));
                           currentConditionalRawPtr = static_cast<ConditionalToken*>(currentConditionalRawPtr->falseCaseTokens.back().get());
                        }

                        state = ParserState::postConditional;
                        slice.begin = c + 1;
                     }
                  }
                  // If we hit a token it indicates
                  else if (*c == '[')
                  {
                     int errorIdx = static_cast<int>(slice.begin - syntaxSpecification.begin);
                     errors.emplace_back(false, std::string("Expected conditional or closing: ] "), errorIdx, errorIdx + 1);
                     slice.begin = c + 1;
                     break;
                  }
                  break;
               
               default:
                  break;
            }
         }

         slice.end = syntaxSpecification.end;
         // If we hit the end of the end of the template we generate an error depending on the case we were in,
         // or just add the final bit as a string token
         int errorIdx;
         switch (state)
         {
            case ParserState::postConditional:
               errorIdx = static_cast<int>(slice.begin - syntaxSpecification.begin);
               errors.emplace_back(false, std::string("Expected opening block delimiter: { "), errorIdx - 1, errorIdx);
               break;
            case ParserState::inBlock:
               errorIdx = static_cast<int>(slice.begin - syntaxSpecification.begin);
               errors.emplace_back(false, std::string("Opening block { never closed "), errorIdx - 1, errorIdx);
               break;
            case ParserState::inToken:
               errorIdx = static_cast<int>(slice.begin - syntaxSpecification.begin);
               errors.emplace_back(false, std::string("Opening token [ never closed "), errorIdx - 1, errorIdx);
               break;
            case ParserState::postBlock:
               errorIdx = static_cast<int>(slice.begin - syntaxSpecification.begin);
               errors.emplace_back(false, std::string("Ended without closing a conditional statement: ]"), errorIdx - 1, errorIdx);
            case ParserState::open:
               if (!isEmpty(slice))
               {
                  if (currentConditionalRawPtr != nullptr)
                  {
                     // The case where we had some text after a closed, 0 level conditional statement, and it wasn't still in it's first if(){ block
                     if (currentConditionalRawPtr->parentToken == nullptr && currentConditionalRawPtr->caseState == false)
                     {
                        tokens.emplace_back(ut::make_unique<StringToken>(slice));
                     }
                     // The case where we had some text left over after a nested conditional statement
                     else
                     {
                        errorIdx = static_cast<int>(slice.begin - syntaxSpecification.begin);
                        errors.emplace_back(false, std::string("Ended without closing a conditional statement: ]"), errorIdx - 1, errorIdx);
                     }
                  }
                  else
                  {
                     tokens.emplace_back(ut::make_unique<StringToken>(slice));
                  }
               }
            default:
               break;
         }
         return errors.empty();
      }
};
}
