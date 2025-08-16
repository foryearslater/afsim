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

#include "SingleAttribute.hpp"

#include <QPushButton>
#include <QTextBlock>

#include "CheckBox.hpp"
#include "Editor.hpp"
#include "EditorPrefObject.hpp"
#include "ParseResults.hpp"
#include "PartManagerPlugin.hpp"
#include "Project.hpp"
#include "ProxyWatcher.hpp"
#include "Undo.hpp"
#include "WkfEnvironment.hpp"
#include "WsfPProxyBasicValue.hpp"
#include "WsfParseAlternate.hpp"
#include "WsfParseSequence.hpp"
#include "WsfParser.hpp"

const std::map<QString, QString> PartManager::SingleAttribute::mBinaryAttributeMap = {{"on", {"off"}},
                                                                                      {"destructible", {"indestructible"}},
                                                                                      {"operational", {"non_operational"}}};

PartManager::SingleAttribute::SingleAttribute(const WsfPProxyNode& aAttrNode,
                                              const std::string&   aAttributeName,
                                              QWidget*             aParent /*= nullptr*/)
   : Attribute(aParent)
   , mAttrNode(aAttrNode)
   , mAttributeName(aAttributeName)
{
   mUi.SetupUi(this);
   connect(mUi.ResetButton(), &QPushButton::clicked, this, &SingleAttribute::ResetAttrValue);

   mProxyCallbacks +=
      wizard::WatchProxy(aAttrNode.GetPath()).ThisUpdate.Connect(&SingleAttribute::HandleAttrValueUpdated, this);

   UpdateFormElements();
}

//! Enable/disable various GUI elements on the widget based on the state of the node.
void PartManager::SingleAttribute::UpdateFormElements()
{
   const bool attrHasBasicType = mAttrNode.IsOfBasicType();

   if (mUi.ContentWidget())
   {
      // If the widget represents a basic type (e.g. WSF_PLATFORM), do not let
      // the user edit it.
      mUi.ContentWidget()->setDisabled(attrHasBasicType);
   }

   mUi.ResetButton()->setEnabled(!mAttrNode.IsInherited() && !attrHasBasicType);

   if (parentWidget())
   {
      // Force a repaint on our parent widget, so that styling can be applied;
      // e.g., causing the parent to highlight this attribute widget if the
      // attribute is considered overridden.)
      parentWidget()->update();
   }
}

//! Updates the path of the attribute node when one of its parents has been
//! renamed. (And therefore the path to this attribute changed.)
void PartManager::SingleAttribute::UpdateParentPath(const WsfPProxyPath& aOldParentPath, const WsfPProxyPath& aNewParentPath)
{
   mAttrNode.UpdateParentPath(aOldParentPath, aNewParentPath);
   UpdateAttribute();
}

//! Returns whether or not the value of this attribute is inherited.
bool PartManager::SingleAttribute::IsInherited() const
{
   return mAttrNode.IsInherited();
}

QString PartManager::SingleAttribute::GetAzElAttributeString(const QString& aValueStr)
{
   QString     retStr  = "";
   QStringList attrStr = QString::fromStdString(mAttributeName).split(" ");
   if (attrStr.size() > 1 && (attrStr[0] == "Azimuth" || attrStr[0] == "Elevation"))
   {
      QString boundStr = attrStr[1];
      QString oppositeBoundStr;
      QString lowerAttrStr = attrStr[0].toLower();
      retStr               = lowerAttrStr + "_slew_limits ";
      if (boundStr == "min")
      {
         boundStr               = "Min";
         oppositeBoundStr       = "Max";
         QString oppositeValStr = GetAzElNodeValStr(lowerAttrStr + oppositeBoundStr);
         retStr.append(aValueStr);
         retStr.append(" " + oppositeValStr);
      }
      else
      {
         boundStr               = "Max";
         oppositeBoundStr       = "Min";
         QString oppositeValStr = GetAzElNodeValStr(lowerAttrStr + oppositeBoundStr);
         retStr.append(oppositeValStr);
         retStr.append(" " + aValueStr);
      }
   }
   return retStr;
}

QString PartManager::SingleAttribute::GetAzElNodeValStr(const QString& aAttribute)
{
   std::string   attrStdString = aAttribute.toStdString();
   WsfPProxyNode attrNode      = mAttrNode.GetParent() + attrStdString;
   if (attrNode.IsValid())
   {
      WsfPProxyValue attrVal = attrNode.GetValue();
      return QString::fromStdString(attrVal.ValueToString());
   }
   return QString();
}

//! When the reset button is clicked, set our proxy node's value to the value
//! of its parent node (i.e. its inherited value).
void PartManager::SingleAttribute::ResetAttrValue()
{
   wizard::ParseResults* results = wizard::Project::Instance()->WaitForParseResults();
   if (results)
   {
      UtTextDocumentRange attribRange;
      wizard::Editor*     editorPtr    = GetEditor(attribRange);
      QTextCursor         attributeLoc = editorPtr->textCursor();
      attributeLoc.setPosition(ut::cast_to_int(attribRange.GetBegin()));
      editorPtr->setTextCursor(attributeLoc);

      QString inheritedVal = QString::fromStdString(mAttrNode.GetInherited().GetValue().ValueToString());
      QString resetStr     = GetAzElAttributeString(inheritedVal);
      if (!resetStr.isEmpty())
      {
         attributeLoc.setPosition(ut::cast_to_int(attribRange.GetBegin()));
         attributeLoc.setPosition(ut::cast_to_int(attribRange.GetEnd() + 1), QTextCursor::KeepAnchor);
         editorPtr->setTextCursor(attributeLoc);
         attributeLoc.insertText(resetStr);
      }
      else
      {
         if (!inheritedVal.isEmpty())
         {
            attributeLoc.setPosition(ut::cast_to_int(attribRange.GetBegin()));
            attributeLoc.setPosition(ut::cast_to_int(attribRange.GetEnd() + 1), QTextCursor::KeepAnchor);
            editorPtr->setTextCursor(attributeLoc);
            QString attrQString = QString::fromStdString(mAttributeName).toLower();
            attrQString.replace(" ", "_");
            if (dynamic_cast<PartManager::CheckBox*>(this))
            {
               bool    inheritedBool = (inheritedVal == "true") ? true : false;
               QString resetStr      = (inheritedBool) ? attrQString : mBinaryAttributeMap.at(attrQString);
               attributeLoc.insertText(resetStr);
            }
            else
            {
               QString resetStr = attrQString + " " + inheritedVal;
               attributeLoc.insertText(resetStr);
            }
         }
         else
         {
            editorPtr->DeleteLine();
         }
      }
   }
}

//! Set the main content widget of this attribute. For instance, a ViPpmCheckBox
//! would pass in a QCheckBox* as its content widget.
void PartManager::SingleAttribute::SetContentWidget(QWidget* aContentWidget)
{
   mUi.SetContentWidget(aContentWidget);
   UpdateFormElements();
}

//! Return the proxy node represented by this widget.
const WsfPProxyNode& PartManager::SingleAttribute::ProxyNode() const
{
   return mAttrNode;
}

//! If it is our proxy node that has been modified, update the widget to reflect
//! the changes.
void PartManager::SingleAttribute::HandleAttrValueUpdated(const wizard::ProxyChange& aProxyChange)
{
   if (aProxyChange.reason() == wizard::ProxyChange::cUPDATED)
   {
      UpdateAttribute();
   }
}


void PartManager::SingleAttribute::AttributeValueUpdated(const QString& aVal)
{
   UpdateAttribute();

   wizard::ParseResults* results = wizard::Project::Instance()->WaitForParseResults();
   if (results)
   {
      const QString       typeStr = PartManager::Plugin::GetNodeTypeString(mAttrNode.GetParent());
      UtTextDocumentRange attribRange;
      wizard::Editor*     editorPtr    = GetEditor(attribRange);
      QTextCursor         attributeLoc = editorPtr->textCursor();
      attributeLoc.setPosition(ut::cast_to_int(attribRange.GetBegin()));
      QString curBlockStr = attributeLoc.block().text();
      curBlockStr         = curBlockStr.trimmed();
      QString checkedVal  = CheckValue(aVal);
      QString changedStr  = GetAzElAttributeString(checkedVal);
      if (changedStr.isEmpty())
      {
         changedStr = QString::fromStdString(mAttributeName).toLower();
         changedStr.replace(" ", "_");
         changedStr.append(" ");
         changedStr.append(checkedVal);
      }

      QString typeBlockStr = typeStr;
      if (curBlockStr.startsWith("edit"))
      {
         typeBlockStr.prepend("edit ");
      }
      else if (curBlockStr.startsWith("add"))
      {
         typeBlockStr.prepend("add ");
      }

      if (!mAttrNode.GetSourceDocumentRange().Valid() && !curBlockStr.startsWith(typeBlockStr))
      {
         // add a new block for this attribute..
         QTextCursor bottomCursor = editorPtr->textCursor();
         bottomCursor.setPosition(ut::cast_to_int(attribRange.GetEnd()));
         bottomCursor.setPosition(bottomCursor.block().previous().position());
         bottomCursor.movePosition(QTextCursor::EndOfLine);

         QString tabSpace   = "";
         int     indentSize = wkfEnv.GetPreferenceObject<wizard::EditorPrefObject>()->TabSpace();
         for (int i = 0; i < indentSize; ++i)
         {
            tabSpace.append(" ");
         }

         if (typeStr == "platform")
         {
            changedStr = QString::fromStdString(mAttributeName).toLower();
            changedStr.replace(" ", "_");
         }
         else
         {
            QString changedVal = changedStr;
            changedStr         = "edit ";
            changedStr.append(typeStr);
            if (typeStr != "mover" && typeStr != "fuel")
            {
               changedStr.append(" ");
               changedStr.append(QString::fromStdString(mAttrNode.GetParent().GetAttrName()));
            }
            changedStr.append("\n" + tabSpace + tabSpace);
            changedStr.append(changedVal);
            changedStr.append("\n" + tabSpace);
            changedStr.append("end_");
            changedStr.append(typeStr);
         }

         changedStr.prepend("\n" + tabSpace);
         bottomCursor.insertText(changedStr);
         editorPtr->setTextCursor(bottomCursor);
         editorPtr->textCursor().movePosition(QTextCursor::StartOfLine);
         editorPtr->textCursor().clearSelection();
         int curPos = editorPtr->textCursor().blockNumber();
         editorPtr->GoToLine(curPos - 1);
      }

      else
      {
         // a block already exists for this attribute...
         // add to it or update it
         if (mAttrNode.GetSourceDocumentRange().Valid())
         {
            // the attribute already exists in the block, update it
            attributeLoc.setPosition(ut::cast_to_int(attribRange.GetBegin()));
            attributeLoc.setPosition(ut::cast_to_int(attribRange.GetEnd() + 1), QTextCursor::KeepAnchor);
            editorPtr->setTextCursor(attributeLoc);
            attributeLoc.insertText(changedStr);
         }
         else
         {
            // the attribute does not exist in the block, add to the bottom of it
            QTextCursor bottomCursor = editorPtr->textCursor();
            bottomCursor.setPosition(ut::cast_to_int(attribRange.GetEnd()));
            bottomCursor.setPosition(bottomCursor.block().previous().position());
            bottomCursor.movePosition(QTextCursor::EndOfLine);

            QString tabSpace   = "";
            int     indentSize = wkfEnv.GetPreferenceObject<wizard::EditorPrefObject>()->TabSpace();
            for (int i = 0; i < indentSize; ++i)
            {
               tabSpace.append(" ");
            }
            if (typeStr == "" || typeStr == "platform")
            {
               changedStr.prepend("\n" + tabSpace);
            }
            else
            {
               changedStr.prepend("\n" + tabSpace + tabSpace);
            }
            bottomCursor.insertText(changedStr);
            editorPtr->setTextCursor(bottomCursor);
            editorPtr->textCursor().movePosition(QTextCursor::StartOfLine);
            editorPtr->textCursor().clearSelection();
            int curPos = editorPtr->textCursor().blockNumber();
            editorPtr->GoToLine(curPos - 1);
         }
      }
   }
}

void PartManager::SingleAttribute::AttributeValueUpdated(bool aVal)
{
   UpdateAttribute();
   wizard::ParseResults* results = wizard::Project::Instance()->WaitForParseResults();
   if (results)
   {
      if (mAttrNode.GetParent().IsValid())
      {
         QString typeStr  = PartManager::Plugin::GetNodeTypeString(mAttrNode.GetParent());
         QString attrName = QString::fromStdString(mAttrNode.GetAttrName());

         QString changedStr = (aVal) ? attrName : mBinaryAttributeMap.at(attrName);

         UtTextDocumentRange attribRange;
         wizard::Editor*     editorPtr    = GetEditor(attribRange);
         QTextCursor         attributeLoc = editorPtr->textCursor();

         attributeLoc.setPosition(ut::cast_to_int(attribRange.GetBegin()));
         QString curBlockStr = attributeLoc.block().text();
         curBlockStr         = curBlockStr.trimmed();

         QString typeBlockStr = typeStr;
         if (curBlockStr.startsWith("edit"))
         {
            typeBlockStr.prepend("edit ");
         }
         else if (curBlockStr.startsWith("add"))
         {
            typeBlockStr.prepend("add ");
         }

         QString tabSpace   = "";
         int     indentSize = wkfEnv.GetPreferenceObject<wizard::EditorPrefObject>()->TabSpace();
         for (int i = 0; i < indentSize; ++i)
         {
            tabSpace.append(" ");
         }

         if (!mAttrNode.GetSourceDocumentRange().Valid() && !curBlockStr.startsWith(typeBlockStr))
         {
            // add a new block for this attribute..
            QTextCursor bottomCursor = editorPtr->textCursor();
            bottomCursor.setPosition(ut::cast_to_int(attribRange.GetEnd()));
            bottomCursor.setPosition(bottomCursor.block().previous().position());
            bottomCursor.movePosition(QTextCursor::EndOfLine);
            if (typeStr == "platform")
            {
               changedStr = QString::fromStdString(mAttributeName).toLower();
               changedStr.replace(" ", "_");
            }
            else
            {
               QString changedVal = changedStr;
               changedStr         = "edit ";
               changedStr.append(typeStr);
               if (typeStr != "mover" && typeStr != "fuel")
               {
                  changedStr.append(" ");
                  changedStr.append(QString::fromStdString(mAttrNode.GetParent().GetAttrName()));
               }
               changedStr.append("\n" + tabSpace + tabSpace);
               changedStr.append(changedVal);
               changedStr.append("\n" + tabSpace + "end_");
               changedStr.append(typeStr);
            }

            changedStr.prepend("\n" + tabSpace);
            bottomCursor.insertText(changedStr);
            editorPtr->setTextCursor(bottomCursor);
            editorPtr->textCursor().movePosition(QTextCursor::StartOfLine);
            editorPtr->textCursor().clearSelection();
            int curPos = editorPtr->textCursor().blockNumber();
            editorPtr->GoToLine(curPos - 1);
         }
         else
         {
            // a block already exists for this attribute...
            // add to it or update it
            if (mAttrNode.GetSourceDocumentRange().Valid())
            {
               // the attribute already exists in the block, update it
               attributeLoc.setPosition(ut::cast_to_int(attribRange.GetBegin()));
               attributeLoc.setPosition(ut::cast_to_int(attribRange.GetEnd() + 1), QTextCursor::KeepAnchor);
               editorPtr->setTextCursor(attributeLoc);
               attributeLoc.insertText(changedStr);
            }
            else
            {
               // the attribute does not exist in the block, add to the bottom of it
               QTextCursor bottomCursor = editorPtr->textCursor();
               bottomCursor.setPosition(ut::cast_to_int(attribRange.GetEnd()));
               bottomCursor.setPosition(bottomCursor.block().previous().position());
               bottomCursor.movePosition(QTextCursor::EndOfLine);

               if (typeStr == "" || typeStr == "platform")
               {
                  changedStr.prepend("\n" + tabSpace);
               }
               else
               {
                  changedStr.prepend("\n" + tabSpace + tabSpace);
               }
               bottomCursor.insertText(changedStr);
               editorPtr->setTextCursor(bottomCursor);
               editorPtr->textCursor().movePosition(QTextCursor::StartOfLine);
               editorPtr->textCursor().clearSelection();
               int curPos = editorPtr->textCursor().blockNumber();
               editorPtr->GoToLine(curPos - 1);
            }
         }
      }
   }
}

wizard::Editor* PartManager::SingleAttribute::GetEditor(UtTextDocumentRange& aRange)
{
   if (mAttrNode.IsValid())
   {
      aRange.SetRange(mAttrNode.GetSourceDocumentRange().GetBegin(), mAttrNode.GetSourceDocumentRange().GetEnd());
      aRange.mSource                     = mAttrNode.GetSourceDocumentRange().mSource;
      WsfPProxyIndexNode* proxyIndexNode = mAttrNode.FindIndexNode();
      if (proxyIndexNode == nullptr)
      {
         WsfPProxyNode parentNode = mAttrNode.GetParent();
         if (parentNode.IsOfType("Mover"))
         {
            proxyIndexNode = parentNode.FindIndexNode();
            if (proxyIndexNode != nullptr)
            {
               for (auto iter = proxyIndexNode->mEntries.rbegin(); iter != proxyIndexNode->mEntries.rend(); ++iter)
               {
                  const WsfParseNode* parseNode = iter->mNodePtr;
                  if (parseNode && parseNode->GetParent())
                  {
                     WsfParseNode* parent = parseNode->GetParent();

                     aRange.SetRange(parent->SubtreeRange().GetBegin(), parent->SubtreeRange().GetEnd());
                     aRange.mSource = parent->SubtreeRange().mSource;

                     std::string     fileName  = aRange.mSource->GetFilePath().GetNormalizedPath();
                     wizard::Editor* editorPtr = wizard::Project::Instance()->GotoFile(fileName);
                     QTextCursor     tmpCursor = editorPtr->textCursor();
                     tmpCursor.setPosition(ut::cast_to_int(aRange.GetBegin()));
                     editorPtr->setTextCursor(tmpCursor);
                     QString curBlockStr = editorPtr->textCursor().block().text();
                     curBlockStr         = curBlockStr.trimmed();

                     if (curBlockStr.startsWith("mover") || curBlockStr.startsWith("edit mover") ||
                         curBlockStr.startsWith("platform") || curBlockStr.startsWith("edit platform"))
                     {
                        return editorPtr;
                     }
                  }
               }
               parentNode     = parentNode.GetParent();
               proxyIndexNode = parentNode.FindIndexNode();
            }
         }
         while (proxyIndexNode == nullptr && parentNode.IsValid())
         {
            proxyIndexNode = parentNode.FindIndexNode();
            parentNode     = parentNode.GetParent();
         }
      }
      std::vector<WsfPProxyIndexEntry> entryVec;
      if (proxyIndexNode->mEntries.size() == 0)
      {
         entryVec = proxyIndexNode->mParentPtr->mEntries;
      }
      else
      {
         entryVec = proxyIndexNode->mEntries;
      }
      for (auto iter = entryVec.rbegin(); iter != entryVec.rend(); ++iter)
      {
         if (proxyIndexNode->mEntries.size() ==
             1) // there is only 1 and it's in the platform definition.  No need to look for an 'edit'
         {
            if (!aRange.Valid())
            {
               const WsfParseNode* parseNode = iter->mNodePtr;
               if (parseNode && parseNode->GetParent())
               {
                  WsfParseNode* parent = parseNode->GetParent();
                  aRange.SetRange(parent->SubtreeRange().GetBegin(), parent->SubtreeRange().GetEnd());
                  aRange.mSource = parent->SubtreeRange().mSource;
               }
            }
            std::string fileName = aRange.mSource->GetFilePath().GetNormalizedPath();
            return wizard::Project::Instance()->GotoFile(fileName);
         }
         const WsfParseNode* parseNode = iter->mNodePtr;
         if (parseNode && parseNode->GetParent())
         {
            WsfParseNode* parent = parseNode->GetParent();
            aRange.SetRange(parent->SubtreeRange().GetBegin(), parent->SubtreeRange().GetEnd());
            aRange.mSource            = parent->SubtreeRange().mSource;
            std::string     fileName  = aRange.mSource->GetFilePath().GetNormalizedPath();
            wizard::Editor* editorPtr = wizard::Project::Instance()->GotoFile(fileName);
            QTextCursor     tmpCursor = editorPtr->textCursor();
            tmpCursor.setPosition(ut::cast_to_int(aRange.GetBegin()));
            editorPtr->setTextCursor(tmpCursor);
            QString curBlockStr = editorPtr->textCursor().block().text();
            curBlockStr         = curBlockStr.trimmed();
            // loop through block().previous() until we get to platform definition or edit platform
            // if edit platform is found, mark this as the document range we want to deal with...
            while (!curBlockStr.startsWith("platform") && !curBlockStr.startsWith("edit platform"))
            {
               QTextBlock previousBlock   = editorPtr->textCursor().block().previous();
               curBlockStr                = previousBlock.text();
               QTextCursor curBlockCursor = editorPtr->textCursor();
               curBlockCursor.setPosition(previousBlock.position());
               editorPtr->setTextCursor(curBlockCursor);
            }
            if (curBlockStr.startsWith("edit platform"))
            {
               return editorPtr;
            }
            else if (curBlockStr.startsWith("platform"))
            {
               return editorPtr;
            }
         }
      }
   }
   return nullptr;
}

// This function ensures the string that will eventually be placed in the text editor will be a valid string, i.e.
// '1degree' is valid input but not valid in the text editor whereas '1 degree' is
QString PartManager::SingleAttribute::CheckValue(const QString& aVal)
{
   QString     retStr  = aVal;
   QStringList valList = aVal.split(" ");
   if (!valList.isEmpty())
   {
      QString curVal = valList[0];
      bool    isNum  = false;
      curVal.toDouble(&isNum);
      if (!isNum)
      {
         QString numVal  = "";
         QString unitStr = "";
         for (auto curChar : curVal)
         {
            if (curChar.isDigit() || curChar == '.')
            {
               numVal.append(curChar);
            }
            else
            {
               unitStr.append(curChar);
            }
         }
         if (curVal.startsWith("-"))
         {
            numVal.prepend("-");
         }

         retStr = numVal + " " + unitStr;
      }
   }
   return retStr;
}


void PartManager::SingleAttribute::GetAllAlternates(WsfParseRule*                    aRulePtr,
                                                    std::set<WsfParseRule*>&         aProcessed,
                                                    std::vector<WsfParseAlternate*>& aAlternates)
{
   if (!aRulePtr || !aProcessed.insert(aRulePtr).second)
   {
      return;
   }
   WsfParseRuleDictionary* nested = aRulePtr->GetNestedRules();
   if (nested)
   {
      for (auto&& n2r : nested->mNameToRule)
      {
         GetAllAlternates(n2r.second, aProcessed, aAlternates);
      }
   }
   if (aRulePtr->Type() == WsfParseRule::cALTERNATE)
   {
      aAlternates.push_back(static_cast<WsfParseAlternate*>(aRulePtr));
   }
   std::vector<WsfParseRule*> seq = aRulePtr->GetSequence();
   std::vector<WsfParseRule*> alt = aRulePtr->GetAlternates();
   for (auto&& s : seq)
   {
      GetAllAlternates(s, aProcessed, aAlternates);
   }
   for (auto&& a : alt)
   {
      GetAllAlternates(a, aProcessed, aAlternates);
   }
}
