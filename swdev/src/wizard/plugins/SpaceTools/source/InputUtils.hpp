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

#ifndef INPUTEDITUTILS_HPP
#define INPUTEDITUTILS_HPP

#include <string>
#include <utility>
#include <vector>

#include "UtTextDocument.hpp"
#include "WsfPM_Platform.hpp"

namespace SpaceTools
{

namespace InputUtils
{

std::pair<size_t, size_t> FindNextComment(const std::string& aText, size_t aStartPos);
std::string               HandleComments(const std::string& aText);

std::vector<std::pair<UtTextDocumentRange, size_t>> GetBlockRanges(const std::string& aBlock,
                                                                   const std::string& aPlatformName);

WsfPM_Platform GetPlatformProxy(const std::string& aPlatformName);

} // namespace InputUtils

} // namespace SpaceTools

#endif // INPUTEDITUTILS_HPP
