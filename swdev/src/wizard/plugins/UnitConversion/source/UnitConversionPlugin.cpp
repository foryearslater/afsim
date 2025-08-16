// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2018 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "UnitConversionPlugin.hpp"

#include <QMenu>

#include "ParseResults.hpp"
#include "Project.hpp"
#include "Signals.hpp"
#include "WsfEditor.hpp"

WKF_PLUGIN_DEFINE_SYMBOLS(UnitConversion::Plugin,
                          "Unit Conversion",
                          "Provides context menus to convert values in the editor.",
                          "wizard")

UnitConversion::Plugin::Plugin(const QString& aPluginName, const size_t aUniqueId)
   : wizard::Plugin(aPluginName, aUniqueId)
{
   connect(wizSignals, &wizard::Signals::EditorContextMenu, this, &Plugin::OnEditorContextMenu);
}

void UnitConversion::Plugin::OnEditorContextMenu(wizard::Editor* aEditor, QMenu& aMenu)
{
   wizard::WsfEditor* editorPtr = dynamic_cast<wizard::WsfEditor*>(aEditor);
   if (!editorPtr)
      return;
   wizard::ParseResults* parseResultsPtr = editorPtr->GetProject()->GetParseResults();
   if (!parseResultsPtr || !parseResultsPtr->Proxy() || parseResultsPtr->ProxyRegistry() == nullptr)
      return;
   WsfPProxyRegistry* proxyRegistryPtr = parseResultsPtr->ProxyRegistry().get();
   if (editorPtr->GetContextMenuCharacterPosition() < 0)
      return;
   UtTextDocumentRange range(editorPtr->GetSource(),
                             editorPtr->GetContextMenuCharacterPosition(),
                             editorPtr->GetContextMenuCharacterPosition());
   WsfParseNode*       nodePtr = parseResultsPtr->FindLastParseNode(range);
   if (!nodePtr)
      return;

   WsfParseNode* n = nodePtr;
   while (n)
   {
      WsfParseRule* rulePtr = n->GetTopLevelRule();
      if (rulePtr && rulePtr->Type() == WsfParseRule::cVALUE)
      {
         WsfParseValue* valRulePtr = (WsfParseValue*)rulePtr;
         if (valRulePtr->GetType())
         {
            WsfPProxyType*      proxyTypePtr = proxyRegistryPtr->GetType(valRulePtr);
            WsfPProxyBasicType* basicTypePtr = dynamic_cast<WsfPProxyBasicType*>(proxyTypePtr);
            if (basicTypePtr)
            {
               int unitId = basicTypePtr->GetUnitTypeId();
               if (unitId != -1)
               {
                  UtUnitType* unitTypePtr = UtUnits::GetUnitType((UtUnits::UnitType)unitId);
                  while (n->GetParent() && n->GetParent()->GetTopLevelRule() == valRulePtr)
                  {
                     n = n->GetParent();
                  }
                  UtTextDocumentRange range = n->SubtreeRange();
                  if (range.Valid())
                  {
                     std::string       text = range.Text();
                     std::stringstream ss(text);
                     double            val1;
                     std::string       unit1;
                     if (ss >> val1 >> unit1)
                     {
                        int                               currentUnit = unitTypePtr->ReadUnit(unit1);
                        double                            stdVal = unitTypePtr->ConvertToStandard(val1, currentUnit);
                        const std::map<int, std::string>& unitToString   = unitTypePtr->GetUnitToString();
                        QMenu*                            convertSubmenu = nullptr;
                        for (auto&& iter : unitToString)
                        {
                           if (iter.first != currentUnit)
                           {
                              if (!convertSubmenu)
                              {
                                 convertSubmenu = aMenu.addMenu("Convert To");
                              }
                              double            newVal = unitTypePtr->ConvertFromStandard(stdVal, iter.first);
                              std::stringstream newValStr;
                              newValStr << newVal << ' ' << iter.second;
                              QAction*    actionPtr = convertSubmenu->addAction(newValStr.str().c_str());
                              std::string str       = newValStr.str();
                              QObject::connect(actionPtr,
                                               &QAction::triggered,
                                               [range, str]() { Plugin::ReplaceRange(range, str); });
                           }
                        }
                     }
                  }
               }
            }
         }
      }
      n = n->GetParent();
   }
}

void UnitConversion::Plugin::ReplaceRange(UtTextDocumentRange aRange, const std::string& aText)
{
   wizard::TextSource* sourcePtr = (wizard::TextSource*)aRange.mSource;
   if (sourcePtr != nullptr)
   {
      sourcePtr->Replace(aRange, aText);
   }
}
