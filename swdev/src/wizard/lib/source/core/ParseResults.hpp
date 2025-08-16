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

#ifndef PARSERESULTS_HPP
#define PARSERESULTS_HPP

#include <string>
#include <vector>

#include "ProxyMerge.hpp"
#include "UtTextDocument.hpp"
#include "ViExport.hpp"
#include "WsfPProxyNode.hpp"
#include "WsfPProxyRegistry.hpp"
#include "WsfPProxyType.hpp"

class WsfPProxy;
class WsfPProxyIndex;
class WsfParseError;
class WsfParseIndex;
class WsfParseNode;
class WsfParseSourceInclude;
class WsfParser;

namespace wizard
{
class ParseResults;
class ParseWorker;
class Project;
class TextSource;
class WsfScriptData;
class WsfScriptTypes;

class VI_EXPORT ParseResultsProxyData
{
public:
   explicit ParseResultsProxyData()
      : mProxyHashPtr(nullptr)
      , mProxy(nullptr)
   {
   }

   ParseResultsProxyData(const ParseResultsProxyData&) = delete;
   ParseResultsProxyData(ParseResultsProxyData&&)      = default;
   ParseResultsProxyData& operator=(const ParseResultsProxyData&) = delete;
   ParseResultsProxyData& operator=(ParseResultsProxyData&&) = default;

   virtual ~ParseResultsProxyData() = default;
   // Swap ownership of proxy data
   void SwapProxy(ParseResultsProxyData& aOtherParseResults, bool aSwapProxyPointer);

   void GiveProxy(std::unique_ptr<WsfPProxy> aProxy, std::unique_ptr<ProxyHash> aProxyHashPtr);

   // Given a proxy path, finds other proxy objects which are related;  Does not traverse to the base type.
   // For instance, here 'rdr-1' could be copied across of of these locations, accumulating changes on the way:
   // platform.P1.sensor.rdr-1
   // platformType.PT1.sensor.rdr-1
   // platformType.PT2.sensor.rdr-1
   // ----------------------
   // sensorType.RADAR_TYPE
   //
   // This routine returns the first 3 paths.  The last path is the type which can be found by other means.
   void FindProxySourcePaths(const WsfPProxyPath& aPath, std::vector<WsfPProxyPath>& aSources);

   UtTextDocumentRange FindDefinitionLocation(const WsfPProxyPath& aPath);

   //! Finds all the references for an AFSIM input block corresponding to a particular proxy path
   //! @param aPath is the proxy path
   //! @param aLocations are the references of the AFSIM block
   void FindModificationLocations(const WsfPProxyPath& aPath, std::vector<UtTextDocumentRange>& aLocations);

   //! Finds all the references for the start of a AFSIM input block corresponding to a particular proxy path
   //! @param aPath is the proxy path
   //! @param aLocations are the references of the start of the AFSIM block
   void FindDefinitionStartLocations(const WsfPProxyPath& aPath, std::vector<UtTextDocumentRange>& aLocations);

   //! Finds all the references for an AFSIM input block corresponding to a particular proxy path
   //! @param aPath is the proxy path
   //! @param aLocations are the references of the AFSIM block
   //! @note use this if you need to find references in base types as well as the derived type (corresponding to aPath)
   void FindModificationLocationsExtended(const WsfPProxyPath& aPath, std::vector<UtTextDocumentRange>& aLocations);

   //! Finds all the references for the start of a AFSIM input block corresponding to a particular proxy path
   //! @param aPath is the proxy path
   //! @param aLocations are the references of the start of the AFSIM block
   //! @note use this if you need to find references in base types as well as the derived type (corresponding to aPath)
   void FindDefinitionStartLocationsExtended(const WsfPProxyPath& aPath, std::vector<UtTextDocumentRange>& aLocations);

   void FindDerivedTypes(const WsfPProxyPath& aTypePath, std::vector<WsfPProxyPath>& aDerivedTypePaths);

   WsfPProxy*           Proxy() const { return mProxy.get(); }
   WsfPProxyNode        ProxyRootNode() const { return WsfPProxyNode::FromProxy(mProxy.get()); }
   WsfPProxyStructValue ProxyRoot() const { return mProxy ? mProxy->mRoot : WsfPProxyStructValue(); }
   const std::shared_ptr<WsfPProxyRegistry>& ProxyRegistry() { return mProxy->GetRegistry(); }
   ProxyHash*                                GetProxyHash() { return mProxyHashPtr.get(); }
   WsfPProxyIndex*                           ProxyIndex() { return mProxy ? mProxy->mIndex.get() : nullptr; }
   WsfPProxyPathMap*                         ProxyPathMap() { return mProxy ? mProxy->mPathMap.get() : nullptr; }
   bool                                      isProxyDataOld() const { return !mProxy || !mProxy->mIndex; }

protected:
   std::unique_ptr<ProxyHash> mProxyHashPtr;
   std::unique_ptr<WsfPProxy> mProxy;
};

typedef std::map<UtTextDocument*, WsfParseSourceInclude*> ParseIncludes;
typedef std::vector<UtStringRef>                          ParseIncludePaths;

//! Defines a subset of parse results which stay valid while computing the new parse results
class VI_EXPORT ParseResultsTiny : public ParseResultsProxyData
{
public:
   explicit ParseResultsTiny(Project* aProjectPtr);
   ~ParseResultsTiny() override;

   void FreeScriptTypes();

   void TakeResults(ParseResults& aResults);

   bool IncludesFile(TextSource* aSourcePtr) const;

   bool FindBestRelativePath(UtPath aFullPath, std::string& aRelativeIncludePath);

   // The last modification sequence number that was applied before these results were computed
   long long mSequenceNumber;
   Project*  mProjectPtr;

   // note: the WsfParseSourceInclude values should not be accessed.
   //  this map should only be used to determine if a file is included.
   ParseIncludes     mFirstIncludes;
   ParseIncludePaths mIncludePaths;

   WsfScriptData*  mScriptDataPtr;
   WsfScriptTypes* mScriptTypesPtr;
};


//! Provides access to the results of parsing the WSF inputs.
class VI_EXPORT ParseResults : public ParseResultsTiny
{
public:
   friend class ParseWorker;

   explicit ParseResults(Project* aProjectPtr);
   ~ParseResults() override;
   void Clear();

   void AssignResults(Project*                                           aProjectPtr,
                      WsfParser*                                         aParserPtr,
                      const WsfParseSourceInclude*                       aRootInclude,
                      WsfParseNode*                                      aParseTreePtr,
                      WsfParseNode*                                      aLastNodePtr,
                      std::map<UtTextDocument*, WsfParseSourceInclude*>& aFirstIncludes);

   WsfParseSourceInclude* FindSourceData(UtTextDocument* aSourcePtr);
   bool                   GetRangeNodes(const UtTextDocumentRange& aRange, std::vector<WsfParseNode*>& aNodes);

   bool          FindIncludeAtPoint(UtTextDocument*         aSourcePtr,
                                    const UtTextRange&      aLocation,
                                    WsfParseSourceInclude*& aSourceIncludePtr,
                                    int&                    aEntryIndex,
                                    TextSource*&            aIncludeSourcePtr);
   WsfParseNode* FindLastParseNode(const UtTextDocumentRange& aPosition);

   WsfParseNode* FindBlock(WsfParseNode* aNodePtr);

   void FindSubcommands(WsfParseNode* aNodePtr, std::vector<std::string>& aCommands);

   void FindDefinitions(const std::string& aName, const std::string& aDefinitionKind, std::vector<WsfParseNode*>& aNodes);

   void FindReferences(const std::string& aName, const std::string& aDefinitionKind, std::vector<WsfParseNode*>& aNodes);

   void FindDefinitionsOfType(const WsfParseTypePath& aPath, std::vector<WsfParseNode*>& aNodes, bool aAllowNested = false);
   void FindDefinitionsOfType(const std::string&          aDefinitionType,
                              std::vector<WsfParseNode*>& aNodes,
                              bool                        aAllowNested = false);

   void FindNamedNodes(const std::string& aCategory, const std::string& aName, std::vector<WsfParseNode*>& aNodes);
   void GetBaseTypes(const WsfParseTypePath& aKind, std::vector<std::string>& aBaseTypeNames, bool aIncludeUserTypes = true);

   //! Finds the names of all definitions of a given type.
   //! @param aNameType The definition type to find the names of.
   //! @param aNameList An out-parameter containing the names of the discovered names.
   //!                  Found names are added to this parameter.
   void FindNames(const std::string& aNameType, std::vector<std::string>& aNameList);

   void FindIncludes(TextSource* aSourcePtr, std::vector<UtTextDocumentRange>& aIncludes);

   void FindReferencedFiles(std::vector<WsfParseNode*>& aFileReferenceNodes);

   // aEntryTypeRef is of type WsfParseIndex::EntryType
   WsfPProxyPath FindProxyPathFromLocation(const UtTextDocumentRange& aLocation, int& aEntryTypeRef);

   WsfParseType* FindCurrentType(WsfParseNode* aNodePtr);

   WsfParseIndex* GetParseIndex();

   enum FileNotificationType
   {
      // indicates there is an error (usually about a required type not being found)
      cERROR_BIT = 1 << 29,
      // indicates there is a warning (usually about a type not being found)
      cWARNING_BIT = 1 << 28,
      // indicates there is some information to be had
      // this is not set for things like syntax errors with unknown causes
      cINFORMATION_BIT = 1 << 27,
      // The notification type bitwise-and'd with this is the notification 'kind'
      cKIND_MASK = 0xffff,
      // Information about a type definition
      cTYPE_KIND = 1,
      // Information about an include
      cINCLUDE_KIND = 2,
      // Information about a type reference
      cREFERENCE_KIND = 3,
      // Error from the parse phase -- unmatched text
      cSYNTAX_ERROR_KIND = 4
   };

   struct FileNotification
   {
      bool operator<(FileNotification& aRhs) const { return mRange.GetBegin() < aRhs.mRange.GetBegin(); }
      FileNotification(const UtTextRange& aRange, unsigned int aNotificationType)
         : mRange(aRange)
         , mNotificationType(aNotificationType)
      {
      }
      bool         IsValid() const { return mNotificationType != 0; }
      int          GetNotificationKind() const { return mNotificationType & cKIND_MASK; }
      UtTextRange  mRange;
      unsigned int mNotificationType;
   };
   using FileNotificationList = std::vector<FileNotification>;
   FileNotificationList* GetFileNotifications(TextSource* aSourcePtr);
   FileNotification      GetNotification(const UtTextDocumentLocation& aLocation);


   //! Stores information about a 'type' at a more abstract level
   //! allowing references to types even if they are predefined or 'lazily' defined.
   struct TypeInformation
   {
      enum TypeFlags
      {
         //! Is it a built-in type like WSF_PLATFORM
         cBASIC_TYPE = 1,
         //! Is it a 'lazy' type which has no entry in the symbol table -- like antenna patterns
         cLAZY_TYPE = 2,
         //! Did the parser fail to find the type?
         cTYPE_LOOKUP_FAIL = 4,
         //! Is the name invalid because of duplication
         cDUPLICATE_NAME = 8
      };
      TypeInformation()
         : mParent(nullptr)
         , mType(nullptr)
         , mFlags(0)
      {
      }
      bool             IsValid() const { return (mFlags & cTYPE_LOOKUP_FAIL) == 0; }
      WsfParseType*    mParent;
      WsfParseTypePath mTypeKey;
      WsfParseType*    mType; // may be empty

      int mFlags;
   };

   bool FindTypeInfo(TypeInformation& aInformation);
   bool FindUserTypeDocumentation(TypeInformation& aType, std::string& aDocumentation);

   bool          TypeInformationFromPosition(const UtTextDocumentLocation& aLocation, TypeInformation& aInformation);
   bool          TypeInformationFromNode(WsfParseNode* aNodePtr, TypeInformation& aInformation);
   bool          FindBaseType(const TypeInformation& aType, TypeInformation& aBaseType);
   WsfParseNode* FindCreateTypeNode(const TypeInformation& aType);
   WsfParseNode* FindTypeDefinitionNode(const TypeInformation& aType);
   std::vector<WsfParseNode*> FindTypeReferenceNodes(const TypeInformation& aType);

   static WsfPProxyNode FindProxyReference(WsfPProxyNode aContext, WsfParseTypePath aPath);

   bool TypeKeyMatches(const WsfParseTypePath& aQuery, const WsfParseTypePath& aTest)
   {
      if (aQuery.size() >= 2 && aTest.size() >= 2)
      {
         if (aQuery[1] == aTest[1])
         {
            if (aQuery[0] == "platform_part")
            {
               for (auto&& it : mPlatformPartKinds)
               {
                  if (it == aTest[0])
                  {
                     return true;
                  }
               }
               return false;
            }
            return aQuery[0] == aTest[0];
         }
      }
      return false;
   }


   WsfParser*                         parser() const { return mParserPtr; }
   const std::vector<WsfParseError*>& errors() const { return mErrors; }
   WsfParseNode*                      parseTree() const { return mParseTreePtr; }
   WsfParseNode*                      lastNode() const { return mLastNodePtr; }
   const WsfParseSourceInclude*       rootInclude() const { return mRootInclude; }

   //! List of leaf nodes which mark the switching of files, in the order of parse tree traversal.
   //! Used to quickly search the nodes in a file.
   std::vector<WsfParseNode*> mFileTransitionNodes;

protected:
   WsfParser* mParserPtr;

   std::vector<WsfParseError*> mErrors;

   WsfParseNode* mParseTreePtr;

   //! Due to the linked-list nature of the tree, finding the last node is O(Root Commands in all files)
   //! Store the last node for quick access.
   WsfParseNode* mLastNodePtr;

   const WsfParseSourceInclude* mRootInclude;

   std::vector<UtStringRef> mPlatformPartKinds;

   void IndexNames(); // WIZARD_TODO

   void FindSubcommandsP(WsfParseRule* aReaderPtr, std::vector<std::string>& aCommands);

   typedef std::map<UtTextDocument*, FileNotificationList> FileNotificationMap;
   FileNotificationMap                                     mFileNotifications;
   std::set<WsfParseTypePath>                              mTypeNames;
   bool                                                    mTypeNamesIndexed;
};
} // namespace wizard

#endif
