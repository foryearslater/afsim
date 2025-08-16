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

#ifndef PROJECTBROWSERITEM_HPP
#define PROJECTBROWSERITEM_HPP

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QMap>
#include <QSet>
#include <QString>
#include <QVariant>

#include "AbstractModel.hpp"
#include "Util.hpp"

namespace wizard
{
class Project;
}

namespace ProjectBrowser
{
class DirectoryItem;
class ExternalFilesItem;
class ExternalDirectoryItem;
class FileItem;
class RootItem;

class Item : public wizard::AbstractModelItem
{
public:
   enum ItemType
   {
      cHIDDEN_ROOT,
      cSCENARIO_ROOT,
      cDIRECTORY,
      cFILE,
      cEXTERNAL_FILES,
      cEXTERNAL_DIRECTORY
   };

   explicit Item(ItemType aItemType);

   virtual QString    GetName() const = 0;
   virtual QVariant   GetFontRole();
   virtual QString    GetDisplayRole();
   virtual QFileInfo* GetFileInfo();
   virtual QString    GetToolTip();
   virtual QVariant   GetDecorationRole();
   virtual void       RemovingFromProjectDirectory();

   RootItem* FindRoot();
   bool      IsRoot() const;
   bool      IsDir() const;
   bool      IsFile() const;
   bool      IsExternal() const;
   bool      IsExternalDir() const;

   bool SortsBefore(const Item& aRhs) const;
   int  InsertPosition(const Item& aNewItem) const;
   void AddAbstractItemSorted(Item* aNewItem);

   ItemType mItemType;
};

// -----------------------------------------------------------------------------

class RootItem : public Item
{
public:
   RootItem();

   QString  GetName() const override;
   QVariant GetDecorationRole() override;
   void     Clear();
   void     Populate(wizard::Project* aProjectPtr);
   void     Rescan();
   void     UpdateIncludedFiles(const QSet<QString>& aIncludedFileSet);
   void     UpdateIncludePaths(const QString& aWorkingDirectory, const QMap<QString, QString>& aIncludePathMap);

   void SetDirectories(const QStringList& aPaths);
   void AddDirectory(QString aPath);
   void RemoveDirectory(DirectoryItem* aItemPtr);

   void SetStartupFiles(const QStringList& aPaths);

   DirectoryItem* FindContainingDirectoryItem(QString aPath);

   Item*              FindFromAbsolutePath(QString aPath, bool aSearchExternal = true);
   void               FileAddedInDirectory(FileItem* aItemPtr, const QString& aFullPath);
   void               RemovingDirectory(const QString& aContainedFile);
   ExternalFilesItem* GetExternalFilesItem();

   ExternalFilesItem*      NeedExternalFilesItem();
   const QString&          GetWorkingDirectory();
   QMap<QString, QString>& GetIncludePaths() { return mIncludePaths; }

protected:
   void IncludedFileInserted(const QString& aFilePath);
   void IncludedFileRemoved(const QString& aFilePath);
   void RemoveFileFromExternalList(const QString& aFilePath);

   QSet<QString>          mExternalIncludes;
   QSet<QString>          mInternalIncludes;
   QSet<QString>          mStartupFiles;
   bool                   mIsRescanning;
   QString                mWorkingDirectory;
   QMap<QString, QString> mIncludePaths;
};

// -----------------------------------------------------------------------------

class HiddenRootItem : public Item
{
public:
   HiddenRootItem();
   QString  GetName() const override;
   QVariant GetDecorationRole() override;
};

// -----------------------------------------------------------------------------

class ExternalFilesItem : public Item
{
public:
   ExternalFilesItem();

   QString  GetName() const override;
   QString  GetToolTip() override;
   QVariant GetDecorationRole() override;

   Item* FindFromAbsolutePath(QString aPath);
   Item* FindDirFromAbsolutePath(QString aPath);

   FileItem* AddFile(const QString& aPath);
   void      RemoveFile(const QString& aPath);

   void Rescan();

   QMap<QString, FileItem*> mFiles;
};

// -----------------------------------------------------------------------------

class DirectoryItem : public Item
{
public:
   explicit DirectoryItem(const QFileInfo& aInfo);

   QString    GetName() const override;
   QFileInfo* GetFileInfo() override;
   QVariant   GetDecorationRole() override;

   Item*     FindFromAbsolutePath(QString aPath);
   void      RemovingFromProjectDirectory() override;
   void      Populate(bool aRescanning = false, bool aExternal = false);
   FileItem* AddFileToLargeDirectory(const QString& aPath);
   void      Rescan();

   Item* FindFileItem(const QString& aName);

   typedef std::map<QString, Item*, wizard::UtilFileStringCompareQt> FileMap;

   QFileInfo mFileInfo;  //! A QFileInfo pointing to this directory
   QDir      mDirectory; //! A QDir pointing to this directory
   QDateTime mLastModified;
   FileMap   mFiles; //! A map of subdirectories contained in this directory
   bool      mTooManyFilesToLoad;
};

// -----------------------------------------------------------------------------

class FileItem : public Item
{
public:
   FileItem(const QFileInfo& aInfo, const QString& aName);

   QString    GetDisplayRole() override;
   QVariant   GetFontRole() override;
   QString    GetName() const override;
   QFileInfo* GetFileInfo() override;
   QVariant   GetDecorationRole() override;

   void InitInProjectDirectory(const QString& aFullPath);
   void RemovingFromProjectDirectory() override;
   void SetIsInScenario(bool aIsInScenario);
   void SetMainFileIndex(int aMainFileIndex);
   bool FileIsModified();

   QFileInfo     mFileInfo;
   QString       mName;
   bool          mIsInScenario;
   unsigned char mMainFileIndex;
};

class ExternalDirectoryItem : public Item
{
public:
   explicit ExternalDirectoryItem(const QFileInfo& aInfo, const QString& aName);

   QString    GetName() const override;
   QFileInfo* GetFileInfo() override;

   Item* FindFromAbsolutePath(QString aPath);

   void Rescan();

   QFileInfo mFileInfo; //! A QFileInfo pointing to this directory
   QString   mName;
};
} // namespace ProjectBrowser

#endif // PROJECTBROWSERITEM_HPP
