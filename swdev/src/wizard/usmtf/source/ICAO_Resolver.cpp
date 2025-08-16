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

#include "ICAO_Resolver.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <unordered_map>

#include "UtLLPos.hpp"
#include "UtPath.hpp"
#include "UtRunEnvManager.hpp"

namespace usmtf
{
ICAO_Resolver::ICAO_Resolver()
{
   LoadCodeMap();
}

std::string ICAO_Resolver::LookUpCode(const std::string& aIcaoCode) const noexcept
{
   auto it = mCodes.find(aIcaoCode);
   if (it != mCodes.end())
   {
      return it->second;
   }
   return std::string();
}

void ICAO_Resolver::LoadCodeMap(bool aForceReload) noexcept
{
   if (mHasLoadedCodes && !aForceReload)
   {
      return;
   }
   mCodes.clear();

   UtPath utPath = UtRunEnvManager::GetRunPath();
   utPath.Up();
   UtPath      icaoPath = UtPath{utPath.GetNormalizedPath() + "/resources/data/usmtf/icao.csv"};
   std::string path;
   if (icaoPath.Exists()) // installed
   {
      path = icaoPath.GetNormalizedPath();
   }
   else // development
   {
      ReadConf();
      path = mConf["Path"] + "/icao.csv";
   }
   std::ifstream icaoFile(path, std::ifstream::in);
   std::string   line{}, value{}, icao{}, lat{}, lon{};
   while (std::getline(icaoFile, line))
   {
      std::stringstream ss{line};
      std::string       row[3];
      for (int i = 0; getline(ss, value, ','); i++)
      {
         row[i] = value;
      }
      icao = row[0];
      lat  = row[1];
      lon  = row[2];
      UtLLPos           ll{std::stod(lat), std::stod(lon)};
      std::stringstream temp{};
      temp << ll;
      mCodes[icao] = temp.str();
   }
   mHasLoadedCodes = true;
}

std::unordered_map<std::string, std::string> ICAO_Resolver::ReadConf() const noexcept
{
   if (mHasLoadedConf)
   {
      return mConf;
   }
   UtPath        utPath = UtRunEnvManager::GetRunPath();
   std::string   file   = utPath.GetNormalizedPath() + "/usmtf.conf";
   std::ifstream conf(file);
   std::string   line;
   std::string   value{};
   std::getline(conf, line);
   std::stringstream ss{line};
   std::string       row[2];
   for (int i = 0; getline(ss, value, '='); i++)
   {
      row[i] = value;
   }
   std::string key = row[0];
   value           = row[1];
   mConf[key]      = value;
   mHasLoadedConf  = true;
   return mConf;
}

bool                                         ICAO_Resolver::mHasLoadedCodes = false;
bool                                         ICAO_Resolver::mHasLoadedConf  = false;
std::unordered_map<std::string, std::string> ICAO_Resolver::mCodes;
std::unordered_map<std::string, std::string> ICAO_Resolver::mConf;
} // namespace usmtf
