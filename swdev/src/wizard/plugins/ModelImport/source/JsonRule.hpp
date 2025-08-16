// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2019 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef JSONRULE_HPP
#define JSONRULE_HPP

namespace Json
{
//! Meta-type which defines how different types are read from and written to Json files.
//! All behavior is defined through static functions.
//! For convenience, it is recommended that the alias 'type' is used to refer to the type.
//! @see ArrayReader
//! @see ObjectReader
//! @see ArrayWriter
//! @see ObjectWriter
//! @note Only the functions corresponding to desired behavior need to be defined.
//! @note The following code snippet may be copied and modified to implement new Rules.

/**
template<>
struct Rule<T>       //Replace T with the type being specialized for.
{
   using type = T;   //Replace T with the type being specialized for.
//Required for Values:
   //! Defines how VALUES are to be read from file.
   //! @param aSource This is the file to read from.
   //! @param aValue This is the value to fill with the read contents.
   static void Read(Json::FileReader& aSource, type& aValue);

   //! Defines how VALUES are to be written to file.
   //! @param aDestination The file to write to.
   //! @param aValue The value to write to file.
   static void Write(Json::FileWriter& aDestination, const type& aValue);

//Required for Keys:
   //! Defines how KEYS are to be read from file.
   //! Keys are required to be in quotation marks.
   //! For types that define this, it will typically be the same as Read.
   //! @param aSource This is the file to read from.
   //! @param aValue This is the value to fill with the read contents.
   static void ReadKey(Json::FileReader& aSource, type& aValue);

   //! Defines how KEYS are to be written to file.
   //! Keys are required to be in quotation marks.
   //! For types that define this, it will typically be the same as Write.
   //! @param aDestination The file to write to.
   //! @param aValue The value to write to file.
   static void WriteKey(Json::FileWriter& aDestination, const type& aValue);

   //! Defines how keys are to be compared to a key in a file.
   //! @param aKey This is the key present in the file.
   //! @param aValue This is the value to compare to the key in the file.
   //! @returns True if the keys match, false otherwise.
   static bool KeyEquals(const std::string& aKey, const type& aValue);
};
*/
template<typename T, typename = void>
struct Rule;
} // namespace Json

#include <array>
#include <list>
#include <sstream>
#include <string>
#include <vector>

#include <QStringList>

#include "UtTypeTraits.hpp"
#include "detail/JsonReader.hpp"
#include "detail/JsonWriter.hpp"

namespace Json
{
//! @note Current implementation does not have range checking.
template<typename Arithmetic>
struct Rule<Arithmetic, ut::enable_if_t<std::is_arithmetic<Arithmetic>::value>>
{
   using type = Arithmetic;

   //! Defines how VALUES are to be read from file.
   //! @param aSource This is the file to read from.
   //! @param aValue This is the value to fill with the read contents.
   static void Read(Json::FileReader& aSource, type& aValue)
   {
      if (aSource.IsNull())
      {
         aValue = 0;
      }
      else if (aSource.IsString())
      {
         throw ReaderException("Unexpected quotes around arithmetic type.", aSource.GetLine());
      }
      else
      {
         std::stringstream ss;
         ss << aSource.Get();
         ss >> aValue;
      }
   }

   //! Defines how VALUES are to be written to file.
   //! @param aDestination The file to write to.
   //! @param aValue The value to write to file.
   static void Write(Json::FileWriter& aDestination, const type& aValue) { aDestination.PutRaw(aValue); }
};

//! Reading to or writing from a single character is not supported.
template<>
struct Rule<char>
{
};

//! Reading to or writing from a single character is not supported.
template<>
struct Rule<signed char>
{
};

//! Reading to or writing from a single character is not supported.
template<>
struct Rule<unsigned char>
{
};

template<>
struct Rule<std::string>
{
   using type = std::string;

   //! Defines how VALUES are to be read from file.
   //! @param aSource This is the file to read from.
   //! @param aValue This is the value to fill with the read contents.
   static void Read(Json::FileReader& aSource, type& aValue)
   {
      if (aSource.IsNull())
      {
         aValue = "";
      }
      else if (!aSource.IsString())
      {
         throw ReaderException("Expected string to be in quotes.", aSource.GetLine());
      }
      else
      {
         aValue = aSource.Get();
      }
   }

   //! Defines how VALUES are to be written to file.
   //! @param aDestination The file to write to.
   //! @param aValue The value to write to file.
   static void Write(Json::FileWriter& aDestination, const type& aValue) { aDestination.PutString(aValue); }

   //! Defines how KEYS are to be read from file.
   //! Keys are required to be in quotation marks.
   //! For types that define this, it will typically be the same as Read.
   //! @param aSource This is the file to read from.
   //! @param aValue This is the value to fill with the read contents.
   static void ReadKey(Json::FileReader& aSource, type& aValue) { Read(aSource, aValue); }

   //! Defines how KEYS are to be written to file.
   //! Keys are required to be in quotation marks.
   //! For types that define this, it will typically be the same as Write.
   //! @param aDestination The file to write to.
   //! @param aValue The value to write to file.
   static void WriteKey(Json::FileWriter& aDestination, const type& aValue) { Write(aDestination, aValue); }

   //! Defines how keys are to be compared to a key in a file.
   //! @param aKey This is the key present in the file.
   //! @param aValue This is the value to compare to the key in the file.
   //! @returns True if the keys match, false otherwise.
   static bool KeyEquals(const std::string& aKey, const type& aValue) { return aKey == aValue; }
};

template<>
struct Rule<QString>
{
   using type = QString;

   //! Defines how VALUES are to be read from file.
   //! @param aSource This is the file to read from.
   //! @param aValue This is the value to fill with the read contents.
   static void Read(Json::FileReader& aSource, type& aValue)
   {
      if (aSource.IsNull())
      {
         aValue = "";
      }
      else if (!aSource.IsString())
      {
         throw ReaderException("Expected string to be in quotes.", aSource.GetLine());
      }
      else
      {
         // QByteArray is implicitly convertible to QString, so this is functionally equivalent to
         // QString::fromStdString. However, using QByteArray as an intermediate masks a string
         // termination vulnerability identified by static code analysis, with sink in QString's
         // use of 'strlen'. The source is input from a file stream in Json::FileReader, which
         // should be null-terminated, so this is apparently a false positive, but this workaround
         // might be unnecessary in a future version of Qt.
         aValue = QByteArray::fromStdString(aSource.Get());
      }
   }

   //! Defines how VALUES are to be written to file.
   //! @param aDestination The file to write to.
   //! @param aValue The value to write to file.
   static void Write(Json::FileWriter& aDestination, const type& aValue)
   {
      aDestination.PutString(aValue.toStdString());
   }

   //! Defines how KEYS are to be read from file.
   //! Keys are required to be in quotation marks.
   //! For types that define this, it will typically be the same as Read.
   //! @param aSource This is the file to read from.
   //! @param aValue This is the value to fill with the read contents.
   static void ReadKey(Json::FileReader& aSource, type& aValue) { Read(aSource, aValue); }

   //! Defines how KEYS are to be written to file.
   //! Keys are required to be in quotation marks.
   //! For types that define this, it will typically be the same as Write.
   //! @param aDestination The file to write to.
   //! @param aValue The value to write to file.
   static void WriteKey(Json::FileWriter& aDestination, const type& aValue) { Write(aDestination, aValue); }

   //! Defines how keys are to be compared to a key in a file.
   //! @param aKey This is the key present in the file.
   //! @param aValue This is the value to compare to the key in the file.
   //! @returns True if the keys match, false otherwise.
   static bool KeyEquals(const std::string& aKey, const type& aValue) { return aKey == aValue.toStdString(); }
};

//! @note Reading into a char* could be dangerous. Therefore it is not supported.
template<>
struct Rule<const char*>
{
   using type = const char*;

   //! Defines how VALUES are to be written to file.
   //! @param aDestination The file to write to.
   //! @param aValue The value to write to file.
   static void Write(Json::FileWriter& aDestination, const type& aValue) { aDestination.PutString(aValue); }

   //! Defines how KEYS are to be written to file.
   //! Keys are required to be in quotation marks.
   //! For types that define this, it will typically be the same as Write.
   //! @param aDestination The file to write to.
   //! @param aValue The value to write to file.
   static void WriteKey(Json::FileWriter& aDestination, const type& aValue) { Write(aDestination, aValue); }

   //! Defines how keys are to be compared to a key in a file.
   //! @param aKey This is the key present in the file.
   //! @param aValue This is the value to compare to the key in the file.
   //! @returns True if the keys match, false otherwise.
   static bool KeyEquals(const std::string& aKey, const type& aValue) { return aKey == aValue; }
};

//! @note String literals deduce to this type.
template<int N>
struct Rule<char[N]>
{
   using type = char[N];

   //! Defines how VALUES are to be written to file.
   //! @param aDestination The file to write to.
   //! @param aValue The value to write to file.
   static void Write(Json::FileWriter& aDestination, const type& aValue) { aDestination.PutString(aValue); }

   //! Defines how KEYS are to be written to file.
   //! Keys are required to be in quotation marks.
   //! For types that define this, it will typically be the same as Write.
   //! @param aDestination The file to write to.
   //! @param aValue The value to write to file.
   static void WriteKey(Json::FileWriter& aDestination, const type& aValue) { Write(aDestination, aValue); }

   //! Defines how keys are to be compared to a key in a file.
   //! @param aKey This is the key present in the file.
   //! @param aValue This is the value to compare to the key in the file.
   //! @returns True if the keys match, false otherwise.
   static bool KeyEquals(const std::string& aKey, const type& aValue) { return aKey == aValue; }
};

template<>
struct Rule<bool>
{
   using type = bool;

   //! Defines how VALUES are to be read from file.
   //! @param aSource This is the file to read from.
   //! @param aValue This is the value to fill with the read contents.
   static void Read(Json::FileReader& aSource, type& aValue)
   {
      if (aSource.IsNull())
      {
         aValue = false; // This is probably the most logical option.
      }
      else if (aSource.IsString())
      {
         throw ReaderException("Unexpected quotes around bool.", aSource.GetLine());
      }
      else if (aSource.Get() == "true")
      {
         aValue = true;
      }
      else if (aSource.Get() == "false")
      {
         aValue = false;
      }
      else
      {
         throw ReaderException("A boolean must be 'true' or 'false'", aSource.GetLine());
      }
   }

   //! Defines how VALUES are to be written to file.
   //! @param aDestination The file to write to.
   //! @param aValue The value to write to file.
   static void Write(Json::FileWriter& aDestination, const type& aValue)
   {
      if (aValue)
      {
         aDestination.PutRaw("true");
      }
      else
      {
         aDestination.PutRaw("false");
      }
   }
};

//! Reads or writes all values in the array.
//! If there are n values to read, then if...
//!    n <  N: the remining elements are unaffected.
//!    n == N: all values are read.
//!    n >  N: only the first N values are read.
template<typename T, int N>
struct Rule<std::array<T, N>>
{
   using type = std::array<T, N>;

   //! Defines how VALUES are to be read from file.
   //! @param aSource This is the file to read from.
   //! @param aValue This is the value to fill with the read contents.
   static void Read(Json::FileReader& aSource, type& aValue)
   {
      Json::ArrayReader reader(aSource);

      std::size_t index = 0;

      while (reader && index < aValue.size())
      {
         reader.ReadValue(aValue[index]);
         index++;
      }
   }

   //! Defines how VALUES are to be written to file.
   //! @param aDestination The file to write to.
   //! @param aValue The value to write to file.
   static void Write(Json::FileWriter& aDestination, const type& aValue)
   {
      Json::ArrayWriter writer(aDestination);

      for (const T& element : aValue)
      {
         writer.WriteValue(element);
      }
   }
};

template<typename T>
struct Rule<std::vector<T>>
{
   using type = std::vector<T>;

   //! Defines how VALUES are to be read from file.
   //! @param aSource This is the file to read from.
   //! @param aValue This is the value to fill with the read contents.
   static void Read(Json::FileReader& aSource, type& aValue)
   {
      Json::ArrayReader reader(aSource);

      aValue.resize(0); // Keeps previous capacity

      while (reader)
      {
         T temp;
         reader.ReadValue(temp);
         aValue.push_back(std::move(temp));
      }
   }

   //! Defines how VALUES are to be written to file.
   //! @param aDestination The file to write to.
   //! @param aValue The value to write to file.
   static void Write(Json::FileWriter& aDestination, const type& aValue)
   {
      Json::ArrayWriter writer(aDestination);

      for (const T& element : aValue)
      {
         writer.WriteValue(element);
      }
   }
};

template<typename T>
struct Rule<std::list<T>>
{
   using type = std::list<T>;

   //! Defines how VALUES are to be read from file.
   //! @param aSource This is the file to read from.
   //! @param aValue This is the value to fill with the read contents.
   static void Read(Json::FileReader& aSource, type& aValue)
   {
      Json::ArrayReader reader(aSource);

      aValue.clear();

      while (reader)
      {
         T temp;
         reader.ReadValue(temp);
         aValue.push_back(std::move(temp));
      }
   }

   //! Defines how VALUES are to be written to file.
   //! @param aDestination The file to write to.
   //! @param aValue The value to write to file.
   static void Write(Json::FileWriter& aDestination, const type& aValue)
   {
      Json::ArrayWriter writer(aDestination);

      for (const T& element : aValue)
      {
         writer.WriteValue(element);
      }
   }
};

template<>
struct Rule<QStringList>
{
   using type = QStringList;

   //! Defines how VALUES are to be read from file.
   //! @param aSource This is the file to read from.
   //! @param aValue This is the value to fill with the read contents.
   static void Read(Json::FileReader& aSource, type& aValue)
   {
      Json::ArrayReader reader(aSource);

      aValue.clear();

      while (reader)
      {
         QString temp;
         reader.ReadValue(temp);
         aValue << std::move(temp);
      }
   }

   //! Defines how VALUES are to be written to file.
   //! @param aDestination The file to write to.
   //! @param aValue The value to write to file.
   static void Write(Json::FileWriter& aDestination, const type& aValue)
   {
      Json::ArrayWriter writer(aDestination);

      for (const QString& element : aValue)
      {
         writer.WriteValue(element);
      }
   }
};

} // namespace Json

#endif
