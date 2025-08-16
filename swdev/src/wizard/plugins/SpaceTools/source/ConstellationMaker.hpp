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

#ifndef CONSTELLATIONMAKER_HPP
#define CONSTELLATIONMAKER_HPP

#include <memory>

#include "ConstellationMakerUI.hpp"
#include "WsfConstellationOptions.hpp"

namespace wizard
{

class Editor;
class Environment;
class MainWindow;
class Project;

} // namespace wizard

class QMenu;
class UtPath;

namespace SpaceTools
{

//! A class that manages the work of the Constellation Maker tool.
//!
//! This class owns the options and the UI, and coordinates the action
//! between the user, the options and the eventual output.
class ConstellationMaker : public QObject
{
   Q_OBJECT

public:
   explicit ConstellationMaker();
   ~ConstellationMaker() override = default;

public slots:
   void OnProjectOpened(wizard::Project* aProjectPtr);
   void OnProjectClosed(wizard::Project* aProjectPtr);
   void SaveOptions();
   void EditorContextMenu(wizard::Editor* aEditorPtr, QMenu& aMenu);
   void GenerateConstellation();
   void GenerateGenerator();
   void ShowDialogFromMenu();

private:
   void ShowDialog();
   void ReloadFileAsNeeded(const UtPath& aFilePath);

   std::unique_ptr<ConstellationMakerUI> mConstellationUI_Ptr;
   WsfConstellationOptions               mOptions;
   wizard::Project*                      mProjectPtr;
};

} // namespace SpaceTools

#endif // CONSTELLATIONMAKER_HPP
