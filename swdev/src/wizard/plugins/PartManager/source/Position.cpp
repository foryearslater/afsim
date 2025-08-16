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

#include "Position.hpp"

#include "ProxyWatcher.hpp"
#include "Undo.hpp"

PartManager::Position::Position(const WsfPProxyNode& platformNode, QWidget* parent /*= nullptr*/)
   : MultiAttribute(parent)
   , mPlatform(WsfPM_MilPlatform(platformNode))
   , mAltLineEdit(new UnitValueLineEdit(platformNode + "altitude", "altitude", UtUnits::cLENGTH, this))
   , mAltAglCheckBox(new CheckBox(platformNode + "altitudeReference", "altitude reference", this))
   , mLatUi(new UtQtLatPosLineEdit(this))
   , mLonUi(new UtQtLonPosLineEdit(this))
{
   mUi.setupUi(this);
   mUi.copyButton->setIcon(QIcon::fromTheme("copy"));
   mUi.pasteButton->setIcon(QIcon::fromTheme("paste"));

   mAltAglCheckBox->SetText("AGL");
   mAltAglCheckBox->setToolTip("Is altitude referenced Above Ground Level?");

   LatLineEdit()->setToolTip("Latitude");
   LonLineEdit()->setToolTip("Longitude");
   mAltLineEdit->setToolTip("Altitude");

   LatLineEdit()->setPlaceholderText("Latitude");
   LonLineEdit()->setPlaceholderText("Longitude");
   mAltLineEdit->GetLineEdit()->setPlaceholderText("Altitude");

   connect(mUi.copyButton, &QPushButton::clicked, this, &Position::CopyPositionToClipboard);
   connect(mUi.pasteButton, &QPushButton::clicked, this, &Position::PastePositionFromClipboard);

   connect(LatLineEdit(), &UtQtLatPosLineEdit::returnPressed, this, &Position::UpdatePlatformLatLon);
   connect(LonLineEdit(), &UtQtLonPosLineEdit::returnPressed, this, &Position::UpdatePlatformLatLon);
   connect(LatLineEdit(), &UtQtLatPosLineEdit::editingFinished, this, &Position::HandleLatLonEditingFinished);
   connect(LonLineEdit(), &UtQtLonPosLineEdit::editingFinished, this, &Position::HandleLatLonEditingFinished);

   // Latitude and longitude are two parts of the same attribute, and therefore
   // must share a single reset button.
   connect(mLatUi.ResetButton(), &QPushButton::clicked, this, &Position::ResetLatLonToInherited);
   mLonUi.ResetButton()->setHidden(true);

   AddAttribute(mAltLineEdit);
   AddAttribute(mAltAglCheckBox);

   const WsfPProxyPath positionPath = mPlatform.PositionNode().GetPath();
   mCallbacks += wizard::WatchProxy(positionPath).ThisUpdate.Connect(&Position::HandleProxyChanged, this);
   UpdateAttribute();

   mUi.positionLayout->addWidget(mLatUi.Widget());
   mUi.positionLayout->addWidget(mLonUi.Widget(), 0, Qt::AlignLeft);
   mUi.altLayout->addWidget(mAltLineEdit);
   mUi.altLayout->addWidget(mAltAglCheckBox);
}

void PartManager::Position::UpdateFormContent()
{
   MultiAttribute::UpdateFormContent();
   UpdateLatLongWidgets();
}

void PartManager::Position::UpdateFormElements()
{
   MultiAttribute::UpdateFormElements();
   mLatUi.ResetButton()->setDisabled(IsLatLongInherited() || mPlatform.IsOfBasicType());
}

void PartManager::Position::UpdateParentPath(const WsfPProxyPath& oldParentPath, const WsfPProxyPath& newParentPath)
{
   MultiAttribute::UpdateParentPath(oldParentPath, newParentPath);
   mPlatform.UpdateParentPath(oldParentPath, newParentPath);
   UpdateAttribute();
}

bool PartManager::Position::IsInherited() const
{
   return IsLatLongInherited() && MultiAttribute::IsInherited();
}

void PartManager::Position::HandleProxyChanged(const wizard::ProxyChange& proxyChange)
{
   if (proxyChange.reason() == wizard::ProxyChange::cUPDATED)
   {
      UpdateAttribute();
   }
}

void PartManager::Position::ResetLatLonToInherited()
{
   wizard::UndoContext undoCtx("Reset attribute value");
   mPlatform.PositionNode().ResetToInheritedValue();
   UpdateAttribute();
}

void PartManager::Position::UpdatePlatformLatLon()
{
   mPlatform.SetPosition(WsfProxy::Position(LatLineEdit()->GetValue(), LonLineEdit()->GetValue()));
}

void PartManager::Position::HandleLatLonEditingFinished()
{
   if (LatLineEdit()->isModified() || LonLineEdit()->isModified())
   {
      UpdatePlatformLatLon();
   }
}

void PartManager::Position::CopyPositionToClipboard()
{
   UtLengthValue alt;
   mAltLineEdit->GetValue(alt);

   const std::string posStr =
      wsf::proxy::StringFromPosition(mPlatform.Position(), mPlatform.Altitude(), mPlatform.AltitudeReference());

   QApplication::clipboard()->setText(QString::fromStdString(posStr));
}

void PartManager::Position::PastePositionFromClipboard()
{
   const std::string  posStr = QApplication::clipboard()->text().toStdString();
   WsfProxy::Position latLong;
   WsfProxy::Length   altitude;
   WsfProxy::Int      altitudeRef;

   if (wsf::proxy::PositionFromString(posStr, latLong, altitude, altitudeRef))
   {
      wizard::UndoContext undoCtx("Paste position");

      mPlatform.SetPosition(latLong);
      mPlatform.SetAltitude(altitude);
      mPlatform.SetAltitudeReference(altitudeRef.GetValue());
   }
}

//! Latitude and longitude share a reset button. Since the reset button for
//! longitude is hidden, its line edit will extend further to the right than the
//! latitude line edit. We want the size of the line edits to match though,
//! giving a visual cue to the user that these values are related and share a
//! reset button. So explicitly resize the longitude line edit to match the
//! latitude line edit on all resize events.
void PartManager::Position::resizeEvent(QResizeEvent* event)
{
   MultiAttribute::resizeEvent(event);
   LonLineEdit()->setFixedSize(LatLineEdit()->size());
}

//! Updates the latitude and longitude content/input widgets with values from
//! the proxy.
void PartManager::Position::UpdateLatLongWidgets()
{
   const bool               IsOfBasicType = mPlatform.PositionNode().IsOfBasicType();
   const WsfProxy::Position pos           = mPlatform.Position();

   LatLineEdit()->setEnabled(!IsOfBasicType);
   LonLineEdit()->setEnabled(!IsOfBasicType);
   mUi.pasteButton->setEnabled(!IsOfBasicType);

   if (!pos.IsUnset())
   {
      LatLineEdit()->SetValue(pos.GetLatitude());
      LonLineEdit()->SetValue(pos.GetLongitude());
   }
   else
   {
      LatLineEdit()->clear();
      LonLineEdit()->clear();
      LatLineEdit()->setModified(false);
      LonLineEdit()->setModified(false);
   }
}

bool PartManager::Position::IsLatLongInherited() const
{
   return mPlatform.Position().IsInherited();
}

UtQtLatPosLineEdit* PartManager::Position::LatLineEdit() const
{
   return dynamic_cast<UtQtLatPosLineEdit*>(mLatUi.ContentWidget());
}

UtQtLonPosLineEdit* PartManager::Position::LonLineEdit() const
{
   return dynamic_cast<UtQtLonPosLineEdit*>(mLonUi.ContentWidget());
}
