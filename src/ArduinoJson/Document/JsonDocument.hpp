// ArduinoJson - https://arduinojson.org
// Copyright © 2014-2023, Benoit BLANCHON
// MIT License

#pragma once

#include <ArduinoJson/Array/ElementProxy.hpp>
#include <ArduinoJson/Memory/Allocator.hpp>
#include <ArduinoJson/Memory/MemoryPool.hpp>
#include <ArduinoJson/Object/JsonObject.hpp>
#include <ArduinoJson/Object/MemberProxy.hpp>
#include <ArduinoJson/Strings/StoragePolicy.hpp>
#include <ArduinoJson/Variant/JsonVariantConst.hpp>
#include <ArduinoJson/Variant/VariantTo.hpp>

ARDUINOJSON_BEGIN_PUBLIC_NAMESPACE

// A JSON document.
// https://arduinojson.org/v6/api/jsondocument/
class JsonDocument : public detail::VariantOperators<const JsonDocument&> {
  friend class detail::VariantAttorney;

 public:
  explicit JsonDocument(size_t capa,
                        Allocator* alloc = detail::DefaultAllocator::instance())
      : _allocator(alloc), _pool(allocPool(capa)) {}

  // Copy-constructor
  JsonDocument(const JsonDocument& src)
      : JsonDocument(src.capacity(), src._allocator) {
    set(src);
  }

  // Move-constructor
  JsonDocument(JsonDocument&& src) : _allocator(src._allocator), _pool(0, 0) {
    // TODO: use the copy and swap idiom
    moveAssignFrom(src);
  }

  // Construct from variant, array, or object
  template <typename T>
  JsonDocument(const T& src,
               typename detail::enable_if<
                   detail::is_same<T, JsonVariant>::value ||
                   detail::is_same<T, JsonVariantConst>::value ||
                   detail::is_same<T, JsonArray>::value ||
                   detail::is_same<T, JsonArrayConst>::value ||
                   detail::is_same<T, JsonObject>::value ||
                   detail::is_same<T, JsonObjectConst>::value>::type* = 0)
      : JsonDocument(src.memoryUsage()) {
    set(src);
  }

  // disambiguate
  // TODO: still needed?
  JsonDocument(JsonVariant src) : JsonDocument(src.memoryUsage()) {
    set(src);
  }

  ~JsonDocument() {
    freePool();
  }

  JsonDocument& operator=(const JsonDocument& src) {
    // TODO: use the copy and swap idiom
    copyAssignFrom(src);
    return *this;
  }

  JsonDocument& operator=(JsonDocument&& src) {
    // TODO: use the copy and swap idiom
    moveAssignFrom(src);
    return *this;
  }

  template <typename T>
  JsonDocument& operator=(const T& src) {
    size_t requiredSize = src.memoryUsage();
    if (requiredSize > capacity())
      reallocPool(requiredSize);
    set(src);
    return *this;
  }

  // Reduces the capacity of the memory pool to match the current usage.
  // https://arduinojson.org/v6/api/JsonDocument/shrinktofit/
  void shrinkToFit() {
    ptrdiff_t bytes_reclaimed = _pool.squash();
    if (bytes_reclaimed == 0)
      return;

    void* old_ptr = _pool.buffer();
    void* new_ptr = _allocator->reallocate(old_ptr, _pool.capacity());

    ptrdiff_t ptr_offset =
        static_cast<char*>(new_ptr) - static_cast<char*>(old_ptr);

    _pool.movePointers(ptr_offset);
    _data.movePointers(ptr_offset, ptr_offset - bytes_reclaimed);
  }

  // Reclaims the memory leaked when removing and replacing values.
  // https://arduinojson.org/v6/api/jsondocument/garbagecollect/
  bool garbageCollect() {
    // make a temporary clone and move assign
    JsonDocument tmp(*this);
    if (!tmp.capacity())
      return false;
    tmp.set(*this);
    moveAssignFrom(tmp);
    return true;
  }

  // Casts the root to the specified type.
  // https://arduinojson.org/v6/api/jsondocument/as/
  template <typename T>
  T as() {
    return getVariant().template as<T>();
  }

  // Casts the root to the specified type.
  // https://arduinojson.org/v6/api/jsondocument/as/
  template <typename T>
  T as() const {
    return getVariant().template as<T>();
  }

  // Empties the document and resets the memory pool
  // https://arduinojson.org/v6/api/jsondocument/clear/
  void clear() {
    _pool.clear();
    _data.setNull();
  }

  // Returns true if the root is of the specified type.
  // https://arduinojson.org/v6/api/jsondocument/is/
  template <typename T>
  bool is() {
    return getVariant().template is<T>();
  }

  // Returns true if the root is of the specified type.
  // https://arduinojson.org/v6/api/jsondocument/is/
  template <typename T>
  bool is() const {
    return getVariant().template is<T>();
  }

  // Returns true if the root is null.
  // https://arduinojson.org/v6/api/jsondocument/isnull/
  bool isNull() const {
    return getVariant().isNull();
  }

  // Returns the number of used bytes in the memory pool.
  // https://arduinojson.org/v6/api/jsondocument/memoryusage/
  size_t memoryUsage() const {
    return _pool.size();
  }

  // Returns trues if the memory pool was too small.
  // https://arduinojson.org/v6/api/jsondocument/overflowed/
  bool overflowed() const {
    return _pool.overflowed();
  }

  // Returns the depth (nesting level) of the array.
  // https://arduinojson.org/v6/api/jsondocument/nesting/
  size_t nesting() const {
    return variantNesting(&_data);
  }

  // Returns the capacity of the memory pool.
  // https://arduinojson.org/v6/api/jsondocument/capacity/
  size_t capacity() const {
    return _pool.capacity();
  }

  // Returns the number of elements in the root array or object.
  // https://arduinojson.org/v6/api/jsondocument/size/
  size_t size() const {
    return _data.size();
  }

  // Copies the specified document.
  // https://arduinojson.org/v6/api/jsondocument/set/
  bool set(const JsonDocument& src) {
    return to<JsonVariant>().set(src.as<JsonVariantConst>());
  }

  // Replaces the root with the specified value.
  // https://arduinojson.org/v6/api/jsondocument/set/
  template <typename T>
  typename detail::enable_if<!detail::is_base_of<JsonDocument, T>::value,
                             bool>::type
  set(const T& src) {
    return to<JsonVariant>().set(src);
  }

  // Clears the document and converts it to the specified type.
  // https://arduinojson.org/v6/api/jsondocument/to/
  template <typename T>
  typename detail::VariantTo<T>::type to() {
    clear();
    return getVariant().template to<T>();
  }

  // Creates an array and appends it to the root array.
  // https://arduinojson.org/v6/api/jsondocument/createnestedarray/
  JsonArray createNestedArray() {
    return add().to<JsonArray>();
  }

  // Creates an array and adds it to the root object.
  // https://arduinojson.org/v6/api/jsondocument/createnestedarray/
  template <typename TChar>
  JsonArray createNestedArray(TChar* key) {
    return operator[](key).template to<JsonArray>();
  }

  // Creates an array and adds it to the root object.
  // https://arduinojson.org/v6/api/jsondocument/createnestedarray/
  template <typename TString>
  JsonArray createNestedArray(const TString& key) {
    return operator[](key).template to<JsonArray>();
  }

  // Creates an object and appends it to the root array.
  // https://arduinojson.org/v6/api/jsondocument/createnestedobject/
  JsonObject createNestedObject() {
    return add().to<JsonObject>();
  }

  // Creates an object and adds it to the root object.
  // https://arduinojson.org/v6/api/jsondocument/createnestedobject/
  template <typename TChar>
  JsonObject createNestedObject(TChar* key) {
    return operator[](key).template to<JsonObject>();
  }

  // Creates an object and adds it to the root object.
  // https://arduinojson.org/v6/api/jsondocument/createnestedobject/
  template <typename TString>
  JsonObject createNestedObject(const TString& key) {
    return operator[](key).template to<JsonObject>();
  }

  // Returns true if the root object contains the specified key.
  // https://arduinojson.org/v6/api/jsondocument/containskey/
  template <typename TChar>
  bool containsKey(TChar* key) const {
    return _data.getMember(detail::adaptString(key)) != 0;
  }

  // Returns true if the root object contains the specified key.
  // https://arduinojson.org/v6/api/jsondocument/containskey/
  template <typename TString>
  bool containsKey(const TString& key) const {
    return _data.getMember(detail::adaptString(key)) != 0;
  }

  // Gets or sets a root object's member.
  // https://arduinojson.org/v6/api/jsondocument/subscript/
  template <typename TString>
  FORCE_INLINE typename detail::enable_if<
      detail::IsString<TString>::value,
      detail::MemberProxy<JsonDocument&, TString>>::type
  operator[](const TString& key) {
    return {*this, key};
  }

  // Gets or sets a root object's member.
  // https://arduinojson.org/v6/api/jsondocument/subscript/
  template <typename TChar>
  FORCE_INLINE typename detail::enable_if<
      detail::IsString<TChar*>::value,
      detail::MemberProxy<JsonDocument&, TChar*>>::type
  operator[](TChar* key) {
    return {*this, key};
  }

  // Gets a root object's member.
  // https://arduinojson.org/v6/api/jsondocument/subscript/
  template <typename TString>
  FORCE_INLINE typename detail::enable_if<detail::IsString<TString>::value,
                                          JsonVariantConst>::type
  operator[](const TString& key) const {
    return JsonVariantConst(_data.getMember(detail::adaptString(key)));
  }

  // Gets a root object's member.
  // https://arduinojson.org/v6/api/jsondocument/subscript/
  template <typename TChar>
  FORCE_INLINE typename detail::enable_if<detail::IsString<TChar*>::value,
                                          JsonVariantConst>::type
  operator[](TChar* key) const {
    return JsonVariantConst(_data.getMember(detail::adaptString(key)));
  }

  // Gets or sets a root array's element.
  // https://arduinojson.org/v6/api/jsondocument/subscript/
  FORCE_INLINE detail::ElementProxy<JsonDocument&> operator[](size_t index) {
    return {*this, index};
  }

  // Gets a root array's member.
  // https://arduinojson.org/v6/api/jsondocument/subscript/
  FORCE_INLINE JsonVariantConst operator[](size_t index) const {
    return JsonVariantConst(_data.getElement(index));
  }

  // Appends a new (null) element to the root array.
  // Returns a reference to the new element.
  // https://arduinojson.org/v6/api/jsondocument/add/
  FORCE_INLINE JsonVariant add() {
    return JsonVariant(&_pool, _data.addElement(&_pool));
  }

  // Appends a value to the root array.
  // https://arduinojson.org/v6/api/jsondocument/add/
  template <typename TValue>
  FORCE_INLINE bool add(const TValue& value) {
    return add().set(value);
  }

  // Appends a value to the root array.
  // https://arduinojson.org/v6/api/jsondocument/add/
  template <typename TChar>
  FORCE_INLINE bool add(TChar* value) {
    return add().set(value);
  }

  // Removes an element of the root array.
  // ⚠️ Doesn't release the memory associated with the removed element.
  // https://arduinojson.org/v6/api/jsondocument/remove/
  FORCE_INLINE void remove(size_t index) {
    _data.remove(index);
  }

  // Removes a member of the root object.
  // ⚠️ Doesn't release the memory associated with the removed element.
  // https://arduinojson.org/v6/api/jsondocument/remove/
  template <typename TChar>
  FORCE_INLINE typename detail::enable_if<detail::IsString<TChar*>::value>::type
  remove(TChar* key) {
    _data.remove(detail::adaptString(key));
  }

  // Removes a member of the root object.
  // ⚠️ Doesn't release the memory associated with the removed element.
  // https://arduinojson.org/v6/api/jsondocument/remove/
  template <typename TString>
  FORCE_INLINE
      typename detail::enable_if<detail::IsString<TString>::value>::type
      remove(const TString& key) {
    _data.remove(detail::adaptString(key));
  }

  FORCE_INLINE operator JsonVariant() {
    return getVariant();
  }

  FORCE_INLINE operator JsonVariantConst() const {
    return getVariant();
  }

 private:
  void replacePool(detail::MemoryPool pool) {
    _pool = pool;
  }

  JsonVariant getVariant() {
    return JsonVariant(&_pool, &_data);
  }

  JsonVariantConst getVariant() const {
    return JsonVariantConst(&_data);
  }

  detail::MemoryPool allocPool(size_t requiredSize) {
    size_t capa = detail::addPadding(requiredSize);
    return {reinterpret_cast<char*>(_allocator->allocate(capa)), capa};
  }

  void reallocPool(size_t requiredSize) {
    size_t capa = detail::addPadding(requiredSize);
    if (capa == _pool.capacity())
      return;
    freePool();
    replacePool(allocPool(detail::addPadding(requiredSize)));
  }

  void freePool() {
    auto p = getPool()->buffer();
    if (p)
      _allocator->deallocate(p);
  }

  void copyAssignFrom(const JsonDocument& src) {
    reallocPool(src.capacity());
    set(src);
  }

  void moveAssignFrom(JsonDocument& src) {
    freePool();
    _data = src._data;
    _pool = src._pool;
    src._data.setNull();
    src._pool = {0, 0};
  }

  detail::MemoryPool* getPool() {
    return &_pool;
  }

  detail::VariantData* getData() {
    return &_data;
  }

  const detail::VariantData* getData() const {
    return &_data;
  }

  detail::VariantData* getOrCreateData() {
    return &_data;
  }

  Allocator* _allocator;
  detail::MemoryPool _pool;
  detail::VariantData _data;
};

inline void convertToJson(const JsonDocument& src, JsonVariant dst) {
  dst.set(src.as<JsonVariantConst>());
}

ARDUINOJSON_END_PUBLIC_NAMESPACE
