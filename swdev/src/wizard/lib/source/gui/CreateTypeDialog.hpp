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
#ifndef CREATETYPEDIALOG_HPP
#define CREATETYPEDIALOG_HPP
#include <QDialog>

#include "UtMemory.hpp"
#include "WsfPM_ObjectMap.hpp"
#include "ui_CreateType.h"

class QMimeData;

namespace wizard
{
class PasteContext;

class CreateTypeDialog : public QDialog
{
   Q_OBJECT
public:
   CreateTypeDialog(WsfPM_ObjectMap aTypeMap, const std::string& aTypeName, QWidget* aParentPtr);
   ~CreateTypeDialog() override = default;

   static void PasteHandler(const QMimeData* aMime, const PasteContext& aContext, bool aExecute);
   void        SelectFile(const std::string& aPath);
private slots:
   void accept() override;
   void BrowseDirectory();
   void NameEdited(const QString& aName);
   void ToggledNewFileGroup();
   void ToggledExistingFileGroup();
   void BaseTypeSelected(const QString& aTypeName);

private:
   UtPath FilePath();

   void UpdateValidity();

   WsfPM_ObjectMap mTypeMap;
   Ui::CreateType  mUi;
   bool            mNameValid;
   bool            mApplyToTextImmediately;
};
} // namespace wizard

#endif
