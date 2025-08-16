// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2021 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef PROXY_UTIL_HPP
#define PROXY_UTIL_HPP

#include <string>
#include <vector>

// Wizard includes
#include "ViExport.hpp"

// Util includes
class UtTextDocumentRange;

// Wsf includes
class WsfParseNode;
class WsfPProxyNode;
struct WsfPProxyIndexEntry;
class WsfPM_Zone;

namespace wizard
{
namespace ProxyUtil
{

//! Given a WsfPProxyIndexEntry, find the WsfParseNode representing the start of the respective AFSIM input block
//! @param aEntry is the proxy entry
//! @return the parse node representing the start of the respective AFSIM input block
WsfParseNode VI_EXPORT* FindBlockStartNodeFromEntry(const WsfPProxyIndexEntry& aEntry) noexcept;

//! Given a WsfPProxyIndexEntry, find the WsfParseNode representing the end of the respective AFSIM input block
//! @param aEntry is the proxy entry
//! @return the parse node representing the end of the respective AFSIM input block
WsfParseNode VI_EXPORT* FindBlockEndNodeFromEntry(const WsfPProxyIndexEntry& aEntry) noexcept;

//! Finds locations in the parse tree representing the start of an corresponding to entries in the proxy index tree
//! @param aEntries is the list of entries from the proxy index tree
//! @param aLocations is the list of locations in the parse tree
void VI_EXPORT FindBlockStartLocations(const std::vector<WsfPProxyIndexEntry>& aEntries,
                                       std::vector<UtTextDocumentRange>&       aLocations) noexcept;

//! Finds locations in the parse tree representing the end of an corresponding to entries in the proxy index tree
//! @param aEntries is the list of entries from the proxy index tree
//! @param aLocations is the list of locations in the parse tree
void VI_EXPORT FindBlockEndLocations(const std::vector<WsfPProxyIndexEntry>& aEntries,
                                     std::vector<UtTextDocumentRange>&       aLocations) noexcept;

//! Finds locations in the parse tree representing an AFSIM input block corresponding to entries in the proxy index tree
//! @param aEntries is the list of entries from the proxy index tree
//! @param aLocations is the list of locations in the parse tree
void VI_EXPORT FindBlockLocations(const std::vector<WsfPProxyIndexEntry>& aEntries,
                                  std::vector<UtTextDocumentRange>&       aLocations) noexcept;

//! Finds the definition of a zone or zone set given its name.
//! @param aZoneName is the name of the zone to look up
//! @param aParentPlatName is the name of a parent platform to check in addition to the global zone namespace
WsfPM_Zone VI_EXPORT FindZoneNodeByName(const std::string& aZoneName,
                                        const std::string& aParentPlatName = "Global Zones") noexcept;

//! Marks the range pointed to by a proxy node as invalid
//! @param aProxyNode is the proxy node whose range is to become invalid
//! @pre the range is syntax-highlighted as valid
//! @post the range is syntax-highlighted as an error (red text)
void VI_EXPORT InvalidateInput(const WsfPProxyNode& aProxyNode) noexcept;

} // namespace ProxyUtil
} // namespace wizard

#endif // PROXY_UTIL_HPP
