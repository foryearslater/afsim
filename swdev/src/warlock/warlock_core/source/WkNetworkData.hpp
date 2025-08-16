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

#ifndef WK_NETWORK_DATA_HPP
#define WK_NETWORK_DATA_HPP

#include "warlock_core_export.h"

#include <QColor>
#include <QDataStream>
#include <QHostAddress>
#include <QString>

#include "DisEntityId.hpp"
#include "UtCompilerAttributes.hpp"
#include "UtOptional.hpp"
#include "UtReflectObject.hpp"

namespace warlock
{
class Platform;

namespace net
{
//! Utility class for initializing NetworkData and its derived types.
//! ObjectData::Field<T> requires T to be constructible from a NetworkDataInit.
struct NetworkDataInit
{
   NetworkDataInit() = default;
   //! Implicit conversion from raw string.
   NetworkDataInit(const char* aName, bool aOptional = false)
      : mName(aName)
      , mOptional(aOptional)
   {
   }

   QString mName;
   bool    mOptional = false;
};

//! Base type for all data types being serialized over the network.
//! Derived types should use the DeriveData<T> alias defined below.
class WARLOCK_CORE_EXPORT NetworkData : public ut::reflect::Node
{
public:
   //! All NetworkData derived classes (except for Packets)
   //!    need to be constructible from a NetworkDataInit.
   explicit NetworkData(NetworkDataInit aInit) noexcept;

   //! Reads the data from the stream.
   //! Automatically handles the optional-ness of the data.
   //! Forwards to virtual Deserialze.
   void ReadFrom(QDataStream& aBuffer);

   //! Writes the data to the stream.
   //! Automatically handles the optional-ness of the data.
   //! Forwards to virtual Serialize.
   void WriteTo(QDataStream& aBuffer) const;

   const QString& Name() const noexcept;

   //! Returns whether the value is optional.
   //! Optional values are serialized as a boolean, optionally followed by the value.
   //! Required values skip the bool.
   bool IsOptional() const noexcept;

   //! Returns true if a value has been set.
   //! Always returns true for non-optional data.
   //! Otherwise, returns false until a value has been inserted.
   //! @note For ObjectData, returns true if any sub-objects are set.
   virtual bool IsValueSet() const noexcept;

   //! Marks the value as unset.
   //! This only sets a flag, not affect any value in memory.
   void Reset() noexcept;

protected:
   //! Writes the contained data into aBuffer.
   virtual void Serialize(QDataStream& aBuffer) const = 0;

   //! Reads the buffer into the contained data.
   virtual void Deserialize(QDataStream& aBuffer) = 0;

   //! Sets the mIsValueSet flag.
   void ValueSet() noexcept;

private:
   const QString mName;
   const bool    mOptional   = false;
   bool          mIsValueSet = false;
};

//! Utility for defining types derived from NetworkData.
//! If a type U derives from DeriveData<U>, a ut::reflect::Visitor can specialize on type U.
template<typename T>
using DeriveData = ut::reflect::Derive<T, NetworkData>;

//! Base type for basic data.
//! Using a base type lets basic data interact more cleanly with visitor classes.
class BasicDataBase : public DeriveData<BasicDataBase>
{
public:
   using ReflectBase::ReflectBase;

   virtual QVariant GetVariant() const = 0;
};

//! Helper template for defining network info for many primitive types, such as bool, int, double, and QString.
//! @warning T must be a type serializable with QDataStream and convertible to QVariant.
template<typename T>
class BasicData final : public ut::reflect::Derive<BasicData<T>, BasicDataBase>
{
public:
   explicit BasicData(NetworkDataInit aInit, const T& aDefaultValue = T())
      : ut::reflect::ReflectBaseT<BasicData<T>>(std::move(aInit))
      , mData(aDefaultValue)
   {
   }

   using type = T;

   QVariant GetVariant() const override { return QVariant{mData}; }

   //! Returns the contained value.
   //! If no value is set, returns the default value.
   const T& Get() const noexcept { return mData; }
   //! Returns the contained value if set.
   //! If no value is set, returns the given value.
   template<typename U>
   T GetOr(U&& aDefault) const
   {
      static_assert(std::is_convertible<U, T>::value, "U is not convertible to T.");
      if (this->IsValueSet())
      {
         return mData;
      }
      return T{std::forward<U>(aDefault)};
   }

   //! Sets the contained value.
   void Set(const T& aData) noexcept
   {
      mData = aData;
      this->ValueSet();
   }
   //! Sets the contained value.
   void Set(T&& aData) noexcept
   {
      mData = std::move(aData);
      this->ValueSet();
   }

private:
   void Serialize(QDataStream& aBuffer) const final { aBuffer << mData; }
   void Deserialize(QDataStream& aBuffer) final { aBuffer >> mData; }

   T mData;
};

using Boolean = BasicData<bool>;
using Int32   = BasicData<qint32>;
using Float32 = BasicData<float>;
using Float64 = BasicData<double>;
using String  = BasicData<QString>;
using Color   = BasicData<QColor>;

//! Wrapper for a dis entity id type.
//! Eventually, overloads will be added for setting/getting via warlock::Platform*.
class WARLOCK_CORE_EXPORT PlatformId final : public DeriveData<PlatformId>
{
public:
   using ReflectBase::ReflectBase;

   //! Returns the full entity id.
   //! Each part can be accessed individually below.
   DisEntityId Get() const noexcept;
   //! Returns the entity id as "site:application:entity".
   QString GetDisIdString() const noexcept;
   //! Returns the individual values of the entity id.
   //@{
   quint16 GetSite() const noexcept;
   quint16 GetApplication() const noexcept;
   quint16 GetEntity() const noexcept;
   //@}

   //! Looks up the warlock::Platform from the contained id and returns it.
   //! If not found, returns null.
   warlock::Platform* GetPlatform() const;

   //! Sets the contained value.
   void Set(DisEntityId aId) noexcept;
   //! Sets the contained value from its components.
   void Set(quint16 aSite, quint16 aApplication, quint16 aEntity) noexcept;
   //! Sets the contained value by looking up the platform's dis id.
   void SetPlatform(const warlock::Platform* aPlatform) noexcept;

private:
   void Serialize(QDataStream& aBuffer) const final;
   void Deserialize(QDataStream& aBuffer) final;

   DisEntityId mId;
};

//! Base type for list data.
//! Using a base type lets lists interact more cleanly with visitor classes.
class WARLOCK_CORE_EXPORT ListBase : public DeriveData<ListBase>
{
public:
   using ReflectBase::ReflectBase;

   //! Returns the size of the list.
   //! If the list is optional, returns 0.
   virtual int Size() const noexcept = 0;
   //! Returns true if the list is empty.
   //! If the list is optional, returns true.
   bool Empty() const noexcept;

   //! Clears the list.
   virtual void Clear() = 0;

   //! Returns the element at index aIndex.
   //! If there is no data at that index, throws std::out_of_range.
   //! List elements are always nameless.
   //! @note List<T> uses covariant return types.
   virtual const NetworkData& Get(int aIndex) const = 0;
   virtual NetworkData&       Get(int aIndex)       = 0;

   //! Returns a NetworkData item representative of the list's contents.
   //! Can be used to get info such as field names and default values.
   //! @note List<T> uses covariant return types.
   virtual const NetworkData& GetTemplate() const noexcept = 0;

protected:
   //! Append is used in Deserialize and is declared in the base class.
   //! @note List<T> makes Append() public and uses covariant return types.
   virtual NetworkData& Append() = 0;

private:
   void Serialize(QDataStream& aBuffer) const final;
   void Deserialize(QDataStream& aBuffer) final;
};

//! A list of a specific type.
//! Introduces methods for constructing the list on the sending side.
template<typename T>
class List final : public ut::reflect::Derive<List<T>, ListBase>
{
   static_assert(std::is_base_of<NetworkData, T>::value, "T must derive from warlock::net::NetworkData");
   static_assert(std::is_constructible<T, NetworkDataInit>::value,
                 "T must be constructible from (warlock::net::NetworkDataInit).");

public:
   explicit List(NetworkDataInit aInit) noexcept
      : ut::reflect::ReflectBaseT<List<T>>(aInit)
   {
   }

   //! Returns the size of the list.
   //! If the list is optional, returns 0.
   int Size() const noexcept final { return static_cast<int>(mData.size()); }

   //! Clears the list.
   void Clear() final
   {
      mData.clear();
      this->ValueSet();
   }

   //! Returns the element at index aIndex.
   //! If there is not data at that index, throws std::out_of_range.
   //! List elements are always nameless.
   //! @note Covariant return type with base class.
   const T& Get(int aIndex) const final { return *mData.at(aIndex); }
   T&       Get(int aIndex) final { return *mData.at(aIndex); }

   //! Returns an item representative of the list contents.
   //! @note Covariant return type with base class.
   const T& GetTemplate() const noexcept final { return mTemplate; }

   //! Appends a T to the list and returns a reference to it.
   //! The reference is guaranteed to remain pointer-stable.
   //! The inserted element is non-optional and nameless.
   CPP17_NODISCARD T& Append() final { return Insert(Size()); }

   //! Prepends a T to the list and returns a reference to it.
   //! The reference is guaranteed to remain pointer-stable.
   //! The inserted element is non-optional and nameless.
   CPP17_NODISCARD T& Prepend() { return Insert(0); }

   //! Inserts a T at position aIndex and returns a reference to it.
   //! The reference is guaranteed to remain pointer-stable.
   //! If aIndex is negative or too large, throws std::out_of_range.
   //! Setting aIndex to Size() appends an item to the list.
   //! The inserted element is non-optional and nameless.
   CPP17_NODISCARD T& Insert(int aIndex)
   {
      if (aIndex < 0 || aIndex > Size())
      {
         throw std::out_of_range("Insertion index out of range.");
      }
      auto it     = mData.begin() + aIndex;
      auto ptr    = ut::make_unique<T>(NetworkDataInit());
      auto retval = ptr.get();
      mData.insert(it, std::move(ptr));
      this->ValueSet();
      return *retval;
   }

private:
   std::vector<std::unique_ptr<T>> mData;
   T                               mTemplate{NetworkDataInit()};
};

//! Base class for all aggregate data types.
//! Derived objects may choose to inherit from DeriveObject<T>, where T is the derived type.
//! This lets visitor classes specialize on the derived types.
//! Members should be declared and inline initialized as follows:
//! @code
//!    Field<IntData> x{ this, "x" [, 4] }; // 4 is an optional default value
//!    OptionalField<StringData> y{ this, "y" [, "default"] }; // "default" is an optional default value
//!    ListField<DoubleData> z{ this, "z" /*no default*/ };
//!    Field<SomeObjectDataType> obj{ this, "obj" /*no default*/ };
//! @endcode
class WARLOCK_CORE_EXPORT ObjectData : public ut::reflect::Derive<ObjectData, ut::reflect::ObjectNode<NetworkData>>
{
public:
   //! Ideally, 'using ReflectBase::ReflectBase;' would be sufficient to define the constructor.
   //! However, there is an issue in GCC 4.8.5 that prevents this from working for derived types.
   //! Instead, constructors may be defined by copy-pasting the following line and replacing the class name.
   explicit ObjectData(NetworkDataInit aInit)
      : ReflectBase(aInit)
   {
   }

   //! Overrides IsValueSet() to also return true if any sub-objects are set.
   bool IsValueSet() const noexcept final;

protected:
   template<typename T>
   struct OptionalField : Field<T>
   {
      template<typename... Args, typename = ut::enable_if_t<std::is_constructible<T, NetworkDataInit, Args...>::value>>
      OptionalField(ObjectData* aParent, const char* aName, Args&&... aArgs) noexcept
         : Field<T>(aParent, NetworkDataInit{aName, true}, std::forward<Args>(aArgs)...)
      {
      }
   };
   template<typename T>
   using ListField = Field<warlock::net::List<T>>;

private:
   void Serialize(QDataStream& aBuffer) const final;
   void Deserialize(QDataStream& aBuffer) final;
};

//! Helper type for deriving object types.
//! If a type U derives from DeriveObject<U>, a ut::reflect::Visitor can specialize on type U.
//! If this is not needed, simply deriving from ObjectData is fine.
template<typename T>
using DeriveObject = ut::reflect::Derive<T, ObjectData>;

//! Contains info about the sender of a packet.
struct SenderInfo
{
   QHostAddress mAddress;
   quint16      mPort = 0;
   qint64       mPID  = 0;

   friend bool operator==(const SenderInfo& aLeft, const SenderInfo& aRight) noexcept
   {
      return aLeft.mAddress == aRight.mAddress && aLeft.mPort == aRight.mPort && aLeft.mPID == aRight.mPID;
   }
};

//! Base packet type. All data going over the network should be put into a derived packet type.
//! Packets are always non-optional and have no name.
//! Derived packets must derive from DerivePacket<T>, where T is the derived type.
class WARLOCK_CORE_EXPORT Packet : public DeriveObject<Packet>
{
public:
   //! ALL packet types must be default constructible.
   Packet();

   //! Should return a string unique to the current packet type.
   //! If the packet's data changes, it is encouraged to return a new string to prevent version mismatch.
   virtual QString PacketType() const = 0;

   //! Returns metadata of a sent or received packet.
   //@{
   const QHostAddress& SenderAddress() const noexcept;
   int                 SenderPort() const noexcept;
   qint64              SenderPID() const noexcept;
   int                 ByteCount() const noexcept;

   const SenderInfo& GetSender() const noexcept;
   //@}

   //! Sets the packet metadata.
   void SetMetadata(const SenderInfo& aSender, int aByteCount) const noexcept;

private:
   //! Network::Publish() takes a const Packet&, promising that it will not modify the packet's contents.
   //! The metadata fields below, contained in the network packet header, ought to be modifiable by Publish().
   //! The modified fields will be seen by receivers of the Network::PacketSent() signal.
   mutable SenderInfo mSender;
   mutable int        mByteCount = 0;
};

//! In order to be registered, a derived packet type must derive from DerivePacket<T>, where T is the derived packet type.
//! This is enforced by static_asserts, and is necessary for the proper callbacks to be called when receiving packets.
template<typename T>
using DerivePacket = ut::reflect::Derive<T, Packet>;
} // namespace net
} // namespace warlock

#endif
