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


#ifndef CREATEPLATFORMDIALOG_HPP
#define CREATEPLATFORMDIALOG_HPP

#include <map>
#include <vector>

#include <QDialog>

#include "UtMemory.hpp"
#include "ViExport.hpp"
#include "WsfPProxyBasicValues.hpp"
#include "WsfPProxyNode.hpp"
#include "ui_CreatePlatform.h"

class UtTextDocument;
class WsfPProxy;
class QMimeData;

namespace wizard
{
class PasteContext;

class VI_EXPORT CreatePlatformDialog : public QDialog
{
   Q_OBJECT

public:
   ~CreatePlatformDialog() override                  = default;
   CreatePlatformDialog(const CreatePlatformDialog&) = delete;
   CreatePlatformDialog(CreatePlatformDialog&&)      = default;
   CreatePlatformDialog& operator=(const CreatePlatformDialog&) = delete;
   CreatePlatformDialog& operator=(CreatePlatformDialog&&) = default;

   static void PasteHandler(const QMimeData* aMime, const PasteContext& aContext, bool aExecute);

   explicit CreatePlatformDialog(WsfPProxy* proxy, QWidget* parent = nullptr);

   void SetPlatformPosition(const UtLatPos& aLat, const UtLonPos& aLon);

   void SelectPlatformType(const std::string& aName);
public slots:
   void CreatePlatform();
   void UpdateSavedPlatType(int index);
   void UpdateSavedFile(int index);
   void OnNameTextChanged(const QString& aName);

   // use the set platform type to choose the most appropriate file
   void ChooseFileFromType();

private:
   /// A pair representing a file and how many platforms are instantiated in
   /// the file. (The number of "hits.")
   typedef std::pair<std::string, size_t> FileHit;

   /// A map from a file path to its number of hits.
   typedef std::map<std::string, size_t> FileHitMap;

   /// A vector of file hits, to be sorted.
   typedef std::vector<FileHit> FileHitList;

   void PopulateTypes();
   void PopulateFiles();

   void SelectFile(const std::string& aFilePath);

   WsfPProxyNode PlatTypeAtIndex(int index);
   WsfPProxyNode SelectedPlatType();
   std::string   SaveFilePathAtIndex(int index);
   std::string   SelectedSaveFilePath();
   std::string   SetRelativeDocPath(const UtTextDocument* doc);
   void          InitFileHitMap(FileHitMap& fileHitMap);
   void          BuildFileHitMap(FileHitMap& fileHitMap);
   void          SortFileHitMap(const FileHitMap& fileHitMap, FileHitList& fileHitList);
   static bool   SortAlphabetical(const FileHit& entry1, const FileHit& entry2);
   static bool   SortMostHits(const FileHit& entry1, const FileHit& entry2);

   /// Used for maintaining the user's previous combo box selections over
   /// multiple closing/opening of this dialog.
   static std::string mPrevPlatTypeName;
   static std::string mPrevSaveFilePath;

   bool               mPosValid;
   WsfProxy::Position mPos;
   WsfPProxy*         mProxy;
   Ui::CreatePlatform mUi;
   bool               mApplyToTextImmediately;
};
} // namespace wizard

#endif // CREATEPLATFORMDIALOG_HPP
