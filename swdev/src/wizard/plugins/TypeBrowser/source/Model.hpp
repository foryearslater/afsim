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

#ifndef MODEL_HPP
#define MODEL_HPP

#include <QTimer>

#include "AbstractModel.hpp"
#include "WsfPProxyNode.hpp"

class WsfPProxy;

namespace wizard
{
class Project;
class ProxyChange;
} // namespace wizard

namespace TypeBrowser
{
class Item;

class Model : public wizard::AbstractModel
{
   Q_OBJECT
   friend class Item;

public:
   Model();
   ~Model() override;

   QVariant   data(const QModelIndex& index, int role) const override;
   bool       setData(const QModelIndex& index, const QVariant& value, int role) override { return false; }
   QMimeData* mimeData(const QModelIndexList& indexes) const override;

   Qt::ItemFlags flags(const QModelIndex& index) const override;


   enum
   {
      cPLATFORM,
      cPROCESSOR,
      cWEAPON,
      cCOMM,
      cSENSOR,
      cMOVER,
      cFUEL,
      cWEAPON_EFFECT,
      cROUTE_NETWORK,
      cROUTE,
      cAERO,
      cZONE,
      cANTENNA_PATTERNS,
      cACOUSTIC_SIGNATURES,
      cINFRARED_SIGNATURES,
      cOPTICAL_SIGNATURES,
      cRADAR_SIGNATURES,
      cINHERENT_CONTRAST,
      cMASKING_PATTERNS,
      cREFLECTIVITY,
      cEW_EFFECT,
      cEA_TECHNIQUE,
      cEP_TECHNIQUE,
      cELECTRONIC_ATTACK,
      cELECTRONIC_PROTECT,
      cTYPE_KIND_COUNT
   };
   QVariant ChooseIcon(int aLookup) const;

   Item* GetRootItem() { return mRootItem; }

   bool ShowUnusedTypes() const { return mShowUnusedTypes; }
   void SetShowUnusedTypes(bool aVal);
   void Hibernate(bool aHibernate = true);
private slots:
   void DeferredUpdate();

private:
   void OnProxyTypeChange(const wizard::ProxyChange&);
   void ProxyAvailable(WsfPProxy* aProxyPtr);

   bool TypeHasBuiltins(int aType) { return aType <= cWEAPON_EFFECT || aType == cEW_EFFECT; }
   struct TypeSource
   {
      int                  mTypeKind;
      WsfPProxyNode        mNode;
      const WsfPProxyPath* mBasePathPtr;
      bool                 mIsCoreType;
   };
   void  ParseUpdateDeferred(wizard::Project* aProject, bool aFull);
   void  ScanForTypes(const WsfPProxyNode&               aRoot,
                      std::map<std::string, TypeSource>& aTypeRoots,
                      int                                aScenarioIndex,
                      bool                               aScanBasicTypes);
   Item* RealizeType(const std::string& aName, std::map<std::string, TypeSource>& aTypeSources);

   static std::string KindNameFromPath(const WsfPProxyStructValue& aRoot, const WsfPProxyPath& aPath);
   Item*              GetOrMakeType(const std::string& aTypeKind);


   Item* mRootItem;

   std::map<std::string, Item*> mTypes;
   Item*                        mBaseTypeItems[cTYPE_KIND_COUNT];
   bool                         mShowUnusedTypes;
   bool                         mHibernate;
   QTimer                       mDeferredUpdateOnProxyChange;
   UtCallbackHolder             mCallbacks;
   UtCallbackHolder             mProxyWatcherCallbacks;
   WsfPProxy*                   mProxyPtr;
};
} // namespace TypeBrowser

#endif
