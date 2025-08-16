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

#ifndef WIZ_ANNOTATION_PLUGIN_HPP
#define WIZ_ANNOTATION_PLUGIN_HPP

#include <unordered_map>

#include "Plugin.hpp"
#include "PointOfInterestPropertiesWidget.hpp"
#include "ProxyChange.hpp"
#include "RangeRingPropertiesWidget.hpp"
#include "annotation/WkfAnnotationPlugin.hpp"

namespace WizAnnotation
{
class Plugin : public wizard::PluginT<Annotation::Plugin>
{
public:
   Plugin(const QString& aPluginName, const size_t aUniqueId);
   ~Plugin() override = default;

   void BuildViewerContextMenu(QMenu* aMenu, vespa::VaViewer* aViewerPtr) override;

   QList<wkf::Action*> GetActions() const override; //{ return QList<wkf::Action*>(); }

private:
   void OnProxyAvailable(WsfPProxy* aProxyPtr);

   using ParserFunc = void (Plugin::*)(const WsfPProxyNode&);
   void ParseAnnotationNode(WsfPM_ObjectMap& aMap, ParserFunc aFunc);
   void ParseDecorationNode(const WsfPProxyNode& aDecNode);
   void ParsePoiNode(const WsfPProxyNode& aPoiNode);
   void ParseRangeRingNode(const WsfPProxyNode& aRingNode);

   void DecorationChange(const wizard::ProxyChange& aProxyChange);
   void PoiChange(const wizard::ProxyChange& aProxyChange);
   void RingChange(const wizard::ProxyChange& aProxyChange);

   void DecorateHandler(const QString& aName, const QString& aType);
   void DecoratePreset(const QString& aName, const QString& aPath) override;
   bool DecorateText(const QString& aText) override;
   bool DecorateImage(const QString& aImage) override;
   void ClearDecoration(const QString& aName) override;
   void ClearRangeRing(wkf::Entity* aEntityPtr, unsigned int aUniqueId) override;

   void EntityAddedCB(vespa::VaEntity* aEntity);
   void EntityAddedHandler(const std::string& aName);

   void InsertCommandIntoFile(const std::string& aFileName, const QString& aNewCommand, const std::string& aEntityName = "");
   QString GetPoiCommand(const wizard::PointOfInterestPropertiesWidget::PoiChanges& aChanges);
   void    ProcessPoiChanges(const wizard::PointOfInterestPropertiesWidget::PoiChanges& aChanges);

   QString GetRingCommand(const wizard::RingPropertiesWidget::RingChanges& aChanges);
   void    ProcessRingChanges(const wizard::RingPropertiesWidget::RingChanges& aChanges);

   void ReplaceNameInNode(const WsfPProxyNode& aNode, const QString& aType, const QString& aNewName);

   void DisableFeatures(wizard::WsfExe* aExePtr);

   std::unordered_map<std::string, std::pair<std::string, int>> mCurrentDecorations;
   UtCallbackHolder                                             mUtCallbacks;
   UtCallbackHolder                                             mAnnotationCallbacks;
   vespa::VaCallbackHolder                                      mVaCallbacks;
   WsfPM_Root                                                   mProxyRoot;
};
} // namespace WizAnnotation

// Note: If there are duplicate entity names (on different entity types), the behavior is undefined (for the user) and
// will just attach the decoration to the first entity found When the attachment is modified in the map display, the
// decoration will be moved accordingly in the text editor. However, this may not match up with Warlock/RV

#endif
