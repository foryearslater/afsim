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

#include "Util.hpp"

#include <cassert>
#include <ctime>
#include <fstream>
#include <sstream>

#include <QComboBox>
#include <QDialog>
#include <QDir>
#include <QFile>
#include <QTextDocument>
#include <QUrl>
#include <QUrlQuery>
#include <QUuid>

#include "NewFilePrefObject.hpp"
#include "Project.hpp"
#include "RunEnvManager.hpp"
#include "Signals.hpp"
#include "UtStringUtil.hpp"
#include "WkfEnvironment.hpp"
#include "ui_NewFileSelectorDialog.h"

const QRegExp  wizard::Util::wsfValidInputRegexp("([^/#\\s]|/[^*#\\s])*");
constexpr char cMACRO_DELIM = '%';

//! Returns 'true' if the path points to an executable file.
//! This is a simple check to prevent trying to execute files that are not executable.
bool wizard::Util::IsExecutable(const UtPath& aExePath)
{
   if (aExePath.Stat() == UtPath::cFILE)
   {
#ifdef _WIN32
      // File extension can be tested on windows only
      std::string extension = aExePath.GetFileExtension();
      UtStringUtil::ToLower(extension);
      if (extension != ".exe")
      {
         return false;
      }
#endif
      std::string   sysPath = aExePath.GetSystemPath();
      std::ifstream infile(sysPath.c_str(), std::ios::binary);
      char          hdr[5] = {};
      infile.read(hdr, 4);
      infile.close();
      bool exeHeader(false);
#ifdef _WIN32
      exeHeader = (hdr[0] == 'M' && hdr[1] == 'Z');
#else
      // Executable and link format
      exeHeader = (hdr[0] == 127 && hdr[1] == 'E' && hdr[2] == 'L' && hdr[3] == 'F');
#endif
      return exeHeader;
   }
   return false;
}

namespace
{
std::string GetMonth(std::tm& aTime)
{
   switch (aTime.tm_mon)
   {
   case 0:
      return "Jan";
   case 1:
      return "Feb";
   case 2:
      return "Mar";
   case 3:
      return "Apr";
   case 4:
      return "May";
   case 5:
      return "Jun";
   case 6:
      return "Jul";
   case 7:
      return "Aug";
   case 8:
      return "Sep";
   case 9:
      return "Oct";
   case 10:
      return "Nov";
   case 11:
      return "Dec";
   }
   return "???";
}

std::string LookupHeaderMacro(const std::string& aMacro, const UtPath& aPath)
{
   std::time_t rawTimeNow = std::time(nullptr);
   std::tm*    timeNow    = std::localtime(&rawTimeNow);
   if (aMacro.empty())
   {
      return {cMACRO_DELIM};
   }
   else if (aMacro == "YEAR")
   {
      return std::to_string(1900 + timeNow->tm_year);
   }
   else if (aMacro == "MONTH")
   {
      return GetMonth(*timeNow);
   }
   else if (aMacro == "DAY")
   {
      return std::to_string(timeNow->tm_mday);
   }
   else if (aMacro == "DATE")
   {
      return GetMonth(*timeNow) + " " + std::to_string(timeNow->tm_mday) + ", " + std::to_string(1900 + timeNow->tm_year);
   }
   else if (aMacro == "VERSION")
   {
      return wizard::RunEnvManager::GetSimpleVersionString();
   }
   else if (aMacro == "FILE")
   {
      return aPath.GetFileName();
   }
   else if (aMacro == "DIR")
   {
      UtPath dir = aPath;
      dir.Up();
      return dir.GetSystemPath();
   }
   else if (aMacro == "PATH")
   {
      return aPath.GetSystemPath();
   }
   return cMACRO_DELIM + aMacro + cMACRO_DELIM;
}
} // namespace

// Divide a command-line string into pieces using quotes and spaces
// ex:    'quoted strings' "with either type are joined together" but spaces divide
//   returns:  ["quoted strings", "with either type are joined together", "but", "spaces", "divide"]
QStringList SplitByQuotes(const QString& aCommandLine)
{
   QStringList words;
   int         state    = 0;
   char        endQuote = 0;
   QString     word;
   for (int i = 0; i <= aCommandLine.size(); ++i)
   {
      QChar c = i < aCommandLine.size() ? aCommandLine[i] : '\0';
      if (state == 0)
      {
         switch (c.toLatin1())
         {
         case '\'':
            endQuote = '\'';
            state    = 1;
            break;
         case '"':
            endQuote = '"';
            state    = 1;
            break;
         case ' ':
            if (!word.isEmpty())
               words += word;
            word.clear();
            break;
         case '\0':
            words += word;
            break;
         case '\\':
            state = 2;
            break;
         default:
            word += c;
            break;
         }
      }
      else if (state == 1)
      {
         if (c == endQuote)
         {
            words += word;
            state = 0;
            word.clear();
         }
         else
            word += c;
      }
      else if (state == 2) // allow escaping spaces only
      {
         if (c == ' ')
         {
            word += ' ';
         }
         else if (c == '\0')
         {
            word += '\\';
         }
         else
         {
            word += '\\';
            word += c;
         }
         state = 0;
      }
   }
   return words;
}

// Splits a command line string
//  ie:    -option "hi you" -option2 'abc $(VAR1) def'
// -> {"-option", "hi you", "-option2", "abc MyVar1Value def"}
//  Quote escapes aren't handled yet
// Note: It is expected that a shell will not be processing these arguments, therefore quotes are removed
QStringList wizard::Util::SplitCommandLine(const QString& aCommandLine, const QMap<QString, QString>& aVariables)
{
   QString expanded = aCommandLine;
   // Now apply variable substitution
   // expand variables
   QRegExp varRe("\\$\\((\\w+)\\)");
   {
      bool foundMatches = true;
      // Right now, it's possible to reference variables inside other variables.
      // if $(x)="$(y)" and $(y)="$(x)", we could do this forever.
      // Limit number of substitutions
      const int cSUBST_LIMIT = 500;
      for (int i = 0; i < cSUBST_LIMIT && foundMatches; ++i)
      {
         foundMatches = false;
         if (expanded.contains(varRe))
         {
            QString varName = varRe.cap(1);
            if (aVariables.contains(varName))
            {
               QString varValue = aVariables[varName];
               expanded         = expanded.replace(varRe.cap(), varValue);
               foundMatches     = true;
            }
         }
      }
   }

   QStringList args = SplitByQuotes(expanded);

   // remove any empty strings
   while (args.contains(""))
   {
      args.removeOne("");
   }

   return args;
}

// checks if a file is probably plain ascii
bool wizard::Util::IsNonTextFile(const QString& aPath)
{
   if (!QFile::exists(aPath))
      return true;
   QFile f(aPath);
   f.open(QFile::ReadOnly);
   QByteArray bytes = f.read(100000);
   for (int i = 0; i < bytes.size(); ++i)
   {
      unsigned char byte = (unsigned char)bytes[(int)i];
      // Some files use a few extended ascii characters (like the degrees symbol)
      // so we will allow extended ascii
      // if (byte > 127)
      //{
      //   return true;
      //}
      // null before EOF
      if (byte == 0 && i + 1 < bytes.size())
         return true;
      // if (byte == 0 && i + 1 < bytes.size()) return true; // null char not at end
   }
   if (f.size() > 100 * 1000000)
   {
      return true; // very large file
   }
   return false;
}

int wizard::Util::FilesInDirectory(QDir& aDir, int aStopAt /*= 10000*/)
{
   int fileCount = aDir.count();
   if (fileCount > aStopAt)
      return fileCount;

   QStringList subdirs = aDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

   for (int i = 0; i < subdirs.size() && fileCount < aStopAt; ++i)
   {
      aDir.cd(subdirs[i]);
      fileCount += FilesInDirectory(aDir, aStopAt - fileCount);
      aDir.cdUp();
   }
   return fileCount;
}

std::string wizard::Util::MakeTempFileName()
{
   QDir    tmpDir  = QDir::tempPath();
   QString uid     = QUuid::createUuid().toString().replace("{", "").replace("}", "").replace("-", "");
   QString tmpPath = tmpDir.filePath(QString("afide") + uid);
   return tmpPath.toStdString();
}

bool wizard::Util::CreateNewFile(const UtPath& aName)
{
   auto* newFilePreferences = wkfEnv.GetPreferenceObject<NewFilePrefObject>();
   bool  retval             = false;

   if (newFilePreferences != nullptr)
   {
      const int count = newFilePreferences->GetHeadingCount();

      const HeadingData* heading = nullptr;

      if (count == 1)
      {
         // Get pointer from iterator
         heading = newFilePreferences->First();
      }
      else if (count > 1)
      {
         QDialog                   dialog;
         Ui::HeaderSelectionDialog ui;
         ui.setupUi(&dialog);

         for (const auto& heading : newFilePreferences->Data())
         {
            ui.comboBox->addItem(QString::fromStdString(heading.mName));
         }

         QString selection;
         bool    accepted = false;

         QObject::connect(&dialog,
                          &QDialog::accepted,
                          [&]()
                          {
                             selection = ui.comboBox->currentText();
                             accepted  = true;
                          });

         dialog.exec();

         if (accepted)
         {
            heading = newFilePreferences->GetHeading(selection.toStdString());
         }
      }

      if (count == 0 || heading != nullptr)
      {
         std::ofstream file(aName.GetSystemPath());
         if (file.is_open())
         {
            retval = true;

            if (heading != nullptr)
            {
               std::string macro;
               bool        inMacro = false;
               bool        newLine = true;

               for (char c : heading->mText)
               {
                  if (newLine)
                  {
                     newLine = false;
                     if (heading->mEnableComment)
                     {
                        file << "# ";
                     }
                  }

                  switch (c)
                  {
                  case '\n':
                     if (inMacro)
                     {
                        inMacro = false;
                        file << cMACRO_DELIM << macro;
                     }
                     file << '\n';
                     newLine = true;
                     break;
                  // Macros cannot contain whitespace
                  case ' ':
                  case '\t':
                     if (inMacro)
                     {
                        inMacro = false;
                        file << cMACRO_DELIM << macro;
                     }
                     file << c;
                     break;
                  case cMACRO_DELIM:
                     if (inMacro)
                     {
                        inMacro = false;
                        file << LookupHeaderMacro(macro, aName);
                        macro.clear();
                     }
                     else if (heading->mEnableMacros)
                     {
                        inMacro = true;
                     }
                     else
                     {
                        file << cMACRO_DELIM;
                     }
                     break;
                  default:
                     if (inMacro)
                     {
                        macro += c;
                     }
                     else
                     {
                        file << c;
                     }
                  }
               }
               if (inMacro)
               {
                  file << cMACRO_DELIM << macro;
               }
               if (!newLine)
               {
                  file << '\n';
               }
            }
         }
      }
   }

   if (retval)
   {
      auto* project = wizard::Project::Instance();
      if (project != nullptr)
      {
         wizSignals->CheckFilesForModification();
         project->OpenFile(aName.GetSystemPath());
      }
   }

   return retval;
}

QString wizard::Util::QuotePathWithSpaces(const QString& aPath)
{
   if (!aPath.startsWith("\"") && aPath.indexOf(' ') != -1)
   {
      return '"' + aPath + '"';
   }
   return aPath;
}

QString wizard::Util::StripHtml(const QString& htmlStr)
{
   QTextDocument htmlConverter;

   htmlConverter.setHtml(htmlStr);

   return htmlConverter.toPlainText();
}
// Given this text:     "100 target"
//  will produce this:  ".*100.*target.*"
//  to find any of these: 100_target, 1000___target, 091312100583_jqlkrjasdtargetaweoiuasldf
QRegExp wizard::Util::SimpleRegex(const QString& aText)
{
   QString     regexStr;
   QStringList words = aText.split(" ");
   for (int i = 0; i < words.size(); ++i)
   {
      regexStr += ".*";
      regexStr += QRegExp::escape(words[i]);
   }
   regexStr += ".*";
   return QRegExp(regexStr, Qt::CaseInsensitive);
}

QString wizard::Util::BuildFileLineColURL(const QString& aFilePath, int aLine, int aColumn, const QString& aLabel)
{
   QString lbl = aLabel;
   if (lbl.isEmpty())
   {
      lbl = aFilePath;
   }

   QUrl      url = QUrl::fromLocalFile(aFilePath);
   QUrlQuery query(url);
   query.addQueryItem("line", QString::number(aLine));
   if (aColumn > 0)
   {
      query.addQueryItem("col", QString::number(aColumn));
   }
   url.setQuery(query);
   return BuildHRef(url.toString(), lbl);
}

QString wizard::Util::BuildHRef(const QString& aUrl, const QString& aLabel)
{
   QString urlStr = "<a href=\"$url\">$path</a>";
   urlStr.replace("$url", aUrl);
   urlStr.replace("$path", aLabel);
   return urlStr;
}
