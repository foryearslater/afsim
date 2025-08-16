// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2019 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "PointOfInterestPropertiesWidget.hpp"

#include <QMessageBox>

#include "ProjectWorkspace.hpp"
#include "ProxyWatcher.hpp"
#include "TextSourceCache.hpp"
#include "WkfBullseye.hpp"
#include "WsfPProxyNode.hpp"

wizard::PointOfInterestPropertiesWidget::PointOfInterestPropertiesWidget(QWidget* parent, Qt::WindowFlags f)
   : Annotation::PointOfInterestPropertiesWidget(parent, f)
   , mFileLabel(new QLabel("File", this))
   , mFileComboBox(new QComboBox(this))
{
   mCallbackHolder +=
      wizard::ProxyWatcher::ProxyAvailable.Connect([this](WsfPProxy* aProxyPtr) { mProxyRoot = WsfPM_Root(aProxyPtr); });

   mUi.gridLayout->removeWidget(mUi.mBullseyeLabel);
   mUi.gridLayout->removeItem(mUi.horizontalLayout);
   mUi.gridLayout->removeWidget(mUi.mButtonBox);
   mUi.gridLayout->addWidget(mFileLabel, 4, 0);
   mUi.gridLayout->addWidget(mFileComboBox, 4, 1);
   mUi.gridLayout->addWidget(mUi.mBullseyeLabel, 5, 0);
   mUi.gridLayout->addItem(mUi.horizontalLayout, 5, 1);
}

void wizard::PointOfInterestPropertiesWidget::SetEntity(vespa::VaEntity* aEntityPtr, bool aCreate)
{
   if (aEntityPtr && mEntityPtr != aEntityPtr)
   {
      mChanges                    = PoiChanges();
      mChanges.originalEntityName = aEntityPtr->GetName();
   }
   mFileLabel->setVisible(aCreate);
   mFileComboBox->setVisible(aCreate);
   if (aCreate)
   {
      PopulateFiles();
   }
   QSize size(width(), sizeHint().height());
   resize(size);
   Annotation::PointOfInterestPropertiesWidget::SetEntity(aEntityPtr, aCreate);
}

void wizard::PointOfInterestPropertiesWidget::AcceptChanges()
{
   std::string   entityName    = mUi.mName->text().toStdString();
   WsfPProxyNode poiNode       = mProxyRoot + "pois" + entityName;
   bool          alreadyExists = poiNode.IsValid() && poiNode.GetAttrName() == entityName;
   if (mCreatingNewPoi && !alreadyExists)
   {
      std::string fileName = mFileComboBox->currentText().toStdString();
      auto        it       = mFilePaths.find(fileName);
      if (it != mFilePaths.end())
      {
         fileName = it->second;
      }
      mChanges.info.mName = entityName;
      mChanges.info.mLat  = mEntityPtr->GetPosition().GetLat();
      mChanges.info.mLon  = mEntityPtr->GetPosition().GetLon();
      mChanges.info.mAlt  = mEntityPtr->GetPosition().GetAlt();
      AssignAltitudeString();
      mChanges.info.mBullseye = mUi.mBullseyeCheckbox->isChecked();
      emit NewPoiAccepted(fileName, mChanges);
      mChanges.originalEntityName = mEntityPtr->GetName();
   }
   else if (mCreatingNewPoi && alreadyExists)
   {
      // Addresses user adding a new POI via the dialog then subsequently adding a POI with the same name in the editor
      // while the creation dialog is already open.
      CancelChanges();
      QMessageBox::warning(nullptr,
                           "Name Exists",
                           "A POI with the name " + QString::fromStdString(entityName) +
                              " already exists in the editor. Could not create new POI.");
   }
   else if (mChanges.nameChanged || mChanges.positionChanged || mChanges.bullseyeChanged)
   {
      emit ChangesAccepted(mChanges);
      mChanges.originalEntityName = entityName;
   }
}

void wizard::PointOfInterestPropertiesWidget::NameOperation()
{
   mChanges.nameChanged = true;
   mChanges.info.mName  = mUi.mName->text().toStdString();
}

void wizard::PointOfInterestPropertiesWidget::PositionOperation(const vespa::VaPosition& aPosition)
{
   Annotation::PointOfInterestPropertiesWidget::PositionOperation(aPosition);
   mChanges.positionChanged = true;
   mChanges.info.mLat       = aPosition.GetLat();
   mChanges.info.mLon       = aPosition.GetLon();
   mChanges.info.mAlt       = aPosition.GetAlt();
   AssignAltitudeString();
}
void wizard::PointOfInterestPropertiesWidget::BullseyeOperation(bool aBullseye)
{
   Annotation::PointOfInterestPropertiesWidget::BullseyeOperation(aBullseye);
   mChanges.bullseyeChanged = true;
   mChanges.info.mBullseye  = aBullseye;
}

void wizard::PointOfInterestPropertiesWidget::OverwriteOriginalValues(
   const Annotation::PointOfInterestPropertiesWidget::PoiInfo& aInfo)
{
   Annotation::PointOfInterestPropertiesWidget::OverwriteOriginalValues(aInfo);
   mChanges.positionChanged = true;
   mChanges.info.mLat       = aInfo.mLat;
   mChanges.info.mLon       = aInfo.mLon;
   mChanges.info.mAlt       = aInfo.mAlt;
   mChanges.info.mBullseye  = aInfo.mBullseye;
}

void wizard::PointOfInterestPropertiesWidget::PopulateFiles()
{
   mFilePaths.clear();
   mFileComboBox->clear();
   // Remove the working directory from all paths displayed to the user
   std::string workingDir       = wizard::Project::Instance()->WorkingDirectory().GetNormalizedPath() + "/";
   auto        RemoveWorkingDir = [workingDir](std::string& aOriginal)
   {
      size_t pos = aOriginal.find(workingDir);
      if (pos != std::string::npos)
      {
         aOriginal.erase(pos, workingDir.size());
      }
   };

   // Get all the pois that are already in files
   WsfPM_ObjectMap                         poiMap = mProxyRoot + "pois";
   std::unordered_map<std::string, size_t> poiFileTracker;
   std::vector<WsfPProxyNode>              nodes;
   poiMap.GetAllValues(nodes);
   for (const auto& nodeIt : nodes)
   {
      if (nodeIt.GetAttrName() != "WSF_POIS")
      {
         std::string filePath = nodeIt.GetSourceDocumentRange().mSource->GetFilePath().GetNormalizedPath();
         if (poiFileTracker.count(filePath) > 0)
         {
            poiFileTracker[filePath]++;
         }
         else
         {
            poiFileTracker[filePath] = 1;
         }
      }
   }
   std::vector<std::pair<std::string, size_t>> fileList;
   for (const auto& it : poiFileTracker)
   {
      std::string fileName = it.first + (" (" + std::to_string(it.second) + " point(s) of interest)");
      RemoveWorkingDir(fileName);
      fileList.push_back(std::pair<std::string, size_t>(fileName, it.second));
      mFilePaths.emplace(fileName, it.first);
   }

   // Get the remaining files. When processing these, make sure files with existing pois are skipped
   auto sourceList = wizard::ProjectWorkspace::Instance()->GetSourceCache()->GetSources();
   for (const auto& it : sourceList)
   {
      std::string path = it.second->GetFilePath().GetNormalizedPath();
      auto        fileIt =
         std::find_if(poiFileTracker.begin(),
                      poiFileTracker.end(),
                      [path](const std::pair<std::string, size_t>& aFileInfo) { return path == aFileInfo.first; });
      if (fileIt == poiFileTracker.end())
      {
         RemoveWorkingDir(path);
         fileList.push_back(std::pair<std::string, size_t>(path, 0));
         mFilePaths.emplace(path, it.second->GetFilePath().GetNormalizedPath());
      }
   }

   // Sort the files based on the quantity of pois, then lexicographically
   std::sort(fileList.begin(),
             fileList.end(),
             [](const std::pair<std::string, size_t>& aFile1, const std::pair<std::string, size_t>& aFile2)
             {
                if (aFile1.second != aFile2.second)
                {
                   return aFile1.second > aFile2.second;
                }
                return (aFile1.first < aFile2.first);
             });
   QStringList files;
   for (const auto& it : fileList)
   {
      files.push_back(QString::fromStdString(it.first));
   }
   mFileComboBox->addItems(files);
}

std::string wizard::PointOfInterestPropertiesWidget::GetEntityUiName() const
{
   return mUi.mName->text().toStdString();
}

std::string wizard::PointOfInterestPropertiesWidget::GetOriginalName() const
{
   return mOriginalValues.mName;
}

void wizard::PointOfInterestPropertiesWidget::AssignAltitudeString()
{
   std::string altText = mUi.mAltitude->text().toStdString();
   if (!altText.empty() && mUi.mAltitude->IsValid())
   {
      std::istringstream ss(altText);
      double             value;
      std::string        unit;
      ss >> value >> unit;
      mChanges.altString = QString::number(value) + " " + QString::fromStdString(unit);
   }
}
