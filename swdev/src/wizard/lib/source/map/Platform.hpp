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

#ifndef PLATFORM_HPP
#define PLATFORM_HPP

#include <QString>
#include <QTextCursor>

#include "UtCallbackHolder.hpp"
#include "ViExport.hpp"
#include "WkfPlatform.hpp"
#include "WsfPM_Platform.hpp"
#include "route/WkfAttachmentRoute.hpp"
#include "route/WkfRouteBrowserInterface.hpp"

namespace wizard
{
class ProxyChange;
class Editor;

struct QueuedOperation
{
   QString mPlatformName;
   QString mRouteName;
};

class VI_EXPORT Platform : public wkf::Platform
{
public:
   Platform(const std::string& aName, unsigned int aId, WsfPM_Platform& aPlatform);
   ~Platform() override;

   void GotoDefinition();
   //! Get the proxy platform
   //! @return the proxy platform
   const WsfPM_Platform& GetPlatform() const noexcept { return mPlatform; }
   //! Get the proxy platform
   //! @return the proxy platform
   WsfPM_Platform& GetPlatform() noexcept { return mPlatform; }
   bool            CheckPlatformDataChange(const QString& aDataName, const QString& aDataValue);
   void            PlatformInitialize();

   void                                                       RemovePlatformFromEditor();
   const std::vector<std::pair<Editor*, UtTextDocumentRange>> GetEditRanges();
   void UpdateRoute(const wkf::RouteBrowserInterface::RouteInfo& aRouteInfo);

   void AddNewRouteInText(const wkf::RouteBrowserInterface::RouteInfo& aRouteInfo);
   void AddQueuedOperation(int aIndex, bool aIsInsertion)
   {
      mQueuedOperations.emplace_back(std::pair<int, bool>(aIndex, aIsInsertion));
   }
   void RemoveWaypointFromEditor(wkf::AttachmentRoute* aRoutePtr, int aWaypointIndex = -1);

   //! Gets the selected waypoints
   //! @return a list of the selected waypoints
   //! @note returned by value so that programmatic edits don't modify the returned result
   const std::vector<unsigned int>& GetSelectedWaypoints() { return mSelectedWaypoints; }
   void                             ClearSelectedWaypoints() { mSelectedWaypoints.clear(); }
   //! Updates this platform's selected waypoint list with the selected waypoints of its route
   //! @param aRoute is this platform's route
   void UpdateSelectedWaypoints(wkf::AttachmentRoute* aRoute) noexcept;
   void HandleQueuedDeleteWaypoint();
   void ResetDeletedIndex() { mWaitingForDelete = -1; }

   //! Get the document range for a specified attribute proxy node
   //! @param aAttrNode is the proxy node for the attribute
   //! @return the attribute range for the specified attribute proxy node
   //! @note This function will seek out 'edit platform' and return that range if one exists
   UtTextDocumentRange GetAttributeDocumentRange(WsfPProxyNode aAttrNode);
   //! Gets the attribute range for a the given platform's attribute
   //! @param aAttribute is the attribute to find
   //! @return the attribute range for the specified attribute
   UtTextDocumentRange GetAttributeRangeByName(const QString& aAttribute);
   void AddWaypointToEditor(const wkf::RouteBrowserInterface::WaypointInfo& aWaypointInfo, const int aWaypointIndex = -1);

   size_t GetDtedId() const { return mDtedId; }

   //! Determines whether this platform has a mover
   //! @return whether this platform has a mover
   bool PlatformHasMover() const noexcept;

   //! Determines whether this platform can have a route
   //! @return whether this platform can have a route
   bool IsRouteable() const noexcept;

   //! Determines whether this platform has a route
   //! @return whether this platform has a route
   bool PlatformHasRoute() const noexcept;

   //! Determines whether this platform uses a global route
   //! @return whether this platform uses a global route
   bool IsUsingGlobalRoute() const noexcept;

   //! Get the proxy route node
   //! @retval "the WsfPM_Route node" if there is a route
   //! @retval "an empty WsfPM_Route node" if there is no route
   WsfPM_Route GetProxyRoute() const noexcept;

   //! Sets the "wrapped" flag
   //! @note the VtkInterface to avoid a move of both the platform and the first waypoint
   void Wrap() noexcept;

   //! Unsets the "wrapped" flag
   //! @note the VtkInterface to avoid a move of both the platform and the first waypoint
   void Unwrap() noexcept;

   //! Determine the "wrapped" flag status
   //! @return whether the VtkInterface wrapped the platform
   bool IsWrapped() const noexcept;

private:
   void OnProxyModified(const ProxyChange& aProxyChange);
   void OnGlobalRouteListChange(const wizard::ProxyChange& aProxyChange);
   void ProxyUpdate();
   void SetupGroupings();

   bool CheckPositionChanged(const UtLatPos& aLatValue, const UtLonPos& aLonValue);
   bool CheckLatitudeChanged(const QString& aDataValue);
   bool CheckLongitudeChanged(const QString& aDataValue);
   bool CheckAltitudeChanged(const QString& aDataValue);
   //! Checks if the heading changed and updates it accordingly
   //! @return whether or not the check succeeded (true means it found the heading tag or the heading is different)
   //! @note this is different than the other check functions because heading is not always defined in the script
   bool CheckHeadingChanged();
   //! Checks to see if the heading unit is valid
   //! @note this is necessary because of an occasional mismatch between the UtTextDocumentRange (from the proxy) and
   //! the wizard::Editor cursor
   bool IsValidUnitForHeading(const QString& aUnitStr) const noexcept;
   bool CheckTypeChanged(const QString& aDataValue);

   void AttributeChanged(const QString& aDataName, const QString& aDataValue);

   void UpdatePlatformInTextEditor(const QString& aNewLine);
   void AddNewAttributeInText(const QString& aAttributeName, const QString& aDataValue);

   //! Constructs the new text to append to the platform definition
   //! @param aStartCursor is the cursor pointing to the start of the platform definition
   //! @param aEndCursor is the cursor pointing to the end of the platform definition
   QString ConstructTextToAppendForPlatform(const QTextCursor& aStartCursor,
                                            const QTextCursor& aEndCursor,
                                            const QString&     aAttributeName,
                                            const QString&     aDataValue) const noexcept;

   void AddNewRouteInText(wkf::AttachmentRoute* aRoutePtr);
   void UpdateTextRange();
   void ReplaceTabs(QStringList& aList);

   //! Updates the relevant wizard::Editor info for the wizard::Platform
   void UpdateEditorInfo() noexcept;

   //! Get the editor that contains this platform's definition
   //! @retval "the wizard::Editor containing the "edit platform" definition" if that exists
   //! @retval "the wizard::Editor containing the "platform" definition" otherwise
   Editor* GetEditor() noexcept;

   unsigned int   mId;
   WsfPM_Platform mPlatform;
   size_t         mDtedId;

   Editor*             mEditorPtr             = nullptr;
   Editor*             mEditPlatformEditorPtr = nullptr;
   std::string         mFileName              = "";
   UtTextDocumentRange mPlatformTextRange;

   UtCallbackHolder mProxyCallbacks;
   bool             mDirty;
   bool             mWrapped;
   double           mSimTime;
   double           mLastLat;
   double           mLastLon;

   //! Adds/removes waypoints when the okay/apply button is pressed in the route browser dialog
   std::vector<std::pair<int, bool>> mQueuedOperations;
   //! Tracks which waypoints were previously selected prior to a route move operation
   int mWaitingForDelete; // index of a waypoint scheduled for deletion, -1 if no such waypoint exists
   std::vector<unsigned int> mSelectedWaypoints;
};
} // namespace wizard

VA_DECLARE_OBJECT_TYPE(wizard::Platform)
#endif
