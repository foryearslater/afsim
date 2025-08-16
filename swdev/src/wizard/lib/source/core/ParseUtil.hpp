// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2016 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef PARSEUTIL_HPP
#define PARSEUTIL_HPP

#include "ViExport.hpp"
class WsfParseNode;
#include "WsfParseTypePath.hpp"

namespace wizard
{
namespace ParseUtil
{

bool VI_EXPORT FindReferenceType(WsfParseNode* aNodePtr, WsfParseTypePath& aTypeKey, bool& aNested);

bool VI_EXPORT FindTypeNode(WsfParseNode* aNodePtr, WsfParseNode*& aTypeNodePtr);

//! Convert a WsfParseNode's flags into a string list
//! @param aNode is the WsfParseNode whose flags to convert to a string list
//! @return a string list representing the flags of the WsfParseNode
std::vector<std::string> VI_EXPORT GetFlagStringList(const WsfParseNode* aNode) noexcept;

namespace Context
{

//! Represents the context type
enum class Type
{
   NotApplicable, //!< Context does not apply to this block
   Explicit,      //!< This block has explicit context
   Implicit       //!< This block has implicit context
                  //! @see BlockContext::GetContext
};

//! Context (or scope) for a generic block of AFSIM input
//! @par AFSIM input context
//!   Every command in AFSIM input has context (or scope).
//!   The context of a command depends on where it is used.
//!   For the most part, the context of AFSIM input commands is explicit.
//!   For example,
//!   @code
//!     platform
//!       position 0n 0e
//!     end_platform
//!   @endcode
//!   position has explicit context inside the platform block.
//!   However, some commands have implicit context, meaning that they can be defined outside of the explicit context.
//!   For example,
//!   @code
//!     platform
//!       route
//!         position 0n 0e
//!       end_route
//!     end_platform
//!   @endcode
//!   route has implicit context within a mover block, since a route is defined on the mover, not the platform.
struct VI_EXPORT BlockContext
{
   BlockContext() = default;
   BlockContext(Type aContextType)
      : mType(std::move(aContextType))
   {
   }

   //! Determines the context of a WsfParseNode
   //! @param aNode is the node of which to determine the block context
   //! @pre The WsfParseNode needs the cBLOCK_NODE flag set
   //! @note Since implicit block context depends on the AFSIM input block, we cannot determine implicit context here.
   //! @note There are multiple ways of finding the block context, but the easiest way is to start from a "block_node".
   static BlockContext GetContext(WsfParseNode* aNode) noexcept;

   //! The type of context
   const Type mType{Type::NotApplicable};
};

//! Context (or scope) for a block defining a mover
struct VI_EXPORT MoverContext : BlockContext
{
   MoverContext() = default;
   MoverContext(Type aContextType)
      : BlockContext(std::move(aContextType))
   {
   }

   //! Determines the context of a WsfParseNode
   static MoverContext GetContext(WsfParseNode* aNode) noexcept;
};

} // namespace Context
} // namespace ParseUtil
} // namespace wizard

#endif
