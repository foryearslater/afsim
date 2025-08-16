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

#ifndef JSONREADER_HPP
#define JSONREADER_HPP

#include <fstream>
#include <stdexcept>
#include <string>

namespace Json
{
class ReaderException : public std::runtime_error
{
public:
   explicit ReaderException(const std::string& aWhat, int aLine);
};

//! A wrapper around a std::istream (non-owning).
//! Provides functionality to facilitate reading Json files.
//! @see Json::ArrayReader
//! @see Json::ObjectReader
//! @see Json::ReadFile
class FileReader
{
public:
   friend class ArrayReader;
   friend class ObjectReader;

   //! Initializes FileReader with a file.
   //! Can optionally be used with std::cin.
   //! @param aSource The input to read from.
   explicit FileReader(std::istream& aSource);

   FileReader(const FileReader&) = delete;
   FileReader(FileReader&&)      = delete;

   FileReader& operator=(const FileReader&) = delete;
   FileReader& operator=(FileReader&&) = delete;

   //! Returns false if the underlying stream has been invalidated.
   explicit operator bool() const noexcept;

   //! Returns true if the underlying stream has been invalidated.
   bool operator!() const noexcept;

   //! Returns true if the read token is null.
   bool IsNull() const noexcept;

   //! Returns true if the read token is a string.
   bool IsString() const noexcept;

   //! Returns the text of the read token.
   //! @note String values do not include the quotation marks.
   const std::string& Get() const noexcept;

   //! Returns the line number of the read token.
   int GetLine() const noexcept;

private:
   std::istream& mSource;
   std::string   mQueued;
   int           mLineNumber = 1;
   bool          mIsString   = false;

   //! Ignores the next aAmount read tokens.
   //! @param aAmount The number of values to ignore. Default = 1.
   //! @note This function is private, but intended for use by ArrayReader and ObjectReader.
   void Ignore(int aAmount = 1) noexcept;

   //! Ignores the next complex value. I.e. array, object, or token.
   //! @note This function is private, but intended for use by ArrayReader and ObjectReader.
   void IgnoreValue() noexcept;

   //! Checks if the read token matches a given non-string token. If it matches, can continue on to the next token.
   //! @param aValue This is the token to check against.
   //! @param aPopIfTrue Determines whether or not to continue on to the next token in the case of a match. Default = true.
   //! @note This function is private, but intended for use by ArrayReader and ObjectReader.
   bool CheckSymbol(const std::string& aValue, bool aPopIfTrue = true);

   // Internal use only
   void Parse_() noexcept;

   // Internal use only
   // First time reading past end of file, returns NULL.
   char GetChar_() noexcept;
};

//! Reads Json arrays (bracketed list of values) from a Json::FileReader.
//! Intended for use in Json::Rule<T> specializations.
//! Example usage:
//! void Json::Rule<List>::Read(Json::FileReader& aSource, List& aValue)
//! {
//!    Json::ArrayReader array(aSource);
//!    aValue.Clear();
//!    while(array)
//!    {
//!       List::ValueType temp;
//!       array.ReadValue(temp);
//!       aValue.Insert(std::move(temp));
//!    }
//! }
//! @see Json::Rule
class ArrayReader
{
public:
   //! Opens an array from a Json::FileReader.
   //! @param aSource This is the file to read from.
   //! @throws Json::ReaderException If there is no array to read.
   explicit ArrayReader(FileReader& aSource);

   //! Continues to the end of the array. The next read token is the closing bracket.
   ~ArrayReader() noexcept;

   ArrayReader(const ArrayReader&) = delete;
   ArrayReader(ArrayReader&&)      = delete;

   ArrayReader& operator=(const ArrayReader&) = delete;
   ArrayReader& operator=(ArrayReader&&) = delete;

   //! Reads a value from the array.
   //! @param aValue This is the variable to store the value into.
   //! @note Type T requires that `static void Json::Rule<T>::Read(Json::FileReader&, T&)` exists.
   template<typename T>
   void ReadValue(T& aValue)
   {
      using Json::Rule;

      if (mEnd)
      {
         return;
      }
      if (mComma)
      {
         if (!mSource.CheckSymbol(","))
         {
            throw ReaderException("Expected ',' between Array members.", mSource.GetLine());
         }
      }
      mComma = true;

      Rule<T>::Read(mSource, aValue);
      mSource.Ignore();

      if (mSource.CheckSymbol("]", false))
      {
         mEnd = true;
      }
   }

   //! Returns true if not at the end of the array.
   explicit operator bool() const noexcept;

   //! Returns true if at the end of the array.
   bool operator!() const noexcept;

private:
   FileReader& mSource;
   bool        mComma = false;
   bool        mEnd   = false;
};

//! Reads Json objects (braced list of "Key" : Value pairs) from a Json::FileReader.
//! Intended for use in Json::Rule<T> specializations.
//! Example usage:
//! void Json::Rule<Person>(Json::FileReader& aSource, Person& aValue)
//! {
//!    Json::ObjectReader object(aSource);
//!    while(object)
//!    {
//!       object.ReadField("Name", aValue.mName);
//!       object.ReadField("Age", aValue.mAge);
//!       object.LoopGuard();
//!    }
//! }
//! @see Json::Rule
class ObjectReader
{
public:
   //! Opens an object from a Json::FileReader.
   //! @param aSource This is the file to read from.
   //! @throws Json::ReaderException If there is no object to read.
   explicit ObjectReader(FileReader& aSource);

   //! Continues to the end of the array. The next read token is the closing brace.
   ~ObjectReader() noexcept;

   ObjectReader(const ObjectReader&) = delete;
   ObjectReader(ObjectReader&&)      = delete;

   ObjectReader& operator=(const ObjectReader&) = delete;
   ObjectReader& operator=(ObjectReader&&) = delete;

   //! Reads a key-value pair from the object.
   //! @param aKey This is the variable to store the key into.
   //! @param aValue This is the variable to store the value into.
   //! @note Type Key requires that 'static void Rule<Key>::ReadKey(JsonReader&, Key&)' exists.
   //! @note Type T requires that `static void Rule<T>::Read(Json::FileReader&, T&)` exists.
   template<typename Key, typename T>
   void ReadValue(Key& aKey, T& aValue)
   {
      if (mEnd)
      {
         return;
      }

      if (mComma)
      {
         if (!mSource.CheckSymbol(","))
         {
            throw ReaderException("Expected ',' between Object members.", mSource.GetLine());
         }
      }
      mComma = false;

      Rule<Key>::ReadKey(mSource, aKey);
      mSource.Ignore();

      if (mSource.CheckSymbol(":"))
      {
         throw ReaderException("Expected ':' between Object key and value.", mSource.GetLine());
      }
      mComma = true;

      Rule<T>::Read(mSource, aValue);
      mSource.Ignore();

      if (mSource.CheckSymbol("}", false))
      {
         mEnd = true;
      }
      mCanReadField = true;
   }

   //! Reads a value from the object, given that the next key matches.
   //! @param aKey This is key to check for a match with.
   //! @param aValue This is the variable to store the value into.
   //! @returns True if a value was read, false otherwise.
   //! @note Type Key requires that 'static bool Rule<Key>::KeyEquals(const std::string&, Key&)' exists.
   //! @note Type T requires that `static void Rule<T>::Read(Json::FileReader&, T&)` exists.
   template<typename Key, typename T>
   bool ReadField(const Key& aKey, T& aValue)
   {
      if (mEnd)
      {
         return false;
      }
      if (mComma)
      {
         if (!mSource.CheckSymbol(","))
         {
            throw ReaderException("Expected ',' between Object members.", mSource.GetLine());
         }
      }
      mComma = false;

      if (!Rule<Key>::KeyEquals(mSource.Get(), aKey))
      {
         return false;
      }
      mSource.Ignore();

      if (!mSource.CheckSymbol(":"))
      {
         throw ReaderException("Expected ':' between Object key and value.", mSource.GetLine());
      }
      mComma = true;

      Rule<T>::Read(mSource, aValue);
      mSource.Ignore();

      if (mSource.CheckSymbol("}", false))
      {
         mEnd = true;
      }
      mCanReadField = true;

      return true;
   }

   //! Prevents infinite loops in ReadField loops.
   void LoopGuard();

   //! Returns true if not at the end of the object.
   explicit operator bool() const noexcept;

   //! Returns true if at the end of the object.
   bool operator!() const noexcept;

private:
   FileReader& mSource;
   bool        mComma        = false;
   bool        mEnd          = false;
   bool        mCanReadField = true;
};

//! Reads a file with a given name and fills a value with its contents. If file cannot be opened, does nothing.
//! @param aFileName This is the name of the file to read from.
//! @param aValue This is the value to fill with the file's contents.
//! @note Type T requires `static void Rule<T>::Read(Json::FileReader&, T&)` exists.
template<typename T>
void ReadFile(const std::string& aFileName, T& aValue)
{
   std::ifstream fileStream(aFileName.c_str());

   if (fileStream.is_open())
   {
      Json::FileReader reader(fileStream);

      Rule<T>::Read(reader, aValue);
   }
}

//! Reads a file with a given name and returns its contents as an object.
//! @param aFileName This is the name of the file to read from.
//! @returns Returns the file's contents. If the file could not be opened, returns a default-constructed T.
//! @note Type T requires `static void Rule<T>::Read(Json::FileReader&, T&)` exists.
template<typename T>
T ReadFile(const std::string& aFileName)
{
   T temp;

   std::ifstream fileStream(aFileName);

   if (fileStream.is_open())
   {
      Json::FileReader reader(fileStream);

      Rule<T>::Read(reader, temp);
   }

   return temp;
}
} // namespace Json

#endif
