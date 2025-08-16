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

#include "DockWidget.hpp"

#include <cctype>

#include <QLabel>
#include <QMenu>

#include "ChangeWrapper.hpp"
#include "ColorProxyUtil.hpp"
#include "EditorHelpers.hpp"
#include "EditorPrefObject.hpp"
#include "Environment.hpp"
#include "FindReferencesResultDialog.hpp"
#include "ParseResults.hpp"
#include "Project.hpp"
#include "ProxyWatcher.hpp"
#include "UtColor.hpp"
#include "UtOptional.hpp"
#include "UtQt.hpp"
#include "UtQtMemory.hpp"
#include "UtRandom.hpp"
#include "UtTextDocument.hpp"
#include "VaUtils.hpp"
#include "WkfAction.hpp"
#include "WkfPlatform.hpp"
#include "WkfScenario.hpp"
#include "WkfViewer.hpp"
#include "WkfVtkEnvironment.hpp"
#include "WsfEditor.hpp"
#include "WsfPM_Zone.hpp"
#include "zone/WkfAttachmentZone.hpp"
#include "zone/WkfAttachmentZoneCircle.hpp"
#include "zone/WkfAttachmentZoneEllipse.hpp"
#include "zone/WkfAttachmentZonePolygon.hpp"
#include "zone/WkfZonePoint.hpp"
#include "zone_browser/WkfZoneBrowserDockWidget.hpp"


namespace
{
unsigned int cRANDOM_SEED = 15;

struct TextReplacementInfo
{
   TextReplacementInfo(const UtTextDocumentRange& aRange,
                       const QString&             aAttributeName,
                       const QString&             aInsert,
                       bool                       aNewText = false,
                       bool                       aOneWord = false)
      : mEntryRange(aRange)
      , mAttributeName(aAttributeName)
      , mInsert(aInsert)
      , mNewText(aNewText)
      , mOneWord(aOneWord)
   {
   }

   UtTextDocumentRange mEntryRange;
   QString             mAttributeName;
   QString             mInsert;
   bool                mNewText;
   bool                mOneWord;
};

void InsertNewCommandHandler(std::vector<TextReplacementInfo>& aInfo,
                             const WsfPProxyNode&              aProxyNode,
                             const std::string&                aCommandName,
                             const QString&                    aInsert)
{
   // Insert new command on the second last line of the annotation definition
   UtTextDocumentRange wholeRange   = aProxyNode.GetSourceDocumentRange();
   size_t              tabSize      = wkfEnv.GetPreferenceObject<wizard::EditorPrefObject>()->TabSpace();
   size_t              numberSpaces = tabSize + tabSize;
   std::string         spaces;
   spaces.insert(0, numberSpaces, 0x20);
   std::string commandText = wholeRange.Text();
   // Get the location of the last whitespace (occurs before the end_block) and iterate backwards until reaching the prior block.
   size_t index = commandText.find_last_of(" \t\r\n\f");
   while (index != 0 && std::isspace(commandText[--index]) != 0)
   {
   }
   UtTextDocumentRange insertRange(wholeRange.mSource, wholeRange.GetBegin() + index + 1, wholeRange.GetBegin() + index + 1);
   QString newCommand = +"\n" + QString::fromStdString(spaces) + QString::fromStdString(aCommandName) + " " + aInsert;
   aInfo.emplace_back(insertRange, "", newCommand, true);
}

UtTextDocumentRange GetLastEntry(const WsfPProxyNode& aProxyNode)
{
   WsfPProxyIndexNode* indexNode = aProxyNode.FindIndexNode();
   if (indexNode)
   {
      size_t entryCount = indexNode->mEntries.size();
      if (entryCount > 0)
      {
         WsfPProxyIndexEntry lastEntry = indexNode->mEntries[entryCount - 1];
         UtTextDocumentRange range     = lastEntry.mNodePtr->GetParent()->SubtreeRange();
         return range;
      }
   }
   return UtTextDocumentRange();
}


WsfPM_Zone GetPlatformTypeZone(WsfPM_Platform aPlatNode, const std::string& aZoneName)
{
   if (aPlatNode.IsValid())
   {
      // get the platform type
      auto       platNodeType = aPlatNode.GetTypeName();
      WsfPM_Root proxyRoot{wizard::ProxyWatcher::GetActiveProxy()};
      auto       platformTypeMap = proxyRoot.platformTypes();
      for (auto&& it = platformTypeMap.begin(); it != platformTypeMap.end(); ++it)
      {
         auto platformTypeNode = platformTypeMap.AtT(it);
         auto curTypeName      = platformTypeNode.GetName();
         if (platNodeType == curTypeName)
         {
            auto zoneNode = static_cast<WsfPM_Zone>(platformTypeNode.Zone() + aZoneName);
            if (zoneNode.GetSourceDocumentRange().Valid())
            {
               return zoneNode;
            }
         }
      }
   }
   return WsfPM_Zone();
}

std::vector<TextReplacementInfo> GetColorReplacementCommands(const WsfPProxyNode& aProxyNode,
                                                             const std::string&   aAttributeName,
                                                             const std::string&   aCommandName,
                                                             const QString&       aInsert)
{
   // Due to color being represented as a struct, automatic removal of duplicates cannot be done without manual parsing,
   // so it is not performed. Replacement will occur at the latest instance. Color can be overidden, so check whether
   // floats or ints/presets occur later (or neither), and act accordingly
   std::vector<TextReplacementInfo> info;
   WsfPProxyValue                   proxyColorVal = aProxyNode.GetValue().GetAttr(aAttributeName);
   WsfPProxyNode                    colorNode     = aProxyNode + aAttributeName;
   WsfPProxyValue                   proxyRedVal   = colorNode.GetValue().GetAttr("red");
   WsfPProxyNode                    rNode         = aProxyNode + aAttributeName + "red";
   WsfPProxyNode                    floatNode     = aProxyNode + aAttributeName + "floatPlaceholder";
   WsfPProxyNode                    hexNode       = aProxyNode + aAttributeName + "hexPlaceholder";

   UtTextDocumentRange normalRange;
   UtTextDocumentRange floatRange;
   UtTextDocumentRange hexRange;
   if (proxyColorVal.IsValid() && !proxyColorVal.IsUnset() && rNode.FindIndexNode())
   {
      normalRange = GetLastEntry(rNode);
   }
   if (proxyRedVal.IsValid() && floatNode.FindIndexNode())
   {
      floatRange = GetLastEntry(floatNode);
   }
   if (proxyRedVal.IsValid() && hexNode.FindIndexNode())
   {
      hexRange = GetLastEntry(hexNode);
   }

   // Add a new command
   if (normalRange.GetBegin() == 0 && floatRange.GetBegin() == 0 && hexRange.GetBegin() == 0)
   {
      InsertNewCommandHandler(info, aProxyNode, aCommandName, aInsert);
   }
   // Replace named colors, or rgb int colors
   // Named strings work because the preset is implictly already converted to rgb, so its still able to find "red".
   else if (normalRange.GetBegin() > floatRange.GetBegin() && normalRange.GetBegin() > hexRange.GetBegin())
   {
      info.emplace_back(normalRange, "", aInsert);
   }
   // Replace rgb float colors
   else if (floatRange.GetBegin() > hexRange.GetBegin() && floatRange.GetBegin() > normalRange.GetBegin())
   {
      info.emplace_back(floatRange, "", aInsert);
   }
   else
   {
      info.emplace_back(hexRange, "", aInsert);
   }
   return info;
}

class ZoneWrapper : public wizard::ChangeWrapper
{
public:
   ZoneWrapper(wizard::Editor* aEditor, const TextReplacementInfo& aInfo)
      : wizard::ChangeWrapper(aEditor, aInfo.mEntryRange)
      , mInfo(aInfo)
   {
   }
   void Process(bool aUniqueRange = true)
   {
      if (mInfo.mOneWord && !mInfo.mNewText)
      {
         QTextCursor cursor = GetEditor()->textCursor();
         cursor.setPosition(mInfo.mEntryRange.GetBegin());
         cursor.setPosition(mInfo.mEntryRange.GetEnd() + 1, QTextCursor::KeepAnchor);
         GetEditor()->setTextCursor(cursor);
         wizEnv.BeginUndoCapture();
         cursor.insertText(mInfo.mInsert);
         if (aUniqueRange)
         {
            size_t pos[3];
            GetEditor()->GetCurrentPosition(pos[0], pos[1], pos[2]);
            std::string lineStr = GetEditor()->GetLineText(pos[1]);
            if (lineStr.find_first_not_of(" \t\r\n") == lineStr.npos)
            {
               GetEditor()->DeleteLine();
            }
         }
         wizEnv.EndUndoCapture();
      }
      else if (mInfo.mNewText)
      {
         GetEditor()->GoToRange(mInfo.mEntryRange);
         GetEditor()->insertPlainText(mInfo.mInsert);
      }
      else
      {
         wizEnv.BeginUndoCapture();
         if (!wizard::EditorHelpers::ReplaceTextInEditor(mInfo.mEntryRange, mInfo.mAttributeName, mInfo.mInsert))
         {
            throw UtException("Failed to replace text.");
         }
         wizEnv.EndUndoCapture();
      }
   }

private:
   TextReplacementInfo mInfo;
};

void AddToSortedChanges(std::multiset<ZoneWrapper>& aSortedChanges, const std::vector<TextReplacementInfo>& aList)
{
   for (const auto& it : aList)
   {
      wizard::Editor* editorPtr =
         wizard::Project::Instance()->GotoFile(it.mEntryRange.mSource->GetFilePath().GetNormalizedPath());
      aSortedChanges.insert(ZoneWrapper(editorPtr, it));
   }
}

std::string ColorToString(const QColor& aColor, bool aUseAlpha = false)
{
   std::stringstream ss;
   ss << aColor.red() << " " << aColor.green() << " " << aColor.blue();
   if (aUseAlpha)
   {
      ss << " " << aColor.alpha();
   }
   return ss.str();
};

bool ZoneMatch(const std::string& aPlatform, const std::string& aName, const wkf::ZoneIdentifier& aZone)
{
   return (aPlatform == aZone.mPlatformName && aName == aZone.mZoneName);
}
}; // namespace

ZoneEditor::DockWidget::DockWidget(wkf::ZoneBrowserDataContainer& aDataContainer, QWidget* aParentPtr)
   : ZoneBrowserDockWidget(aDataContainer, aParentPtr)
{
   mUI.mTreeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
   mUI.mTreeWidget->installEventFilter(this);
   connect(mUI.mTreeWidget, &QTreeWidget::customContextMenuRequested, this, &DockWidget::CreateZoneContextMenu);
   connect(&mDataContainer, &wkf::ZoneBrowserDataContainer::ZoneUpdated, this, &DockWidget::ZoneUpdatedCB);
   mCallbacks.Add(wkf::Observer::AttachmentDeleted.Connect(&DockWidget::AttachmentDeletedCB, this));
}

void ZoneEditor::DockWidget::CreateZoneContextMenu(const QPoint& aPoint)
{
   QTreeWidgetItem* itemPtr = mUI.mTreeWidget->itemAt(aPoint);
   if (itemPtr != nullptr)
   {
      QWidget* platformWidgetPtr = mUI.mTreeWidget->itemWidget(itemPtr, 0);
      if (platformWidgetPtr != nullptr)
      {
         QWidget*                  widgetPtr     = platformWidgetPtr->layout()->itemAt(0)->widget();
         wkf::ZoneSelectionWidget* zoneWidgetPtr = dynamic_cast<wkf::ZoneSelectionWidget*>(widgetPtr);
         if (zoneWidgetPtr != nullptr)
         {
            QMenu*       menuPtr = ut::qt::make_qt_ptr<QMenu>();
            wkf::Action* gotoDefPtr =
               ut::qt::make_qt_ptr<wkf::Action>(QIcon::fromTheme("find_text"), QString("Go To Definition"), menuPtr);
            menuPtr->addAction(gotoDefPtr);
            connect(gotoDefPtr,
                    &QAction::triggered,
                    [&, zoneWidgetPtr]()
                    {
                       auto isEqual = [zoneWidgetPtr](const WsfPM_Zone& aZone)
                       {
                          std::string zoneName     = aZone.GetName();
                          std::string platformName = aZone.GetParent().GetParent().GetName();
                          if (platformName.empty())
                          {
                             platformName = "Global Zones";
                          }

                          return (zoneWidgetPtr->GetName() == zoneName &&
                                  zoneWidgetPtr->mOwningPlatformName == platformName);
                       };

                       auto foundZone = std::find_if(mZoneProxies.begin(), mZoneProxies.end(), isEqual);
                       if (foundZone != mZoneProxies.end())
                       {
                          if (wizard::Project::Instance() && wizard::Project::Instance()->TinyParseResults())
                          {
                             wizard::ParseResultsTiny* resultsPtr = wizard::Project::Instance()->TinyParseResults();
                             WsfPProxyIndex*           indexPtr   = resultsPtr->ProxyIndex();

                             if (indexPtr != nullptr)
                             {
                                WsfPProxyPath                    endPath = (*foundZone).GetPath();
                                std::vector<UtTextDocumentRange> ranges;
                                resultsPtr->FindModificationLocationsExtended(endPath, ranges);
                                wizard::FindReferencesResultDialog::GoToLocationList(wizard::Project::Instance(), ranges);
                             }
                          }
                       }
                    });
            menuPtr->exec(mUI.mTreeWidget->mapToGlobal(aPoint));
            delete menuPtr;
         }
      }
   }
}

void ZoneEditor::DockWidget::ApplyRandomColorPreferences()
{
   mRandom.SetSeed(cRANDOM_SEED);
   SetAllColors(true);
}

void ZoneEditor::DockWidget::ApplySetColorPreferences(const QColor& aLineColor, const QColor& aFillColor)
{
   mRandom.SetSeed(0);
   mDefaultLineColor = aLineColor;
   mDefaultFillColor = aFillColor;
   SetAllColors();
}

void ZoneEditor::DockWidget::RemoveZoneProxy(const WsfPM_Zone& aZone)
{
   auto isEqual = [aZone](const WsfPM_Zone& aZone2)
   {
      return (aZone.GetName() == aZone2.GetName() &&
              aZone.GetParent().GetParent().GetName() == aZone2.GetParent().GetParent().GetName());
   };

   auto foundZone = std::find_if(mZoneProxies.begin(), mZoneProxies.end(), isEqual);
   if (foundZone != mZoneProxies.end())
   {
      mZoneProxies.erase(foundZone);
   }
}

bool ZoneEditor::DockWidget::ZoneProxyFound(const WsfPM_Zone& aZone)
{
   auto isEqual = [aZone](const WsfPM_Zone& aZone2)
   {
      return (aZone.GetName() == aZone2.GetName() &&
              aZone.GetParent().GetParent().GetName() == aZone2.GetParent().GetParent().GetName());
   };

   auto foundZone = std::find_if(mZoneProxies.begin(), mZoneProxies.end(), isEqual);
   return foundZone != mZoneProxies.end();
}

void ZoneEditor::DockWidget::DrawZone(const std::string& aPlatform,
                                      const std::string& aName,
                                      const QColor&      aLineColor,
                                      const QColor&      aFillColor,
                                      bool               aPreview)
{
   wkf::ZoneIdentifier addedZone;
   auto                setCommonDataLambda =
      [&, aLineColor, aFillColor](wkf::AttachmentZone* aZonePtr, wkf::ZoneData* aDataPtr, vespa::VaEntity*& aAnchorPtr)
   {
      aZonePtr->Initialize();
      aZonePtr->Load();
      aZonePtr->SetAuxData(aDataPtr->GetAuxData());
      aZonePtr->SetMaxAltitudeSet(aDataPtr->IsMaxAltSet());
      aZonePtr->SetMinAltitudeSet(aDataPtr->IsMinAltSet());
      aZonePtr->SetBaseHeight(aDataPtr->GetMinAlt());
      aZonePtr->SetHeight(aDataPtr->GetMaxAlt() - aDataPtr->GetMinAlt());
      aZonePtr->SetMaxAltitude(aDataPtr->GetMaxAlt());
      aZonePtr->SetMinAltitude(aDataPtr->GetMinAlt());
      aZonePtr->SetColor(UtQtColor(aLineColor));
      aZonePtr->SetEnableFill(true);
      aZonePtr->SetBlendFactor(1 - aFillColor.alphaF());
      aZonePtr->SetFillColor(UtQtColor(aFillColor));

      bool altInvalid =
         aDataPtr->IsMaxAltSet() && aDataPtr->IsMinAltSet() && aDataPtr->GetMaxAlt() < aDataPtr->GetMinAlt();
      aZonePtr->SetStateVisibility(!altInvalid);


      wkf::AttachmentZoneStruct attachmentZone;
      attachmentZone.mAttachedToOwner = !aDataPtr->GetLocationDefined();
      attachmentZone.mMinAltitude     = aDataPtr->GetMinAlt();
      attachmentZone.mZonePtr         = aZonePtr;
      attachmentZone.mReferenceName   = aDataPtr->GetReferencePlatformName();

      if (attachmentZone.mAttachedToOwner)
      {
         auto iter = mAttachmentZoneMap.find(aAnchorPtr->GetUniqueId());
         if (iter != mAttachmentZoneMap.end())
         {
            iter->second.push_back(attachmentZone);
         }
         else
         {
            std::vector<wkf::AttachmentZoneStruct> vec = {attachmentZone};
            mAttachmentZoneMap.emplace(aAnchorPtr->GetUniqueId(), vec);
         }
      }
      addedZone.mZones.push_back(attachmentZone);
   };

   auto setPolygonalZoneDataLambda = [aLineColor](wkf::AttachmentZonePolygon* aZonePtr, wkf::PolygonalZoneData* aZoneDataPtr)
   {
      if (aZoneDataPtr->UseLatLon())
      {
         aZonePtr->SetBaseHeight(aZoneDataPtr->GetMinAlt());
         aZonePtr->SetHeight(aZoneDataPtr->GetMaxAlt() - aZoneDataPtr->GetMinAlt());
      }
      else
      {
         aZonePtr->SetBaseHeight(aZonePtr->GetParent().GetPosition().GetAlt() - aZoneDataPtr->GetMinAlt());
         aZonePtr->SetHeight(-(aZoneDataPtr->GetMaxAlt() - aZoneDataPtr->GetMinAlt()));
      }

      for (auto& curPoint : aZonePtr->GetZonePoints())
      {
         curPoint->SetColor(UtQtColor(aLineColor));
      }
      aZonePtr->SetClosed(true);
   };

   auto currentScenario = dynamic_cast<vespa::VaScenario*>(vaEnv.GetStandardScenario());
   if (currentScenario)
   {
      std::map<std::string, vespa::VaEntity*> anchorMap;
      wkf::ZoneSetData*                       zoneDataPtr = nullptr;
      GetAnchorEntityData(aPlatform, aName, zoneDataPtr, anchorMap, aPreview);

      if (!anchorMap.empty() && zoneDataPtr)
      {
         for (const auto& zonePtr : zoneDataPtr->GetZones())
         {
            wkf::AttachmentZone* attachmentZonePtr = nullptr;
            auto                 zoneEntity        = vaEnv.FindEntity(zonePtr->GetZoneName());
            if (zoneEntity != nullptr)
            {
               attachmentZonePtr = dynamic_cast<wkf::AttachmentZone*>(zoneEntity->FindAttachment(aName));
            }
            if (attachmentZonePtr)
            {
               auto zoneIter = anchorMap.find(zonePtr->GetZoneName());
               if (zoneIter != anchorMap.end())
               {
                  auto& anchorPtr = zoneIter->second;
                  setCommonDataLambda(attachmentZonePtr, zonePtr.get(), anchorPtr);
                  if (zonePtr->GetZoneType() == wkf::ZoneType::cPOLYGONAL)
                  {
                     auto polyZonePtr      = dynamic_cast<wkf::AttachmentZonePolygon*>(attachmentZonePtr);
                     auto polygonalDataPtr = dynamic_cast<wkf::PolygonalZoneData*>(zonePtr.get());
                     if (polyZonePtr != nullptr && polygonalDataPtr != nullptr)
                     {
                        setPolygonalZoneDataLambda(polyZonePtr, polygonalDataPtr);
                        if (!aPreview)
                        {
                           polyZonePtr->SetNeverShowZones(false);
                           polyZonePtr->SetHideZonePoints(true);
                        }
                        // reload/build is necessary to avoid unusual rotation behavior
                        polyZonePtr->Load();
                     }
                  }
               }
            }
         }
         if (!addedZone.mZones.empty())
         {
            addedZone.mPlatformName = aPlatform;
            addedZone.mZoneName     = aName;
            auto it                 = std::find_if(mDrawZones.begin(),
                                   mDrawZones.end(),
                                   [&](const wkf::ZoneIdentifier& aZone) { return ZoneMatch(aPlatform, aName, aZone); });

            if (it == mDrawZones.end())
            {
               // only add the zone if it doesn't already exist in the draw list
               mDrawZones.emplace_back(addedZone);
            }
         }
      }
   }
}

void ZoneEditor::DockWidget::HideZone(const std::string& aPlatform, const std::string& aName)
{
   std::map<std::string, vespa::VaEntity*> anchorMap;
   wkf::ZoneSetData*                       zoneDataPtr = nullptr;
   GetAnchorEntityData(aPlatform, aName, zoneDataPtr, anchorMap, false);
   if (!anchorMap.empty() && zoneDataPtr)
   {
      for (const auto& zonePtr : zoneDataPtr->GetZones())
      {
         auto zoneIter = anchorMap.find(zonePtr->GetZoneName());
         if (zoneIter != anchorMap.end())
         {
            auto zoneName = zonePtr->GetZoneName();
            auto foundEnt = vaEnv.FindEntity(zonePtr->GetZoneName());
            if (foundEnt != nullptr)
            {
               auto attachmentZonePtr =
                  dynamic_cast<wkf::AttachmentZone*>(vaEnv.FindEntity(zonePtr->GetZoneName())->FindAttachment(aName));
               if (attachmentZonePtr != nullptr)
               {
                  attachmentZonePtr->ClearSelection(vaEnv.GetStandardViewer());
                  attachmentZonePtr->SetStateVisibility(false);
                  attachmentZonePtr->Load();
               }
            }
         }
      }
   }
}

void ZoneEditor::DockWidget::GetAnchorEntityData(const std::string&                       aPlatform,
                                                 const std::string&                       aName,
                                                 wkf::ZoneSetData*&                       aDataPtr,
                                                 std::map<std::string, vespa::VaEntity*>& aAnchors,
                                                 bool                                     aPreview)
{
   auto getAnchorLambda = [&](wkf::ZoneData* aZonePtr, vespa::VaEntity*& aAnchorPtr, vespa::VaEntity*& aParentPtr)
   {
      auto zoneName  = aZonePtr->GetZoneName();
      auto anchorEnt = vaEnv.FindEntity(zoneName);
      if (anchorEnt != nullptr)
      {
         aAnchorPtr = anchorEnt;
      }
      else
      {
         auto* scenarioPtr = vaEnv.GetStandardScenario();
         if (scenarioPtr)
         {
            aAnchorPtr = new vespa::VaEntity(zoneName);
            if (scenarioPtr->AddEntity(aAnchorPtr))
            {
               aAnchorPtr->Initialize();
               vespa::VaPosition position;
               position.SetLat(aZonePtr->GetReferenceLat());
               position.SetLon(aZonePtr->GetReferenceLon());
               aAnchorPtr->SetPosition(position);
               aAnchorPtr->SetHeading(aZonePtr->GetHeading());
               mEntityTracker.emplace_back(aAnchorPtr);
            }
         }
      }
   };

   auto getRefAnchor = [&](const std::string& aReferenceName, vespa::VaEntity*& aAnchorPtr)
   {
      auto it = mAnchorZoneMap.find(aReferenceName);
      if (it != mAnchorZoneMap.end())
      {
         aAnchorPtr = it->second;
      }
      else
      {
         auto* scenarioPtr = vaEnv.GetStandardScenario();
         if (scenarioPtr)
         {
            aAnchorPtr = scenarioPtr->FindPlatformByName(aReferenceName);
            if (aAnchorPtr)
            {
               mAnchorZoneMap.emplace(aReferenceName, aAnchorPtr);
            }
         }
      }
   };

   wkf::ZoneSetData* zoneSetPtr{nullptr};
   if (aPlatform == "Global Zones" /*wkf::cGLOBAL_NAME*/)
   {
      zoneSetPtr = (aPreview ? mDataContainer.GetPreviewZone() : mDataContainer.GetGlobalZone(aName));

      if (zoneSetPtr != nullptr)
      {
         if (vaEnv.GetStandardScenario() != nullptr)
         {
            vespa::VaEntity* anchorPtr{nullptr};
            for (const auto& zonePtr : zoneSetPtr->GetZones())
            {
               if (zonePtr->GetLocationDefined() || zonePtr->GetReferencePlatformName().empty() ||
                   zonePtr->GetReferencePlatformName() == "Global Zones" /*wkf::cGLOBAL_NAME*/)
               {
                  getAnchorLambda(zonePtr.get(), anchorPtr, anchorPtr);
               }
               else
               {
                  getRefAnchor(zonePtr->GetReferencePlatformName(), anchorPtr);
               }
               if (anchorPtr)
               {
                  aAnchors.emplace(zonePtr->GetZoneName(), anchorPtr);
               }
            }
         }
      }
   }
   else
   {
      zoneSetPtr = (aPreview ? mDataContainer.GetPreviewZone() : mDataContainer.GetLocalZone(aPlatform, aName));

      if (zoneSetPtr != nullptr)
      {
         vespa::VaEntity* platformAnchorPtr{nullptr};
         auto             it = mAnchorZoneMap.find(aPlatform);
         if (it != mAnchorZoneMap.end())
         {
            platformAnchorPtr = it->second;
         }
         else
         {
            auto* scenarioPtr = vaEnv.GetStandardScenario();
            if (scenarioPtr)
            {
               platformAnchorPtr = scenarioPtr->FindPlatformByName(aPlatform);
               if (platformAnchorPtr)
               {
                  mAnchorZoneMap.emplace(aPlatform, platformAnchorPtr);
               }
            }
         }

         for (const auto& zonePtr : zoneSetPtr->GetZones())
         {
            auto             zoneName = zonePtr->GetZoneName();
            vespa::VaEntity* anchorPtr{nullptr};
            if (zonePtr->GetLocationDefined())
            {
               getAnchorLambda(zonePtr.get(), anchorPtr, platformAnchorPtr);
            }
            else if (!zonePtr->GetReferencePlatformName().empty())
            {
               getRefAnchor(zonePtr->GetReferencePlatformName(), anchorPtr);
            }
            else if (platformAnchorPtr)
            {
               anchorPtr = platformAnchorPtr;
            }

            if (anchorPtr)
            {
               aAnchors.emplace(zoneName, anchorPtr);
            }
         }
      }
   }
   aDataPtr = zoneSetPtr;
}


void ZoneEditor::DockWidget::AddZone(wkf::ZoneSetData* aDataPtr, bool aShow)
{
   // Find the platform this zone is attached to in the tree and call PopulateDropdown on it.
   // Global zones will have the platform name "Global Zones"
   std::string platformName     = aDataPtr->GetPlatformName();
   auto        platformCategory = mUI.mTreeWidget->findItems(QString::fromStdString(platformName), Qt::MatchExactly);
   aShow                        = mUI.mAllCheckBox->isChecked() ? true : aShow;

   QTreeWidgetItem* itemPtr = nullptr;
   if (!platformCategory.isEmpty())
   {
      itemPtr = platformCategory[0];
   }
   else
   {
      itemPtr = ut::qt::make_qt_ptr<QTreeWidgetItem>(mUI.mTreeWidget->invisibleRootItem(),
                                                     QStringList(QString::fromStdString(platformName)));
      // On update, reinsert the platform to its previous position. Automatically place global zones at the top
      auto currentIndex = mUI.mTreeWidget->indexOfTopLevelItem(itemPtr);
      if (platformName == "Global Zones")
      {
         mPrevIndex = 0;
      }
      else
      {
         auto globalCategory = mUI.mTreeWidget->findItems(QString::fromStdString("Global Zones"), Qt::MatchExactly);
         if (!globalCategory.isEmpty() && mPrevIndex == 0)
         {
            mPrevIndex = currentIndex; // ensure the item is added below "Global zones"
         }
      }

      if (currentIndex != mPrevIndex && mPrevIndex < mUI.mTreeWidget->topLevelItemCount())
      {
         mUI.mTreeWidget->takeTopLevelItem(currentIndex);
         mUI.mTreeWidget->insertTopLevelItem(mPrevIndex, itemPtr);
      }
      // Set the next item added to the end
      mPrevIndex = mUI.mTreeWidget->topLevelItemCount();
   }
   AddZoneToBrowser(itemPtr, aDataPtr, aShow);
   mUI.mAllCheckBox->setCheckState(CalculateAllBoxState());
}

void ZoneEditor::DockWidget::PreviewPendingDeletionCB()
{
   auto it = std::find_if(mDrawZones.begin(),
                          mDrawZones.end(),
                          [&](const wkf::ZoneIdentifier& aZone)
                          {
                             return ZoneMatch(mDataContainer.GetPreviewZone()->GetPlatformName(),
                                              mDataContainer.GetPreviewZone()->GetSetName(),
                                              aZone);
                          });

   if (it != mDrawZones.end())
   {
      if (it->mPlatformName.empty() || it->mPlatformName == "Global Zones")
      {
         // If there is a reference platform, then this zone is attached to a platform and only the attachment should be deleted.
         for (auto& zone : it->mZones)
         {
            if (!zone.mReferenceName.empty() && zone.mAttachedToOwner)
            {
               if (vaEnv.GetStandardScenario() != nullptr)
               {
                  if (vaEnv.GetStandardScenario()->FindPlatformByName(zone.mReferenceName) != nullptr)
                  {
                     zone.mZonePtr->GetParent().RemoveAttachment(zone.mZonePtr->GetUniqueId());
                  }
               }
            }
            else
            {
               auto entity = std::find(mEntityTracker.begin(), mEntityTracker.end(), &zone.mZonePtr->GetParent());
               if (entity != mEntityTracker.end())
               {
                  mEntityTracker.erase(entity);
               }
               vaEnv.DeleteEntity(&zone.mZonePtr->GetParent());
            }
         }
      }
      else
      {
         // This isn't a global zone, so it must be attached to a platform. Don't delete the platform -- only the attachment.
         if (vaEnv.GetStandardScenario() != nullptr)
         {
            if (vaEnv.GetStandardScenario()->FindPlatformByName(it->mPlatformName) != nullptr)
            {
               for (auto zone : it->mZones)
               {
                  zone.mZonePtr->GetParent().RemoveAttachment(zone.mZonePtr->GetUniqueId());
               }
            }
         }
      }
      mDrawZones.erase(it);
   }
}

void ZoneEditor::DockWidget::AttachmentDeletedCB(vespa::VaAttachment* aAttachPtr, const vespa::VaAttachmentSubId& aId)
{
   wkf::AttachmentZone* zoneAttachmentPtr = dynamic_cast<wkf::AttachmentZone*>(aAttachPtr);
   if (!zoneAttachmentPtr)
   {
      return;
   }

   auto it = std::find_if(mDrawZones.begin(),
                          mDrawZones.end(),
                          [&](const wkf::ZoneIdentifier& aZone)
                          {
                             for (auto zone : aZone.mZones)
                             {
                                if (zone.mZonePtr == zoneAttachmentPtr)
                                {
                                   return true;
                                }
                             }
                             return false;
                          });

   // If the zone containing the attachment is currently being drawn, delete it.
   if (it != mDrawZones.end())
   {
      mDrawZones.erase(it);
   }
}


void ZoneEditor::DockWidget::SetZoneLineColor(const std::string& aPlatform, const std::string& aName, const QColor& aColor)
{
   ZoneBrowserDockWidget::SetZoneLineColor(aPlatform, aName, aColor);
   auto isEqual = [&](const WsfPM_Zone& aZone)
   {
      std::string zoneName     = aZone.GetName();
      std::string platformName = aZone.GetParent().GetParent().GetName();
      if (platformName.empty())
      {
         platformName = "Global Zones";
      }

      return (aName == zoneName && aPlatform == platformName);
   };
   if (mButtonClicked)
   {
      auto foundZone = std::find_if(mZoneProxies.begin(), mZoneProxies.end(), isEqual);
      if (foundZone != mZoneProxies.end())
      {
         if (foundZone->IsDefinition() || foundZone->IsZoneSet())
         {
            // Remove existing line color and replace it with the new color
            if (foundZone->IsValid())
            {
               QString                    newValue = QString::fromStdString(ColorToString(aColor, true));
               std::multiset<ZoneWrapper> sortedChanges;

               // there is a chance that the zone is valid but the colors are defined in the zone definition located
               // within the platform_type definition. Check to see if this is the case and act accordingly.
               if (!foundZone->GetSourceDocumentRange().Valid())
               {
                  WsfPM_Platform platNode = static_cast<WsfPM_Platform>(foundZone->GetParent().GetParent());
                  if (platNode.IsValid())
                  {
                     auto zoneNode = GetPlatformTypeZone(platNode, foundZone->GetName());
                     if (zoneNode.IsValid())
                     {
                        AddToSortedChanges(sortedChanges,
                                           GetColorReplacementCommands(zoneNode, "lineColor", "line_color", newValue));
                     }
                  }
               }
               else
               {
                  AddToSortedChanges(sortedChanges,
                                     GetColorReplacementCommands(*foundZone, "lineColor", "line_color", newValue));
               }

               auto it = sortedChanges.begin();
               for (auto changeIt : sortedChanges)
               {
                  bool sameRange = false;
                  if (std::next(it) != sortedChanges.end())
                  {
                     sameRange = (++it)->GetRange() == changeIt.GetRange();
                  }
                  changeIt.Process(!sameRange);
               }
            }
         }
      }
   }
}

void ZoneEditor::DockWidget::SetZoneFillColor(const std::string& aPlatform, const std::string& aName, const QColor& aColor)
{
   ZoneBrowserDockWidget::SetZoneFillColor(aPlatform, aName, aColor);
   auto isEqual = [&](const WsfPM_Zone& aZone)
   {
      std::string zoneName     = aZone.GetName();
      std::string platformName = aZone.GetParent().GetParent().GetName();
      if (platformName.empty())
      {
         platformName = "Global Zones";
      }

      return (aName == zoneName && aPlatform == platformName);
   };

   if (mButtonClicked)
   {
      auto foundZone = std::find_if(mZoneProxies.begin(), mZoneProxies.end(), isEqual);
      if (foundZone != mZoneProxies.end())
      {
         if (foundZone->IsDefinition() || foundZone->IsZoneSet())
         {
            // Remove existing fill color and replace it with the new color
            if (foundZone->IsValid())
            {
               QString                    newValue = QString::fromStdString(ColorToString(aColor, true));
               std::multiset<ZoneWrapper> sortedChanges;
               // there is a chance that the zone is valid but the colors are defined in the zone definition located
               // within the platform_type definition. Check to see if this is the case and act accordingly.
               if (!foundZone->GetSourceDocumentRange().Valid())
               {
                  WsfPM_Platform platNode = static_cast<WsfPM_Platform>(foundZone->GetParent().GetParent());
                  if (platNode.IsValid())
                  {
                     auto zoneNode = GetPlatformTypeZone(platNode, foundZone->GetName());
                     if (zoneNode.IsValid())
                     {
                        AddToSortedChanges(sortedChanges,
                                           GetColorReplacementCommands(zoneNode, "fillColor", "fill_color", newValue));
                     }
                  }
               }
               else
               {
                  AddToSortedChanges(sortedChanges,
                                     GetColorReplacementCommands(*foundZone, "fillColor", "fill_color", newValue));
               }
               auto it = sortedChanges.begin();
               for (auto changeIt : sortedChanges)
               {
                  bool sameRange = false;
                  if (std::next(it) != sortedChanges.end())
                  {
                     sameRange = (++it)->GetRange() == changeIt.GetRange();
                  }
                  changeIt.Process(!sameRange);
               }
            }
         }
      }
   }
}

void ZoneEditor::DockWidget::ZoneUpdatedCB(const std::string& aZoneName, const std::string& aPlatformName)
{
   auto isEqual = [&](const WsfPM_Zone& aZone)
   {
      std::string zoneName     = aZone.GetName();
      std::string platformName = aZone.GetParent().GetParent().GetName();
      if (platformName.empty())
      {
         platformName = "Global Zones";
      }

      return (aZoneName == zoneName && aPlatformName == platformName);
   };

   auto foundZone = std::find_if(mZoneProxies.begin(), mZoneProxies.end(), isEqual);
   if (foundZone != mZoneProxies.end() && (foundZone->IsDefinition() || foundZone->IsZoneSet()))
   {
      if (foundZone->GetSourceDocumentRange().Valid())
      {
         auto fillColor = foundZone->GetFillColor();
         if (fillColor.has_value())
         {
            mPrevFillColor = UtQtColor(fillColor.value());
         }
         auto lineColor = foundZone->GetLineColor();
         if (lineColor.has_value())
         {
            mPrevLineColor = UtQtColor(lineColor.value());
         }
      }
      else
      {
         // check for a change in the zone defined in platform_type
         auto zoneNode = GetPlatformTypeZone(foundZone->GetParent().GetParent(), foundZone->GetName());
         if (zoneNode.IsValid())
         {
            auto fillColor = zoneNode.GetFillColor();
            if (fillColor.has_value())
            {
               mPrevFillColor = UtQtColor(fillColor.value());
            }
            auto lineColor = zoneNode.GetLineColor();
            if (lineColor.has_value())
            {
               mPrevLineColor = UtQtColor(lineColor.value());
            }
         }
      }
   }
   ZoneBrowserDockWidget::ZoneUpdatedCB(aZoneName, aPlatformName);
}

void ZoneEditor::DockWidget::SetAllColors(bool aRandom)
{
   mButtonClicked = false;
   std::string platformName;
   std::string zoneName;
   auto        isEqual = [&](const WsfPM_Zone& aZone)
   {
      auto platform = aZone.GetParent().GetParent().GetName();
      if (platform.empty())
      {
         platform = "Global Zones";
      }
      return (zoneName == aZone.GetName() && platformName == platform);
   };
   auto widgetList = mUI.mTreeWidget->findChildren<wkf::ZoneSelectionWidget*>();
   for (const auto& widgetPtr : widgetList)
   {
      QColor lineColor(GetLineColor(aRandom));
      QColor fillColor(GetFillColor(aRandom));
      platformName   = widgetPtr->mOwningPlatformName;
      zoneName       = widgetPtr->GetName();
      auto foundZone = std::find_if(mZoneProxies.begin(), mZoneProxies.end(), isEqual);
      if (foundZone != mZoneProxies.end())
      {
         if (!foundZone->GetSourceDocumentRange().Valid())
         {
            WsfPM_Platform platNode = static_cast<WsfPM_Platform>(foundZone->GetParent().GetParent());
            if (platNode.IsValid())
            {
               auto zoneNode = GetPlatformTypeZone(platNode, foundZone->GetName());
               if (zoneNode.IsValid())
               {
                  auto fill = zoneNode.GetFillColor();
                  if (fill.has_value())
                  {
                     fillColor = UtQtColor(fill.value());
                  }
                  auto line = zoneNode.GetLineColor();
                  if (line.has_value())
                  {
                     lineColor = UtQtColor(line.value());
                  }
               }
            }
         }
         else if (foundZone->IsDefinition() || foundZone->IsZoneSet())
         {
            auto fill = foundZone->GetFillColor();
            if (fill.has_value())
            {
               fillColor = UtQtColor(fill.value());
            }

            auto line = foundZone->GetLineColor();
            if (line.has_value())
            {
               lineColor = UtQtColor(line.value());
            }
         }
      }
      widgetPtr->mLineColorButtonPtr->SetColor(lineColor);
      widgetPtr->mFillColorButtonPtr->SetColor(fillColor);
      emit widgetPtr->mLineColorButtonPtr->ColorChanged(lineColor);
      emit widgetPtr->mFillColorButtonPtr->ColorChanged(fillColor);
   }
}

void ZoneEditor::DockWidget::DeleteZone(wkf::ZoneSetData* aDataPtr)
{
   auto it = std::find_if(mDrawZones.begin(),
                          mDrawZones.end(),
                          [&](const wkf::ZoneIdentifier& aZone)
                          { return ZoneMatch(aDataPtr->GetPlatformName(), aDataPtr->GetSetName(), aZone); });

   // If the zone is currently being drawn, delete the attachment and remove it from mDrawZones.
   if (it != mDrawZones.end())
   {
      mDrawZones.erase(it);
   }

   // Find the platform this zone is attached to in the tree and remove the zone entry.
   auto platformCategory =
      mUI.mTreeWidget->findItems(QString::fromStdString(aDataPtr->GetPlatformName()), Qt::MatchExactly);
   if (!platformCategory.isEmpty())
   {
      mPrevIndex = mUI.mTreeWidget->indexOfTopLevelItem(platformCategory[0]);
      for (int i = 0; i < platformCategory[0]->childCount(); i++)
      {
         QWidget*                  platformWidgetPtr = mUI.mTreeWidget->itemWidget(platformCategory[0]->child(i), 0);
         QWidget*                  widgetPtr         = platformWidgetPtr->layout()->itemAt(0)->widget();
         wkf::ZoneSelectionWidget* zoneWidgetPtr     = dynamic_cast<wkf::ZoneSelectionWidget*>(widgetPtr);
         if (zoneWidgetPtr != nullptr)
         {
            std::string label = zoneWidgetPtr->GetName();
            if (label == aDataPtr->GetSetName())
            {
               // Store various values for use when updating a zone.
               mPrevVisible    = zoneWidgetPtr->mVisibilityBoxPtr->isChecked();
               mPrevLineColor  = zoneWidgetPtr->mLineColorButtonPtr->GetColor();
               mPrevFillColor  = zoneWidgetPtr->mFillColorButtonPtr->GetColor();
               mPrevChildIndex = i;
               platformCategory[0]->removeChild(platformCategory[0]->child(i));

               // After removing the child, if the platform entry is empty, remove the entry.
               if (platformCategory[0]->childCount() == 0)
               {
                  mUI.mTreeWidget->invisibleRootItem()->removeChild(platformCategory[0]);
               }
               break;
            }
         }
      }
   }
}

void ZoneEditor::DockWidget::RemovePlatform(const std::string& aPlatformName)
{
   // stop maintaining the AttachmentZone belonging to or referencing the deleted platform
   auto removalLambda = [&, aPlatformName](wkf::ZoneIdentifier& aZoneData)
   {
      bool retVal{false};
      if (aZoneData.mPlatformName == aPlatformName)
      {
         retVal = true;
      }
      return retVal;
   };
   mDrawZones.erase(std::remove_if(mDrawZones.begin(), mDrawZones.end(), removalLambda), mDrawZones.end());

   // In the browser, get rid of any zones that reference the deleted platform
   QList<QWidget*>                  menuOptions = mUI.mTreeWidget->findChildren<QWidget*>();
   QList<wkf::ZoneSelectionWidget*> deleteList;
   for (auto curWidget : menuOptions)
   {
      wkf::ZoneSelectionWidget* zoneSelectionWidget = dynamic_cast<wkf::ZoneSelectionWidget*>(curWidget);
      if (zoneSelectionWidget)
      {
         if (zoneSelectionWidget->mReferencePlatformName == aPlatformName ||
             zoneSelectionWidget->mOwningPlatformName == aPlatformName)
         {
            deleteList.push_back(zoneSelectionWidget);
         }
      }
   }

   for (auto& curItem : deleteList)
   {
      delete (curItem);
   }

   // in the browser, if the deleted platform had zones, get rid of the platform's entire subtree
   // Note: this does not delete child ZoneSelectionWidgets due to how the widget was constructed, so manually delete those
   for (int i = 0; i < mUI.mTreeWidget->topLevelItemCount(); ++i)
   {
      QTreeWidgetItem* itemPtr = mUI.mTreeWidget->topLevelItem(i);
      if (itemPtr->text(0).toStdString() == aPlatformName)
      {
         delete itemPtr;
         i--;
         continue;
      }
      if (itemPtr->isExpanded())
      {
         // this is essentially a redraw of the dropdown menu: complications with how the dropdown is implemented make
         // it difficult to do otherwise
         itemPtr->setExpanded(false);
         itemPtr->setExpanded(true);
      }
   }
}
