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

#include "Item.hpp"

#include "Model.hpp"

namespace
{
const std::map<PlatformBrowser::Item::NodeType, std::string> cNODE_TYPE_TO_NAMES = {
   {PlatformBrowser::Item::NodeType::cNONE, "none"},
   {PlatformBrowser::Item::NodeType::cPLATFORM, "platforms"},
   {PlatformBrowser::Item::NodeType::cCOMM, "comms"},
   {PlatformBrowser::Item::NodeType::cMOVER, "movers"},
   {PlatformBrowser::Item::NodeType::cPROCESSOR, "processors"},
   {PlatformBrowser::Item::NodeType::cSENSOR, "sensors"},
   {PlatformBrowser::Item::NodeType::cWEAPON, "weapons"},
   {PlatformBrowser::Item::NodeType::cFUEL, "fuels"}};

const std::list<PlatformBrowser::Item::NodeType> cNODE_SUBTYPES = {{PlatformBrowser::Item::NodeType::cCOMM},
                                                                   {PlatformBrowser::Item::NodeType::cMOVER},
                                                                   {PlatformBrowser::Item::NodeType::cPROCESSOR},
                                                                   {PlatformBrowser::Item::NodeType::cSENSOR},
                                                                   {PlatformBrowser::Item::NodeType::cWEAPON},
                                                                   {PlatformBrowser::Item::NodeType::cFUEL}};
} // namespace

PlatformBrowser::Item::~Item()
{
   RemoveAllAbstractItems(true);
}

PlatformBrowser::Item* PlatformBrowser::Item::FindChild(const std::string& aString)
{
   auto it = mChildren.find(aString);
   if (it != mChildren.end())
   {
      return it->second;
   }
   else
   {
      return nullptr;
   }
}

void PlatformBrowser::Item::AddChild(Item* aChild)
{
   int index = -1;
   int count = GetAbstractItemCount();
   for (int i = 0; i < count; ++i)
   {
      auto* otherItem = dynamic_cast<Item*>(GetAbstractItem(i));
      if (aChild->SortsBefore(otherItem))
      {
         index = i;
         break;
      }
   }

   AddAbstractItem(aChild, index);
   mChildren[aChild->mName] = aChild;
}

bool PlatformBrowser::Item::HasChildren() const
{
   return !mChildren.empty();
}

size_t PlatformBrowser::Item::GetNumChildren() const
{
   return mChildren.size();
}

bool PlatformBrowser::Item::SortsBefore(Item* aOtherPtr)
{
   if (mNodeType < aOtherPtr->mNodeType)
   {
      return true;
   }
   else if ((mNodeType == aOtherPtr->mNodeType) && (mName < aOtherPtr->mName))
   {
      return true;
   }
   return false;
}

bool PlatformBrowser::Item::UpdatePart(NodeType aPartKind, const std::string& aName, const WsfPProxyNode& aValue)
{
   bool  added    = false;
   Item* partNode = FindChild(aName);
   if (partNode == nullptr)
   {
      partNode        = new Item(aPartKind, aValue);
      partNode->mName = aName;
      AddChild(partNode);
      added = true;
   }
   else
   {
      partNode->SetProxyValue(aValue);
   }
   return added;
}

void PlatformBrowser::Item::SetProxyValue(const WsfPProxyNode& aData)
{
   mData = aData;

   switch (mNodeType)
   {
   case NodeType::cPLATFORM:
   {
      WsfPProxyNode         node              = mData;
      bool                  added             = false;
      std::set<std::string> remainingChildren = GetChildrenNames();
      for (const auto& partType : cNODE_SUBTYPES)
      {
         node += cNODE_TYPE_TO_NAMES.at(partType);
         WsfPProxyObjectMap* mapPtr = node.GetValue().GetObjectMap();
         if (mapPtr)
         {
            auto& vals = mapPtr->GetValues();
            for (auto& val : vals)
            {
               const std::string& name = val.first;
               remainingChildren.erase(name);
               node += name;
               added |= UpdatePart(partType, name, node);
               node.TrimPath(3);
            }
         }
         node.TrimPath(2);
      }
      for (const auto& i : remainingChildren)
      {
         removeChild(i);
      }
      AbstractItemChanged();
   }
   break;
   default:
      break;
   }
}

WsfPProxyPath PlatformBrowser::Item::GetProxyPath() const
{
   auto* modelPtr = dynamic_cast<Model*>(GetModel());
   if (!modelPtr)
   {
      return WsfPProxyPath();
   }

   auto* parentPtr = dynamic_cast<Item*>(GetAbstractItemParent());
   if (!parentPtr)
   {
      return (modelPtr->ProxyRoot() + "platform").GetPath();
   }

   WsfPProxyPath path = parentPtr->GetProxyPath();
   if (mNodeType != NodeType::cPLATFORM)
   {
      path.Push(parentPtr->mData.GetValue(), GetNodeTypeString());
   }

   path += mName;

   return path;
}

const std::string& PlatformBrowser::Item::GetNodeTypeString() const
{
   return cNODE_TYPE_TO_NAMES.at(mNodeType);
}

WsfPProxyNode PlatformBrowser::Item::GetProxyNode() const
{
   auto* modelPtr = static_cast<Model*>(GetModel());
   if (modelPtr && modelPtr->ProxyRoot().IsValid())
   {
      return modelPtr->ProxyRoot() + GetProxyPath();
   }
   return WsfPProxyNode();
}

std::set<std::string> PlatformBrowser::Item::GetChildrenNames() const
{
   std::set<std::string> childNames;
   for (const auto& i : mChildren)
   {
      if (childNames.find(i.first) == childNames.end())
      {
         childNames.insert(i.first);
      }
   }
   return childNames;
}

void PlatformBrowser::Item::removeChild(const std::string& aName)
{
   if (mChildren.find(aName) != mChildren.end())
   {
      Item* child = mChildren[aName];
      RemoveAbstractItem(child);
      delete child;
      mChildren.erase(aName);
   }
}
