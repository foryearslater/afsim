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

#include "WizAnnotationPlugin.hpp"

#include <cctype>

#include <QTextBlock>

#include "ChangeWrapper.hpp"
#include "EditorHelpers.hpp"
#include "PointOfInterestPropertiesWidget.hpp"
#include "Project.hpp"
#include "ProxyWatcher.hpp"
#include "RangeRingPropertiesWidget.hpp"
#include "UtQtColorButton.hpp"
#include "UtQtMemory.hpp"
#include "UtTextDocument.hpp"
#include "WkfViewer.hpp"
#include "WsfEditor.hpp"
#include "WsfExe.hpp"
#include "WsfExeManager.hpp"
#include "WsfPProxyRegistry.hpp"

WKF_PLUGIN_DEFINE_SYMBOLS(WizAnnotation::Plugin,
                          "Annotations",
                          "Provides an interface for annotation operations between the map display and text editor.",
                          "wizard")

namespace
{
const std::string cNAME_FIELD = "NAME";

// Necessary because syntax won't be correctly highlighted unless the grammar follows this specific format
// e.g. (new (type decorations $1) (type decorations WSF_DECORATIONS)) [apply($$)]
// However, this "type" causes issues when traversing the objectMap since it gets included as an attribute, so these
// nodes should just be ignored
const std::unordered_set<std::string> cOMIT_TYPE_SET = {"WSF_DECORATIONS", "WSF_POIS", "WSF_RANGERINGS"};

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

std::vector<TextReplacementInfo> GetSingleTextReplacementCommands(const WsfPProxyNode& aProxyNode,
                                                                  const std::string&   aAttributeName,
                                                                  const std::string&   aCommand,
                                                                  bool                 aAdd)
{
   std::vector<TextReplacementInfo> info;
   WsfPProxyNode                    attributeNode = aProxyNode + aAttributeName;
   std::string                      sourceText    = attributeNode.GetSourceText();
   if (aAdd && sourceText.empty())
   {
      size_t      tabSize      = wkfEnv.GetPreferenceObject<wizard::EditorPrefObject>()->TabSpace();
      size_t      numberSpaces = tabSize + tabSize;
      std::string spaces;
      spaces.insert(0, numberSpaces, 0x20);
      QString             command     = QString::fromStdString("\n" + spaces + aCommand);
      UtTextDocumentRange wholeRange  = aProxyNode.GetSourceDocumentRange();
      std::string         commandText = wholeRange.Text();
      // Get the location of the last whitespace (occurs before the end_block) and iterate backwards until reaching the prior block.
      size_t index = commandText.find_last_of(" \t\r\n\f");
      while (index != 0 && std::isspace(commandText[--index]) != 0)
      {
      }
      UtTextDocumentRange insertRange(wholeRange.mSource,
                                      wholeRange.GetBegin() + index + 1,
                                      wholeRange.GetBegin() + index + 1);
      info.emplace_back(insertRange, "", command, true, true);
   }
   else if (!aAdd && sourceText == aCommand)
   {
      WsfPProxyIndexNode* indexNode  = attributeNode.FindIndexNode();
      size_t              entryCount = indexNode->mEntries.size();
      WsfPProxyIndexEntry lastEntry  = indexNode->mEntries[entryCount - 1];
      UtTextDocumentRange entryRange = lastEntry.mNodePtr->GetParent()->SubtreeRange();
      info.emplace_back(entryRange, QString::fromStdString(aAttributeName), "", false, true);
      // Remove any commands that were overidden. This should be done in backwards order to prevent any issues with ranges changing
      if (entryCount > 1)
      {
         for (size_t i = entryCount - 1; i != 0; --i)
         {
            WsfPProxyIndexEntry entry = indexNode->mEntries[i - 1];
            UtTextDocumentRange range = entry.mNodePtr->GetParent()->SubtreeRange();
            info.emplace_back(range, QString::fromStdString(aAttributeName), "", false, true);
         }
      }
   }
   return info;
}

void RemoveQuotesFromQuotableString(std::string& aQuotableString)
{
   if (aQuotableString.size() > 2 && aQuotableString[0] == '"' && aQuotableString[aQuotableString.size() - 1] == '"')
   {
      aQuotableString = aQuotableString.substr(1, aQuotableString.size() - 2);
   }
}

std::string GetProxyNodeString(const WsfPProxyNode& aNode, const std::string& aAttr)
{
   std::string    retVal;
   WsfPProxyValue stringVal = aNode.GetValue().GetAttr(aAttr);
   if (stringVal.IsValid())
   {
      retVal = stringVal.ValueToString();
      RemoveQuotesFromQuotableString(retVal);
   }
   return retVal;
}

QColor GetProxyNodeColor(const WsfPProxyNode& aNode, const std::string& aAttr)
{
   QColor        retVal(Qt::white);
   WsfPProxyNode colorNode = aNode + aAttr;

   if (colorNode.IsValid())
   {
      WsfPProxyValue valR = colorNode.GetValue().GetAttr("red");
      WsfPProxyValue valG = colorNode.GetValue().GetAttr("green");
      WsfPProxyValue valB = colorNode.GetValue().GetAttr("blue");
      WsfPProxyValue valA = colorNode.GetValue().GetAttr("alpha");
      if (valR.IsValid() && valG.IsValid() && valB.IsValid())
      {
         int r;
         int g;
         int b;
         int a = 255;
         valR.GetIntegerValue(r);
         valG.GetIntegerValue(g);
         valB.GetIntegerValue(b);
         if (valA.IsValid())
         {
            valA.GetIntegerValue(a);
         }
         retVal.setRgba(qRgba(r, g, b, a));
      }
   }
   return retVal;
}

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

// Used to process multiple operations in one parse
std::vector<TextReplacementInfo> GetTextReplacementCommands(const WsfPProxyNode& aProxyNode,
                                                            const std::string&   aAttributeName,
                                                            const std::string&   aCommandName,
                                                            const QString&       aInsert)
{
   std::vector<TextReplacementInfo> info;
   WsfPProxyValue                   proxyVal = aProxyNode.GetValue().GetAttr(aAttributeName);
   WsfPProxyNode attributeNode               = aProxyNode + aAttributeName; // this will be valid if proxyVal is valid
   if (proxyVal.IsValid() && !proxyVal.IsUnset() && attributeNode.FindIndexNode())
   {
      // The user repeat commands with different values to override the original, so it is necessary to get the last entry
      if (!proxyVal.ValueToString().empty() && attributeNode.FindIndexNode())
      {
         WsfPProxyIndexNode* indexNode  = attributeNode.FindIndexNode();
         size_t              entryCount = indexNode->mEntries.size();
         WsfPProxyIndexEntry lastEntry  = indexNode->mEntries[entryCount - 1];
         UtTextDocumentRange entryRange = lastEntry.mNodePtr->GetParent()->SubtreeRange();
         info.emplace_back(entryRange, QString::fromStdString(aCommandName), aInsert);
         // Remove any commands that were overridden. This should be done in backwards order to prevent any issues with
         // ranges changing
         if (entryCount > 1)
         {
            for (size_t i = entryCount - 1; i != 0; --i)
            {
               WsfPProxyIndexEntry entry = indexNode->mEntries[i - 1];
               UtTextDocumentRange range = entry.mNodePtr->GetParent()->SubtreeRange();
               info.emplace_back(range, QString::fromStdString(aCommandName), "");
            }
         }
      }
   }
   else if (!aInsert.isEmpty())
   {
      InsertNewCommandHandler(info, aProxyNode, aCommandName, aInsert);
   }
   return info;
}

class AnnotationWrapper : public wizard::ChangeWrapper
{
public:
   AnnotationWrapper(wizard::Editor* aEditor, const TextReplacementInfo& aInfo)
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

void AddToSortedChanges(std::multiset<AnnotationWrapper>& aSortedChanges, const std::vector<TextReplacementInfo>& aList)
{
   for (const auto& it : aList)
   {
      wizard::Editor* editorPtr =
         wizard::Project::Instance()->GotoFile(it.mEntryRange.mSource->GetFilePath().GetNormalizedPath());
      aSortedChanges.insert(AnnotationWrapper(editorPtr, it));
   }
}

// Functions used for quick access to attribute values from proxy nodes
//=========================================================================================================
std::pair<UtLatPos, UtLonPos> GetProxyNodeLatLon(const WsfPProxyNode& aNode)
{
   std::pair<double, double> latLon = {0, 0};
   WsfPProxyValue            posVal = aNode.GetValue().GetAttr("position");
   if (posVal.IsValid())
   {
      WsfProxy::Position proxyPos = *(WsfProxy::Position*)(posVal);
      latLon.first                = proxyPos.GetLatitude();
      latLon.second               = proxyPos.GetLongitude();
   }
   return latLon;
}

UtLengthValue GetProxyNodeAltitude(const WsfPProxyNode& aNode)
{
   UtLengthValue  alt    = 0;
   WsfPProxyValue altVal = aNode.GetValue().GetAttr("altitude");
   if (altVal.IsValid())
   {
      WsfProxy::Length proxyAlt = *(WsfProxy::Length*)(altVal);
      alt                       = proxyAlt.GetValue();
   }
   return alt;
}

bool GetProxyNodeBool(const WsfPProxyNode& aNode, const std::string& aAttr)
{
   bool           retVal = false;
   WsfPProxyValue val    = aNode.GetValue().GetAttr(aAttr);
   if (val.IsValid())
   {
      retVal = val.GetBoolValue();
   }
   return retVal;
}

UtAngleValue GetProxyNodeAngle(const WsfPProxyNode& aNode, const std::string& aAttr)
{
   WsfPProxyValue angleVal = aNode.GetValue().GetAttr(aAttr);
   if (angleVal.IsValid())
   {
      WsfProxy::Angle proxyAngle = *(WsfProxy::Angle*)(angleVal);
      return proxyAngle.GetValue();
   }
   return UtAngleValue(0.0);
}

int GetProxyNodeInt(const WsfPProxyNode& aNode, const std::string& aAttr)
{
   int            retVal = 0;
   WsfPProxyValue intVal = aNode.GetValue().GetAttr(aAttr);
   if (intVal.IsValid())
   {
      intVal.GetIntegerValue(retVal);
   }
   return retVal;
}

UtLengthValue GetProxyNodeLength(const WsfPProxyNode& aNode, const std::string& aAttr)
{
   WsfPProxyValue lenVal = aNode.GetValue().GetAttr(aAttr);
   if (lenVal.IsValid())
   {
      WsfProxy::Length proxyLength = *(WsfProxy::Length*)(lenVal);
      return proxyLength.GetValue();
   }
   return UtLengthValue(0.0);
}

wizard::Editor* GetEditorAtRange(const WsfPProxyNode& aProxyNode)
{
   UtTextDocumentRange prevRange = aProxyNode.GetSourceDocumentRange();
   UtTextDocumentRange newRange  = prevRange;
   newRange.SetBegin(prevRange.GetEnd() + 1);
   newRange.SetEnd(prevRange.GetEnd() + 1);
   wizard::Editor* editorPtr = wizard::Project::Instance()->GotoFile(prevRange.mSource->GetFilePath().GetNormalizedPath());
   editorPtr->GoToRange(newRange);
   return editorPtr;
}

template<typename T>
void AddLineToCommand(QString& aCommand, bool aUse, const std::string& aLineBeginning, const T& aValue)
{
   if (aUse)
   {
      std::stringstream ss;
      ss << "\n" << aLineBeginning << " " << aValue;
      aCommand += QString::fromStdString(ss.str());
   }
}

template<typename T>
T BoundValue(T aVal, T aMin, T aMax)
{
   if (aVal > aMax)
   {
      return aMax;
   }
   else if (aVal < aMin)
   {
      return aMin;
   }
   return aVal;
}

void RemoveRing(const std::string& aName)
{
   if (!aName.empty())
   {
      wkf::AttachmentRangeRing* ringPtr = vaEnv.FindFirstAttachment<wkf::AttachmentRangeRing>(aName);
      if (ringPtr)
      {
         ringPtr->GetParent().RemoveAttachment(ringPtr->GetUniqueId());
      }
   }
};

QString GetTab()
{
   size_t      tabSize = wkfEnv.GetPreferenceObject<wizard::EditorPrefObject>()->TabSpace();
   std::string spaces;
   spaces.insert(0, tabSize, 0x20);
   return QString::fromStdString(spaces);
}

std::string ColorToString(const QColor& aColor)
{
   std::stringstream ss;
   ss << aColor.red() << " " << aColor.green() << " " << aColor.blue() << " " << aColor.alpha();
   return ss.str();
};
} // namespace

WizAnnotation::Plugin::Plugin(const QString& aPluginName, const size_t aUniqueId)
   : wizard::PluginT<Annotation::Plugin>(aPluginName, aUniqueId)
{
   mUtCallbacks += wizard::ProxyWatcher::ProxyAvailable.Connect(&Plugin::OnProxyAvailable, this);
   // Point of interest connections
   const wizard::PointOfInterestPropertiesWidget* poiProperties =
      dynamic_cast<wizard::PointOfInterestPropertiesWidget*>(mDisplayInterface->GetPoiPropertiesWidget());
   const wizard::RingPropertiesWidget* ringProperties =
      dynamic_cast<wizard::RingPropertiesWidget*>(mDisplayInterface->GetRingPropertiesWidget());

   connect(poiProperties,
           &wizard::PointOfInterestPropertiesWidget::ChangesAccepted,
           this,
           &WizAnnotation::Plugin::ProcessPoiChanges,
           Qt::DirectConnection);
   connect(
      poiProperties,
      &wizard::PointOfInterestPropertiesWidget::NewPoiAccepted,
      this,
      [&](const std::string& aFileName, const wizard::PointOfInterestPropertiesWidget::PoiChanges& aChanges)
      { InsertCommandIntoFile(aFileName, GetPoiCommand(aChanges)); },
      Qt::DirectConnection);

   connect(ringProperties,
           &wizard::RingPropertiesWidget::ChangesAccepted,
           this,
           &WizAnnotation::Plugin::ProcessRingChanges,
           Qt::DirectConnection);
   connect(
      ringProperties,
      &wizard::RingPropertiesWidget::NewRingAccepted,
      this,
      [&](const std::string& aFileName, const wizard::RingPropertiesWidget::RingChanges& aChanges)
      { InsertCommandIntoFile(aFileName, GetRingCommand(aChanges), aChanges.entityName); },
      Qt::DirectConnection);
   connect(
      ringProperties,
      &wizard::RingPropertiesWidget::NewPoiRingAccepted,
      this,
      [&](const std::string&                               aDestFile,
          const wizard::RingPropertiesWidget::RingChanges& aChanges,
          const std::string&                               aName,
          vespa::VaPosition&                               aPos)
      {
         wizard::PointOfInterestPropertiesWidget::PoiChanges poiChanges;
         poiChanges.info.mName = aName;
         poiChanges.info.mLat  = aPos.GetLat();
         poiChanges.info.mLon  = aPos.GetLon();
         poiChanges.info.mAlt  = aPos.GetAlt();
         InsertCommandIntoFile(aDestFile, GetPoiCommand(poiChanges) + GetRingCommand(aChanges));
      },
      Qt::DirectConnection);

   connect(wizSignals, &wizard::Signals::WsfExeChanged, this, &Plugin::DisableFeatures);
}

void WizAnnotation::Plugin::BuildViewerContextMenu(QMenu* aMenu, vespa::VaViewer* aViewerPtr)
{
   if (aViewerPtr == vaEnv.GetStandardViewer() && mEnableFeatures)
   {
      auto addMenu = aMenu->findChild<QMenu*>("Add at Location");
      if (!addMenu)
      {
         addMenu = aMenu->addMenu("Add at Location");
         addMenu->setObjectName("Add at Location");
         UtQtTranslucentMenu(*addMenu);
      }

      vespa::VaPosition p = vaEnv.GetCurrentCursorPosition();
      auto bullseye       = ut::qt::make_qt_ptr<QAction>(QIcon::fromTheme("bullseye"), QString("Bullseye"), addMenu);
      addMenu->addAction(bullseye);
      auto pointOfInterest = ut::qt::make_qt_ptr<QAction>(QIcon::fromTheme("poi"), QString("Point of Interest"), addMenu);
      addMenu->addAction(pointOfInterest);
      auto rangeRing = ut::qt::make_qt_ptr<QAction>(QIcon::fromTheme("rangering"), QString("Range Ring"), addMenu);
      addMenu->addAction(rangeRing);

      connect(rangeRing, &QAction::triggered, this, [=]() { mDisplayInterface->AddRangeRing(p.GetLat(), p.GetLon()); });
      connect(bullseye,
              &QAction::triggered,
              this,
              [=]()
              {
                 auto bullseye = mDisplayInterface->AddPOI<wkf::Bullseye>("bullseye", p.GetLat(), p.GetLon());
                 mDisplayInterface->ShowPOIProperties(bullseye, true);
              });
      connect(pointOfInterest,
              &QAction::triggered,
              this,
              [=]()
              {
                 auto poi = mDisplayInterface->AddPOI<wkf::PointOfInterest>("pointOfInterest", p.GetLat(), p.GetLon(), "");
                 mDisplayInterface->ShowPOIProperties(poi, true);
              });
   }
}

QList<wkf::Action*> WizAnnotation::Plugin::GetActions() const
{
   mDisplayInterface->GetBullseyeAction()->disconnect();
   return Annotation::Plugin::GetActions();
}

void WizAnnotation::Plugin::OnProxyAvailable(WsfPProxy* aProxyPtr)
{
   mAnnotationCallbacks.Clear();
   mVaCallbacks.Clear();
   if (mEnableFeatures)
   {
      mProxyRoot            = WsfPM_Root(aProxyPtr);
      WsfPProxyPath decPath = (mProxyRoot + "decorations").GetPath();
      mAnnotationCallbacks.Add(wizard::WatchProxy(decPath).AnyUpdate.Connect(&Plugin::DecorationChange, this));
      WsfPProxyPath poiPath = (mProxyRoot + "pois").GetPath();
      mAnnotationCallbacks.Add(wizard::WatchProxy(poiPath).AnyUpdate.Connect(&Plugin::PoiChange, this));
      WsfPProxyPath ringPath = (mProxyRoot + "rangeRings").GetPath();
      mAnnotationCallbacks.Add(wizard::WatchProxy(ringPath).AnyUpdate.Connect(&Plugin::RingChange, this));

      WsfPM_ObjectMap poiMap(mProxyRoot + "pois");
      WsfPM_ObjectMap decMap(mProxyRoot + "decorations");
      WsfPM_ObjectMap rrMap(mProxyRoot + "rangeRings");
      ParseAnnotationNode(poiMap, &Plugin::ParsePoiNode);
      ParseAnnotationNode(decMap, &Plugin::ParseDecorationNode);
      ParseAnnotationNode(rrMap, &Plugin::ParseRangeRingNode);

      // If attachments are deleted from the map due to their parent (platform or entity) being deleted, they will not
      // be removed from the text editor.
      mVaCallbacks.Add(vespa::VaObserver::EntityAdded.Connect(&Plugin::EntityAddedCB, this));
   }
}

void WizAnnotation::Plugin::ParseAnnotationNode(WsfPM_ObjectMap& aMap, ParserFunc aFunc)
{
   if (aMap.IsValid())
   {
      std::vector<WsfPProxyNode> nodes;
      aMap.GetAllValues(nodes);
      for (auto& node : nodes)
      {
         (this->*aFunc)(node);
      }
   }
}

void WizAnnotation::Plugin::ParseDecorationNode(const WsfPProxyNode& aDecNode)
{
   if (cOMIT_TYPE_SET.count(aDecNode.GetAttrName()) == 0)
   {
      std::string  entityName = aDecNode.GetName();
      wkf::Entity* ent        = dynamic_cast<wkf::Entity*>(vaEnv.FindEntity(entityName));
      if (ent)
      {
         auto RemoveDec = [&]()
         {
            auto* decorator = ent->FindFirstAttachmentOfType<Annotation::AnnotationDecorator>();
            if (decorator)
            {
               ent->RemoveAttachment(decorator->GetUniqueId());
            }
         };
         WsfPProxyValue val     = aDecNode.GetValue().GetAttr("type");
         std::string    decType = val.ValueToString();
         if (decType == "text")
         {
            val                 = aDecNode.GetValue().GetAttr("text");
            std::string decText = val.ValueToString();
            // Technically this won't match with Mystic/Wk, but single quotes and strings that start with a quote but
            // don't end with one are marked invalid anyway
            if (!decText.empty() && decText != "\"" && !(decText[0] == '\"' && decText[decText.size() - 1] != '\"'))
            {
               RemoveQuotesFromQuotableString(decText);
               mDisplayInterface->AddDecoration(*ent, QString::fromStdString(decText), "");
            }
            else
            {
               RemoveDec();
            }
         }
         else if (decType == "preset")
         {
            val             = aDecNode.GetValue().GetAttr("preset");
            QString decPath = mDisplayInterface->DecorationPresetToPath(val.ValueToString());
            if (!decPath.isEmpty())
            {
               mDisplayInterface->AddDecoration(*ent, "", decPath);
            }
            else
            {
               RemoveDec();
            }
         }
         else if (decType == "image")
         {
            val                  = aDecNode.GetValue().GetAttr("path");
            std::string tempPath = val.ValueToString();
            RemoveQuotesFromQuotableString(tempPath);
            QString decPath = QString::fromStdString(tempPath);
            mDisplayInterface->Decorate(QString::fromStdString(entityName), QIcon(decPath), decPath);
         }
         else // BadValue (meaning no other types are valid either)
         {
            RemoveDec();
         }
      }
   }
}

void WizAnnotation::Plugin::ParsePoiNode(const WsfPProxyNode& aPoiNode)
{
   if (cOMIT_TYPE_SET.count(aPoiNode.GetAttrName()) == 0)
   {
      std::pair<UtLatPos, UtLonPos> latLon   = GetProxyNodeLatLon(aPoiNode);
      bool                          bullseye = GetProxyNodeBool(aPoiNode, "bullseye");
      if (!bullseye)
      {
         mDisplayInterface->AddDetailedPOI<wkf::PointOfInterest>("pointOfInterest",
                                                                 latLon.first,
                                                                 latLon.second,
                                                                 GetProxyNodeAltitude(aPoiNode),
                                                                 aPoiNode.GetName(),
                                                                 false);
      }
      else
      {
         mDisplayInterface->AddDetailedPOI<wkf::Bullseye>("bullseye", latLon.first, latLon.second, 0.0, aPoiNode.GetName(), false);
      }
   }
}

void WizAnnotation::Plugin::ParseRangeRingNode(const WsfPProxyNode& aRingNode)
{
   std::string  entityName = GetProxyNodeString(aRingNode, "entityName");
   wkf::Entity* ent        = dynamic_cast<wkf::Entity*>(vaEnv.FindEntity(entityName));
   if (ent && cOMIT_TYPE_SET.count(aRingNode.GetAttrName()) == 0)
   {
      wkf::AttachmentRangeRing::Properties info;
      info.mName                 = QString::fromStdString(aRingNode.GetAttrName());
      info.mShowRangeLabels      = GetProxyNodeBool(aRingNode, "showRangeLabels");
      info.mRangeLabelColor      = GetProxyNodeColor(aRingNode, "rangeLabelColor");
      info.mRingColor            = GetProxyNodeColor(aRingNode, "ringColor");
      info.mRingLineWidth        = GetProxyNodeInt(aRingNode, "ringWidth");
      info.mBaseRadius           = GetProxyNodeLength(aRingNode, "centerRadius");
      info.mAdditionalRings      = GetProxyNodeInt(aRingNode, "additionalRings");
      info.mAdditionalRingRadius = GetProxyNodeLength(aRingNode, "additionalRadii");
      info.mShowAngleLabels      = GetProxyNodeBool(aRingNode, "showAngleLabels");
      info.mAngleLabelColor      = GetProxyNodeColor(aRingNode, "angleLabelColor");
      info.mRadialColor          = GetProxyNodeColor(aRingNode, "radialColor");
      info.mRadialLineWidth      = GetProxyNodeInt(aRingNode, "radialWidth");
      info.mCenterAngle          = GetProxyNodeAngle(aRingNode, "centerAngle");
      info.mWidth                = GetProxyNodeAngle(aRingNode, "angleWidth");
      info.mRadialRate           = GetProxyNodeAngle(aRingNode, "radialRate");
      info.mAlignWithPlatform    = !GetProxyNodeBool(aRingNode, "alignNorth");

      if (ent->IsA_TypeOf<wkf::Platform>())
      {
         wkf::Platform* plat = dynamic_cast<wkf::Platform*>(ent);
         mDisplayInterface->AddRangeRing(*plat, info);
      }
      else if (ent->IsA_TypeOf<wkf::PointOfInterest>())
      {
         wkf::PointOfInterest* poi = dynamic_cast<wkf::PointOfInterest*>(ent);
         mDisplayInterface->AddRangeRing(poi, info);
      }
   }
}

void WizAnnotation::Plugin::DecorationChange(const wizard::ProxyChange& aProxyChange)
{
   WsfPProxyNode node = aProxyChange.changed();
   // node cannot be checked for validity, because a "removed" reason is "invalid"
   if (aProxyChange.reason() == wizard::ProxyChange::cUPDATED)
   {
      auto nodeList = node.GetPath().ToStringList(mProxyRoot.GetValue());
      if (nodeList.size() >= 3)
      {
         std::string  entityName = nodeList[1];
         wkf::Entity* ent        = dynamic_cast<wkf::Entity*>(vaEnv.FindEntity(entityName));
         if (ent)
         {
            ParseDecorationNode(WsfPProxyNode(mProxyRoot + nodeList[0] + entityName));
         }
      }
   }
   else if (aProxyChange.reason() == wizard::ProxyChange::cADDED)
   {
      auto nodeList = node.GetPath().ToStringList(mProxyRoot.GetValue());
      ParseDecorationNode(node);
      std::string    entityName = GetProxyNodeString(node, "entityName");
      wkf::Platform* ent        = dynamic_cast<wkf::Platform*>(vaEnv.FindEntity(entityName));
      if (ent)
      {
         emit mDisplayInterface->SetDecorationOptionState(true, ent);
      }
   }
   else if (aProxyChange.reason() == wizard::ProxyChange::cREMOVED)
   {
      std::string entityName = node.GetAttrName();
      if (!entityName.empty())
      {
         wkf::Entity* ent = dynamic_cast<wkf::Entity*>(vaEnv.FindEntity(entityName));
         if (ent)
         {
            auto* decorator = ent->FindFirstAttachmentOfType<Annotation::AnnotationDecorator>();
            if (decorator)
            {
               ent->RemoveAttachment(decorator->GetUniqueId());
            }
         }
      }
   }
}

void WizAnnotation::Plugin::PoiChange(const wizard::ProxyChange& aProxyChange)
{
   auto AssignInfoValues = [&](wkf::PointOfInterest* aPoiPtr, Annotation::PointOfInterestPropertiesWidget::PoiInfo& aInfo)
   {
      aInfo.mName = aPoiPtr->GetName();
      aInfo.mLat  = aPoiPtr->GetPosition().GetLat();
      aInfo.mLon  = aPoiPtr->GetPosition().GetLon();
      aInfo.mAlt  = aPoiPtr->GetPosition().GetAlt();
   };

   WsfPProxyNode node = aProxyChange.changed();
   // Renaming the poi first removes it, then re-adds it
   if (aProxyChange.reason() == wizard::ProxyChange::cUPDATED)
   {
      std::string commandName = node.GetAttrName();
      auto        nodeList    = node.GetPath().ToStringList(mProxyRoot.GetValue());
      if (node.IsValid() && nodeList.size() >= 3)
      {
         std::string poiName = nodeList[1];
         auto        poiPtr  = vaEnv.FindEntity<wkf::PointOfInterest>(poiName);
         if (poiPtr)
         {
            Annotation::PointOfInterestPropertiesWidget::PoiInfo info;
            AssignInfoValues(poiPtr, info);
            WsfPProxyNode poiNode = mProxyRoot + nodeList[0] + nodeList[1];
            if (commandName == "position")
            {
               std::pair<UtLatPos, UtLonPos> latLon = GetProxyNodeLatLon(poiNode);
               vespa::VaPosition             curPos = poiPtr->GetPosition();
               curPos.SetLat(latLon.first);
               curPos.SetLon(latLon.second);
               poiPtr->SetPosition(curPos);
               info.mLat = latLon.first;
               info.mLon = latLon.second;
            }
            else if (commandName == "altitude")
            {
               // Altitude should always be visually 0 if using a bullseye, regardless of the input
               UtLengthValue     alt    = (!poiPtr->IsA_TypeOf<wkf::Bullseye>()) ? GetProxyNodeAltitude(poiNode) :
                                                                                   static_cast<UtLengthValue>(0.0);
               vespa::VaPosition curPos = poiPtr->GetPosition();
               curPos.SetAlt(alt);
               poiPtr->SetPosition(curPos);
               info.mAlt = alt;
            }
            else if (commandName == "bullseye")
            {
               vaEnv.DeleteEntity(poiPtr);
               ParsePoiNode(poiNode);
               EntityAddedHandler(nodeList[1]);
               // poi needs to be re-acquired because the original one was destroyed
               poiPtr         = vaEnv.FindEntity<wkf::PointOfInterest>(poiName);
               info.mBullseye = GetProxyNodeBool(poiNode, "bullseye");
            }
            auto poiProperties = mDisplayInterface->GetPoiPropertiesWidget();
            if (poiProperties && poiProperties->isVisible() && poiProperties->GetEntity() &&
                poiProperties->GetEntity()->GetName() == poiPtr->GetName())
            {
               // If editing text while dialog is open, canceling dialog will revert to whatever is in the text
               poiProperties->OverwriteOriginalValues(info);
               poiProperties->SetEntity(poiPtr, false);
            }
         }
      }
   }
   else if (aProxyChange.reason() == wizard::ProxyChange::cADDED)
   {
      auto nodeList = node.GetPath().ToStringList(mProxyRoot.GetValue());
      auto poiProperties =
         dynamic_cast<wizard::PointOfInterestPropertiesWidget*>(mDisplayInterface->GetPoiPropertiesWidget());
      if (poiProperties && poiProperties->isVisible())
      {
         auto poiPtr = vaEnv.FindEntity<wkf::PointOfInterest>(nodeList[1]);
         if ((poiPtr && poiProperties->GetEntityUiName() == poiPtr->GetName()) ||
             (poiProperties->GetOriginalName() == nodeList[1]))
         {
            // The user really shouldn't be adding or renaming POIs to be the same name as something that is actively
            // being edited via the dialog This will close the dialog when the dialog's name or original name matches
            // something in the editor
            poiProperties->close();
         }
      }
      ParsePoiNode(node);
      EntityAddedHandler(nodeList[1]);
   }
   else if (aProxyChange.reason() == wizard::ProxyChange::cREMOVED)
   {
      std::string poiName = node.GetAttrName();
      if (!poiName.empty())
      {
         wkf::PointOfInterest* poiPtr = vaEnv.FindEntity<wkf::PointOfInterest>(poiName);
         if (poiPtr)
         {
            auto poiProperties = mDisplayInterface->GetPoiPropertiesWidget();
            if (poiProperties && poiProperties->isVisible() && poiProperties->GetEntity() == poiPtr)
            {
               poiProperties->SetEntity(nullptr, false);
               poiProperties->close();
            }
            vaEnv.DeleteEntity(poiPtr);
         }
      }
   }
}

void WizAnnotation::Plugin::RingChange(const wizard::ProxyChange& aProxyChange)
{
   WsfPProxyNode node = aProxyChange.changed();
   if (aProxyChange.reason() == wizard::ProxyChange::cUPDATED)
   {
      std::string commandName = node.GetAttrName();
      auto        nodeList    = node.GetPath().ToStringList(mProxyRoot.GetValue());
      if (node.IsValid() && nodeList.size() >= 3)
      {
         std::string               ringName = nodeList[1];
         wkf::AttachmentRangeRing* ringPtr  = vaEnv.FindFirstAttachment<wkf::AttachmentRangeRing>(ringName);
         WsfPProxyNode             ringNode = mProxyRoot + nodeList[0] + nodeList[1];
         if (commandName == "entityName")
         {
            wkf::AttachmentRangeRing::Properties properties;
            if (ringPtr)
            {
               properties = ringPtr->GetProperties();
               ringPtr->GetParent().RemoveAttachment(ringPtr->GetUniqueId());
               std::string           entName = GetProxyNodeString(ringNode, "entityName");
               wkf::Platform*        plat    = dynamic_cast<wkf::Platform*>(vaEnv.FindEntity(entName));
               wkf::PointOfInterest* poi     = vaEnv.FindEntity<wkf::PointOfInterest>(entName);
               if (plat)
               {
                  mDisplayInterface->AddRangeRing(*plat, properties);
               }
               else if (poi)
               {
                  mDisplayInterface->AddRangeRing(poi, properties);
               }
            }
            else
            {
               ParseRangeRingNode(ringNode);
            }
         }
         if (ringPtr)
         {
            // Because color is stored as a struct, the changes may be reflected in each internal variable as opposed to
            // the struct as whole
            auto CheckColorStruct = [commandName, nodeList](const std::string& aColorType)
            {
               return (commandName == "red" || commandName == "green" || commandName == "blue" || commandName == "alpha") &&
                      nodeList[2] == aColorType;
            };
            if (commandName == "showRangeLabels")
            {
               ringPtr->SetRangeLabelsEnabled(GetProxyNodeBool(ringNode, "showRangeLabels"));
            }
            else if (commandName == "rangeLabelColor" || CheckColorStruct("rangeLabelColor"))
            {
               ringPtr->SetRangeLabelColor(GetProxyNodeColor(ringNode, "rangeLabelColor"));
            }
            else if (commandName == "ringColor" || CheckColorStruct("ringColor"))
            {
               ringPtr->SetRingColor(GetProxyNodeColor(ringNode, "ringColor"));
            }
            else if (commandName == "ringWidth")
            {
               ringPtr->SetRingLineWidth(BoundValue<int>(GetProxyNodeInt(ringNode, "ringWidth"), 1, 5));
            }
            else if (commandName == "centerRadius")
            {
               ringPtr->SetCenterRadius(GetProxyNodeLength(ringNode, "centerRadius"));
            }
            else if (commandName == "additionalRings")
            {
               ringPtr->SetAdditionalRings((unsigned int)BoundValue<int>(GetProxyNodeInt(ringNode, "additionalRings"), 0, 20),
                                           ringPtr->GetAdditionalRadii());
            }
            else if (commandName == "additionalRadii")
            {
               ringPtr->SetAdditionalRings(ringPtr->GetNumberAdditionalRings(),
                                           GetProxyNodeLength(ringNode, "additionalRadii"));
            }
            else if (commandName == "showAngleLabels")
            {
               ringPtr->SetAngleLabelsEnabled(GetProxyNodeBool(ringNode, "showAngleLabels"));
            }
            else if (commandName == "angleLabelColor" || CheckColorStruct("angleLabelColor"))
            {
               ringPtr->SetAngleLabelColor(GetProxyNodeColor(ringNode, "angleLabelColor"));
            }
            else if (commandName == "radialColor" || CheckColorStruct("radialColor"))
            {
               ringPtr->SetRadialColor(GetProxyNodeColor(ringNode, "radialColor"));
            }
            else if (commandName == "radialWidth")
            {
               ringPtr->SetRadialLineWidth(BoundValue<int>(GetProxyNodeInt(ringNode, "radialWidth"), 1, 5));
            }
            else if (commandName == "centerAngle")
            {
               ringPtr->SetCenterAngle(BoundValue<double>(GetProxyNodeAngle(ringNode, "centerAngle"), 0, UtMath::cTWO_PI));
            }
            else if (commandName == "angleWidth")
            {
               ringPtr->SetAngleWidth(BoundValue<double>(GetProxyNodeAngle(ringNode, "angleWidth"), 0, UtMath::cTWO_PI));
            }
            else if (commandName == "radialRate")
            {
               ringPtr->SetRadialRate(BoundValue<double>(GetProxyNodeAngle(ringNode, "radialRate"), 0, UtMath::cTWO_PI));
            }
            else if (commandName == "alignNorth")
            {
               ringPtr->SetAlignWithPlatformHeading(!GetProxyNodeBool(ringNode, "alignNorth"));
            }

            // Account for text editor changing a range ring while the ring dialog is open to the relevant ring
            wizard::RingPropertiesWidget* ringWidget =
               dynamic_cast<wizard::RingPropertiesWidget*>(mDisplayInterface->GetRingPropertiesWidget());
            if (ringWidget && ringWidget->isVisible() && ringWidget->GetRing() &&
                ringWidget->GetRing()->GetName() == ringName)
            {
               std::string entName      = GetProxyNodeString(ringNode, "entityName");
               int         currentIndex = ringWidget->GetCurrentNameIndex();
               auto        entityPtr    = vaEnv.FindEntity(entName);
               if (entityPtr)
               {
                  ringWidget->OverwriteOriginalValues(ringPtr->GetProperties());
                  ringWidget->SetEntity(entityPtr, Annotation::RingPropertiesWidget::RingFlags(false, true));
                  ringWidget->SetCurrentNameIndex(currentIndex);
               }
            }
         }
      }
   }
   else if (aProxyChange.reason() == wizard::ProxyChange::cADDED)
   {
      // Account for user adding or renaming a range ring to be the same name as the one that is being edited (the original name)
      auto                          nodeList = node.GetPath().ToStringList(mProxyRoot.GetValue());
      std::string                   ringName = nodeList[1];
      wizard::RingPropertiesWidget* ringWidget =
         dynamic_cast<wizard::RingPropertiesWidget*>(mDisplayInterface->GetRingPropertiesWidget());
      if (ringWidget && ringWidget->isVisible() && ringWidget->GetCurrentNameIndex() >= 0 && ringWidget->GetRing() &&
          ringWidget->GetRing()->GetName() == ringName)
      {
         ringWidget->close();
      }
      RemoveRing(node.GetAttrName());
      ParseRangeRingNode(node);

      std::string    entityName = GetProxyNodeString(node, "entityName");
      wkf::Platform* ent        = dynamic_cast<wkf::Platform*>(vaEnv.FindEntity(entityName));
      if (ent)
      {
         emit mDisplayInterface->SetRangeRingOptionState(true, ent);
      }
   }
   else if (aProxyChange.reason() == wizard::ProxyChange::cREMOVED)
   {
      RemoveRing(node.GetAttrName());
   }
}


void WizAnnotation::Plugin::DecorateHandler(const QString& aName, const QString& aType)
{
   vespa::VaEntity* ent = vaEnv.FindEntity(aName.toStdString());
   if (ent)
   {
      auto* decPtr = ent->FindFirstAttachmentOfType<Annotation::AnnotationDecorator>();
      if (decPtr)
      {
         QString text      = "";
         QString preset    = "";
         QString imagePath = "";
         if (aType == "text")
         {
            text = "\"" + decPtr->GetText() + "\"";
         }
         else if (aType == "preset")
         {
            QString path = decPtr->GetImageDescription();
            if (mDisplayInterface->DecorationPathToPreset(path) != "")
            {
               preset = mDisplayInterface->DecorationPathToPreset(path);
            }
         }
         else // if aType == "image"
         {
            imagePath = "\"" + decPtr->GetImageDescription() + "\"";
         }
         WsfPProxyNode decNode = mProxyRoot + "decorations" + aName.toStdString();
         if (decNode.IsValid() && decNode.GetAttrName() == aName.toStdString())
         {
            std::multiset<AnnotationWrapper> sortedChanges;
            AddToSortedChanges(sortedChanges, GetTextReplacementCommands(decNode, "text", "text", text));
            AddToSortedChanges(sortedChanges, GetTextReplacementCommands(decNode, "image", "image", imagePath));
            AddToSortedChanges(sortedChanges, GetTextReplacementCommands(decNode, "preset", "preset", preset));
            for (auto changeIt : sortedChanges)
            {
               changeIt.Process();
            }
         }
         else
         {
            QString tab = GetTab();
            QString decLine;
            if (aType == "text")
            {
               decLine = (tab + tab + "text " + text + "\n");
            }
            else if (aType == "preset")
            {
               decLine = (tab + tab + "preset " + preset + "\n");
            }
            else // if aType == "image"
            {
               decLine = (tab + tab + "image " + imagePath + "\n");
            }
            if (ent->IsA_TypeOf<wkf::Platform>())
            {
               WsfPProxyNode platNode = mProxyRoot + "platform" + aName.toStdString();
               if (platNode.IsValid() && platNode.GetAttrName() == aName.toStdString())
               {
                  wizard::Editor* editorPtr = GetEditorAtRange(platNode);
                  QString         newCommand;
                  newCommand += "\n\nvisual_elements\n";
                  newCommand += (tab + "decoration " + aName + "\n");
                  newCommand += decLine;
                  newCommand += (tab + "end_decoration\n");
                  newCommand += "end_visual_elements";
                  editorPtr->insertPlainText(newCommand);
               }
            }
            else if (ent->IsA_TypeOf<wkf::PointOfInterest>())
            {
               WsfPProxyNode poiNode = mProxyRoot + "pois" + aName.toStdString();
               if (poiNode.IsValid() && poiNode.GetAttrName() == aName.toStdString())
               {
                  wizard::Editor* editorPtr = GetEditorAtRange(poiNode);
                  QString         newCommand;
                  newCommand += ("\n" + tab + "decoration " + aName + "\n");
                  newCommand += decLine;
                  newCommand += (tab + "end_decoration");
                  editorPtr->insertPlainText(newCommand);
               }
            }
         }
      }
      // If no decoration on the map, just remove the decoration entirely
      else
      {
         WsfPProxyNode decNode = mProxyRoot + "decorations" + aName.toStdString();
         if (decNode.IsValid() && decNode.GetAttrName() == aName.toStdString())
         {
            wizard::Editor* editorPtr = GetEditorAtRange(decNode);
            editorPtr->Replace(decNode.GetSourceDocumentRange(), "");
         }
      }
   }
}

// Technically these will be re-drawn when the proxy gets changed, but there is no functional difference for the user
void WizAnnotation::Plugin::DecoratePreset(const QString& aName, const QString& aPath)
{
   Annotation::Plugin::DecoratePreset(aName, aPath);
   DecorateHandler(aName, "preset");
}

bool WizAnnotation::Plugin::DecorateText(const QString& aName)
{
   bool changed = Annotation::Plugin::DecorateText(aName);
   if (changed)
   {
      DecorateHandler(aName, "text");
   }
   return changed;
}

bool WizAnnotation::Plugin::DecorateImage(const QString& aName)
{
   bool changed = Annotation::Plugin::DecorateImage(aName);
   if (changed)
   {
      DecorateHandler(aName, "image");
   }
   return changed;
}

void WizAnnotation::Plugin::ClearDecoration(const QString& aName)
{
   WsfPProxyNode decNode = mProxyRoot + "decorations" + aName.toStdString();
   if (decNode.IsValid() && decNode.GetAttrName() == aName.toStdString())
   {
      UtTextDocumentRange range = decNode.GetSourceDocumentRange();
      wizard::Editor* editorPtr = wizard::Project::Instance()->GotoFile(range.mSource->GetFilePath().GetNormalizedPath());
      editorPtr->Replace(range, "");
   }
}

void WizAnnotation::Plugin::ClearRangeRing(wkf::Entity* aEntityPtr, unsigned int aUniqueId)
{
   wkf::AttachmentRangeRing* rangeRing = dynamic_cast<wkf::AttachmentRangeRing*>(vaEnv.FindAttachment(aUniqueId));
   if (rangeRing)
   {
      WsfPM_ObjectMap ringMap  = mProxyRoot + "rangeRings";
      WsfPProxyNode   ringNode = ringMap.Find(rangeRing->GetName());
      if (ringNode.IsValid())
      {
         UtTextDocumentRange range = ringNode.GetSourceDocumentRange();
         wizard::Editor*     editorPtr =
            wizard::Project::Instance()->GotoFile(range.mSource->GetFilePath().GetNormalizedPath());
         editorPtr->Replace(range, "");
      }
   }
}

void WizAnnotation::Plugin::EntityAddedCB(vespa::VaEntity* aEntity)
{
   // pois are handled elsewhere because they are created/added, then assigned a name at a later time.
   if (!(aEntity->IsA_TypeOf<wkf::PointOfInterest>()))
   {
      EntityAddedHandler(aEntity->GetName());
   }
}

void WizAnnotation::Plugin::EntityAddedHandler(const std::string& aName)
{
   // Remove all rings and decorations on the entity (they will get re-added when necessary anyway)
   wkf::Entity* ent = dynamic_cast<wkf::Entity*>(vaEnv.FindEntity(aName));
   if (ent)
   {
      vespa::VaAttachment* attachment = ent->FindAttachment("decorator");
      if (attachment)
      {
         Annotation::Plugin::ClearDecoration(QString::fromStdString(attachment->GetName()));
      }
      auto rrList = ent->FindAttachmentsOfType<wkf::AttachmentRangeRing>();
      for (const auto& it : rrList)
      {
         ent->RemoveAttachment(it->GetUniqueId());
      }
   }

   WsfPProxyNode decNode = mProxyRoot + "decorations" + aName;
   if (decNode.IsValid() && decNode.GetAttrName() == aName)
   {
      ParseDecorationNode(decNode);
   }
   WsfPM_ObjectMap            rrMap = mProxyRoot + "rangeRings";
   std::vector<WsfPProxyNode> nodes;
   rrMap.GetAllValues(nodes);
   for (const auto& it : nodes)
   {
      std::string entityName = GetProxyNodeString(it, "entityName");
      if (entityName == aName)
      {
         RemoveRing(it.GetAttrName());
         ParseRangeRingNode(it);
      }
   }
}

void WizAnnotation::Plugin::InsertCommandIntoFile(const std::string& aFileName,
                                                  const QString&     aNewCommand,
                                                  const std::string& aEntityName)
{
   // If the command is attached to an entity, and the entity is in the specified file, stick the command immediately
   // following the entity
   vespa::VaEntity* ent = vaEnv.FindEntity(aEntityName);
   if (!aEntityName.empty() && ent)
   {
      // If there was already a ring attached to the platform, don't create a new visual_elements block
      if (ent->IsA_TypeOf<wkf::Platform>())
      {
         WsfPProxyNode platNode = mProxyRoot + "platform" + aEntityName;
         if (platNode.IsValid() && platNode.GetSourceDocumentRange().mSource->GetFilePath().GetNormalizedPath() == aFileName)
         {
            WsfPM_ObjectMap            rrMap = mProxyRoot + "rangeRings";
            std::vector<WsfPProxyNode> nodes;
            rrMap.GetAllValues(nodes);
            for (const auto& it : nodes)
            {
               std::string entityName = GetProxyNodeString(it, "entityName");
               if (entityName == aEntityName)
               {
                  wizard::Editor* editorPtr = GetEditorAtRange(it);
                  editorPtr->insertPlainText(aNewCommand);
                  return;
               }
            }
            QString         cmd       = "\nvisual_elements" + aNewCommand + "\nend_visual_elements";
            wizard::Editor* editorPtr = GetEditorAtRange(platNode);
            editorPtr->insertPlainText(cmd);
            return;
         }
      }
      else if (ent->IsA_TypeOf<wkf::PointOfInterest>())
      {
         WsfPProxyNode poiNode = mProxyRoot + "pois" + aEntityName;
         if (poiNode.IsValid() && poiNode.GetSourceDocumentRange().mSource->GetFilePath().GetNormalizedPath() == aFileName)
         {
            wizard::Editor* editorPtr = GetEditorAtRange(poiNode);
            editorPtr->insertPlainText(aNewCommand);
            return;
         }
      }
   }

   // Find an existing visual element block in the file, when possible
   auto FindVisualElement = [aFileName](WsfPM_ObjectMap& aMap, UtTextDocumentRange& aRange)
   {
      std::vector<WsfPProxyNode> nodes;
      aMap.GetAllValues(nodes);
      for (const auto& it : nodes)
      {
         if (cOMIT_TYPE_SET.find(it.GetAttrName()) == cOMIT_TYPE_SET.end())
         {
            aRange = it.GetSourceDocumentRange();
            if (aRange.mSource->GetFilePath().GetNormalizedPath() == aFileName)
            {
               return true;
            }
         }
      }
      return false;
   };

   std::vector<WsfPM_ObjectMap> annotationMapList;
   annotationMapList.emplace_back(mProxyRoot + "pois");
   annotationMapList.emplace_back(mProxyRoot + "decorations");
   annotationMapList.emplace_back(mProxyRoot + "rangeRings");
   bool                insertInBlock = false;
   UtTextDocumentRange insertRange;
   for (auto& it : annotationMapList)
   {
      insertInBlock = FindVisualElement(it, insertRange);
      // don't care about the position within the visual element block, so just break as soon as a block is found
      if (insertInBlock)
      {
         break;
      }
   }
   if (insertInBlock)
   {
      insertRange.SetBegin(insertRange.GetEnd() + 1);
      insertRange.SetEnd(insertRange.GetEnd() + 1);
      wizard::Editor* editorPtr =
         wizard::Project::Instance()->GotoFile(insertRange.mSource->GetFilePath().GetNormalizedPath());
      editorPtr->GoToRange(insertRange);
      editorPtr->insertPlainText(aNewCommand);
   }
   else
   {
      wizard::Editor* editorPtr = wizard::Project::Instance()->GotoFile(aFileName);
      UtTextDocument  textDoc;
      if (textDoc.ReadFile(UtPath(aFileName)))
      {
         insertRange.SetBegin(textDoc.GetText().Size() - 1);
         insertRange.SetEnd(textDoc.GetText().Size() - 1);
         editorPtr->GoToRange(insertRange);
         QString newCommand = "\nvisual_elements" + aNewCommand + "\nend_visual_elements";
         editorPtr->insertPlainText(newCommand);
      }
   }
}

QString WizAnnotation::Plugin::GetPoiCommand(const wizard::PointOfInterestPropertiesWidget::PoiChanges& aChanges)
{
   QString           tab = GetTab();
   std::stringstream posStream;
   posStream << aChanges.info.mLat << " " << aChanges.info.mLon;
   QString newCommand = "\n" + tab + "poi " + QString::fromStdString(aChanges.info.mName);
   newCommand += "\n" + tab + tab + "position " + QString::fromStdString(posStream.str());
   if (aChanges.info.mBullseye)
   {
      newCommand += "\n" + tab + tab + "bullseye";
   }
   // altitude should be omitted on newly created bullseyes
   else if (!aChanges.altString.isEmpty())
   {
      newCommand += "\n" + tab + tab + "altitude " + aChanges.altString;
   }
   newCommand += "\n" + tab + "end_poi";
   return newCommand;
}

void WizAnnotation::Plugin::ProcessPoiChanges(const wizard::PointOfInterestPropertiesWidget::PoiChanges& aChanges)
{
   // multiset used because insertion will always use the same ranges
   std::multiset<AnnotationWrapper> sortedChanges;
   WsfPProxyNode                    poiNode = mProxyRoot + "pois" + aChanges.originalEntityName;
   if (aChanges.positionChanged)
   {
      std::stringstream posStream;
      posStream << aChanges.info.mLat << " " << aChanges.info.mLon;
      AddToSortedChanges(sortedChanges, GetTextReplacementCommands(poiNode, "altitude", "altitude", aChanges.altString));
      AddToSortedChanges(sortedChanges,
                         GetTextReplacementCommands(poiNode, "position", "position", QString::fromStdString(posStream.str())));
   }
   if (aChanges.bullseyeChanged)
   {
      AddToSortedChanges(sortedChanges,
                         GetSingleTextReplacementCommands(poiNode, "bullseye", "bullseye", aChanges.info.mBullseye));
   }
   for (auto changeIt : sortedChanges)
   {
      changeIt.Process();
   }
   if (aChanges.nameChanged)
   {
      ReplaceNameInNode(poiNode, "poi", QString::fromStdString(aChanges.info.mName));
   }
}

QString WizAnnotation::Plugin::GetRingCommand(const wizard::RingPropertiesWidget::RingChanges& aChanges)
{
   std::string tab     = GetTab().toStdString();
   auto        TabLine = [tab](const std::string& aLine) { return (tab + tab + aLine); };
   QString     command = "\n" + QString::fromStdString(tab) + "range_ring " + aChanges.properties.mName;
   AddLineToCommand<std::string>(command, true, TabLine("entity"), aChanges.entityName);
   AddLineToCommand<std::string>(command, aChanges.properties.mShowRangeLabels, TabLine("show_range_labels"), "");
   AddLineToCommand<std::string>(command,
                                 aChanges.rangeLabelColorChanged,
                                 TabLine("range_label_color"),
                                 ColorToString(aChanges.properties.mRangeLabelColor));
   AddLineToCommand<std::string>(command,
                                 aChanges.ringColorChanged,
                                 TabLine("ring_color"),
                                 ColorToString(aChanges.properties.mRingColor));
   AddLineToCommand<int>(command, aChanges.ringLineWidthChanged, TabLine("ring_width"), aChanges.properties.mRingLineWidth);
   AddLineToCommand<std::string>(command,
                                 aChanges.baseRadiusChanged,
                                 TabLine("center_radius"),
                                 aChanges.unitStrings.baseRadius.toStdString());
   AddLineToCommand<int>(command,
                         aChanges.additionalRingsChanged,
                         TabLine("additional_rings"),
                         aChanges.properties.mAdditionalRings);
   AddLineToCommand<std::string>(command,
                                 aChanges.additionalRingRadiusChanged,
                                 TabLine("additional_radii"),
                                 aChanges.unitStrings.additionalRingRadius.toStdString());
   AddLineToCommand<std::string>(command, aChanges.properties.mShowAngleLabels, TabLine("show_angle_labels"), "");
   AddLineToCommand<std::string>(command,
                                 aChanges.angleLabelColorChanged,
                                 TabLine("angle_label_color"),
                                 ColorToString(aChanges.properties.mAngleLabelColor));
   AddLineToCommand<std::string>(command,
                                 aChanges.radialColorChanged,
                                 TabLine("radial_color"),
                                 ColorToString(aChanges.properties.mRadialColor));
   AddLineToCommand<int>(command, aChanges.radialLineWidthChanged, TabLine("radial_width"), aChanges.properties.mRadialLineWidth);
   AddLineToCommand<std::string>(command,
                                 aChanges.centerAngleChanged,
                                 TabLine("center_angle"),
                                 aChanges.unitStrings.centerAngle.toStdString());
   AddLineToCommand<std::string>(command,
                                 aChanges.widthChanged,
                                 TabLine("angle_width"),
                                 aChanges.unitStrings.width.toStdString());
   AddLineToCommand<std::string>(command,
                                 aChanges.radialRateChanged,
                                 TabLine("radial_rate"),
                                 aChanges.unitStrings.radialRate.toStdString());
   AddLineToCommand<std::string>(command, !aChanges.properties.mAlignWithPlatform, TabLine("align_north"), "");
   command += "\n" + QString::fromStdString(tab) + "end_range_ring";
   return command;
}

void WizAnnotation::Plugin::ProcessRingChanges(const wizard::RingPropertiesWidget::RingChanges& aChanges)
{
   // multiset used because insertion will always use the same ranges
   std::multiset<AnnotationWrapper> sortedChanges;
   WsfPProxyNode                    ringNode = mProxyRoot + "rangeRings" + aChanges.originalRingName;
   if (aChanges.showRangeLabelsChanged)
   {
      AddToSortedChanges(sortedChanges,
                         GetSingleTextReplacementCommands(ringNode,
                                                          "showRangeLabels",
                                                          "show_range_labels",
                                                          aChanges.properties.mShowRangeLabels));
   }
   if (aChanges.rangeLabelColorChanged)
   {
      AddToSortedChanges(sortedChanges,
                         GetColorReplacementCommands(ringNode,
                                                     "rangeLabelColor",
                                                     "range_label_color",
                                                     QString::fromStdString(
                                                        ColorToString(aChanges.properties.mRangeLabelColor))));
   }
   if (aChanges.ringColorChanged)
   {
      AddToSortedChanges(sortedChanges,
                         GetColorReplacementCommands(ringNode,
                                                     "ringColor",
                                                     "ring_color",
                                                     QString::fromStdString(ColorToString(aChanges.properties.mRingColor))));
   }
   if (aChanges.ringLineWidthChanged)
   {
      AddToSortedChanges(sortedChanges,
                         GetTextReplacementCommands(ringNode,
                                                    "ringWidth",
                                                    "ring_width",
                                                    QString::fromStdString(
                                                       std::to_string(aChanges.properties.mRingLineWidth))));
   }
   if (aChanges.baseRadiusChanged)
   {
      AddToSortedChanges(sortedChanges,
                         GetTextReplacementCommands(ringNode, "centerRadius", "center_radius", aChanges.unitStrings.baseRadius));
   }
   if (aChanges.additionalRingsChanged)
   {
      AddToSortedChanges(sortedChanges,
                         GetTextReplacementCommands(ringNode,
                                                    "additionalRings",
                                                    "additional_rings",
                                                    QString::fromStdString(
                                                       std::to_string(aChanges.properties.mAdditionalRings))));
   }
   if (aChanges.additionalRingRadiusChanged)
   {
      AddToSortedChanges(sortedChanges,
                         GetTextReplacementCommands(ringNode,
                                                    "additionalRadii",
                                                    "additional_radii",
                                                    aChanges.unitStrings.additionalRingRadius));
   }
   if (aChanges.showAngleLabelsChanged)
   {
      AddToSortedChanges(sortedChanges,
                         GetSingleTextReplacementCommands(ringNode,
                                                          "showAngleLabels",
                                                          "show_angle_labels",
                                                          aChanges.properties.mShowAngleLabels));
   }
   if (aChanges.angleLabelColorChanged)
   {
      AddToSortedChanges(sortedChanges,
                         GetColorReplacementCommands(ringNode,
                                                     "angleLabelColor",
                                                     "angle_label_color",
                                                     QString::fromStdString(
                                                        ColorToString(aChanges.properties.mAngleLabelColor))));
   }
   if (aChanges.radialColorChanged)
   {
      AddToSortedChanges(sortedChanges,
                         GetColorReplacementCommands(ringNode,
                                                     "radialColor",
                                                     "radial_color",
                                                     QString::fromStdString(
                                                        ColorToString(aChanges.properties.mRadialColor))));
   }
   if (aChanges.radialLineWidthChanged)
   {
      AddToSortedChanges(sortedChanges,
                         GetTextReplacementCommands(ringNode,
                                                    "radialWidth",
                                                    "radial_width",
                                                    QString::fromStdString(
                                                       std::to_string(aChanges.properties.mRadialLineWidth))));
   }
   if (aChanges.centerAngleChanged)
   {
      AddToSortedChanges(sortedChanges,
                         GetTextReplacementCommands(ringNode, "centerAngle", "center_angle", aChanges.unitStrings.centerAngle));
   }
   if (aChanges.widthChanged)
   {
      AddToSortedChanges(sortedChanges,
                         GetTextReplacementCommands(ringNode, "angleWidth", "angle_width", aChanges.unitStrings.width));
   }
   if (aChanges.radialRateChanged)
   {
      AddToSortedChanges(sortedChanges,
                         GetTextReplacementCommands(ringNode, "radialRate", "radial_rate", aChanges.unitStrings.radialRate));
   }
   if (aChanges.alignPlatformChanged)
   {
      AddToSortedChanges(
         sortedChanges,
         GetSingleTextReplacementCommands(ringNode, "alignNorth", "align_north", !aChanges.properties.mAlignWithPlatform));
   }

   // Done this way to actually change a multiset value, because normal iterators won't work
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

   if (aChanges.nameChanged)
   {
      ReplaceNameInNode(ringNode, "range_ring", aChanges.properties.mName);
   }
}

void WizAnnotation::Plugin::ReplaceNameInNode(const WsfPProxyNode& aNode, const QString& aType, const QString& aNewName)
{
   UtTextDocumentRange nodeRange = aNode.GetSourceDocumentRange();
   QString             nodeText  = QString::fromStdString(aNode.GetSourceText());
   QStringList         tokenList = wizard::Editor::Split(nodeText);
   if (!tokenList.empty())
   {
      int    nameLabelToken = tokenList.indexOf(aType, 0);
      int    nameIndex      = wizard::Editor::FindNextValidToken(tokenList, nameLabelToken + 1);
      size_t beginIndex     = 0;
      for (int i = 0; i < nameIndex; ++i)
      {
         beginIndex += tokenList[i].size();
      }
      UtTextDocumentRange nameRange = nodeRange;
      nameRange.SetBegin(nodeRange.GetBegin() + beginIndex);
      nameRange.SetEnd(nameRange.GetBegin() + tokenList[nameIndex].size());
      wizEnv.BeginUndoCapture();
      if (!wizard::EditorHelpers::ReplaceTextInEditor(nameRange, "", aNewName))
      {
         throw UtException("Failed to replace text.");
      }
      wizEnv.EndUndoCapture();
   }
}

void WizAnnotation::Plugin::DisableFeatures(wizard::WsfExe* aExePtr)
{
   // When a version preceding 2.6 is loaded, annotations need to be disabled to prevent compatibility issues. It can be
   // re-enabled when 2.6 or later is loaded
   if (aExePtr)
   {
      auto AnnotationsExistP = [](WsfPM_ObjectMap& aMap)
      {
         std::vector<WsfPProxyNode> vec;
         aMap.GetAllValues(vec);
         return !vec.empty();
      };
      auto AnnotationsExistWarning = [&](const QString& aMessage)
      {
         WsfPM_ObjectMap poiMap(mProxyRoot + "pois");
         WsfPM_ObjectMap decMap(mProxyRoot + "decorations");
         WsfPM_ObjectMap rrMap(mProxyRoot + "rangeRings");
         if (AnnotationsExistP(poiMap) || AnnotationsExistP(decMap) || AnnotationsExistP(rrMap))
         {
            QMessageBox::warning(nullptr, "Annotations Invalidated", aMessage);
         }
      };

      std::string version  = aExePtr->GetVersion();
      size_t      decIndex = version.find_first_of('.');
      if (!version.empty() && decIndex != std::string::npos && decIndex + 2 <= version.size())
      {
         QString relevantVersion = QString::fromStdString(version.substr(0, decIndex + 2));
         if (relevantVersion.toDouble() < 2.6)
         {
            mEnableFeatures       = false;
            auto propertiesWidget = mDisplayInterface->GetPoiPropertiesWidget();
            if (propertiesWidget && propertiesWidget->isVisible())
            {
               propertiesWidget->close();
            }
            auto ringWidget = mDisplayInterface->GetRingPropertiesWidget();
            if (ringWidget && ringWidget->isVisible())
            {
               ringWidget->close();
            }
            AnnotationsExistWarning("Annotations are not supported on AFSIM simulation applications before version "
                                    "2.6. They will be disabled until an appropriate executable is loaded.");
         }
         else
         {
            mEnableFeatures = true;
         }
      }
   }
}
