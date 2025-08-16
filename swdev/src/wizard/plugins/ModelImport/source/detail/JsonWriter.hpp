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

#ifndef JSONWRITER_HPP
#define JSONWRITER_HPP

#include <fstream>
#include <stdexcept>
#include <string>

namespace Json
{
class WriterException : public std::runtime_error
{
public:
   explicit WriterException(const std::string& aWhat);
};

//! A wrapper around a std::ostream (non-owning).
//! Provides functionality to facilitate writing and formatting Json files.
//! @see Json::ArrayWriter
//! @see Json::ObjectWriter
//! @see Json::WriteFile
class FileWriter
{
public:
   friend class ArrayWriter;
   friend class ObjectWriter;

   //! Initializes the FileWriter with a file.
   //! Can optionally be used used with std::cout or std::cerr.
   //! @param aDestination The output to write to.
   explicit FileWriter(std::ostream& aDestination);

   FileWriter(const FileWriter&) = delete;
   FileWriter(FileWriter&&)      = delete;

   FileWriter& operator=(const FileWriter&) = delete;
   FileWriter& operator=(FileWriter&&) = delete;

   //! Puts a value into the file using default STL stream operators.
   //! @param aValue This is the value to write to the file.
   //! @returns Returns a reference to the calling object.
   //! @note Type T requires `operator<<(std::ostream&, const T&)` exists.
   template<typename T>
   FileWriter& PutRaw(const T& aValue)
   {
      mDestination << aValue;
      return *this;
   }

   //! Puts a properly formatted string into the file.
   //! Includes beginning and ending quotation marks and formatting escaped characters.
   //! @param aValue This is the string to write to the file.
   FileWriter& PutString(const std::string& aValue);

   //! Puts a properly formatted string character into the file.
   //! Characters requiring escape sequences are replaced.
   //! @param aValue This is the character to write to the file.
   FileWriter& PutChar(char aValue);

   //! Writes a new line into the file and sutomatically indents.
   void NewLine();

private:
   std::ostream& mDestination;
   int           mIndentation = 0;
};

//! Writes Json arrays (bracketed list of values) to a Json::FileWriter.
//! Intended for use in Json::Rule<T> specializations.
//! Example usage:
//! void Json::Rule<List>::Write(Json::FileWriter& aDestination, const List& aValue)
//! {
//!    Json::ArrayWriter array(aDestination);
//!    for(const auto& element : aValue)
//!    {
//!       array.WriteValue(element);
//!    }
//! }
//! @see Json::Rule
class ArrayWriter
{
public:
   //! Begins writing an array to a Json::FileWriter.
   //! @param aDestination This is the file to write to.
   explicit ArrayWriter(FileWriter& aDestination);

   //! Closes the array with a bracket.
   ~ArrayWriter() noexcept;

   ArrayWriter(const ArrayWriter&) = delete;
   ArrayWriter(ArrayWriter&&)      = delete;

   ArrayWriter& operator=(const ArrayWriter&) = delete;
   ArrayWriter& operator=(ArrayWriter&&) = delete;

   //! Puts a value into the array.
   //! @param aValue This is the value to add to the array.
   //! @note Type T requires `static void Rule<T>::Write(FileWriter&, const T&)` exists.
   template<typename T>
   void WriteValue(const T& aValue)
   {
      if (mComma)
      {
         mDestination.PutRaw(',');
      }
      mDestination.NewLine();

      mComma = true;

      Rule<T>::Write(mDestination, aValue);
   }

private:
   FileWriter& mDestination;
   bool        mComma = false;
};

//! Writes Json objects (braced list of "Key" : Value pairs) to a Json::FileWriter.
//! Intended for use in Json::Rule<T> specializations.
//! Example usage:
//! void Json::Rule<Person>::Write(Json::FileWriter& aDestination, const Person& aValue)
//! {
//!    Json::ObjectWriter object(aDestination);
//!    object.WriteValue("Name", aValue.mName);
//!    object.WriteValue("Age", aValue.mAge);
//! }
//! @see Json::Rule
class ObjectWriter
{
public:
   //! Begins writing an object to a Json::FileWriter.
   //! @param aDestination This is the file to write to.
   explicit ObjectWriter(FileWriter& aDestination);

   //! Closes the object with a brace.
   ~ObjectWriter() noexcept;

   ObjectWriter(const ObjectWriter&) = delete;
   ObjectWriter(ObjectWriter&&)      = delete;

   ObjectWriter& operator=(const ObjectWriter&) = delete;
   ObjectWriter& operator=(ObjectWriter&&) = delete;

   //! Puts a key-value pair into the object.
   //! @param aKey This is the key for the object field to write.
   //! @param aValue This is the value to put in the field.
   //! @note Type Key requires 'static void Rule<T>::WriteKey(FileWriter&, const Key&)` exists.
   //! @note Type T requires `static void Rule<T>::Write(FileWriter&, const T&)` exists.
   template<typename Key, typename T>
   void WriteValue(const Key& aKey, const T& aValue)
   {
      if (mComma)
      {
         mDestination.PutRaw(',');
      }
      mDestination.NewLine();

      mComma = true;

      Rule<Key>::WriteKey(mDestination, aKey);
      mDestination.PutRaw(": ");
      Rule<T>::Write(mDestination, aValue);
   }

private:
   FileWriter& mDestination;
   bool        mComma = false;
};

//! Writes a file with a given name and fills its contents with an object.
//! @param aFileName This is the name of the file to read from.
//! @param aValue This is the value to write to the file.
//! @note Type T requires `static void Rule<T>::Write(Json::FileReader&, T&)` exists.
//! @note If T is not defined to write as an array or object, the Json file will not be valid.
template<typename T>
void WriteFile(const std::string& aFileName, const T& aValue)
{
   using Json::Rule;

   std::ofstream fileStream(aFileName);

   Json::FileWriter reader(fileStream);

   Rule<T>::Write(reader, aValue);
}
} // namespace Json

#endif
