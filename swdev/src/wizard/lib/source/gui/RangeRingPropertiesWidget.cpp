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

#include "RangeRingPropertiesWidget.hpp"

#include "ProjectWorkspace.hpp"
#include "ProxyWatcher.hpp"
#include "TextSourceCache.hpp"
#include "WsfPProxyNode.hpp"

wizard::RingPropertiesWidget::RingPropertiesWidget(QWidget* parent, Qt::WindowFlags f)
   : Annotation::RingPropertiesWidget(parent, f)
   , mFileLabel(new QLabel("File", this))
   , mFileComboBox(new QComboBox(this))
{
   mCallbackHolder +=
      wizard::ProxyWatcher::ProxyAvailable.Connect([this](WsfPProxy* aProxyPtr) { mProxyRoot = WsfPM_Root(aProxyPtr); });

   mUi.gridLayout->removeWidget(mUi.mButtonBox);
   mUi.gridLayout->addWidget(mFileLabel, 16, 0);
   mUi.gridLayout->addWidget(mFileComboBox, 16, 1);
   mUi.gridLayout->addWidget(mUi.mButtonBox, 17, 1);
}

int wizard::RingPropertiesWidget::GetCurrentNameIndex()
{
   return mUi.mName->currentIndex();
}

void wizard::RingPropertiesWidget::SetCurrentNameIndex(int aIndex)
{
   if (aIndex >= 0 && mUi.mName->count() > aIndex)
   {
      mUi.mName->setCurrentIndex(aIndex);
   }
}

void wizard::RingPropertiesWidget::SetEntity(vespa::VaEntity*                                   aEntityPtr,
                                             const Annotation::RingPropertiesWidget::RingFlags& aFlags)
{
   mFileLabel->setVisible(aFlags.mNew);
   mFileComboBox->setVisible(aFlags.mNew);
   QSize size(width(), sizeHint().height());
   resize(size);
   if (aFlags.mNew)
   {
      PopulateFiles(aFlags.mEntityContext ? aEntityPtr : nullptr);
   }
   Annotation::RingPropertiesWidget::SetEntity(aEntityPtr, aFlags);

   mChanges                  = RingChanges();
   mChanges.newRing          = aFlags.mNew;
   mChanges.entityContext    = aFlags.mEntityContext;
   mChanges.originalRingName = mUi.mName->currentText().toStdString();
   if (aFlags.mNew)
   {
      // When the ring is new, write out anything that differs from the default
      CheckDefaults();
   }
}

void wizard::RingPropertiesWidget::OverwriteOriginalValues(const wkf::AttachmentRangeRing::Properties& aInfo)
{
   auto defaults = RingChanges().properties;
   ;
   auto current                         = mRingPtr->GetProperties();
   mChanges.additionalRingRadiusChanged = (defaults.mAdditionalRingRadius != current.mAdditionalRingRadius) &&
                                          (defaults.mAdditionalRingRadius != aInfo.mAdditionalRingRadius);
   mChanges.additionalRingsChanged =
      (defaults.mAdditionalRings != current.mAdditionalRings) && (defaults.mAdditionalRings != aInfo.mAdditionalRings);
   mChanges.angleLabelColorChanged =
      (defaults.mAngleLabelColor != current.mAngleLabelColor) && (defaults.mAngleLabelColor != aInfo.mAngleLabelColor);
   mChanges.baseRadiusChanged =
      (defaults.mBaseRadius != current.mBaseRadius) && (defaults.mBaseRadius != aInfo.mBaseRadius);
   mChanges.centerAngleChanged =
      (defaults.mCenterAngle != current.mCenterAngle) && (defaults.mCenterAngle != aInfo.mCenterAngle);
   mChanges.radialColorChanged =
      (defaults.mRadialColor != current.mRadialColor) && (defaults.mRadialColor != aInfo.mRadialColor);
   mChanges.radialLineWidthChanged =
      (defaults.mRadialLineWidth != current.mRadialLineWidth) && (defaults.mRadialLineWidth != aInfo.mRadialLineWidth);
   mChanges.radialRateChanged =
      (defaults.mRadialRate != current.mRadialRate) && (defaults.mRadialRate != aInfo.mRadialRate);
   mChanges.rangeLabelColorChanged =
      (defaults.mRangeLabelColor != current.mRangeLabelColor) && (defaults.mRangeLabelColor != aInfo.mRangeLabelColor);
   mChanges.ringColorChanged = (defaults.mRingColor != current.mRingColor) && (defaults.mRingColor != aInfo.mRingColor);
   mChanges.ringLineWidthChanged =
      (defaults.mRingLineWidth != current.mRingLineWidth) && (defaults.mRingLineWidth != aInfo.mRingLineWidth);
   mChanges.showAngleLabelsChanged =
      (defaults.mShowAngleLabels != current.mShowAngleLabels) && (defaults.mShowAngleLabels != aInfo.mShowAngleLabels);
   mChanges.showRangeLabelsChanged =
      (defaults.mShowRangeLabels != current.mShowRangeLabels) && (defaults.mShowRangeLabels != aInfo.mShowRangeLabels);
   mChanges.widthChanged = (defaults.mWidth != current.mWidth) && (defaults.mWidth != aInfo.mWidth);
   Annotation::RingPropertiesWidget::OverwriteOriginalValues(aInfo);
}

void wizard::RingPropertiesWidget::CheckDefaults()
{
   auto defaults                        = RingChanges().properties;
   auto current                         = mRingPtr->GetProperties();
   mChanges.additionalRingRadiusChanged = (defaults.mAdditionalRingRadius != current.mAdditionalRingRadius);
   mChanges.additionalRingsChanged      = (defaults.mAdditionalRings != current.mAdditionalRings);
   mChanges.angleLabelColorChanged      = (defaults.mAngleLabelColor != current.mAngleLabelColor);
   mChanges.baseRadiusChanged           = (defaults.mBaseRadius != current.mBaseRadius);
   mChanges.centerAngleChanged          = (defaults.mCenterAngle != current.mCenterAngle);
   mChanges.radialColorChanged          = (defaults.mRadialColor != current.mRadialColor);
   mChanges.radialLineWidthChanged      = (defaults.mRadialLineWidth != current.mRadialLineWidth);
   mChanges.radialRateChanged           = (defaults.mRadialRate != current.mRadialRate);
   mChanges.rangeLabelColorChanged      = (defaults.mRangeLabelColor != current.mRangeLabelColor);
   mChanges.ringColorChanged            = (defaults.mRingColor != current.mRingColor);
   mChanges.ringLineWidthChanged        = (defaults.mRingLineWidth != current.mRingLineWidth);
   mChanges.showAngleLabelsChanged      = (defaults.mShowAngleLabels != current.mShowAngleLabels);
   mChanges.showRangeLabelsChanged      = (defaults.mShowRangeLabels != current.mShowRangeLabels);
   mChanges.widthChanged                = (defaults.mWidth != current.mWidth);
}

void wizard::RingPropertiesWidget::AcceptChangesP()
{
   mChanges.properties  = mRingPtr->GetProperties();
   mChanges.unitStrings = UnitStrings(mUi.mAdditionalRadii->text(),
                                      mUi.mCenterRadius->text(),
                                      mUi.mCenterAngle->text(),
                                      mUi.mAngleWidth->text(),
                                      mUi.mRadialRate->text());
   mChanges.entityName  = mEntityPtr->GetName();
   // Instead of setting the name in wizard, don't set it at all and grab it instead from the ui
   mChanges.properties.mName = mUi.mName->currentText();
   if (mChanges.newRing)
   {
      std::string fileName = mFileComboBox->currentText().toStdString();
      fileName             = mFilePaths[fileName];
      if (mChanges.entityContext)
      {
         emit NewRingAccepted(fileName, mChanges);
      }
      else
      {
         vespa::VaPosition pos(mEntityPtr->GetPosition());
         emit              NewPoiRingAccepted(fileName, mChanges, mEntityPtr->GetName(), pos);
      }
   }
   else
   {
      // Necessary so the proxy handles the operation correctly
      if (mChanges.nameChanged)
      {
         mRingPtr->SetName(mChanges.originalRingName);
      }
      emit ChangesAccepted(mChanges);
   }
}

void wizard::RingPropertiesWidget::PopulateFiles(vespa::VaEntity* aEntityPtr)
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

   // Get the file that has the selected entity
   std::string entityPath = "";
   if (aEntityPtr)
   {
      if (aEntityPtr->IsA_TypeOf<wkf::Platform>())
      {
         WsfPProxyNode platNode = mProxyRoot + "platform" + aEntityPtr->GetName();
         entityPath             = platNode.GetSourceDocumentRange().mSource->GetFilePath().GetNormalizedPath();
      }
      else
      {
         WsfPProxyNode poiNode = mProxyRoot + "pois" + aEntityPtr->GetName();
         entityPath            = poiNode.GetSourceDocumentRange().mSource->GetFilePath().GetNormalizedPath();
      }
   }
   // Get all the rings that are already in files
   WsfPM_ObjectMap                         ringMap = mProxyRoot + "rangeRings";
   std::unordered_map<std::string, size_t> ringFileTracker;
   std::vector<WsfPProxyNode>              nodes;
   ringMap.GetAllValues(nodes);
   for (const auto& nodeIt : nodes)
   {
      if (nodeIt.GetAttrName() != "WSF_RANGERINGS")
      {
         std::string filePath = nodeIt.GetSourceDocumentRange().mSource->GetFilePath().GetNormalizedPath();
         if (filePath != entityPath)
         {
            if (ringFileTracker.count(filePath) > 0)
            {
               ringFileTracker[filePath]++;
            }
            else
            {
               ringFileTracker[filePath] = 1;
            }
         }
      }
   }
   std::vector<std::pair<std::string, size_t>> fileList;
   for (const auto& it : ringFileTracker)
   {
      std::string fileName = it.first + (" (" + std::to_string(it.second) + " range ring(s))");
      RemoveWorkingDir(fileName);
      fileList.push_back(std::pair<std::string, size_t>(fileName, it.second));
      mFilePaths.emplace(fileName, it.first);
   }

   // Get the remaining files. When processing these, make sure files with existing range rings are skipped
   auto sourceList = wizard::ProjectWorkspace::Instance()->GetSourceCache()->GetSources();
   for (const auto& it : sourceList)
   {
      std::string path = it.second->GetFilePath().GetNormalizedPath();
      auto        fileIt =
         std::find_if(ringFileTracker.begin(),
                      ringFileTracker.end(),
                      [path](const std::pair<std::string, size_t>& aFileInfo) { return path == aFileInfo.first; });
      if (fileIt == ringFileTracker.end() && it.second->GetFilePath().GetNormalizedPath() != entityPath)
      {
         RemoveWorkingDir(path);
         fileList.push_back(std::pair<std::string, size_t>(path, 0));
         mFilePaths.emplace(path, it.second->GetFilePath().GetNormalizedPath());
      }
   }

   // First display the file with the relevant entity (when applicable). Then, sort the files based on the quantity of
   // range rings, then lexicographically
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
   if (!entityPath.empty())
   {
      std::string pathCopy = entityPath;
      RemoveWorkingDir(entityPath);
      entityPath += " (entity location)";
      mFilePaths.emplace(entityPath, pathCopy);
      files.push_back(QString::fromStdString(entityPath));
   }
   for (const auto& it : fileList)
   {
      files.push_back(QString::fromStdString(it.first));
   }
   mFileComboBox->addItems(files);
}

std::unordered_set<std::string> wizard::RingPropertiesWidget::GetRingList() const
{
   std::unordered_set<std::string> ringList;
   // Get all the rings defined in the editor, including those not shown on the map
   WsfPM_ObjectMap                         ringMap = mProxyRoot + "rangeRings";
   std::unordered_map<std::string, size_t> ringFileTracker;
   std::vector<WsfPProxyNode>              nodes;
   ringMap.GetAllValues(nodes);
   for (const auto& nodeIt : nodes)
   {
      std::string name = nodeIt.GetAttrName();
      if (name != "WSF_RANGERINGS")
      {
         ringList.emplace(name);
      }
   }
   return ringList;
}

void wizard::RingPropertiesWidget::SetCenterRadius()
{
   mChanges.baseRadiusChanged = true;
   Annotation::RingPropertiesWidget::SetCenterRadius();
}

void wizard::RingPropertiesWidget::SetAdditionalRadii()
{
   mChanges.additionalRingRadiusChanged = true;
   Annotation::RingPropertiesWidget::SetAdditionalRadii();
}

void wizard::RingPropertiesWidget::SetNumberAdditonalRings(const int aValue)
{
   mChanges.additionalRingsChanged = true;
   Annotation::RingPropertiesWidget::SetNumberAdditonalRings(aValue);
}

void wizard::RingPropertiesWidget::SetCenterAngle()
{
   mChanges.centerAngleChanged = true;
   Annotation::RingPropertiesWidget::SetCenterAngle();
}

void wizard::RingPropertiesWidget::SetAngleWidth()
{
   mChanges.widthChanged = true;
   Annotation::RingPropertiesWidget::SetAngleWidth();
}

void wizard::RingPropertiesWidget::SetRadialRate()
{
   mChanges.radialRateChanged = true;
   Annotation::RingPropertiesWidget::SetRadialRate();
}

void wizard::RingPropertiesWidget::RingColorUpdated(const QColor& aColor)
{
   mChanges.ringColorChanged = true;
   Annotation::RingPropertiesWidget::RingColorUpdated(aColor);
}

void wizard::RingPropertiesWidget::RadialColorUpdated(const QColor& aColor)
{
   mChanges.radialColorChanged = true;
   Annotation::RingPropertiesWidget::RadialColorUpdated(aColor);
}

void wizard::RingPropertiesWidget::SetRingLineWidth(const int aValue)
{
   mChanges.ringLineWidthChanged = true;
   Annotation::RingPropertiesWidget::SetRingLineWidth(aValue);
}

void wizard::RingPropertiesWidget::SetRadialLineWidth(const int aValue)
{
   mChanges.radialLineWidthChanged = true;
   Annotation::RingPropertiesWidget::SetRadialLineWidth(aValue);
}

void wizard::RingPropertiesWidget::RangeLabelsToggled(bool aState)
{
   mChanges.showRangeLabelsChanged = true;
   Annotation::RingPropertiesWidget::RangeLabelsToggled(aState);
}

void wizard::RingPropertiesWidget::AngleLabelsToggled(bool aState)
{
   mChanges.showAngleLabelsChanged = true;
   Annotation::RingPropertiesWidget::AngleLabelsToggled(aState);
}

void wizard::RingPropertiesWidget::RangeLabelColorUpdated(const QColor& aColor)
{
   mChanges.rangeLabelColorChanged = true;
   Annotation::RingPropertiesWidget::RangeLabelColorUpdated(aColor);
}

void wizard::RingPropertiesWidget::AngleLabelColorUpdated(const QColor& aColor)
{
   mChanges.angleLabelColorChanged = true;
   Annotation::RingPropertiesWidget::AngleLabelColorUpdated(aColor);
}

void wizard::RingPropertiesWidget::SetAlignWithPlatform(bool aState)
{
   mChanges.alignPlatformChanged = true;
   Annotation::RingPropertiesWidget::SetAlignWithPlatform(aState);
}

void wizard::RingPropertiesWidget::NameChanged(const QString& aName)
{
   if (mActiveIndex == mUi.mName->currentIndex()) // if this is a real text change, and not a selection change
   {
      mChanges.nameChanged = true;
      mUi.mName->setItemText(mUi.mName->currentIndex(), aName);
   }
}

void wizard::RingPropertiesWidget::SelectionChanged(int aIndex)
{
   Annotation::RingPropertiesWidget::SelectionChanged(aIndex);
   mChanges                  = RingChanges();
   mChanges.newRing          = mCurrentFlags.mNew;
   mChanges.entityContext    = mCurrentFlags.mEntityContext;
   mChanges.originalRingName = mUi.mName->itemText(aIndex).toStdString();
}
