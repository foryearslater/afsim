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

#ifndef ITEM_HPP
#define ITEM_HPP

#include <map>

#include "AbstractModel.hpp"
#include "WsfPProxyNode.hpp"

namespace PlatformBrowser
{

//! An item in the platform model.
class Item : public wizard::AbstractModelItem
{
public:
   //! Node types
   enum class NodeType
   {
      cNONE,
      cPLATFORM,
      cCOMM,
      cMOVER,
      cPROCESSOR,
      cSENSOR,
      cWEAPON,
      cFUEL
   };

   Item() = default;
   Item(NodeType aNodeType, const WsfPProxyNode& aData)
      : wizard::AbstractModelItem()
      , mNodeType(aNodeType)
   {
      SetProxyValue(aData);
   }
   ~Item() override;

   std::string mName{""};

   void               SetProxyValue(const WsfPProxyNode& aData);
   WsfPProxyValue     GetProxyValue() const { return mData.GetValue(); }
   WsfPProxyNode      GetProxyNode() const;
   Item*              FindChild(const std::string& aInput);
   void               AddChild(Item* aChild);
   bool               HasChildren() const;
   size_t             GetNumChildren() const;
   WsfPProxyPath      GetProxyPath() const;
   const std::string& GetNodeTypeString() const;
   NodeType           GetNodeType() const { return mNodeType; }

   std::set<std::string> GetChildrenNames() const;
   void                  removeChild(const std::string& aName);
   void                  clearAll() { mChildren.clear(); }

private:
   bool UpdatePart(NodeType aPartKind, const std::string& aName, const WsfPProxyNode& aValue);
   bool SortsBefore(Item* aOtherPtr);

   NodeType                     mNodeType{NodeType::cNONE};
   WsfPProxyNode                mData;
   std::map<std::string, Item*> mChildren;
};

} // namespace PlatformBrowser

#endif
