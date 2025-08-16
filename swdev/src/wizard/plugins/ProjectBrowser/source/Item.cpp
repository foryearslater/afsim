// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2003-2015 The Boeing Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************


#include "Item.hpp"

#include <QApplication>
#include <QMessageBox>

#include "PrefObject.hpp"
#include "Project.hpp"
#include "ProjectWorkspace.hpp"
#include "TextSource.hpp"
#include "UiResources.hpp"
#include "UtPath.hpp"
#include "UtQtMemory.hpp"
#include "View.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"

ProjectBrowser::Item::Item(Item::ItemType aItemType)
   : mItemType(aItemType)
{
}

QVariant ProjectBrowser::Item::GetFontRole()
{
   return QVariant();
}

QString ProjectBrowser::Item::GetDisplayRole()
{
   return GetName();
}

QFileInfo* ProjectBrowser::Item::GetFileInfo()
{
   return nullptr;
}

QString ProjectBrowser::Item::GetToolTip()
{
   QFileInfo* infoPtr = GetFileInfo();
   return infoPtr ? infoPtr->absoluteFilePath() : QString();
}

QVariant ProjectBrowser::Item::GetDecorationRole()
{
   return QVariant();
}

void ProjectBrowser::Item::RemovingFromProjectDirectory() {}

ProjectBrowser::RootItem* ProjectBrowser::Item::FindRoot()
{
   Item* p = this;
   while (p->GetAbstractItemParent() && !p->IsRoot())
   {
      p = dynamic_cast<Item*>(p->GetAbstractItemParent());
   }
   assert(!p || p->mItemType == cSCENARIO_ROOT);
   return dynamic_cast<RootItem*>(p);
}

bool ProjectBrowser::Item::IsRoot() const
{
   return mItemType == cSCENARIO_ROOT;
}

bool ProjectBrowser::Item::IsDir() const
{
   return mItemType == cDIRECTORY;
}

bool ProjectBrowser::Item::IsFile() const
{
   return mItemType == cFILE;
}
bool ProjectBrowser::Item::IsExternal() const
{
   return mItemType == cEXTERNAL_FILES;
}

bool ProjectBrowser::Item::IsExternalDir() const
{
   return mItemType == cEXTERNAL_DIRECTORY;
}

bool ProjectBrowser::Item::SortsBefore(const Item& aRhs) const
{
   if (mItemType != aRhs.mItemType)
   {
      return mItemType < aRhs.mItemType;
   }
   return GetName() < aRhs.GetName();
}

// Binary search for the sorted insertion location
int ProjectBrowser::Item::InsertPosition(const Item& aNewItem) const
{
   int ct  = GetAbstractItemCount();
   int low = 0, high = ct;
   while (high > low)
   {
      int idx = (high + low) / 2;
      if ((dynamic_cast<Item*>(GetAbstractItem(idx)))->SortsBefore(aNewItem))
      {
         low = idx + 1;
      }
      else
      {
         high = idx;
      }
   }
   return low;
}

void ProjectBrowser::Item::AddAbstractItemSorted(Item* aNewItem)
{
   AddAbstractItem(aNewItem, InsertPosition(*aNewItem));
}

// -----------------------------------------------------------------------------

ProjectBrowser::RootItem::RootItem()
   : Item(cSCENARIO_ROOT)
   , mIsRescanning(false)
{
}

ProjectBrowser::DirectoryItem* ProjectBrowser::RootItem::FindContainingDirectoryItem(QString aPath)
{
   for (int i = 0; i < GetAbstractItemCount(); ++i)
   {
      Item* itemPtr = dynamic_cast<Item*>(GetAbstractItem(i));
      if (itemPtr->mItemType == Item::cDIRECTORY)
      {
         DirectoryItem* dirPtr = dynamic_cast<DirectoryItem*>(itemPtr);
         dirPtr->mDirectory.relativeFilePath(aPath);
         QString dirPath = dirPtr->mDirectory.absolutePath();
         if (aPath.startsWith(dirPath))
         {
            return dirPtr;
         }
      }
   }
   return nullptr;
}

void ProjectBrowser::RootItem::AddDirectory(QString aPath)
{
   bool canAdd = (FindContainingDirectoryItem(aPath) == nullptr);
   assert(canAdd); // Caller should make sure it's a valid directory to add
   if (canAdd)
   {
      DirectoryItem* dirPtr      = new DirectoryItem(QFileInfo(aPath));
      int            insertIndex = -1;
      Item*          extItem     = GetExternalFilesItem();
      if (extItem)
      {
         insertIndex = extItem->GetItemIndex();
      }
      AddAbstractItem(dirPtr, insertIndex);
      dirPtr->Populate();
      //       QDir dir = dirPtr->mFileInfo.absoluteDir();
      //       QStringList externalList = mExternalIncludes.toList();
      //       QString dirRex("^" + QRegExp::escape(dir.absolutePath()) + ".*");
      //       QStringList matchedFiles = externalList.filter(dirRex);
      //       QSet<QString> matchedSet = QSet<QString>::fromList(matchedFiles);
      //       mExternalIncludes.subtract(matchedSet);
      //       mInternalIncludes.unite(matchedSet);
   }
}

void ProjectBrowser::RootItem::UpdateIncludedFiles(const QSet<QString>& aIncludedFileSet)
{
   QSet<QString> removedFiles(mInternalIncludes);
   removedFiles.unite(mExternalIncludes);
   removedFiles.subtract(aIncludedFileSet);

   for (QSet<QString>::Iterator i = removedFiles.begin(); i != removedFiles.end(); ++i)
   {
      IncludedFileRemoved(*i);
   }

   QSet<QString> newFiles(aIncludedFileSet);
   newFiles.subtract(mInternalIncludes);
   newFiles.subtract(mExternalIncludes);

   for (QSet<QString>::Iterator i = newFiles.begin(); i != newFiles.end(); ++i)
   {
      IncludedFileInserted(*i);
   }
}

void ProjectBrowser::RootItem::UpdateIncludePaths(const QString&                aWorkingDirectory,
                                                  const QMap<QString, QString>& aIncludePathSet)
{
   // Save the working directory
   mWorkingDirectory = aWorkingDirectory;

   // Save the include paths
   mIncludePaths.clear();
   mIncludePaths = aIncludePathSet;
}

void ProjectBrowser::RootItem::IncludedFileInserted(const QString& aFilePath)
{
   if (mInternalIncludes.contains(aFilePath))
   {
      return;
   }
   if (mExternalIncludes.contains(aFilePath))
   {
      return;
   }
   Item* itemPtr = FindFromAbsolutePath(aFilePath, false);
   if (itemPtr && itemPtr->IsFile())
   {
      dynamic_cast<FileItem*>(itemPtr)->SetIsInScenario(true);
      mInternalIncludes.insert(aFilePath);
   }
   else
   {
      DirectoryItem* dirPtr = FindContainingDirectoryItem(aFilePath);
      if (dirPtr && dirPtr->mTooManyFilesToLoad)
      {
         dirPtr->AddFileToLargeDirectory(aFilePath);
      }
      else
      {
         NeedExternalFilesItem()->AddFile(aFilePath);
         mExternalIncludes.insert(aFilePath);
      }
   }
}

void ProjectBrowser::RootItem::IncludedFileRemoved(const QString& aFilePath)
{
   if (mInternalIncludes.contains(aFilePath))
   {
      mInternalIncludes.remove(aFilePath);
      Item* itemPtr = FindFromAbsolutePath(aFilePath);
      if (itemPtr && itemPtr->IsFile())
      {
         dynamic_cast<FileItem*>(itemPtr)->SetIsInScenario(false);
      }
   }
   else if (mExternalIncludes.contains(aFilePath))
   {
      RemoveFileFromExternalList(aFilePath);
   }
}

QString ProjectBrowser::RootItem::GetName() const
{
   return "Project";
}

QVariant ProjectBrowser::RootItem::GetDecorationRole()
{
   return wizard::UiResources::GetInstance().GetIconFromTheme(QString("scenario_folder"));
   ;
}

ProjectBrowser::Item* ProjectBrowser::RootItem::FindFromAbsolutePath(QString aPath, bool aSearchExternal)
{
   DirectoryItem* dirPtr = FindContainingDirectoryItem(aPath);
   if (dirPtr)
   {
      return dirPtr->FindFromAbsolutePath(aPath);
   }

   if (aSearchExternal)
   {
      return NeedExternalFilesItem()->FindFromAbsolutePath(aPath);
   }
   return nullptr;
}

void ProjectBrowser::RootItem::FileAddedInDirectory(FileItem* aItemPtr, const QString& aFullPath)
{
   if (mExternalIncludes.contains(aFullPath))
   {
      RemoveFileFromExternalList(aFullPath);
      aItemPtr->SetIsInScenario(true);
      mExternalIncludes.remove(aFullPath);
      mInternalIncludes.insert(aFullPath);
   }
}

void ProjectBrowser::RootItem::RemoveDirectory(DirectoryItem* aItemPtr)
{
   aItemPtr->RemovingFromProjectDirectory();
   RemoveAbstractItem(aItemPtr);
   delete aItemPtr;
   // RemoveAbstractItem(aItemPtr);
}

void ProjectBrowser::RootItem::Clear()
{
   RemoveAllAbstractItems(true);
   mExternalIncludes.clear();
   mInternalIncludes.clear();
   mStartupFiles.clear();
}

void ProjectBrowser::RootItem::Populate(wizard::Project* aProjectPtr)
{
   if (aProjectPtr)
   {
      AddDirectory(QString::fromStdString(aProjectPtr->ProjectDirectory().GetSystemPath()));
   }
}

void ProjectBrowser::RootItem::SetDirectories(const QStringList& aPaths)
{
   QStringList newPaths = aPaths;
retry:
   for (int i = 0; i < GetAbstractItemCount(); ++i)
   {
      Item* itemPtr = dynamic_cast<Item*>(GetAbstractItem(i));
      if (itemPtr->IsDir())
      {
         DirectoryItem* dirPtr  = dynamic_cast<DirectoryItem*>(itemPtr);
         QString        absPath = dirPtr->mDirectory.absolutePath();
         if (!aPaths.contains(absPath))
         {
            RemoveDirectory(dirPtr);
            goto retry;
         }
         else
         {
            newPaths.removeAll(absPath);
         }
      }
   }

   for (int i = 0; i < newPaths.size(); ++i)
   {
      AddDirectory(newPaths[i]);
   }

   // The first directory in the list is the project directory.  it must remain first
   // so move it now.
   for (int i = 0; i < GetAbstractItemCount(); ++i)
   {
      Item* itemPtr = dynamic_cast<Item*>(GetAbstractItem(i));
      if (itemPtr->IsDir())
      {
         DirectoryItem* dirPtr  = dynamic_cast<DirectoryItem*>(itemPtr);
         QString        absPath = dirPtr->mDirectory.absolutePath();
         if (!aPaths.empty() && aPaths[0] == absPath)
         {
            MoveAbstractItem(dirPtr, 0);
         }
      }
   }
}

void ProjectBrowser::RootItem::SetStartupFiles(const QStringList& aPaths)
{
   QSet<QString> startupFiles = QSet<QString>::fromList(aPaths);
   QSet<QString> oldFiles(mStartupFiles);
   oldFiles.subtract(startupFiles);
   for (QSet<QString>::iterator i = oldFiles.begin(); i != oldFiles.end(); ++i)
   {
      FileItem* itemPtr = dynamic_cast<FileItem*>(FindFromAbsolutePath(*i));
      if (itemPtr)
      {
         itemPtr->SetMainFileIndex(0);
      }
   }

   for (int i = 0; i < aPaths.size(); ++i)
   {
      FileItem* itemPtr = dynamic_cast<FileItem*>(FindFromAbsolutePath(aPaths[i]));
      if (itemPtr)
      {
         itemPtr->SetMainFileIndex(i + 1);
      }
      else
      {
         // try to add this file if it wasn't added during the initial scan and the directory was deemed too large
         DirectoryItem* dirPtr = FindContainingDirectoryItem(aPaths[i]);
         if (dirPtr && dirPtr->mTooManyFilesToLoad)
         {
            itemPtr = dirPtr->AddFileToLargeDirectory(aPaths[i]);
            if (itemPtr)
            {
               itemPtr->SetMainFileIndex(i + 1);
            }
         }
      }
   }

   mStartupFiles = startupFiles;
}

void ProjectBrowser::RootItem::Rescan()
{
   if (mIsRescanning)
   {
      return;
   }
   mIsRescanning = true;
   for (int i = 0; i < GetAbstractItemCount(); ++i)
   {
      Item* itemPtr = dynamic_cast<Item*>(GetAbstractItem(i));
      if (itemPtr->IsDir())
      {
         DirectoryItem* dirPtr = dynamic_cast<DirectoryItem*>(itemPtr);
         dirPtr->Rescan();
      }
      else if (itemPtr->IsExternal())
      {
         // ExternalFilesItem* extPtr = dynamic_cast<ExternalFilesItem*>(itemPtr);
         static_cast<ExternalFilesItem*>(itemPtr)->Rescan();
      }
   }
   mIsRescanning = false;
}

ProjectBrowser::ExternalFilesItem* ProjectBrowser::RootItem::GetExternalFilesItem()
{
   if (GetAbstractItemCount() == 0)
   {
      return nullptr;
   }
   Item* itemPtr = dynamic_cast<Item*>(GetAbstractItem(GetAbstractItemCount() - 1));
   if (itemPtr->mItemType == Item::cEXTERNAL_FILES)
   {
      return dynamic_cast<ExternalFilesItem*>(itemPtr);
   }
   return nullptr;
}

ProjectBrowser::ExternalFilesItem* ProjectBrowser::RootItem::NeedExternalFilesItem()
{
   ExternalFilesItem* itemPtr = GetExternalFilesItem();
   if (itemPtr)
   {
      return itemPtr;
   }
   itemPtr = new ExternalFilesItem();
   AddAbstractItem(itemPtr);
   return itemPtr;
}

const QString& ProjectBrowser::RootItem::GetWorkingDirectory()
{
   return mWorkingDirectory;
}

void ProjectBrowser::RootItem::RemoveFileFromExternalList(const QString& aFilePath)
{
   ExternalFilesItem* extRoot = NeedExternalFilesItem();
   mExternalIncludes.remove(aFilePath);
   extRoot->RemoveFile(aFilePath);
   if (mExternalIncludes.isEmpty())
   {
      RemoveAbstractItem(extRoot);
      delete extRoot;
   }
}

void ProjectBrowser::RootItem::RemovingDirectory(const QString& aContainedFile)
{
   if (mInternalIncludes.contains(aContainedFile))
   {
      mInternalIncludes.remove(aContainedFile);
      NeedExternalFilesItem()->AddFile(aContainedFile);
      mExternalIncludes.insert(aContainedFile);
   }
}

// -----------------------------------------------------------------------------

ProjectBrowser::DirectoryItem::DirectoryItem(const QFileInfo& aInfo)
   : Item(cDIRECTORY)
   , mFileInfo(aInfo)
   , mDirectory(aInfo.absoluteFilePath())
   , mTooManyFilesToLoad(false)
{
}

void ProjectBrowser::DirectoryItem::Populate(bool aRescanning, bool aExternal)
{
   if (mTooManyFilesToLoad)
   {
      return;
   }
   if (!aRescanning)
   {
      if (wizard::Util::FilesInDirectory(mDirectory, 100000) >= 100000)
      {
         mTooManyFilesToLoad = true;
         // Creates message through .show() to prevent it from blocking this thread.
         auto box =
            ut::qt::make_qt_ptr<QMessageBox>(QMessageBox::Information,
                                             "Large Directory",
                                             "Directory is too large to load, only included files will be shown.",
                                             QMessageBox::Ok,
                                             wkfEnv.GetMainWindow());
         box->show();
         return;
      }
   }
   // bool thisFolderNeedsUpdated = !aRescanning;
   // if (aRescanning)
   {
      // mFileInfo.setCaching(false);
      // mFileInfo.refresh();
      // thisFolderNeedsUpdated = (mLastModified != mFileInfo.lastModified());
   }
   mLastModified = mFileInfo.lastModified();

   QFileInfoList entries = mDirectory.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
   QSet<QString> fileNameSet;
   for (int i = 0; i < entries.size(); ++i)
   {
      QString filename = entries[i].fileName();
      // don't show files/directories prefixed with '.' or '@'
      // '.' is used for hidden files on nix,
      // we use '@' to hide files from the Wizard's processing, but not from the user
      if (!filename.startsWith(".") && !filename.startsWith("@") &&
          !wkfEnv.GetPreferenceObject<PrefObject>()->GetHiddenFileExtensions().contains(entries[i].suffix()))
      {
         fileNameSet.insert(filename); // entries[i].fileName());
      }
   }

   for (FileMap::iterator i = mFiles.begin(); i != mFiles.end();)
   {
      if (!fileNameSet.contains(i->first))
      {
         RemoveAbstractItem(i->second);
         delete i->second;
         mFiles.erase(i++);
      }
      else
      {
         ++i;
      }
   }

   // QFileInfoList entries = mDirectory.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
   for (int i = 0; i < entries.size(); ++i)
   {
      QFileInfo& info = entries[i];
      if (!fileNameSet.contains(info.fileName()))
      {
         continue;
      }

      if (info.isDir())
      {
         FileMap::iterator iter = mFiles.find(info.fileName());
         if (iter == mFiles.end())
         {
            DirectoryItem* subdirPtr = new DirectoryItem(info);
            mFiles[info.fileName()]  = subdirPtr;
            AddAbstractItemSorted(subdirPtr);
            subdirPtr->Populate(aRescanning, aExternal);
         }
         else if (iter->second->IsDir())
         {
            DirectoryItem* subdirPtr = dynamic_cast<DirectoryItem*>(iter->second);
            subdirPtr->Populate(aRescanning, aExternal);
         }
      }
      else
      {
         Item* itemPtr = FindFileItem(info.fileName());
         if (!itemPtr)
         {
            FileItem* filePtr = new FileItem(info, info.fileName());
            AddAbstractItemSorted(filePtr);
            mFiles[filePtr->GetName()] = filePtr;
            if (!aExternal)
            {
               filePtr->InitInProjectDirectory(info.absoluteFilePath());
            }
         }
      }
   }
}

ProjectBrowser::FileItem* ProjectBrowser::DirectoryItem::AddFileToLargeDirectory(const QString& aPath)
{
   // this directory was deemed too large to load all files, so no files were added to it initially.
   // This is an included file, add it(and only it) now.
   if (mTooManyFilesToLoad)
   {
      QFileInfo info(aPath);
      FileItem* filePtr = new FileItem(info, info.fileName());
      if (filePtr != nullptr)
      {
         AddAbstractItemSorted(filePtr);
         mFiles[filePtr->GetName()] = filePtr;
         filePtr->SetIsInScenario(true);
         filePtr->InitInProjectDirectory(info.absoluteFilePath());
         return filePtr;
      }
   }
   return nullptr;
}

void ProjectBrowser::DirectoryItem::Rescan()
{
   Populate(true);
}

QString ProjectBrowser::DirectoryItem::GetName() const
{
   return mFileInfo.fileName();
}

QFileInfo* ProjectBrowser::DirectoryItem::GetFileInfo()
{
   return &mFileInfo;
}

QVariant ProjectBrowser::DirectoryItem::GetDecorationRole()
{
   // Check to see if parent is the root
   Item* parentItemPtr = static_cast<Item*>(GetAbstractItemParent());
   if (parentItemPtr && parentItemPtr->IsRoot())
   {
      return wizard::UiResources::GetInstance().GetIconFromTheme(QString("folder"));
      ;
   }
   return wizard::UiResources::GetInstance().GetFileIcon(mFileInfo);
}

ProjectBrowser::Item* ProjectBrowser::DirectoryItem::FindFromAbsolutePath(QString aPath)
{
   QString     relPath = mDirectory.relativeFilePath(aPath);
   QRegExp     slash("\\\\|\\/");
   QStringList pathComponents = relPath.split(slash);
   Item*       itemPtr        = this;
   for (int i = 0; i < pathComponents.size() && itemPtr; ++i)
   {
      if (itemPtr->IsDir())
      {
         itemPtr = dynamic_cast<DirectoryItem*>(itemPtr)->FindFileItem(pathComponents[i]);
      }
      else
      {
         itemPtr = nullptr;
      }
   }
   return itemPtr;
}

void ProjectBrowser::DirectoryItem::RemovingFromProjectDirectory()
{
   int count = GetAbstractItemCount();
   for (int i = 0; i < count; ++i)
   {
      dynamic_cast<Item*>(GetAbstractItem(i))->RemovingFromProjectDirectory();
   }
}

ProjectBrowser::Item* ProjectBrowser::DirectoryItem::FindFileItem(const QString& aName)
{
   FileMap::iterator i = mFiles.find(aName);
   if (i != mFiles.end())
   {
      return i->second;
   }
   return nullptr;
}

// -----------------------------------------------------------------------------

ProjectBrowser::FileItem::FileItem(const QFileInfo& aInfo, const QString& aName)
   : Item(Item::cFILE)
   , mFileInfo(aInfo)
   , mName(aName)
   , mIsInScenario(false)
   , mMainFileIndex(0)
{
}

QString ProjectBrowser::FileItem::GetDisplayRole()
{
   QString disp = FileIsModified() ? GetName() + "*" : GetName();
   if (mMainFileIndex)
   {
      disp += " (" + QString::number(mMainFileIndex) + ")";
   }
   return disp;
}

QVariant ProjectBrowser::FileItem::GetFontRole()
{
   if (mMainFileIndex)
   {
      QFont f;
      f.setUnderline(true);
      return f;
   }
   else
   {
      return Item::GetFontRole();
   }
}

QString ProjectBrowser::FileItem::GetName() const
{
   return mName;
}

QFileInfo* ProjectBrowser::FileItem::GetFileInfo()
{
   return &mFileInfo;
}

QVariant ProjectBrowser::FileItem::GetDecorationRole()
{
   // Check for included files
   if (mIsInScenario)
   {
      return wizard::UiResources::GetInstance().GetIcon("wizard_logo_icon");
   }
   else
   {
      Model*            modelPtr = static_cast<Model*>(GetModel());
      const std::string fileType = modelPtr->mFileTypeCache.GetFileType(mFileInfo);
      if (!fileType.empty())
      {
         std::string icon = wizard::FileTypeCache::GetFileTypeIcon(fileType);
         if (!icon.empty())
         {
            return wizard::UiResources::GetInstance().GetIconFromTheme(QString(icon.c_str()));
         }
      }
      return wizard::UiResources::GetInstance().GetFileIcon(mFileInfo);
   }
   return QVariant();
}

void ProjectBrowser::FileItem::InitInProjectDirectory(const QString& aFullPath)
{
   RootItem* root = FindRoot();
   if (root)
   {
      root->FileAddedInDirectory(this, aFullPath);
   }
}

void ProjectBrowser::FileItem::SetMainFileIndex(int aMainFileIndex)
{
   if (aMainFileIndex != (int)mMainFileIndex)
   {
      if (aMainFileIndex < 256)
      {
         mMainFileIndex = (unsigned char)aMainFileIndex;
      }
      AbstractItemChanged();
   }
}

void ProjectBrowser::FileItem::RemovingFromProjectDirectory()
{
   if (mIsInScenario)
   {
      RootItem* root = FindRoot();
      if (root)
      {
         mIsInScenario = false;
         root->RemovingDirectory(mFileInfo.absoluteFilePath());
      }
   }
}

void ProjectBrowser::FileItem::SetIsInScenario(bool aIsInScenario)
{
   if (mIsInScenario != aIsInScenario)
   {
      mIsInScenario = aIsInScenario;
      AbstractItemChanged();
   }
}

bool ProjectBrowser::FileItem::FileIsModified()
{
   wizard::TextSource* src =
      wizard::ProjectWorkspace::Instance()->FindSource(UtPath(mFileInfo.absoluteFilePath().toStdString()).GetSystemPath(),
                                                       false,
                                                       false);
   if (src)
   {
      return src->IsModified();
   }
   return false;
}

// -----------------------------------------------------------------------------

ProjectBrowser::HiddenRootItem::HiddenRootItem()
   : Item(cHIDDEN_ROOT)
{
}

QString ProjectBrowser::HiddenRootItem::GetName() const
{
   return QString();
}

QVariant ProjectBrowser::HiddenRootItem::GetDecorationRole()
{
   return wizard::UiResources::GetInstance().GetIconFromTheme(QString("scenario_folder"));
   ;
}

// -----------------------------------------------------------------------------

ProjectBrowser::ExternalFilesItem::ExternalFilesItem()
   : Item(cEXTERNAL_FILES)
{
}

QString ProjectBrowser::ExternalFilesItem::GetName() const
{
   return "External Items";
}

QString ProjectBrowser::ExternalFilesItem::GetToolTip()
{
   return "Contained items exists outside of the project's directories";
}

QVariant ProjectBrowser::ExternalFilesItem::GetDecorationRole()
{
   return wizard::UiResources::GetInstance().GetIconFromTheme(QString("external_link"));
}

void ProjectBrowser::ExternalFilesItem::RemoveFile(const QString& aPath)
{
   if (!mFiles.contains(aPath))
   {
      return;
   }
   FileItem* itemPtr = mFiles[aPath];
   itemPtr->SetIsInScenario(false);
   auto parentPtr = static_cast<Item*>(itemPtr->GetAbstractItemParent());

   mFiles.remove(aPath);
   // TODO: MAYBE CHANGE
   //  Traverse the tree back to the external item root
   while (parentPtr != this)
   {
      bool removeItem = false;
      if (parentPtr->IsExternalDir())
      {
         // If the item is a external directory
         auto    dirPtr  = static_cast<ExternalDirectoryItem*>(parentPtr);
         QString dirPath = dirPtr->mFileInfo.absoluteFilePath();
         size_t  count   = 0;
         // QMap<QString, FileItem*>::iterator i = mFiles.begin();
         for (auto i = mFiles.begin(); i != mFiles.end(); ++i)
         {
            if (i.key().startsWith(dirPath))
            {
               ++count;
            }
         }
         if (count == 0)
         {
            // No files in this directory are included in the scenario
            removeItem = true;
         }
      }

      auto grandParentPtr = static_cast<Item*>(parentPtr->GetAbstractItemParent());
      if (removeItem)
      {
         grandParentPtr->RemoveAbstractItem(parentPtr);
         delete parentPtr;
      }
      parentPtr = grandParentPtr;
   }
}

void ProjectBrowser::ExternalFilesItem::Rescan()
{
   for (int i = 0; i < GetAbstractItemCount(); ++i)
   {
      auto itemPtr = static_cast<Item*>(GetAbstractItem(i));
      if (itemPtr && itemPtr->IsExternalDir())
      {
         static_cast<ExternalDirectoryItem*>(itemPtr)->Rescan();
      }
   }
}

ProjectBrowser::Item* ProjectBrowser::ExternalFilesItem::FindFromAbsolutePath(QString aPath)
{
   Item* returnItemPtr = nullptr;
   for (int i = 0; i < GetAbstractItemCount() && !returnItemPtr; ++i)
   {
      auto itemPtr = dynamic_cast<Item*>(GetAbstractItem(i));
      if (itemPtr && itemPtr->IsExternalDir())
      {
         returnItemPtr = static_cast<ExternalDirectoryItem*>(itemPtr)->FindFromAbsolutePath(aPath);
      }
      else if (itemPtr->IsFile())
      {
         auto filePtr = static_cast<FileItem*>(itemPtr);
         auto path    = itemPtr->GetFileInfo()->absoluteFilePath();
         if (aPath == path)
         {
            returnItemPtr = filePtr;
         }
      }
   }
   return returnItemPtr;
}

ProjectBrowser::Item* ProjectBrowser::ExternalFilesItem::FindDirFromAbsolutePath(QString aPath)
{
   // Loop through children of the External Items
   for (int i = 0; i < GetAbstractItemCount(); ++i)
   {
      auto itemPtr = static_cast<Item*>(GetAbstractItem(i));
      if (itemPtr->IsExternalDir())
      {
         auto dirPtr = static_cast<ExternalDirectoryItem*>(itemPtr);
         auto path   = dirPtr->GetFileInfo()->absoluteFilePath();
         if (aPath == path)
         {
            return dirPtr;
         }
      }
   }
   return nullptr;
}

ProjectBrowser::FileItem* ProjectBrowser::ExternalFilesItem::AddFile(const QString& aPath)
{
   // Quick return if already added
   if (mFiles.contains(aPath))
   {
      mFiles[aPath]->SetIsInScenario(true);
      return mFiles[aPath];
   }
   QFileInfo info(aPath);

   // Get the file_path entry for this item
   QString                        incPath;  // from file_path command
   QString                        fullPath; // full path with ../ substitutions, etc.
   QMapIterator<QString, QString> iter(FindRoot()->GetIncludePaths());

   while (iter.hasNext())
   {
      iter.next();
      if (aPath.startsWith(iter.value()))
      {
         incPath  = iter.key();
         fullPath = iter.value();
         break;
      }
   }
   if (fullPath.isEmpty())
   {
      QFileInfo info(aPath);
      fullPath = info.absolutePath();
      incPath  = fullPath;
   }
   // Add the file along with directory path
   if (!fullPath.isEmpty())
   {
      auto extDirPtr = dynamic_cast<ExternalDirectoryItem*>(FindDirFromAbsolutePath(fullPath));
      if (extDirPtr == nullptr)
      {
         // Add new directory first
         extDirPtr = new ExternalDirectoryItem(QFileInfo(fullPath), incPath);
         AddAbstractItemSorted(extDirPtr);
      }

      Item* itemPtr = FindFromAbsolutePath(aPath);
      if (itemPtr == nullptr)
      {
         // Add directory contents
         auto dirPtr = new DirectoryItem(QFileInfo(fullPath));
         extDirPtr->AddAbstractItemSorted(dirPtr);
         dirPtr->Populate(false, true);
         itemPtr = dirPtr;
      }

      // Find the entry and set it in the scenario
      auto           filePtr = dynamic_cast<FileItem*>(FindFromAbsolutePath(aPath));
      DirectoryItem* curDir  = dynamic_cast<DirectoryItem*>(itemPtr);
      if (filePtr == nullptr && curDir != nullptr && curDir->mTooManyFilesToLoad)
      {
         filePtr = curDir->AddFileToLargeDirectory(aPath);
      }
      if (filePtr)
      {
         filePtr->SetIsInScenario(true);
         mFiles[aPath] = filePtr;
         return filePtr;
      }
   }
   return nullptr;
}

ProjectBrowser::ExternalDirectoryItem::ExternalDirectoryItem(const QFileInfo& aInfo, const QString& aName)
   : Item(cEXTERNAL_DIRECTORY)
   , mFileInfo(aInfo)
   , mName(aName)
{
}

QString ProjectBrowser::ExternalDirectoryItem::GetName() const
{
   return mName;
}

QFileInfo* ProjectBrowser::ExternalDirectoryItem::GetFileInfo()
{
   return &mFileInfo;
}

ProjectBrowser::Item* ProjectBrowser::ExternalDirectoryItem::FindFromAbsolutePath(QString aPath)
{
   for (int i = 0; i < GetAbstractItemCount(); ++i)
   {
      auto itemPtr = static_cast<Item*>(GetAbstractItem(i));
      if (itemPtr->IsDir())
      {
         return static_cast<DirectoryItem*>(itemPtr)->FindFromAbsolutePath(aPath);
      }
      else if (itemPtr->IsFile())
      {
         auto filePtr = static_cast<FileItem*>(itemPtr);
         auto path    = itemPtr->GetFileInfo()->absoluteFilePath();
         if (aPath == path)
         {
            return filePtr;
         }
      }
   }
   return nullptr;
}

void ProjectBrowser::ExternalDirectoryItem::Rescan()
{
   for (int i = 0; i < GetAbstractItemCount(); ++i)
   {
      auto itemPtr = static_cast<Item*>(GetAbstractItem(i));
      if (itemPtr->IsDir())
      {
         auto dirPtr = static_cast<DirectoryItem*>(itemPtr);
         dirPtr->Rescan();
      }
   }
}
