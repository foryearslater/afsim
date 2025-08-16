// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2003-2013 The Boeing Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#ifndef ATTRIBUTESET_HPP
#define ATTRIBUTESET_HPP
#include <unordered_map>

#include "UtVariant.hpp"
#include "ViExport.hpp"

class UtQtXmlWriter;
class UtQtXmlReader;
class UtQtXmlElementReader;

namespace wizard
{
class AttributeOwner;

//! A set of attributes
//! Attribute values are stored as a UtVariant
//! Each attribute has a name
//! Contains any number of children with unique names
//! The attribute set may be owned by a AttributeOwner
class VI_EXPORT AttributeSet
{
public:
   friend class AttributeOwner;

   explicit AttributeSet(const std::string& aName);

   //! Copy the sub-tree of attributes, leaving owner null
   AttributeSet(const AttributeSet& aSrc);

   ~AttributeSet();

   void Apply(const AttributeSet& aOtherSet);

   void WriteXML(const std::string& aName, UtQtXmlWriter& aWriter) const;
   void ReadXML(UtQtXmlElementReader& aReader);
   //! Read the top-level attribute set from a single entry
   bool ReadSingleXML(UtQtXmlElementReader& aReader);

   std::string ToString();

   void SetValue(const std::string& aName, const UtVariant& aValue);

   bool GetValue(const std::string& aName, UtVariant& aValue) const;

   UtVariant GetValue(const std::string& aName) const;

   bool   GetBool(const std::string& aName, bool aDefautlValue) const;
   double GetDouble(const std::string& aName, double aDefaultValue) const;

   int GetInt(const std::string& aName, int aDefaultValue) const;

   std::string GetString(const std::string& aName, const std::string& aDefaultValue) const;

   AttributeSet& Child(const std::string& aChildName);
   AttributeSet* FindChild(const std::string& aChildName);
   bool          RenameChild(const std::string& aCurrentName, const std::string& aNewName);
   bool          Rename(const std::string& aNewName);
   bool          RemoveChild(const std::string& aChildName);

   bool HasAttribute(const std::string& aName) const;
   bool HasChild(const std::string& aChildName) const;

   void NotifyOwnersOfChange();

   AttributeSet* GetParent() { return mParentSet; }

   const std::string& GetAttributeName() const { return mAttributeName; }

private:
   AttributeSet();

   void BindOwner(AttributeOwner* aOwnerPtr);
   void UnbindOwner(AttributeOwner* aOwnerPtr);

   AttributeSet& operator=(const AttributeSet&); // not implemented as of yet

   using AttributeStringMap = std::unordered_map<std::string, UtVariant>;
   using ChildMap           = std::unordered_map<std::string, AttributeSet*>;
   AttributeOwner*    mOwnerPtr;
   AttributeSet*      mParentSet;
   std::string        mAttributeName;
   AttributeStringMap mStrAttributes;
   ChildMap           mChildren;
};
} // namespace wizard
#endif
