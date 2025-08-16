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

#ifndef SINGLEATTRIBUTE_HPP
#define SINGLEATTRIBUTE_HPP

#include "Attribute.hpp"
#include "AttributeUi.hpp"

namespace wizard
{
class Editor;
class ProxyChange;
} // namespace wizard
#include "WsfPProxyNode.hpp"

namespace PartManager
{
class SingleAttribute : public Attribute
{
public:
   explicit SingleAttribute(const WsfPProxyNode& aAttrNode, const std::string& aAttributeName, QWidget* aParent = nullptr);
   ~SingleAttribute() override = default;

   void UpdateFormElements() override;
   void UpdateParentPath(const WsfPProxyPath& aOldParentPath, const WsfPProxyPath& aNewParentPath) override;
   bool IsInherited() const override;

   void ResetAttrValue();

protected:
   virtual void         SetContentWidget(QWidget* aContentWidget);
   const WsfPProxyNode& ProxyNode() const;
   void                 AttributeValueUpdated(const QString& aVal);
   void                 AttributeValueUpdated(bool aVal);

private:
   void            HandleAttrValueUpdated(const wizard::ProxyChange& aProxyChange);
   wizard::Editor* GetEditor(UtTextDocumentRange& aRange);
   QString         GetAzElAttributeString(const QString& aValueStr);
   QString         GetAzElNodeValStr(const QString& aAttribute);
   QString         CheckValue(const QString& aVal);

   void GetAllAlternates(WsfParseRule*                    aRulePtr,
                         std::set<WsfParseRule*>&         aProcessed,
                         std::vector<WsfParseAlternate*>& aAlternates);

   AttributeUi mUi;
   //! The proxy node that this widget represents. For example, this might
   //! be a proxy node for a platform's "side" attribute.
   WsfPProxyNode mAttrNode;

   std::string      mAttributeName;
   UtCallbackHolder mProxyCallbacks;

   const static std::map<QString, QString> mBinaryAttributeMap;
};
} // namespace PartManager

#endif
