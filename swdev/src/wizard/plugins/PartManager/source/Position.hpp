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

#ifndef POSITION_HPP
#define POSITION_HPP

#include "AttributeUi.hpp"
#include "CheckBox.hpp"
#include "MultiAttribute.hpp"
#include "UnitValueLineEdit.hpp"
#include "UtCallbackHolder.hpp"
#include "UtQtUnitLineEdit.hpp"
#include "WsfPM_MilPlatform.hpp"
#include "ui_Position.h"

class WsfPProxyNode;

namespace wizard
{
class ProxyChange;
}

namespace PartManager
{
//! A multi-attribute that represents a platform's location, featuring Copy and
//! Paste buttons that handle VESPA-compatible textual representations of
//! position.
//!
//! @note Latitude and longitude share a reset button.
class Position : public MultiAttribute
{
public:
   explicit Position(const WsfPProxyNode& platformNode, QWidget* parent = nullptr);
   ~Position() override = default;

   void UpdateFormContent() override;
   void UpdateFormElements() override;
   void UpdateParentPath(const WsfPProxyPath& oldParentPath, const WsfPProxyPath& newParentPath) override;
   bool IsInherited() const override;

   void HandleProxyChanged(const wizard::ProxyChange& proxyChange);

   void ResetLatLonToInherited();
   void UpdatePlatformLatLon();
   void HandleLatLonEditingFinished();
   void CopyPositionToClipboard();
   void PastePositionFromClipboard();

protected:
   void resizeEvent(QResizeEvent* event) override;

private:
   void                UpdateLatLongWidgets();
   bool                IsLatLongInherited() const;
   UtQtLatPosLineEdit* LatLineEdit() const;
   UtQtLonPosLineEdit* LonLineEdit() const;

   UtCallbackHolder   mCallbacks;
   WsfPM_MilPlatform  mPlatform;
   UnitValueLineEdit* mAltLineEdit;
   CheckBox*          mAltAglCheckBox;
   Ui::Position       mUi;
   AttributeUi        mLatUi;
   AttributeUi        mLonUi;
};
} // namespace PartManager

#endif
