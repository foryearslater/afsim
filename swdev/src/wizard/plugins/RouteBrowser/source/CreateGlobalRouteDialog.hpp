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


#ifndef CREATEGLOBALROUTEDIALOG_HPP
#define CREATEGLOBALROUTEDIALOG_HPP

#include <QDialog>

#include "WsfPProxy.hpp"
#include "ui_CreateGlobalRoute.h"

class UtTextDocument;

namespace RouteBrowser
{
class CreateGlobalRouteDialog : public QDialog
{
   // Q_OBJECT

public:
   ~CreateGlobalRouteDialog() override                     = default;
   CreateGlobalRouteDialog(const CreateGlobalRouteDialog&) = delete;
   CreateGlobalRouteDialog(CreateGlobalRouteDialog&&)      = default;
   CreateGlobalRouteDialog& operator=(const CreateGlobalRouteDialog&) = delete;
   CreateGlobalRouteDialog& operator=(CreateGlobalRouteDialog&&) = default;

   explicit CreateGlobalRouteDialog(WsfPProxy* proxy, QWidget* parent = nullptr);

   const std::string GetSelectedFile();
   // set the file that will appear in the combo box
   void SelectFile(const std::string& aFilePath);

private:
   /// A pair representing a file and how many routes are instantiated in
   /// the file. (The number of "hits.")
   typedef std::pair<std::string, size_t> FileHit;

   /// A map from a file path to its number of hits.
   typedef std::map<std::string, size_t> FileHitMap;

   /// A vector of file hits, to be sorted.
   typedef std::vector<FileHit> FileHitList;

   void PopulateFiles();


   std::string SaveFilePathAtIndex(int aIndex);
   std::string SelectedSaveFilePath();
   std::string SetRelativeDocPath(const UtTextDocument* aDoc);
   void        InitFileHitMap(FileHitMap& aFileHitMap);
   void        BuildFileHitMap(FileHitMap& aFileHitMap);
   void        SortFileHitMap(const FileHitMap& aFileHitMap, FileHitList& aFileHitList);
   static bool SortAlphabetical(const FileHit& aEntry1, const FileHit& aEntry2);
   static bool SortMostHits(const FileHit& aEntry1, const FileHit& aEntry2);

   WsfPProxy*            mProxy;
   Ui::CreateGlobalRoute mUI;
};
} // namespace RouteBrowser

#endif // CREATEGLOBALROUTEDIALOG_HPP
