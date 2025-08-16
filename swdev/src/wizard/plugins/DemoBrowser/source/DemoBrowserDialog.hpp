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

#ifndef DEMOBROWSERDIALOG_HPP
#define DEMOBROWSERDIALOG_HPP

#include <QDialog>
#include <QGroupBox>
#include <QMainWindow>
#include <QMap>

#include "ui_DemoBrowser.h"

namespace wkf
{
class MainWindow;
}

namespace DemoBrowser
{
class CollapsibleGroup;

class Item
{
public:
   Item(int aDemoId, const QString& aDemoPath, const QString& aRstPath, const QString& aTypesPath);

   virtual ~Item() = default;

   bool Matches(const QString& aSearchString, const QList<QString>& aAttributesList, bool aSearchFiles);

   QString                             GetAttribute(const QString& aAttribute) const;
   QString                             GetDisplayAttribute(const QString& aAttribute, const QString& aHighlight) const;
   const QList<QMap<QString, QString>> GetMatchedFiles() const { return mMatchedFiles; }

   static void Highlight(QString& aString, const QString& aHighlightString);

   void ClearMatches() { mMatchedFiles.clear(); }

private:
   QString                       mMatchedString;
   QMap<QString, bool>           mMatchedAttributes;
   QList<QMap<QString, QString>> mMatchedFiles;

   QMap<QString, QString> mAttributes;
};

class DemoBrowserDialog : public QDialog
{
public:
   DemoBrowserDialog(QMainWindow* aParent);
   ~DemoBrowserDialog() override = default;

private:
   Ui::DemoBrowser mUI;

   void        Regenerate();
   void        PopulatePages();
   void        GroupPages(const QList<Item>& aList, const QString& aAttribute);
   void        GroupPages(QGroupBox* aGroup, const QList<Item>& aList);
   QList<Item> SearchPages(QList<DemoBrowser::Item> aPageList,
                           const QString&           aSearchString,
                           const QList<QString>&    aAttributeList,
                           bool                     aSearchFileContents);
   void        ClearSearch();

   QGroupBox* mDemoCollapsible;
   QGroupBox* mScenarioCollapsible;
   QGroupBox* mTrainingCollapsible;

   QList<Item> mPageList;

   QMap<QString, QList<Item>> demoPageGroup;
   QMap<QString, QList<Item>> scenariosPageGroup;
   QMap<QString, QList<Item>> trainingPageGroup;
};
} // namespace DemoBrowser

#endif
