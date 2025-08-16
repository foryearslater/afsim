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

#ifndef ADDPLATFORMPART_HPP
#define ADDPLATFORMPART_HPP

#include <QDialog>
#include <QPushButton>

#include "WsfPM_MilPlatform.hpp"
#include "WsfPM_ObjectMap.hpp"
#include "ui_AddPlatformPart.h"

class WsfPProxyNode;

namespace wizard
{
class Editor;
}

namespace PartManager
{
class AddPlatformPart : public QDialog
{
public:
   explicit AddPlatformPart(const WsfPProxyNode& node, QWidget* parent = nullptr);
   ~AddPlatformPart() override = default;

   void PrepopulateForm(QAbstractButton* button);
   void AddPart();

private:
   wizard::Editor* GetEditor();

   //! A map of radio buttons to their respective map of types that should
   //! populate the combo box.
   std::map<QAbstractButton*, WsfPM_ObjectMap> mTypeObjectMaps;

   //! Generated Qt Creator GUI.
   Ui::AddPlatformPart mUi;

   //! The platform that this dialog pertains to (and to which the part will
   //! be added).
   WsfPM_MilPlatform mPlatform;
};
} // namespace PartManager

Q_DECLARE_METATYPE(WsfPProxyNode)
#endif
