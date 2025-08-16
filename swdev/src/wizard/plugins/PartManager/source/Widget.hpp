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

#ifndef WIDGET_HPP
#define WIDGET_HPP

#include <QWidget>

#include "Attribute.hpp"
#include "WsfPM_ObjectMap.hpp"
#include "WsfPProxyNode.hpp"
#include "ui_Widget.h"

class QFormLayout;
namespace wizard
{
class Editor;
class ProxyChange;
} // namespace wizard

namespace PartManager
{
class CheckBox;
class LineEdit;
class EnumComboBox;
class MapComboBox;
class Position;
class UnitValueLineEdit;

class Widget : public QWidget
{
   Q_OBJECT
public:
   explicit Widget(const WsfPProxyNode& aProxyNode, QWidget* aParent = nullptr);
   ~Widget() override = default;

   virtual void SetupUi(bool aIsTopLevel);

   const WsfPProxyNode& GetNode() const;
   QString              GetLabel() const;
   void                 UpdateLabel();
   virtual void         ExpandPath(const WsfPProxyPath& aPath);
   void                 PropagateLinkActivated(const QString& aLink);
   void                 SetContentVisible(bool aVisible);
signals:
   void LinkActivated(const QString& aLink);

protected:
   virtual bool IsAddPartSupported() const;

   CheckBox*          AddCheckBoxAttr(const std::string& aAttrName, const WsfPProxyNode& aAttrNode);
   LineEdit*          AddLineEditAttr(const std::string& aAttrName, const WsfPProxyNode& aAttrNode);
   EnumComboBox*      AddEnumComboBoxAttr(const std::string& aAttrName, const WsfPProxyNode& aAttrNode);
   MapComboBox*       AddMapComboBoxAttr(const std::string&     aAttrName,
                                         const WsfPProxyNode&   aAttrNode,
                                         const WsfPM_ObjectMap& aObjectMap);
   Position*          AddPositionAttr(const std::string& aAttrName, const WsfPProxyNode& aPositionNode);
   UnitValueLineEdit* AddAngleAttr(const std::string& aAttrName, const WsfPProxyNode& aAttrNode);

   QFormLayout* ContentLayout() const;
   size_t       NumOfAttributes() const;

   void paintEvent(QPaintEvent* aEvent) override;

private:
   void         HighlightHeader();
   void         HighlightContent();
   virtual void HandleAddPartButtonPressed();
   void         HandleDeleteButtonPressed();
   void         HandleProxyChanged(const wizard::ProxyChange& aProxyChange);
   void         HandleParentRenames(const WsfPProxyPath& aOldPath, const WsfPProxyPath& aNewPath);
   QString      MakeErrorMsg() const;
   void         UpdateAttrsParentNode(const WsfPProxyPath& aOldPath, const WsfPProxyPath& aNewPath);
   void         AddAttr(const std::string& aAttrName, Attribute* aAttrWidget);

   void DeleteInheritedParts(WsfPProxyNode& aNode);

   wizard::Editor* GetEditor(const WsfPProxyNode& aNode, UtTextDocumentRange& aRange);

   bool PlatformInheritsPart(const WsfPProxyNode& aPlatformNode, const WsfPProxyNode& aPartNode) const;

   Ui::Widget mUi;

   //! The proxy node represented by this widget. For example, the node may
   //! be of a platform or sensor.
   WsfPProxyNode mNode;
   //! A list of widgets representing the attributes of whatever this widget
   //! represents. For example, if mNode represents a platform, then this
   //! list would be a list of widgets representing platform attributes, e.g.,
   //! side.
   AttrList         mAttributes;
   UtCallbackHolder mProxyCallbacks;
};
} // namespace PartManager

#endif
