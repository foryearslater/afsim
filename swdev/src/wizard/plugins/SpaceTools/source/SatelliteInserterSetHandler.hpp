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

#ifndef SATELLITEINSERTERSETHANDLER_HPP
#define SATELLITEINSERTERSETHANDLER_HPP

#include <map>
#include <string>
#include <tuple>

#include <QString>
#include <QStringList>

#include "UtPath.hpp"
#include "UtTextDocument.hpp"
#include "UtTwoLineElement.hpp"

namespace SpaceTools
{

class SatelliteInserterSetHandler
{
public:
   explicit SatelliteInserterSetHandler(const QStringList& aFileList);

   ~SatelliteInserterSetHandler() = default;

   void UpdateTLEs();

private:
   static QStringList SortByLatestStartingPosition(const QStringList& aSpacePlatformsList);
   void               UpdatePlatformTLE(const std::string& aDesignator, const QString& aPlatformName);
   static std::pair<UtTextDocumentRange, size_t> GetSourceRange(const std::string& aPlatformName);
   std::string                                   GetReplacementText(const UtTwoLineElement& aTLE, size_t aColumn);
   static void ReplaceText(const UtTextDocumentRange& aRange, const std::string& aText);

   void LoadPatchFile(const std::string& aFileName);

   std::map<std::string, UtTwoLineElement> mUpdatedTLEs{};
};

} // namespace SpaceTools

#endif // SATELLITEINSERTERSETHANDLER
