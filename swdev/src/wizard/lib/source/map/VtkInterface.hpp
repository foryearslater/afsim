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

#ifndef VTKINTERFACE_HPP
#define VTKINTERFACE_HPP

class QDragEnterEvent;
class QDropEvent;
class QMenu;
class QMimeData;
#include <QObject>
class QWidget;
#include "UtOptional.hpp"
#include "VaCallbackHolder.hpp"
#include "WkfPlatform.hpp"
class WsfPathList;
#include "WsfPM_Root.hpp"

namespace vespa
{
class VaEntity;
class VaViewer;
class VaWidget;
} // namespace vespa

namespace wizard
{
class ZoneSet;
class Platform;
class ProxyChange;

class VtkInterface : public QObject
{
   Q_OBJECT
public:
   VtkInterface();
   ~VtkInterface() override;

   void OnProxyAvailable(WsfPProxy* aProxyPtr);

   // place an order to select a platform when it exists
   void RequestSelectPlatform(const QString& aPlatformName);

   //! Represents the transformation used when moving entities and attachments
   enum class MoveMode
   {
      RotateScenario,    //!< mode for rotation of the entire scenario
      RotateSelection,   //!< mode for rotation of a selection
      TranslateScenario, //!< mode for translation of the entire scenario
      TranslateSelection //!< mode for translation of a selection (the default)
   };

   //! Sets the MoveMode for scenario transformations
   //! @param aMoveMode is the new MoveMode
   void SetMoveMode(MoveMode aMoveMode) noexcept;
   //! Gets the MoveMode for scenario transformations
   //! @returns the current MoveMode
   MoveMode GetMoveMode() const noexcept;

signals:

   void AttachmentAdded(vespa::VaAttachment* aAttachmentPtr);

private:
   void ActionsForNodeRequested(const WsfPProxyNode& aNode, QMenu* aMenuPtr, QWidget* aParentWidgetPtr);
   void PlatformAdded(WsfPM_Platform& aPlatform);
   void PlatformRemoved(const WsfPM_Platform& aPlatform);
   void PlatformUpdated(WsfPM_Platform& aPlatform);

   bool         ZoneExists(WsfPM_Zone& aZone) const;
   void         ZoneAdded(WsfPM_Zone& aZone);
   void         RemoveZone(WsfPM_Zone& aZone);
   void         UpdateZone(WsfPM_Zone& aZone);
   void         UpdateReferencePlatformZones(const std::string& aPlatformName);
   void         OnPlatformListChange(const ProxyChange& aProxyChange);
   void         OnZoneListChange(const ProxyChange& aProxyChange);
   void         OnScenarioChange(const ProxyChange& aProxyChange);
   unsigned int AssignId(const std::string& aPath);
   unsigned int LookupId(const std::string& aPath) const;
   void         ClearData();

   void DragEnter(QDragEnterEvent* aEvent);
   void DragDrop(QDropEvent* aEvent);
   bool PasteMime(const QMimeData* aMimeData, double aLat, double aLon);
   void MoveSelectionAndAttachmentComplete(const wkf::PlatformList&       aPlatformList,
                                           const wkf::EntityList&         aNonPlatformList,
                                           const vespa::VaAttachmentList& aAttachmentList);

   void UpdateClassificationBannerText(const std::string& aText, const UtColor& aColor);
   void StandardWidgetChangedCB(vespa::VaWidget* aWidgetPtr);

   WsfPM_Root                            mProxyRoot;
   UtCallbackHolder                      mProxyCallbacks;
   UtCallbackHolder                      mProxyAvailableCallback;
   unsigned int                          mNextId;
   std::map<std::string, unsigned int>   mNameToIdMap;
   std::map<unsigned int, std::string>   mIdToNameMap;
   std::map<unsigned int, Platform*>     mPlatformMap;
   std::vector<std::unique_ptr<ZoneSet>> mZoneSetList;
   QMetaObject::Connection               mConnection;
   vespa::VaCallbackHolder               mVtkCallbacks;
   ut::optional<QString>                 mPlatformSelectionRequest;

   static const std::string cGLOBAL_ZONES_PREFIX;

   MoveMode mMoveMode{MoveMode::TranslateSelection};
};
} // namespace wizard

#endif
