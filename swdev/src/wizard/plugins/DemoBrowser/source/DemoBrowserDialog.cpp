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

#include "DemoBrowserDialog.hpp"

#include <QDir>
#include <QFileInfo>
#include <QFormLayout>
#include <QTextStream>
#include <QUrlQuery>

#include "CollapsibleGroup.hpp"
#include "Environment.hpp"
#include "RunEnvManager.hpp"
#include "UtPath.hpp"
#include "UtScanDir.hpp"


DemoBrowser::DemoBrowserDialog::DemoBrowserDialog(QMainWindow* aParent)
   : QDialog(aParent, Qt::WindowFlags())
   , mDemoCollapsible(nullptr)
   , mScenarioCollapsible(nullptr)
   , mTrainingCollapsible(nullptr)
{
   mUI.setupUi(this);

   PopulatePages();
   connect(mUI.mSearchButton, &QPushButton::clicked, this, &DemoBrowserDialog::Regenerate);
   connect(mUI.mSearchText, &QLineEdit::returnPressed, this, &DemoBrowserDialog::Regenerate);
   connect(mUI.mSearchText, &QLineEdit::textEdited, this, &DemoBrowserDialog::ClearSearch);

   Regenerate();
}

void DemoBrowser::DemoBrowserDialog::Regenerate()
{
   mUI.mDemoScrollArea->hide();
   mUI.mScenariosScrollArea->hide();
   mUI.mTrainingScrollArea->hide();

   // Clear any matched files
   for (auto& demoPageIter : mPageList)
   {
      demoPageIter.ClearMatches();
   }

   QString searchStr   = mUI.mSearchText->text();
   bool    searchFiles = mUI.mSearchFiles->isChecked();

   demoPageGroup.clear();
   scenariosPageGroup.clear();
   trainingPageGroup.clear();

   if (!searchStr.isEmpty())
   {
      GroupPages(SearchPages(mPageList, searchStr, QList<QString>(), searchFiles), "group");
   }
   else
   {
      GroupPages(mPageList, "group");
   }

   delete mDemoCollapsible;
   mDemoCollapsible = new QGroupBox(mUI.mDemoScrollArea);
   mDemoCollapsible->setVisible(true);

   delete mScenarioCollapsible;
   mScenarioCollapsible = new QGroupBox(mUI.mScenariosScrollArea);
   mScenarioCollapsible->setVisible(true);

   delete mTrainingCollapsible;
   mTrainingCollapsible = new QGroupBox(mUI.mTrainingScrollArea);
   mTrainingCollapsible->setVisible(true);

   // Clear the pages
   delete mUI.mDemoScrollArea->takeWidget();
   mUI.mDemoScrollArea->setWidget(mDemoCollapsible);
   mUI.mDemoScrollAreaWidgetContents = mUI.mDemoScrollArea->widget();
   mUI.mToolBox->setItemText(0, QString("Demos (0)"));

   delete mUI.mScenariosScrollArea->takeWidget();
   mUI.mScenariosScrollArea->setWidget(mScenarioCollapsible);
   mUI.mScenariosScrollAreaWidgetContents = mUI.mScenariosScrollArea->widget();
   mUI.mToolBox->setItemText(1, QString("Scenarios (0)"));

   delete mUI.mTrainingScrollArea->takeWidget();
   mUI.mTrainingScrollArea->setWidget(mTrainingCollapsible);
   mUI.mTrainingScrollAreaWidgetContents = mUI.mTrainingScrollArea->widget();
   mUI.mToolBox->setItemText(2, QString("Training (0)"));

   // Recreate the entries
   QList<QString> groups = demoPageGroup.keys();
   for (QList<QString>::const_iterator groupIter = groups.begin(); groupIter != groups.end(); ++groupIter)
   {
      QString     groupName = *groupIter;
      QList<Item> items     = demoPageGroup.value(groupName);
      mUI.mDemoScrollAreaWidgetContents->setLayout(new QVBoxLayout());
      mUI.mDemoScrollAreaWidgetContents->layout()->setContentsMargins(0, 0, 0, 0);
      mUI.mDemoScrollAreaWidgetContents->layout()->setAlignment(Qt::AlignTop);
      GroupPages(mDemoCollapsible, demoPageGroup.value(groupName));
      mUI.mToolBox->setItemText(0, QString("Demos (") + QString::number(items.size()) + ")");
   }

   groups = scenariosPageGroup.keys();
   for (QList<QString>::const_iterator groupIter = groups.begin(); groupIter != groups.end(); ++groupIter)
   {
      QString     groupName = *groupIter;
      QList<Item> items     = scenariosPageGroup.value(groupName);

      mUI.mScenariosScrollAreaWidgetContents->setLayout(new QVBoxLayout());
      mUI.mScenariosScrollAreaWidgetContents->layout()->setContentsMargins(0, 0, 0, 0);
      mUI.mScenariosScrollAreaWidgetContents->layout()->setAlignment(Qt::AlignTop);
      GroupPages(mScenarioCollapsible, scenariosPageGroup.value(groupName));
      mUI.mToolBox->setItemText(1, QString("Scenarios (") + QString::number(items.size()) + ")");
   }

   groups = trainingPageGroup.keys();
   for (QList<QString>::const_iterator groupIter = groups.begin(); groupIter != groups.end(); ++groupIter)
   {
      QString     groupName = *groupIter;
      QList<Item> items     = trainingPageGroup.value(groupName);

      mUI.mTrainingScrollAreaWidgetContents->setLayout(new QVBoxLayout());
      mUI.mTrainingScrollAreaWidgetContents->layout()->setContentsMargins(0, 0, 0, 0);
      mUI.mTrainingScrollAreaWidgetContents->layout()->setAlignment(Qt::AlignTop);
      GroupPages(mTrainingCollapsible, trainingPageGroup.value(groupName));
      mUI.mToolBox->setItemText(2, QString("Training (") + QString::number(items.size()) + ")");
   }

   mUI.mDemoScrollArea->show();
   mUI.mScenariosScrollArea->show();
   mUI.mTrainingScrollArea->show();
}

void DemoBrowser::DemoBrowserDialog::PopulatePages()
{
   mPageList.clear();

   int                 demoId          = 0;
   std::vector<UtPath> demoSearchPaths = wizard::RunEnvManager::GetDemoSearchPaths();
   for (auto&& it : demoSearchPaths)
   {
      QString demoSearchPath = QString::fromStdString(it.GetSystemPath());

      QString typesPath = demoSearchPath;
      if (it.GetSystemPath() == wizard::RunEnvManager::GetTrainingDir().GetSystemPath())
      {
         typesPath = QString::fromStdString(wizard::RunEnvManager::GetDemosDir().GetSystemPath());
      }

      UtScanDir::DirEntries demosDirEntries;
      UtScanDir::ScanDirRecursive(demoSearchPath.toStdString(), demosDirEntries, "*.rst");
      for (auto&& jt : demosDirEntries)
      {
         QString   rstPath = QString::fromStdString(jt.mFilename);
         QFileInfo rstFileInfo(rstPath);
         QDir      demoDir = rstFileInfo.dir();
         demoDir.cdUp();

         mPageList.append(Item(demoId++, demoDir.canonicalPath(), rstPath, typesPath));
      }
   }
}

void DemoBrowser::DemoBrowserDialog::GroupPages(QGroupBox* aGroup, const QList<Item>& aList)
{
   QString search = mUI.mSearchText->text();

   if (!aGroup->layout())
   {
      aGroup->setLayout(new QVBoxLayout());
      aGroup->layout()->setAlignment(Qt::AlignTop);
   }
   for (auto&& it : aList)
   {
      CollapsibleGroup* group = new CollapsibleGroup(aGroup, it.GetAttribute("name"));

      group->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
      group->SetExpanded(true);
      group->setMinimumWidth(300);
      aGroup->layout()->addWidget(group);
      QWidget*     picAndTags = new QWidget(group);
      QHBoxLayout* patLo      = new QHBoxLayout(picAndTags);
      patLo->setContentsMargins(0, 0, 0, 0);
      group->AddContentWidget(picAndTags);

      QWidget*     buttonGroup = new QWidget(group);
      QHBoxLayout* buttonlo    = new QHBoxLayout(buttonGroup);
      buttonGroup->setLayout(buttonlo);
      buttonlo->addItem(new QSpacerItem(20, 20, QSizePolicy::Expanding));
      QPushButton* openDoc = new QPushButton("Open Documentation", buttonGroup);

      buttonlo->addWidget(openDoc);
      QUrl openDocumentationUrl;
      openDocumentationUrl.setScheme("ide");
      openDocumentationUrl.setHost("documentation");
      openDocumentationUrl.setPath("/docs/" + it.GetAttribute("name") + ".html");
      connect(openDoc, &QPushButton::clicked, [=]() { wizEnv.OpenUrl(openDocumentationUrl); });
      QPushButton* openProj = new QPushButton("Open Project", buttonGroup);
      buttonlo->addWidget(openProj);

      QUrl projectUrl;
      projectUrl.setScheme("ide");
      projectUrl.setHost("project");
      QUrlQuery projectUrlQuery;
      projectUrlQuery.addQueryItem("path", it.GetAttribute("path"));
      projectUrlQuery.addQueryItem("startup", it.GetAttribute("startup"));
      QUrlQuery copyProjectUrlQuery(projectUrlQuery);
      copyProjectUrlQuery.addQueryItem("copy", "on");
      copyProjectUrlQuery.addQueryItem("types_path", it.GetAttribute("types_path"));
      QUrl copyProjectUrl(projectUrl);
      copyProjectUrl.setQuery(copyProjectUrlQuery);
      QUrlQuery openProjectUrlQuery(projectUrlQuery);
      openProjectUrlQuery.addQueryItem("open", "on");
      QUrl openProjectUrl(projectUrl);
      openProjectUrl.setQuery(openProjectUrlQuery);
      connect(openProj, &QPushButton::clicked, [=]() { wizEnv.OpenUrl(openProjectUrl); });

      QPushButton* copyProj = new QPushButton("Copy Project", buttonGroup);
      buttonlo->addWidget(copyProj);
      buttonlo->setContentsMargins(0, 0, 0, 0);
      connect(copyProj, &QPushButton::clicked, [=]() { wizEnv.OpenUrl(copyProjectUrl); });
      group->AddHeaderWidget(buttonGroup);

      QLabel* pic = new QLabel(group);
      pic->setPixmap(QPixmap(it.GetAttribute("image")).scaled(150, 150, Qt::KeepAspectRatio, Qt::SmoothTransformation));
      pic->setMaximumWidth(150);
      patLo->addWidget(pic);
      QWidget*     tags  = new QWidget(group);
      QFormLayout* taglo = new QFormLayout(tags);
      taglo->setContentsMargins(0, 0, 0, 0);
      tags->setLayout(taglo);
      patLo->addWidget(tags);

      QLabel* classificationLabel = new QLabel(it.GetDisplayAttribute("classification", search), group);
      classificationLabel->setWordWrap(true);
      taglo->addRow("<b>Classification:</b>", classificationLabel);

      QLabel* dateLabel = new QLabel(it.GetDisplayAttribute("date", search), group);
      dateLabel->setWordWrap(true);
      taglo->addRow("<b>Date:</b>", dateLabel);

      QLabel* pathLabel = new QLabel(it.GetDisplayAttribute("path", search), group);
      pathLabel->setWordWrap(true);
      taglo->addRow("<b>Path:</b>", pathLabel);

      QLabel* summaryLabel = new QLabel(it.GetDisplayAttribute("summary", search), group);
      summaryLabel->setWordWrap(true);
      taglo->addRow("<b>Summary:</b>", summaryLabel);

      QLabel* tagsLabel = new QLabel(it.GetDisplayAttribute("tags", search), group);
      tagsLabel->setWordWrap(true);
      taglo->addRow("<b>Tags:</b>", tagsLabel);

      const QList<QMap<QString, QString>>& matchedFiles = it.GetMatchedFiles();
      if (!matchedFiles.isEmpty())
      {
         CollapsibleGroup* matchedFilesG = new CollapsibleGroup(group, "Matched Files");
         group->AddContentWidget(matchedFilesG);
         QWidget*     contents = new QWidget(matchedFilesG);
         QGridLayout* grid     = new QGridLayout(contents);
         contents->setLayout(grid);
         matchedFilesG->AddContentWidget(contents);
         grid->addWidget(new QLabel("<b>Filename:</b>", contents), 0, 0);
         grid->addWidget(new QLabel("<b>Content:</b>", contents), 0, 1);
         grid->addWidget(new QLabel("<b>Line:</b>", contents), 0, 2);
         int row = 1;
         for (auto&& it : matchedFiles)
         {
            QUrlQuery openFileUrlQuery(projectUrlQuery);
            openFileUrlQuery.addQueryItem("file", it.value("absolute_path"));
            openFileUrlQuery.addQueryItem("line", it.value("line"));
            openFileUrlQuery.addQueryItem("open", "on");
            QUrl openFileUrl(projectUrl);
            openFileUrl.setQuery(openFileUrlQuery);

            QString fileUrl = "<a href=\"" + openFileUrl.url() + "\">" + it.value("relative_path") + "</a>";
            std::cout << fileUrl.toStdString() << std::endl;
            QLabel* link = new QLabel(fileUrl, contents);
            link->setOpenExternalLinks(false);
            connect(link, &QLabel::linkActivated, &wizEnv, &wizard::Environment::OpenUrl);
            grid->addWidget(link, row, 0);

            QString context = it.value("context");
            Item::Highlight(context, search);
            grid->addWidget(new QLabel(context, contents), row, 1);

            grid->addWidget(new QLabel(it.value("line"), contents), row, 2);

            row++;
         }
      }
   }
   aGroup->setContentsMargins(0, 0, 0, 0);
}

void DemoBrowser::DemoBrowserDialog::GroupPages(const QList<Item>& aDemos, const QString& aAttribute)
{
   for (const auto& demo : aDemos)
   {
      if (demo.GetAttribute(aAttribute) == "Demos")
      {
         demoPageGroup[demo.GetAttribute(aAttribute)].append(demo);
      }
      else if (demo.GetAttribute(aAttribute) == "Scenarios")
      {
         scenariosPageGroup[demo.GetAttribute(aAttribute)].append(demo);
      }
      else if (demo.GetAttribute(aAttribute) == "Training")
      {
         trainingPageGroup[demo.GetAttribute(aAttribute)].append(demo);
      }
   }
}

QList<DemoBrowser::Item> DemoBrowser::DemoBrowserDialog::SearchPages(QList<DemoBrowser::Item> aPageList,
                                                                     const QString&           aSearchString,
                                                                     const QList<QString>&    aAttributeList,
                                                                     bool                     aSearchFileContents)
{
   QList<Item> refinedPageList;

   for (auto& item : aPageList)
   {
      if (item.Matches(aSearchString, aAttributeList, aSearchFileContents))
      {
         refinedPageList.append(item);
      }
   }

   return refinedPageList;
}

void DemoBrowser::DemoBrowserDialog::ClearSearch()
{
   if (mUI.mSearchText->text() == "")
   {
      Regenerate();
   }
}

DemoBrowser::Item::Item(int aDemoId, const QString& aDemoPath, const QString& aRstPath, const QString& aTypesPath)
{
   mAttributes["id"]         = QString::number(aDemoId);
   mAttributes["path"]       = aDemoPath;
   mAttributes["types_path"] = aTypesPath;

   QRegExp regExp("\\.\\.\\s+\\|(.+)\\|\\s+replace::\\s+(.+)");

   QFile rstFile(aRstPath);
   rstFile.open(QIODevice::ReadOnly | QIODevice::Text);

   QFileInfo rstFileInfo(rstFile);
   mAttributes["name"] = rstFileInfo.baseName();

   QTextStream rstStream(&rstFile);
   while (!rstStream.atEnd())
   {
      QString line = rstStream.readLine();

      if (regExp.indexIn(line) != -1)
      {
         mAttributes[regExp.cap(1)] = regExp.cap(2);
      }
   }

   QFileInfo imageFileInfo(mAttributes.value("path") + "/doc/" + mAttributes.value("image"));
   if (imageFileInfo.exists() && imageFileInfo.isFile())
   {
      mAttributes["image"] = imageFileInfo.filePath();
   }
   else
   {
      mAttributes["image"] = "qrc:/icons/afsim_logo_icon.png";
   }
}

bool DemoBrowser::Item::Matches(const QString& aSearchString, const QList<QString>& aAttributesList, bool aSearchFiles)
{
   mMatchedString = aSearchString;

   QList<QString> attributeList = aAttributesList;
   if (attributeList.empty())
   {
      attributeList = mAttributes.keys();
   }

   mMatchedAttributes.clear();
   for (QList<QString>::const_iterator attributeListIter = attributeList.begin(); attributeListIter != attributeList.end();
        ++attributeListIter)
   {
      mMatchedAttributes[*attributeListIter] =
         mAttributes.value(*attributeListIter).contains(aSearchString, Qt::CaseInsensitive);
   }

   mMatchedFiles.clear();
   if (aSearchFiles)
   {
      QDir demoDir(mAttributes.value("path"));

      UtScanDir::DirEntries dirEntries;
      UtScanDir::ScanDirRecursive(demoDir.path().toStdString(), dirEntries, "*.txt");
      for (auto&& it : dirEntries)
      {
         QString absolutePath = demoDir.absoluteFilePath(QString::fromStdString(it.mFilename));
         QString relativePath = demoDir.relativeFilePath(QString::fromStdString(it.mFilename));

         QFile file(absolutePath);
         if (file.open(QIODevice::ReadOnly | QIODevice::Text))
         {
            int         line = 1;
            QTextStream textStream(&file);
            while (!textStream.atEnd())
            {
               QString context = textStream.readLine();

               if (context.contains(aSearchString, Qt::CaseInsensitive))
               {
                  QMap<QString, QString> matchedFile;

                  matchedFile["absolute_path"] = absolutePath;
                  matchedFile["relative_path"] = relativePath;
                  matchedFile["line"]          = QString::number(line);
                  matchedFile["context"]       = context;

                  mMatchedFiles.append(matchedFile);

                  break;
               }

               line++;
            }
         }
      }
   }

   return (mMatchedAttributes.values().contains(true) || !mMatchedFiles.isEmpty());
}

QString DemoBrowser::Item::GetAttribute(const QString& aAttribute) const
{
   return mAttributes.value(aAttribute);
}

QString DemoBrowser::Item::GetDisplayAttribute(const QString& aAttribute, const QString& aHighlight) const
{
   QString retval = mAttributes.value(aAttribute);
   if (!aHighlight.isEmpty())
   {
      Highlight(retval, aHighlight);
   }
   return retval;
}

void DemoBrowser::Item::Highlight(QString& aString, const QString& aHighlightString)
{
   QString highlightStyle = "BACKGROUND-COLOR: yellow";
   QString highlightHtml  = QString("<FONT style=\"%1\">\\1</FONT>").arg(highlightStyle);

   if (!aHighlightString.isEmpty())
   {
      aString.replace(QRegExp("(" + QRegExp::escape(aHighlightString) + ")", Qt::CaseInsensitive), highlightHtml);
   }
}
