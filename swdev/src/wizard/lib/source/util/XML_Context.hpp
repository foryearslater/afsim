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
#ifndef XML_HPP
#define XML_HPP

#include <assert.h>
#include <map>
#include <string>

#include "UtPath.hpp"
#include "UtQtXmlReader.hpp"
#include "UtQtXmlWriter.hpp"

namespace wizard
{
class Enumeration
{
public:
   Enumeration() {}
   void Add(const std::string& aName, int aValue)
   {
      mNameToValue[aName]  = aValue;
      mValueToName[aValue] = aName;
   }


   std::string GetName(int aValue, const char* aDefaultName = nullptr)
   {
      std::map<int, std::string>::iterator i = mValueToName.find(aValue);
      if (i != mValueToName.end())
         return i->second;
      assert(aDefaultName);
      return aDefaultName;
   }

   int GetValue(const std::string& aName, int aDefaultValue = -1)
   {
      std::map<std::string, int>::iterator i = mNameToValue.find(aName);
      if (i != mNameToValue.end())
         return i->second;
      return aDefaultValue;
   }

private:
   std::map<std::string, int> mNameToValue;
   std::map<int, std::string> mValueToName;
};

class XML_Context
{
public:
   explicit XML_Context(UtQtXmlWriter* aWriter);
   explicit XML_Context(UtQtXmlReader* aReader);
   const UtPath& GetFileName() const { return mFileName; }
   void          SetFileName(const UtPath& aPath)
   {
      mFileName      = aPath;
      mFileDirectory = aPath;
      mFileDirectory.Up();
   }
   const UtPath&  GetFileDirectory() const { return mFileDirectory; }
   UtQtXmlWriter& GetWriter() { return *mXmlWriter; }
   UtQtXmlReader& GetReader() { return *mXmlReader; }

protected:
   UtPath         mFileName;
   UtPath         mFileDirectory;
   UtQtXmlWriter* mXmlWriter;
   UtQtXmlReader* mXmlReader;
};
} // namespace wizard

#endif
