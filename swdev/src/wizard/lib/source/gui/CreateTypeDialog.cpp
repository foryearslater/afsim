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
#include "CreateTypeDialog.hpp"

#include <QFileDialog>
#include <QMimeData>
#include <QPushButton>

#include "DragAndDrop.hpp"
#include "ParseResults.hpp"
#include "PasteContext.hpp"
#include "Project.hpp"
#include "ProxyWatcher.hpp"
#include "Undo.hpp"
#include "UtQt.hpp"
#include "Util.hpp"
#include "WsfPM_Root.hpp"
#include "WsfPProxyIndex.hpp"
#include "WsfParseNode.hpp"

wizard::CreateTypeDialog::CreateTypeDialog(WsfPM_ObjectMap aTypeMap, const std::string& aTypeName, QWidget* aParentPtr)
   : QDialog(aParentPtr)
   , mTypeMap(aTypeMap)
   , mNameValid(false)
   , mApplyToTextImmediately(false)
{
   mUi.setupUi(this);

   mUi.name->setValidator(new QRegExpValidator(Util::wsfValidInputRegexp, this));
   mUi.baseTypeList->setValidator(new QRegExpValidator(Util::wsfValidInputRegexp, this));

   connect(mUi.existingFileGroup, &QGroupBox::toggled, this, &CreateTypeDialog::ToggledExistingFileGroup);
   connect(mUi.newFileGroup, &QGroupBox::toggled, this, &CreateTypeDialog::ToggledNewFileGroup);
   connect(mUi.name, &QLineEdit::textEdited, this, &CreateTypeDialog::NameEdited);
   connect(mUi.browseButton, &QToolButton::clicked, this, &CreateTypeDialog::BrowseDirectory);
   connect(mUi.baseTypeList,
           static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::currentIndexChanged),
           this,
           &CreateTypeDialog::BaseTypeSelected);
   connect(mUi.buttonBox, &QDialogButtonBox::rejected, this, &CreateTypeDialog::reject);
   connect(mUi.buttonBox, &QDialogButtonBox::accepted, this, &CreateTypeDialog::accept);

   mUi.existingFile->setEditable(true);
   mUi.newFileDirectory->setEditable(true);

   WsfPM_ObjectMap          types = aTypeMap;
   std::vector<std::string> names = types.GetAllObjectNames();
   std::sort(names.begin(), names.end());
   for (size_t i = 0; i < names.size(); ++i)
   {
      mUi.baseTypeList->addItem(names[i].c_str());
      if (names[i] == aTypeName)
      {
         mUi.baseTypeList->setCurrentIndex((int)i);
      }
   }

   if (Project::Instance())
   {
      ParseResults* results = Project::Instance()->WaitForParseResults();
      if (results)
      {
         // Get a list of all files used by the project
         // Rank by # of types defined in the directory
         std::map<std::string, int> directoryUsage;
         // add all paths to map
         for (auto&& it : results->mFirstIncludes)
         {
            UtPath path = it.first->GetFilePath();
            mUi.existingFile->addItem(path.GetSystemPath().c_str());
            path.Up();
            std::string sysPath = path.GetSystemPath();
            directoryUsage[sysPath];
         }
         // Count types in each directory
         {
            WsfPM_ObjectMap types = aTypeMap;
            types.SwitchToUserTypes();
            WsfPProxyIndexNode* idx = types.FindIndexNode();
            if (idx)
            {
               for (auto&& ci : idx->mChildren)
               {
                  WsfPProxyIndexNode* child = ci.second.get();
                  for (auto&& ent : child->mEntries)
                  {
                     if (ent.mEntryType == WsfPProxyIndex::cADDED)
                     {
                        WsfParseNode* nodePtr = ent.mNodePtr;
                        WsfParseNode* leaf    = nodePtr->NextLeaf();
                        if (leaf && leaf->mValue.mSource)
                        {
                           UtPath dirPath = leaf->mValue.mSource->GetFilePath();
                           dirPath.Up();
                           ++directoryUsage[dirPath.GetSystemPath()];
                        }
                     }
                  }
               }
            }
         }
         // sort and add dirs to combo box
         std::vector<std::pair<int, std::string>> dirList;
         for (std::map<std::string, int>::const_iterator i = directoryUsage.begin(); i != directoryUsage.end(); ++i)
         {
            dirList.push_back(make_pair(-i->second, i->first));
         }
         std::sort(dirList.begin(), dirList.end());
         for (auto&& dir : dirList)
         {
            mUi.newFileDirectory->addItem(dir.second.c_str());
         }
      }
   }

   UpdateValidity();
}

void wizard::CreateTypeDialog::PasteHandler(const QMimeData* aMime, const PasteContext& aContext, bool aExecute)
{
   if (aExecute && !aContext.IsActionChosen("create-type"))
   {
      return;
   }
   if (!aMime || !aMime->hasFormat(DragAndDrop::cPROPERTY_SET_MIME_TYPE))
   {
      return;
   }
   if (!ProxyWatcher::GetActiveProxy())
   {
      return;
   }
   DragAndDrop::PropertyList properties = DragAndDrop::ReadPropertySetMime(aMime);
   if (properties.empty())
   {
      return;
   }
   if (properties.size() != 1)
   {
      return;
   }
   DragAndDrop::PropertyMap pMap = properties[0];
   if (!pMap.contains("proxy-node"))
   {
      return;
   }
   WsfPProxyNode node = DragAndDrop::ToProxyNode(pMap["proxy-node"].toString());
   if (!node)
   {
      return;
   }
   WsfPProxyNode parentNode = node.GetParent();
   WsfPM_Root    proxyRoot(ProxyWatcher::GetActiveProxy());
   // only support platform types for now...
   if (node.GetParent() == proxyRoot.platformTypes())
   {
      if (aExecute)
      {
         CreateTypeDialog* dlg = new CreateTypeDialog(node.GetParent(), node.GetName(), aContext.GetWidget());
         dlg->SelectFile(aContext.GetFilePath());
         dlg->mApplyToTextImmediately = aContext.ApplyToTextImmediately();
         dlg->show();
      }
      else
      {
         QString label = "New Platform Type";
         aContext.AddAvailableAction("create-type", label, false);
      }
   }
}

void wizard::CreateTypeDialog::UpdateValidity()
{
   UtPath dir = FilePath();
   dir.Up();
   bool validDir   = (dir.Stat() == UtPath::cDIRECTORY);
   bool fileExists = FilePath().Stat() == UtPath::cFILE;
   bool validFile;
   if (mUi.newFileGroup->isChecked())
   {
      if (validDir)
      {
         UtQtSetTextColor(mUi.newFileDirectory, QApplication::palette().text().color());
      }
      else
      {
         UtQtSetTextColor(mUi.newFileDirectory, Qt::red);
      }
      if (!fileExists)
      {
         UtQtSetTextColor(mUi.newFileName, QApplication::palette().text().color());
      }
      else
      {
         UtQtSetTextColor(mUi.newFileName, Qt::red);
      }
      validFile = validDir && !fileExists;
   }
   else
   {
      validFile = validDir && fileExists;
      if (validFile)
      {
         UtQtSetTextColor(mUi.existingFile, QApplication::palette().text().color());
      }
      else
      {
         UtQtSetTextColor(mUi.existingFile, Qt::red);
      }
   }
   bool ready = mNameValid && validFile;
   mUi.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(ready);
}

UtPath wizard::CreateTypeDialog::FilePath()
{
   if (mUi.newFileGroup->isChecked())
   {
      UtPath path = mUi.newFileDirectory->currentText().toStdString();
      path += mUi.newFileName->text().toStdString();
      return path;
   }
   else
   {
      return mUi.existingFile->currentText().toStdString();
   }
}

void wizard::CreateTypeDialog::ToggledExistingFileGroup()
{
   mUi.newFileGroup->setChecked(!mUi.existingFileGroup->isChecked());
   UpdateValidity();
}

void wizard::CreateTypeDialog::ToggledNewFileGroup()
{
   mUi.existingFileGroup->setChecked(!mUi.newFileGroup->isChecked());
   UpdateValidity();
}

void wizard::CreateTypeDialog::NameEdited(const QString& aName)
{
   mTypeMap.SwitchToBasicTypes();
   bool alreadyExists = mTypeMap.Exists(aName.toStdString());
   mTypeMap.SwitchToUserTypes();
   alreadyExists = alreadyExists || mTypeMap.Exists(aName.toStdString());
   mNameValid    = !alreadyExists;
   if (mNameValid)
   {
      UtQtSetTextColor(mUi.name, QApplication::palette().text().color());
   }
   else
   {
      UtQtSetTextColor(mUi.name, Qt::red);
   }

   UpdateValidity();
}

void wizard::CreateTypeDialog::BrowseDirectory()
{
   QString dir = QFileDialog::getExistingDirectory(this, "Select Directory", mUi.newFileDirectory->currentText());
   if (!dir.isNull())
   {
      mUi.newFileDirectory->setEditText(dir);
   }
}

void wizard::CreateTypeDialog::BaseTypeSelected(const QString& aTypeName)
{
   UpdateValidity();
}

void wizard::CreateTypeDialog::SelectFile(const std::string& aPath)
{
   UtPath path(aPath);
   if (path.Stat() == UtPath::cFILE)
   {
      mUi.existingFileGroup->setChecked(true);
      mUi.existingFile->setEditText(path.GetSystemPath().c_str());
   }
   else if (!aPath.empty())
   {
      mUi.newFileGroup->setChecked(true);
      path.Up();
      mUi.newFileDirectory->setEditText(path.GetSystemPath().c_str());
   }
   UpdateValidity();
}

void wizard::CreateTypeDialog::accept()
{
   mTypeMap.SwitchToBasicTypes();
   std::string   baseTypeName = mUi.baseTypeList->currentText().toStdString();
   WsfPProxyNode baseNode;
   if (mTypeMap.Exists(baseTypeName))
   {
      baseNode = mTypeMap.Find(baseTypeName);
   }
   else
   {
      mTypeMap.SwitchToUserTypes();
      baseNode = mTypeMap.Find(baseTypeName);
   }
   if (!baseNode)
   {
      return;
   }
   std::string name = mUi.name->text().toStdString();

   std::stringstream ss;
   ss << "Create Type " << name;
   UndoContext(ss.str().c_str());
   mTypeMap.SwitchToUserTypes();
   WsfPProxyStructValue newTypeValue = baseNode.GetValue().Copy(WsfProxy::cCOPY_INHERITED);
   newTypeValue.SetInherited(true);
   newTypeValue.SetBase(baseNode.GetPath());

   mTypeMap.Add(name, newTypeValue);
   WsfPProxyNode newType(mTypeMap + name);
   mTypeMap.GetProxy()->SetSaveFile(newType.GetPath(), FilePath().GetSystemPath());

   QDialog::accept();
}
