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

#ifndef PROJECTBROWSERVIEW_HPP
#define PROJECTBROWSERVIEW_HPP

#include <set>

#include <QMainWindow>
#include <QTreeView>

#include "AbstractModel.hpp"
#include "FileTypeCache.hpp"
#include "UtCallbackHolder.hpp"
#include "UtPath.hpp"

namespace wizard
{
class TextSource;
}

// -----------------------------------------------------------------------------
namespace ProjectBrowser
{
class RootItem;
class DockWidget;

class View : public QTreeView
{
   Q_OBJECT

public:
   explicit View(QWidget* aParent);

   void MoveCursor(int aDirection);

   bool        mExpanding;
   DockWidget* mParentPtr;

protected:
   void rowsInserted(const QModelIndex& parent, int start, int end) override;
   void contextMenuEvent(QContextMenuEvent* event) override;
   void currentChanged(const QModelIndex& current, const QModelIndex& previous) override;
};

// -----------------------------------------------------------------------------

class Model : public wizard::AbstractModel
{
   Q_OBJECT

public:
   explicit Model(QObject* aParent);
   ~Model() override;

public slots:

   void DirectorySetChanged(const std::vector<UtPath>& aDirectories);
   void ProjectStartupFilesChanged(const std::vector<UtPath>& aDirectories);
   void SourceModifiedStateChange(wizard::TextSource* aSourcePtr);

public:
   // Project
   // |-ProjectDir1
   // | |- ~~file-system~~
   // |-ProjectDir2
   // | |- ~~file-system~~
   // |-External Includes
   // | |- file path
   // | | |- file1.txt
   // | | |- file2.txt

   QVariant      data(const QModelIndex& index, int role) const override;
   RootItem*     root();
   Qt::ItemFlags flags(const QModelIndex& index) const override;
   QMimeData*    mimeData(const QModelIndexList& indexes) const override;

   RootItem*                     mActualRoot;
   QSet<UtPath>                  mStartupFiles;
   mutable wizard::FileTypeCache mFileTypeCache;
};
} // namespace ProjectBrowser
#endif
