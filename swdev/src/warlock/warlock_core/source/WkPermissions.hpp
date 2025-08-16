// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2021 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef WkPermissions_HPP
#define WkPermissions_HPP

#include <QString>

#include "WkfEnvironment.hpp"
#include "WkfPermissionManager.hpp"
#include "WkfPermissionPrimitives.hpp"
#include "WkfTeamVisibilityPrefObject.hpp"

// ***********************************************
// Example of a custom permission type, that represents an integer with a dial widget in the Permissions Dialog
//
// This is the code to define the custom type
//
//    struct CustomType : ut::reflect::Derive<CustomType, IntPermission>
//    {
//       using ReflectBase::ReflectBase;
//    };
//
//    QWidget* MakeCustomWidget(CustomType& aNode, wkf::permission::CategoryPage& aPage)
//    {
//       auto* retval = ut::qt::make_qt_ptr<ut::qt::LabeledInput<QDial>>(nullptr, aNode.mName);
//       auto* widget = retval->GetWidget();
//
//       widget->setValue(aNode.Get());
//       retval->setToolTip(aNode.mDescription);
//
//       QObject::connect(&aNode, &CustomType::Changed,
//                        widget, &QDial::setValue);
//
//       QObject::connect(&aPage, &CategoryPage::ApplyChanges, [&aNode, widget]()
//       {
//          aNode.Set(widget->value());
//       });
//       QObject::connect(&aPage, &CategoryPage::RevertChanges, [&aNode, widget]()
//       {
//          widget->setValue(aNode.Get());
//       });
//
//       return retval;
//    }
//
// This is the code to add the permission along with registering the customer widget
//
//   wkfEnv.GetPermissions().AddPermission<CustomType>("Test Page", "Custom", "I should have a custom widget.");
//
//    auto* mainWindow = wkfEnv.GetMainWindow();
//    if (mainWindow)
//    {
//       auto* dialog = mainWindow->GetPermissionDialog();
//       if (dialog)
//       {
//          dialog->GetGuiFactory().Register<CustomType, &MakeCustomWidget>();
//       }
//    }
//
// ***********************************************

namespace warlock
{
struct TeamControl : ut::reflect::Derive<TeamControl, wkf::permission::ObjectPermission>
{
   //! A bug in GCC prevents using the base constructor in this scenario.
   TeamControl(QString aName, QString aDescription = {}) noexcept
      : ReflectBase(aName, aDescription)
   {
   }

   bool HasPermissionToControl(const QString& aTeam) const { return !(mEnable->Get() && (mTeam->Get() != aTeam)); }

   Field<wkf::permission::BoolPermission> mEnable{
      this,
      "Enable Team Restrictions",
      "Restrict the control of platforms to only platforms on the specified team.",
      false};
   Field<wkf::permission::StringPermission> mTeam{this, "Team", "The team name the user is allowed to control.", ""};
};

struct PlatformControl : ut::reflect::Derive<PlatformControl, wkf::permission::ObjectPermission>
{
   //! A bug in GCC prevents using the base constructor in this scenario.
   PlatformControl(QString aName, QString aDescription = {}) noexcept
      : ReflectBase(aName, aDescription)
   {
   }

   bool HasPermissionToControl(const QString& aPlatformName) const
   {
      return !(mEnable->Get() && !mPlatforms->Get().contains(aPlatformName));
   }

   Field<wkf::permission::BoolPermission>       mEnable{this,
                                                  "Enable Platform Restrictions",
                                                  "Restrict the control of platforms to only the platforms specified.",
                                                  false};
   Field<wkf::permission::StringListPermission> mPlatforms{this,
                                                           "Platforms",
                                                           "The names of the platforms the user is allowed to control.",
                                                           QStringList()};
};

static const QString CREATE_REMOVE_ID   = "Restrict Platform Creation and Deletion";
static const QString PLATFORM_CONTOL_ID = "Limit platform control to list";
static const QString TEAM_CONTOL_ID     = "Limit platform control to team";

inline void SetupPermissions()
{
   // Register File Lock permission
   auto& lockFileLoadPerm =
      wkfEnv.GetPermissions().AddBool("General",
                                      "Lock File Loading",
                                      "Prevents the user from navigating the file system via Save/Load file menus.",
                                      false);
   QObject::connect(&lockFileLoadPerm,
                    &wkf::permission::BoolPermission::Changed,
                    [](bool aEnabled) { wkfEnv.SetFileBrowsingLocked(aEnabled); });
   // Register Team Visibility permission
   auto& teamVisPerm = wkfEnv.GetPermissions().AddBool("General",
                                                       "Lock Team Visibility",
                                                       "Prevents the user from changing Team Visibility settings.",
                                                       false);
   QObject::connect(&teamVisPerm,
                    &wkf::permission::BoolPermission::Changed,
                    [](bool aEnabled)
                    { wkfEnv.GetPreferenceObject<wkf::TeamVisibilityPrefObject>()->SetTeamVisibilityLocked(aEnabled); });

   wkfEnv.GetPermissions().AddBool("Platform Control",
                                   CREATE_REMOVE_ID,
                                   "Controls whether the user is allowed to create or delete platforms",
                                   false);
   wkfEnv.GetPermissions().AddPermission<TeamControl>("Platform Control", TEAM_CONTOL_ID);
   wkfEnv.GetPermissions().AddPermission<PlatformControl>("Platform Control", PLATFORM_CONTOL_ID);
}

inline bool HasPermissionToCreateOrRemovePlatforms()
{
   auto* perm = dynamic_cast<wkf::permission::BoolPermission*>(
      wkfEnv.GetPermissions().FindPermission("Platform Control", CREATE_REMOVE_ID));
   return !(perm && perm->Get());
}

inline bool HasPermissionToControlPlatform(const wkf::Platform& aPlatform)
{
   bool retVal = true;

   auto* teamPerm = dynamic_cast<TeamControl*>(wkfEnv.GetPermissions().FindPermission("Platform Control", TEAM_CONTOL_ID));
   if (teamPerm)
   {
      retVal = retVal && teamPerm->HasPermissionToControl(QString::fromStdString(aPlatform.GetSide()));
   }

   auto* platformPerm =
      dynamic_cast<PlatformControl*>(wkfEnv.GetPermissions().FindPermission("Platform Control", PLATFORM_CONTOL_ID));
   if (platformPerm)
   {
      retVal = retVal && platformPerm->HasPermissionToControl(QString::fromStdString(aPlatform.GetName()));
   }

   return retVal;
}
} // namespace warlock

#endif
