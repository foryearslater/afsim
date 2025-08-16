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

#ifndef USMTFFACTORY_HPP
#define USMTFFACTORY_HPP

#include "usmtf_export.h"

#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

#include "Message.hpp"
#include "UtMemory.hpp"

namespace usmtf
{
//! Parser uses the factory to construct Sets and Messages uniformly. If Set/Message Types have a constructor registered
//! (ie, opting in to caring about it) then the parser will use it, otherwise it will make the base class for it and
//! move on. This ensures that parsing always succeeds if the file is valid USMTF, but the interpretation of a generic
//! set/message is always left to the custom types to validate, interpret, transform or otherwise use. Its an opt in
//! system, register things you care to validate and use, ignore validating the rest. This allows an immense flexibility
//! where class authors can always use the Abstractions provided by the lib, yet isolate the things they actually care
//! about. This flexibility is important because there may be discrepancies between what the USMTF defines as correct
//! and what an AFSIM idea can use. We don't want to follow the USMTF standard letter by letter if we can only use some
//! of the sets/segments/message/fields defined to create AFSIM entities.

//! Uses of this is highly documented in pretty much every tests Suite.
template<class T, class U>
class USMTF_Factory
{
public:
   //! Constructor Callback specifies the expected format of the function needed to instantiate objects.
   using ConstructorCb = std::function<std::unique_ptr<T>(const std::string& aType, U aContents)>;

   //! Return the singleton instance of the factory.
   static USMTF_Factory<T, U>* GetFactory();

   //! Cast base pointer to derived. Intended for use by Set and Message Authors.
   //! It will verify both at run time with dynamic_cast that the relationship is valid.
   //! It also verifies at runtime when used that the derived class is registered.
   template<class V>
   const V* CastIfRegistered(const T& aEntity) noexcept;

   //! Template Function for telling the factory about a derived Message and Set type.
   //! Used in conjunction with RegisterEntity. See unit tests for example.
   template<class V>
   static std::unique_ptr<V> Construct(const std::string& aType, U&& aContents);

   //! Register the type string and constructor functions with the factory. Without registering
   //! a derived Message or Set the factory will instantiate the base classes. See unit tests for example.
   void RegisterEntity(const std::string& aType, ConstructorCb aCb) noexcept;

   //! Unregister a previously registers derived type.
   void UnregisterEntity(const std::string& aType) noexcept;

   //! Instantiate an instance of the Set or Message. Used Only by parser.
   std::unique_ptr<T> CreateInstance(const std::string& aType, U aContents);

private:
   USMTF_Factory<T, U>() = default;
   using CallbackMap     = std::unordered_map<std::string, ConstructorCb>;

   // Map type to constructor function.
   CallbackMap                                 mRegisteredEntities;
   static std::unique_ptr<USMTF_Factory<T, U>> mFactoryInstance;
};

//! Convenience for Set and Message Authors. The Template functionality is really
//! only meant for these two uses.
using MessageFactory = USMTF_Factory<Message, std::vector<std::unique_ptr<Set>>>;
using SetFactory     = USMTF_Factory<Set, std::vector<Field>>;

//! Export across DLL boundary, ensures singleton functionality.
#if defined(SWDEV_ALL_USE_DLL)
template class USMTF_EXPORT USMTF_Factory<Message, std::vector<std::unique_ptr<Set>>>;
template class USMTF_EXPORT USMTF_Factory<Set, std::vector<Field>>;

#endif

template<class T, class U>
USMTF_Factory<T, U>* USMTF_Factory<T, U>::GetFactory()
{
   if (!mFactoryInstance)
   {
      mFactoryInstance = std::unique_ptr<USMTF_Factory<T, U>>(new USMTF_Factory<T, U>);
   }

   return mFactoryInstance.get();
}

template<class T, class U>
template<class V>
const V* USMTF_Factory<T, U>::CastIfRegistered(const T& aEntity) noexcept
{
   typename CallbackMap::iterator it = mRegisteredEntities.find(aEntity.GetType());
   if (it != mRegisteredEntities.end())
   {
      return dynamic_cast<const V*>(&aEntity);
   }
   return nullptr; // cast not valid, type not registered so a concrete class could not have been instantiated for the type.
}

template<class T, class U>
template<class V>
std::unique_ptr<V> USMTF_Factory<T, U>::Construct(const std::string& aType, U&& aContents)
{
   return ut::make_unique<V>(aType, std::move(aContents));
}

template<class T, class U>
void USMTF_Factory<T, U>::RegisterEntity(const std::string&                          aType,
                                         typename USMTF_Factory<T, U>::ConstructorCb aConstructorCb) noexcept
{
   mRegisteredEntities[aType] = aConstructorCb;
}

template<class T, class U>
void USMTF_Factory<T, U>::UnregisterEntity(const std::string& aType) noexcept
{
   mRegisteredEntities.erase(aType);
}

template<class T, class U>
std::unique_ptr<T> USMTF_Factory<T, U>::CreateInstance(const std::string& aType, U aContents)
{
   typename CallbackMap::iterator it = mRegisteredEntities.find(aType);
   if (it != mRegisteredEntities.end())
   {
      return (it->second)(aType, std::move(aContents));
   }
   return ut::make_unique<T>(aType, std::move(aContents));
}

template<class T, class U>
std::unique_ptr<USMTF_Factory<T, U>> USMTF_Factory<T, U>::mFactoryInstance;
} // namespace usmtf
#endif
