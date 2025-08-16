// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2020 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef FILE_LOCATOR_WIDGET_HPP
#define FILE_LOCATOR_WIDGET_HPP

#include <QWidget>

#include "ui_FileLocatorWidget.h"

namespace wizard
{

class Editor;

namespace EventOutput
{
namespace editor
{

class Editor;

}

//! Widget providing components used for locating files
//! @par details
//!   This widget provides a set of components to aid in locating AFSIM input files.
//!   the components are:
//! - Path combo box:  stores the file names (with path) that are available for editing by the EventOutput::Dialog
//! - Browse:  opens a dialog to find a file
class FileLocatorWidget : public QWidget
{
   Q_OBJECT
public:
   //! Constructs a FileLocatorWidget
   //! @param aParent is a pointer to the parent widget
   FileLocatorWidget(QWidget* aParent = nullptr);
   //! Destructs a FileLocatorWidget
   ~FileLocatorWidget() override = default;

   //! Sets the currently displayed item of the combo box to the provided file
   //! @param aCurrentFile is the wizard::Editor whose file is to be displayed by the combo box
   //! @note if the file name does not exist in the combo box, the text "Select a File" is displayed instead
   void SwitchToFile(wizard::Editor* aCurrentFile);

   //! Sets the currently displayed item of the combo box to the provided file
   //! @param aCurrentFile is the editor::Editor whose file is to be displayed by the combo box
   //! @note if the file name does not exist in the combo box, the text "Select a File" is displayed instead
   void SwitchToFile(editor::Editor* aCurrentFile);

signals:
   //! Signal to notify connected objects that the currently displayed file name has changed
   //! @param aFileName is the name of the file displayed in the combo box after switch
   void FileChanged(const QString& aFileName);

private slots:
   //! Slot executed when the Browse button is clicked
   void OnBrowse();

private:
   //! Loads the names of the current wizard::Project's AFSIM input files into the combo box
   void LoadFiles();

   //! The user interface for the FileLocatorWidget
   Ui::FileLocatorWidget mUi;
};

} // namespace EventOutput
} // namespace wizard


#endif // FILE_LOCATOR_WIDGET_HPP
