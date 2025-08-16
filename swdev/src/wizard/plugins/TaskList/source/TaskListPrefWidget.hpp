// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2022 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef TASK_LIST_PREF_WIDGET_HPP
#define TASK_LIST_PREF_WIDGET_HPP

// WKF Includes
#include "WkfPrefWidget.hpp"

// TaskList Includes
#include "TaskListPrefObject.hpp"
#include "ui_TaskListPrefWidget.h"

namespace TaskList
{
//! The widget displaying and allowing modifications of the preference data for the TaskList::Plugin
class PrefWidget : public wkf::PrefWidgetT<PrefObject>
{
   Q_OBJECT
public:
   //! Constructs a PrefWidget
   //! @param aParent is the parent QWidget
   explicit PrefWidget(QWidget* aParent = nullptr);
   //! Destructs a PrefWidget
   ~PrefWidget() noexcept override = default;

private slots:
   //! @name Slots for task tag customization
   //@{

   //! Slot executed when the user clicks the Add (+) button
   //! @pre the text in the task tag line edit is not in the list
   //! @post the text in the task tag line edit is now in the task tag list, unless if it is already in there
   void OnAddClicked();
   //! Slot executed when the user clicks the Remove (-) button
   //! @post the selected item in the task tag list is no longer in the list
   void OnRemoveClicked();
   //! Slot executed when user types in the task tag line edit
   //! @param aTag is the current text of the task tag line edit
   //! @note the line edit becomes invalid (red) and the Add (+) becomes disabled if the task tag already exists in the list
   void OnTaskTagEdited(const QString& aTag);

   //@}

   //! @name Slots for column customization
   //@{

   //! Slot executed when the user clicks the Down button
   void OnDownClicked();
   //! Slot executed when the user clicks the Left button
   void OnLeftClicked();
   //! Slot executed when the user clicks the Right button
   void OnRightClicked();
   //! Slot executed when the user clicks the Up button
   void OnUpClicked();
   //! Slot executed when the user updates the column order via the task table
   void OnColumnDataUpdated();

   //@}

private:
   //! Connects signals with their respective slots
   void Connect();

   //! Reads the preference data from the given PrefData
   //! @par details
   //!   It clears out the existing data in the list widget.
   //!   Then, it reconstructs the list widget from the PrefData.
   //! @param aPrefData is the given PrefData
   //! @post mUi contains the current preference data from aPrefData
   void ReadPreferenceData(const PrefData& aPrefData) override;
   //! Writes the preference data from the PrefWidget to the given PrefData
   //! @par details
   //!   It clears out the existing data in the PrefData.
   //!   Then, it writes the preference data from the PrefWidget to the given PrefData.
   //! @param aPrefData is the given PrefData
   //! @post aPrefData contains the current preference data from mUi
   void WritePreferenceData(PrefData& aPrefData) override;

   //! Helper routine to write the column data to the given PrefData
   //! @param aPrefData is the given PrefData to which to write the column data
   void WriteColumnDataP(PrefData& aPrefData);

   //! The PrefWidget UI
   Ui::TaskListPrefWidget mUi;
   //! List synced with the task tag list widget
   //! @note Used to make each task tag unique
   QStringList mTaskTagList;
   //! List synced with the available columns list widget
   //! @note Used to make each column unique
   QStringList mAvailableColumnsList;
   //! List synced with the visible columns list widget
   //! @note Used to make each column unique
   QStringList mVisibleColumnsList;
};

} // end namespace TaskList

#endif // TASK_LIST_PREF_WIDGET_HPP
