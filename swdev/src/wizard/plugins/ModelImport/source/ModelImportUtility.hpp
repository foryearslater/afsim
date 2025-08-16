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
#ifndef MODELIMPORT_UTILITYTYPES_HPP
#define MODELIMPORT_UTILITYTYPES_HPP

#include <map>
#include <ostream>

#include <QDir>
#include <QString>
#include <QStringList>

#include "JsonRule.hpp"

namespace ModelImport
{
//! Allows QString to be written to standard library streams.
//! @param aOut The stream to write to.
//! @param aString The string to write to the stream.
//! @returns A reference to aOut.
std::ostream& operator<<(std::ostream& aOut, const QString& aString);

//! Contains the data associated with a model.
struct ItemData
{
   QString     mName;     //!< The name of the model.
   QString     mType;     //!< E.g. platform_type, sensor, etc.
   QString     mInherits; //!< What the model inherits from.
   QStringList mLabels;   //!< Any generated or user-defined labels to apply.
};

struct FileData
{
   QString               mFile;                   //!< Name of file.
   QString               mPath;                   //!< Path to file. (No leading or closing '/')
   QStringList           mDependencies;           //!< Any files the file includes.
   QStringList           mAdditionalDependencies; //!< Any user-defined dependencies.
   std::vector<ItemData> mDefines;                //!< Any models defined in this file.

   //! Sets the file and path fields of a FileData object.
   //! @param aPath The path to the file.
   void SetPathToFile(const QString& aPath);

   //! Returns a formatted path to the file's containing directory.
   QString PathToDir() const;

   //! Returns a formatted path to the file.
   QString PathToFile() const;
};

namespace detail
{
//! Overload of MakePath.
//! Used for conveniently constructing paths with the Path function, where parts have different types.
const QString& MakePath(const QString& aPath);

//! Overload of MakePath.
//! Used for conveniently constructing paths with the Path function, where parts have different types.
QString MakePath(const std::string& aPath);

//! Overload of MakePath.
//! Used for conveniently constructing paths with the Path function, where parts have different types.
QString MakePath(const char* aPath);

//! Concatenates all parts (with '/' delimeters).
//! It is expected that all arguments are QString, std::string, or const char*
template<typename T, typename... Args>
QString MakePath(const T& aRoot, Args... aRelative)
{
   if (MakePath(aRoot).isEmpty())
   {
      return MakePath(aRelative...);
   }
   return MakePath(aRoot) + '/' + MakePath(aRelative...);
}
} // namespace detail

//! Concatenates all parts (with '/' delimeters) and cleans path to remove redundant '.' and '..' directories.
//! It is expected that all arguments are QString, std::string, or const char*
template<typename... Args>
QString Path(Args... aParts)
{
   return QDir::cleanPath(detail::MakePath(aParts...));
}

//! A generic function which merges two data sets.
//! Both data objects will be sorted to decrease reuntime from O(n^2) to O(n*log n).
//! @param aOldData The old data set which data is merged from.
//! @param aNewData The new data set which data is merged into.
//! @param aComparator A function or function object which defines how items of the data sets are compared. (See note.)
//! @param aMerger A function or function object which defines how an old and new item are to be merged.
//! @note T is a container which defines begin(), end(), and types T::value_type and T::iterator.
//! @note Compare is a function object which defines operator()(const T::value_type& aLeft, const T::value_type& aRight)
//! -> int.
//!       The operator should return -1 if aLeft < aRight, 0 if aLeft == aRight, 1 if aLeft > aRight.
//!       Comparison refers only to the fields being sorted on.
//! @note Merge is a function object which defines operator()(T::value_type& aNew, T::value_type& aOld) -> auto
//!       The operator should move appropriate data from aOld to aNew. Moving (instead of copying) is allowed.
template<typename T, typename Merge, typename Compare>
void MergeData(T& aOldData, T& aNewData, Compare aComparator, Merge aMerger)
{
   using value_type = typename T::value_type;
   using iterator   = typename T::iterator;

   std::sort(aOldData.begin(),
             aOldData.end(),
             [&](const value_type& aLeft, const value_type& aRight) -> bool { return aComparator(aLeft, aRight) < 0; });

   std::sort(aNewData.begin(),
             aNewData.end(),
             [&](const value_type& aLeft, const value_type& aRight) -> bool { return aComparator(aLeft, aRight) < 0; });

   if (aOldData.begin() == aOldData.end() || aNewData.begin() == aNewData.end())
   {
      return;
   }

   iterator oldDataIterator = aOldData.begin();
   iterator newDataIterator = aNewData.begin();

   while (true)
   {
      switch (aComparator(*oldDataIterator, *newDataIterator))
      {
      case -1: // old < new
         oldDataIterator++;
         if (oldDataIterator == aOldData.end())
         {
            return;
         }
         break;
      case 0: // old == new
         aMerger(*newDataIterator, *oldDataIterator);

         oldDataIterator++;
         if (oldDataIterator == aOldData.end())
         {
            return;
         }

         newDataIterator++;
         if (newDataIterator == aNewData.end())
         {
            return;
         }
         break;
      case 1: // old > new
         newDataIterator++;
         if (newDataIterator == aNewData.end())
         {
            return;
         }
         break;
      }
   }
}

using ParseData = typename std::vector<ModelImport::FileData>;
} // namespace ModelImport

namespace Json
{
template<>
struct Rule<ModelImport::ItemData>
{
   using type = ModelImport::ItemData;

   //! Defines how VALUES are to be read from file.
   //! @param aSource This is the file to read from.
   //! @param aValue This is the value to fill with the read contents.
   static void Read(Json::FileReader& aSource, type& aValue)
   {
      Json::ObjectReader reader(aSource);

      while (reader)
      {
         reader.ReadField("Name", aValue.mName);
         reader.ReadField("Type", aValue.mType);
         reader.ReadField("Inherits", aValue.mInherits);
         reader.ReadField("Labels", aValue.mLabels);

         reader.LoopGuard();
      }
   }

   //! Defines how VALUES are to be written to file.
   //! @param aDestination The file to write to.
   //! @param aValue The value to write to file.
   static void Write(Json::FileWriter& aDestination, const type& aValue)
   {
      Json::ObjectWriter writer(aDestination);

      writer.WriteValue("Name", aValue.mName);
      writer.WriteValue("Type", aValue.mType);
      writer.WriteValue("Inherits", aValue.mInherits);
      writer.WriteValue("Labels", aValue.mLabels);
   }
};

template<>
struct Rule<ModelImport::FileData>
{
   using type = ModelImport::FileData;

   //! Defines how VALUES are to be read from file.
   //! @param aSource This is the file to read from.
   //! @param aValue This is the value to fill with the read contents.
   static void Read(Json::FileReader& aSource, type& aValue)
   {
      Json::ObjectReader reader(aSource);

      while (reader)
      {
         reader.ReadField("File", aValue.mFile);
         reader.ReadField("Path", aValue.mPath);
         reader.ReadField("Dependencies", aValue.mDependencies);
         reader.ReadField("AdditionalDependencies", aValue.mAdditionalDependencies);
         reader.ReadField("Defines", aValue.mDefines);

         reader.LoopGuard();
      }
   }

   //! Defines how VALUES are to be written to file.
   //! @param aDestination The file to write to.
   //! @param aValue The value to write to file.
   static void Write(Json::FileWriter& aDestination, const type& aValue)
   {
      Json::ObjectWriter writer(aDestination);

      writer.WriteValue("File", aValue.mFile);
      writer.WriteValue("Path", aValue.mPath);
      writer.WriteValue("Dependencies", aValue.mDependencies);
      writer.WriteValue("AdditionalDependencies", aValue.mAdditionalDependencies);
      writer.WriteValue("Defines", aValue.mDefines);
   }
};
} // namespace Json

#endif
