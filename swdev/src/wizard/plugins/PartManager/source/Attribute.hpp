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

#ifndef ATTRIBUTE_HPP
#define ATTRIBUTE_HPP

#include <QWidget>

class WsfPProxyPath;

namespace PartManager
{
class Attribute : public QWidget
{
public:
   explicit Attribute(QWidget* parent = nullptr);

   void UpdateAttribute();

   //! Updates the content widget(s) of the attribute. When this is called,
   //! derived classes should check the proxy for updated values and update
   //! their input widgets.
   virtual void UpdateFormContent() = 0;

   //! Updates GUI form elements/widgets of the attribute. When this is
   //! called, derived classes should check the proxy for updated values and
   //! update auxiliary widgets such as reset buttons, copy/paste buttons,
   //! etc. It should also enable/disable input widgets based on whether or
   //! or not the widget represents a basic/primitive type and therefore
   //! shouldn't be modifiable.
   //!
   //! The reason UpdateFormContent() and UpdateFormElements() are split into
   //! separate methods is that most attributes are single attributes that
   //! feature only one input widget and one reset button, and therefore
   //! UpdateFormElements() can be shared by all of them, whereas
   //! UpdateFormContent() is different for each type of input widget.
   virtual void UpdateFormElements() = 0;

   //! Updates the path(s) of the proxy node(s) represented by this attribute
   //! when one of its parent nodes has been renamed. (A parent being renamed
   //! affects the paths of all its children.)
   virtual void UpdateParentPath(const WsfPProxyPath& oldParentPath, const WsfPProxyPath& newParentPath) = 0;

   //! @returns Whether or not the attribute is currently displaying values
   //! that are inherited or if one or more have been overridden.
   virtual bool IsInherited() const = 0;
};

typedef std::vector<Attribute*> AttrList;
} // namespace PartManager

#endif
