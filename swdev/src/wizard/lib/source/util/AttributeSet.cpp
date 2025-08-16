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
#include "AttributeSet.hpp"

#include <map>
#include <sstream>

#include "AttributeOwner.hpp"
#include "UtQtXmlReader.hpp"
#include "UtQtXmlWriter.hpp"

wizard::AttributeSet::AttributeSet(const AttributeSet& aSrc)
   : mOwnerPtr(nullptr)
   , mParentSet(nullptr)
   , mStrAttributes(aSrc.mStrAttributes)
   , mChildren(aSrc.mChildren)
{
   for (ChildMap::iterator i = mChildren.begin(); i != mChildren.end(); ++i)
   {
      i->second             = new AttributeSet(*i->second);
      i->second->mParentSet = this;
   }
}

wizard::AttributeSet::AttributeSet(const std::string& aName)
   : mOwnerPtr(nullptr)
   , mParentSet(nullptr)
   , mAttributeName(aName)
{
}

wizard::AttributeSet::AttributeSet()
   : mOwnerPtr(nullptr)
   , mParentSet(nullptr)
{
}

wizard::AttributeSet::~AttributeSet()
{
   for (ChildMap::iterator i = mChildren.begin(); i != mChildren.end(); ++i)
   {
      delete i->second;
   }
}

bool wizard::AttributeSet::GetBool(const std::string& aName, bool aDefaultValue) const
{
   AttributeStringMap::const_iterator i = mStrAttributes.find(aName);
   if (i != mStrAttributes.end())
   {
      return i->second.ToBool();
   }
   return aDefaultValue;
}

int wizard::AttributeSet::GetInt(const std::string& aName, int aDefaultValue) const
{
   AttributeStringMap::const_iterator i = mStrAttributes.find(aName);
   if (i != mStrAttributes.end())
   {
      return i->second.GetInt();
   }
   return aDefaultValue;
}

double wizard::AttributeSet::GetDouble(const std::string& aName, double aDefaultValue) const
{
   AttributeStringMap::const_iterator i = mStrAttributes.find(aName);
   if (i != mStrAttributes.end())
   {
      if (i->second.GetType() == UtVariant::Type::cDOUBLE)
      {
         return (double)i->second.GetDouble();
      }
   }
   return aDefaultValue;
}

std::string wizard::AttributeSet::GetString(const std::string& aName, const std::string& aDefaultValue) const
{
   AttributeStringMap::const_iterator i = mStrAttributes.find(aName);
   if (i != mStrAttributes.end())
   {
      return (std::string)i->second.GetString();
   }
   return aDefaultValue;
}

void wizard::AttributeSet::SetValue(const std::string& aName, const UtVariant& aValue)
{
   mStrAttributes[aName] = aValue;
}

bool wizard::AttributeSet::GetValue(const std::string& aName, UtVariant& aValue) const
{
   AttributeStringMap::const_iterator i = mStrAttributes.find(aName);
   if (i != mStrAttributes.end())
   {
      aValue = i->second;
      return true;
   }
   return false;
}

UtVariant wizard::AttributeSet::GetValue(const std::string& aName) const
{
   AttributeStringMap::const_iterator i = mStrAttributes.find(aName);
   if (i != mStrAttributes.end())
      return i->second;
   return UtVariant();
}

bool wizard::AttributeSet::HasAttribute(const std::string& aName) const
{
   return mStrAttributes.find(aName) != mStrAttributes.end();
}

bool wizard::AttributeSet::HasChild(const std::string& aChildName) const
{
   return mChildren.find(aChildName) != mChildren.end();
}

// Assign all attributes and children from aOtherSet to this
// This does not clear any attributes or children not contained in aOtherSet
void wizard::AttributeSet::Apply(const AttributeSet& aOtherSet)
{
   for (auto&& it : aOtherSet.mStrAttributes)
   {
      mStrAttributes[it.first] = it.second;
   }
   for (auto&& jt : aOtherSet.mChildren)
   {
      Child(jt.first).Apply(*jt.second);
   }
}

wizard::AttributeSet& wizard::AttributeSet::Child(const std::string& aChildName)
{
   AttributeSet*& setPtr = mChildren[aChildName];
   if (!setPtr)
   {
      setPtr             = new AttributeSet(aChildName);
      setPtr->mParentSet = this;
   }
   return *setPtr;
}

bool wizard::AttributeSet::RemoveChild(const std::string& aChildName)
{
   ChildMap::iterator i = mChildren.find(aChildName);
   if (i != mChildren.end())
   {
      delete i->second;
      mChildren.erase(i);
      return true;
   }
   return false;
}

wizard::AttributeSet* wizard::AttributeSet::FindChild(const std::string& aChildName)
{
   ChildMap::iterator i = mChildren.find(aChildName);
   if (i != mChildren.end())
   {
      return i->second;
   }
   return nullptr;
}

bool wizard::AttributeSet::RenameChild(const std::string& aCurrentName, const std::string& aNewName)
{
   AttributeSet* child = FindChild(aCurrentName);
   if (!child)
      return false;
   if (aCurrentName == aNewName)
      return true;
   if (FindChild(aNewName) != nullptr)
      return false;
   mChildren[aNewName] = child;
   mChildren.erase(aCurrentName);
   child->mAttributeName = aNewName;
   return true;
}

bool wizard::AttributeSet::Rename(const std::string& aNewName)
{
   if (mParentSet)
   {
      return mParentSet->RenameChild(mAttributeName, aNewName);
   }
   mAttributeName = aNewName;
   return true;
}

static void WriteVariant(UtQtXmlWriter& aWriter, const UtVariant& aValue, const std::string& aValueName)
{
   std::string valueStr;
   const char* elementName;
   switch (aValue.GetType())
   {
   case UtVariant::Type::cBOOL:
      elementName = "bool";
      valueStr    = aValue.GetBool() ? "1" : "0";
      break;
   case UtVariant::Type::cINT:
      elementName = "int";
      valueStr    = aValue.ToString();
      break;
   case UtVariant::Type::cDOUBLE:
      elementName = "double";
      valueStr    = aValue.ToString();
      break;
   case UtVariant::Type::cSTRING_ENUM:
      elementName = "enum";
      valueStr    = aValue.ToString();
      break;
   case UtVariant::Type::cSTRING:
      elementName = "string";
      valueStr    = aValue.ToString();
      break;
   default:
      return; // don't write other stuff
   }
   aWriter.WriteStartElement(elementName);
   aWriter.WriteAttribute("name", aValueName);
   aWriter.WriteAttribute("val", valueStr);
   aWriter.WriteEndElement();
}

static void ReadVariant(UtQtXmlElementReader& aReader, UtVariant& aValue, std::string& aAttributeName)
{
   std::string elementName = aReader.GetName();
   if (aReader.HasAttribute("name") && aReader.HasAttribute("val"))
   {
      aAttributeName     = aReader.Attribute("name");
      std::string strVal = aReader.Attribute("val");
      if (elementName == "bool")
      {
         aValue = (strVal == "1");
      }
      else if (elementName == "int")
      {
         std::stringstream ss(strVal);
         int               intVal;
         if (ss >> intVal)
         {
            aValue = intVal;
         }
         else
         {
            aValue = UtVariant();
         }
      }
      else if (elementName == "double")
      {
         std::stringstream ss(strVal);
         double            dblVal;
         if (ss >> dblVal)
         {
            aValue = dblVal;
         }
         else
         {
            aValue = UtVariant();
         }
      }
      else if (elementName == "string")
      {
         aValue = elementName;
      }
      else
      {
         aValue = UtVariant();
      }
   }
}

/* Example:
<as>
 <bool name="show" val="1"/>
 <as name="childName">
  <double name="alpha" val="0.4"/>
 </as>
</as>
*/
void wizard::AttributeSet::WriteXML(const std::string& aName, UtQtXmlWriter& aWriter) const
{
   typedef std::map<std::string, UtVariant>     OrderedAttributeMap;
   typedef std::map<std::string, AttributeSet*> OrderedChildMap;

   // Guarantee the order of attributes and children in the XML output.
   const OrderedAttributeMap orderedAttributes(mStrAttributes.begin(), mStrAttributes.end());
   const OrderedChildMap     orderedChildren(mChildren.begin(), mChildren.end());

   aWriter.WriteStartElement("as");
   {
      if (!aName.empty())
      {
         aWriter.WriteAttribute("name", aName);
      }
      for (auto&& it : orderedAttributes)
      {
         WriteVariant(aWriter, it.second, it.first);
      }
      for (auto&& jt : orderedChildren)
      {
         jt.second->WriteXML(jt.first, aWriter);
      }
   }
   aWriter.WriteEndElement();
}

std::string wizard::AttributeSet::ToString()
{
   UtQtXmlWriter writer;
   WriteXML("", writer);
   return writer.ToString();
}

void wizard::AttributeSet::ReadXML(UtQtXmlElementReader& aReader)
{
   // Start element should have already been read

   for (;;)
   {
      UtQtXmlElementReader element(aReader);
      if (!element)
         break;
      if (element.GetName() == "as")
      {
         if (element.HasAttribute("name"))
         {
            Child(element.Attribute("name")).ReadXML(aReader);
         }
      }
      else
      {
         UtVariant   val;
         std::string attrName;
         ReadVariant(element, val, attrName);
         if (!val.IsUnset())
         {
            SetValue(attrName, val);
         }
      }
   }
}

bool wizard::AttributeSet::ReadSingleXML(UtQtXmlElementReader& aReader)
{
   UtQtXmlElementReader childElement(aReader);
   if (!!childElement)
   {
      if (aReader.GetName() == "as")
      {
         ReadXML(aReader);
         return true;
      }
   }
   return false;
}

void wizard::AttributeSet::BindOwner(AttributeOwner* aOwnerPtr)
{
   if (mOwnerPtr != aOwnerPtr)
   {
      assert(mOwnerPtr == nullptr);
      mOwnerPtr = aOwnerPtr;
   }
}

void wizard::AttributeSet::UnbindOwner(AttributeOwner* aOwnerPtr)
{
   assert(mOwnerPtr == aOwnerPtr || !aOwnerPtr);
   mOwnerPtr = nullptr;
}

void wizard::AttributeSet::NotifyOwnersOfChange()
{
   if (mOwnerPtr)
   {
      mOwnerPtr->AttributeStateChanged();
   }
   for (auto&& it : mChildren)
   {
      it.second->NotifyOwnersOfChange();
   }
}
