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
#ifndef COMMENTPLUGIN_HPP
#define COMMENTPLUGIN_HPP

#include "CommentDataContainer.hpp"
#include "CommentSimInterface.hpp"
#include "WkPlugin.hpp"
#include "comment/WkfCommentDockWidget.hpp"
#include "comment/WkfCommentPrefWidget.hpp"

namespace WkComment
{
class Plugin : public warlock::PluginT<SimInterface>
{
public:
   Plugin(const QString& aPluginName, const size_t aUniqueId);

   ~Plugin() override = default;

   QList<wkf::PrefWidget*> GetPreferencesWidgets() const override;

protected:
   void GuiUpdate() override;

private:
   void Cleanup();
   void RemoveComments(const std::vector<unsigned int>& aCommentIds);

   void EnabledChangedHandler(bool aEnabled);
   void ShowTimestampChangedHandler(bool aShow);

   // Use guarded pointers because objects will be owned by main window
   PluginUiPointer<wkf::CommentPrefWidget> mPrefWidgetPtr;
   PluginUiPointer<wkf::CommentDockWidget> mDockWidgetPtr;
   DataContainer                           mDataContainer;
   bool                                    mEnabled;
};
} // namespace WkComment

#endif
