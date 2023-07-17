// ArduinoJson - https://arduinojson.org
// Copyright © 2014-2023, Benoit BLANCHON
// MIT License

#pragma once

#include <ArduinoJson/Object/JsonObjectConst.hpp>
#include <ArduinoJson/Object/MemberProxy.hpp>

ARDUINOJSON_BEGIN_PUBLIC_NAMESPACE

class JsonArray;

// A reference to an object in a JsonDocument.
// https://arduinojson.org/v6/api/jsonobject/
class JsonObject : public detail::VariantOperators<JsonObject> {
  friend class detail::VariantAttorney;

 public:
  typedef JsonObjectIterator iterator;

  // Creates an unbound reference.
  FORCE_INLINE JsonObject() : data_(0), resources_(0) {}

  // INTERNAL USE ONLY
  FORCE_INLINE JsonObject(detail::ObjectData* data,
                          detail::ResourceManager* resource)
      : data_(data), resources_(resource) {}

  operator JsonVariant() const {
    void* data = data_;  // prevent warning cast-align
    return JsonVariant(reinterpret_cast<detail::VariantData*>(data),
                       resources_);
  }

  operator JsonObjectConst() const {
    return JsonObjectConst(data_, resources_);
  }

  operator JsonVariantConst() const {
    return JsonVariantConst(collectionToVariant(data_), resources_);
  }

  // Returns true if the reference is unbound.
  // https://arduinojson.org/v6/api/jsonobject/isnull/
  FORCE_INLINE bool isNull() const {
    return data_ == 0;
  }

  // Returns true if the reference is bound.
  // https://arduinojson.org/v6/api/jsonobject/isnull/
  FORCE_INLINE operator bool() const {
    return data_ != 0;
  }

  // Returns the number of bytes occupied by the object.
  // https://arduinojson.org/v6/api/jsonobject/memoryusage/
  FORCE_INLINE size_t memoryUsage() const {
    return data_ ? data_->memoryUsage() : 0;
  }

  // Returns the depth (nesting level) of the object.
  // https://arduinojson.org/v6/api/jsonobject/nesting/
  FORCE_INLINE size_t nesting() const {
    return detail::VariantData::nesting(collectionToVariant(data_));
  }

  // Returns the number of members in the object.
  // https://arduinojson.org/v6/api/jsonobject/size/
  FORCE_INLINE size_t size() const {
    return data_ ? data_->size() : 0;
  }

  // Returns an iterator to the first key-value pair of the object.
  // https://arduinojson.org/v6/api/jsonobject/begin/
  FORCE_INLINE iterator begin() const {
    if (!data_)
      return iterator();
    return iterator(data_->createIterator(), resources_);
  }

  // Returns an iterator following the last key-value pair of the object.
  // https://arduinojson.org/v6/api/jsonobject/end/
  FORCE_INLINE iterator end() const {
    return iterator();
  }

  // Removes all the members of the object.
  // ⚠️ Doesn't release the memory associated with the removed members.
  // https://arduinojson.org/v6/api/jsonobject/clear/
  void clear() const {
    detail::ObjectData::clear(data_, resources_);
  }

  // Copies an object.
  // https://arduinojson.org/v6/api/jsonobject/set/
  FORCE_INLINE bool set(JsonObjectConst src) {
    if (!data_ || !src.data_)
      return false;

    clear();
    for (auto kvp : src) {
      if (!operator[](kvp.key()).set(kvp.value()))
        return false;
    }

    return true;
  }

  // Gets or sets the member with specified key.
  // https://arduinojson.org/v6/api/jsonobject/subscript/
  template <typename TString>
  FORCE_INLINE
      typename detail::enable_if<detail::IsString<TString>::value,
                                 detail::MemberProxy<JsonObject, TString>>::type
      operator[](const TString& key) const {
    return {*this, key};
  }

  // Gets or sets the member with specified key.
  // https://arduinojson.org/v6/api/jsonobject/subscript/
  template <typename TChar>
  FORCE_INLINE
      typename detail::enable_if<detail::IsString<TChar*>::value,
                                 detail::MemberProxy<JsonObject, TChar*>>::type
      operator[](TChar* key) const {
    return {*this, key};
  }

  // Removes the member at the specified iterator.
  // ⚠️ Doesn't release the memory associated with the removed member.
  // https://arduinojson.org/v6/api/jsonobject/remove/
  FORCE_INLINE void remove(iterator it) const {
    detail::ObjectData::remove(data_, it.iterator_, resources_);
  }

  // Removes the member with the specified key.
  // ⚠️ Doesn't release the memory associated with the removed member.
  // https://arduinojson.org/v6/api/jsonobject/remove/
  template <typename TString>
  FORCE_INLINE void remove(const TString& key) const {
    detail::ObjectData::removeMember(data_, detail::adaptString(key),
                                     resources_);
  }

  // Removes the member with the specified key.
  // ⚠️ Doesn't release the memory associated with the removed member.
  // https://arduinojson.org/v6/api/jsonobject/remove/
  template <typename TChar>
  FORCE_INLINE void remove(TChar* key) const {
    detail::ObjectData::removeMember(data_, detail::adaptString(key),
                                     resources_);
  }

  // Returns true if the object contains the specified key.
  // https://arduinojson.org/v6/api/jsonobject/containskey/
  template <typename TString>
  FORCE_INLINE
      typename detail::enable_if<detail::IsString<TString>::value, bool>::type
      containsKey(const TString& key) const {
    return detail::ObjectData::getMember(data_, detail::adaptString(key)) != 0;
  }

  // Returns true if the object contains the specified key.
  // https://arduinojson.org/v6/api/jsonobject/containskey/
  template <typename TChar>
  FORCE_INLINE
      typename detail::enable_if<detail::IsString<TChar*>::value, bool>::type
      containsKey(TChar* key) const {
    return detail::ObjectData::getMember(data_, detail::adaptString(key)) != 0;
  }

  // Creates an array and adds it to the object.
  // https://arduinojson.org/v6/api/jsonobject/createnestedarray/
  template <typename TString>
  FORCE_INLINE JsonArray createNestedArray(const TString& key) const;

  // Creates an array and adds it to the object.
  // https://arduinojson.org/v6/api/jsonobject/createnestedarray/
  template <typename TChar>
  FORCE_INLINE JsonArray createNestedArray(TChar* key) const;

  // Creates an object and adds it to the object.
  // https://arduinojson.org/v6/api/jsonobject/createnestedobject/
  template <typename TString>
  JsonObject createNestedObject(const TString& key) const {
    return operator[](key).template to<JsonObject>();
  }

  // Creates an object and adds it to the object.
  // https://arduinojson.org/v6/api/jsonobject/createnestedobject/
  template <typename TChar>
  JsonObject createNestedObject(TChar* key) const {
    return operator[](key).template to<JsonObject>();
  }

 private:
  detail::ResourceManager* getResourceManager() const {
    return resources_;
  }

  detail::VariantData* getData() const {
    return detail::collectionToVariant(data_);
  }

  detail::VariantData* getOrCreateData() const {
    return detail::collectionToVariant(data_);
  }

  detail::ObjectData* data_;
  detail::ResourceManager* resources_;
};

ARDUINOJSON_END_PUBLIC_NAMESPACE

ARDUINOJSON_BEGIN_PRIVATE_NAMESPACE

template <typename TDerived>
template <typename T>
typename enable_if<is_same<T, JsonObject>::value, JsonObject>::type
VariantRefBase<TDerived>::to() const {
  return JsonObject(
      VariantData::toObject(getOrCreateData(), getResourceManager()),
      getResourceManager());
}

ARDUINOJSON_END_PRIVATE_NAMESPACE
