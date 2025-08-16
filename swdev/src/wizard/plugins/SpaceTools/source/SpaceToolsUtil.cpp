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

#include "SpaceToolsUtil.hpp"

#include <fstream>
#include <sstream>

#include "UtPath.hpp"

QJsonDocument SpaceTools::Util::GetDatabase(const QString& aPath)
{
   UtPath        path(aPath.toStdString());
   QJsonDocument database{};
   if (path.IsFile() && path.GetFileExtension() == ".json")
   {
      std::ifstream     inFile{path.GetSystemPath()};
      std::stringstream buffer{};

      if (inFile.is_open())
      {
         buffer << inFile.rdbuf();
         database = QJsonDocument::fromJson(QByteArray::fromStdString(buffer.str()));
      }
   }
   return database;
}
