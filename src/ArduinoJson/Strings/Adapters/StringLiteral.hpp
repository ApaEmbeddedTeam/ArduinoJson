// ArduinoJson - https://arduinojson.org
// Copyright © 2014-2024, Benoit BLANCHON
// MIT License

#pragma once

#include <stddef.h>  // size_t
#include <string.h>  // strcmp

#include <ArduinoJson/Polyfills/assert.hpp>
#include <ArduinoJson/Polyfills/attributes.hpp>
#include <ArduinoJson/Strings/StringAdapter.hpp>

ARDUINOJSON_BEGIN_PRIVATE_NAMESPACE

template <size_t N>
class StringLiteral {
 public:
  static const size_t typeSortKey = 2;

  StringLiteral(const char* str) : str_(str) {}

  bool isNull() const {
    return !str_;
  }

  size_t size() const {
    return N - 1;
  }

  char operator[](size_t i) const {
    ARDUINOJSON_ASSERT(str_ != 0);
    ARDUINOJSON_ASSERT(i <= size());
    return str_[i];
  }

  const char* data() const {
    return str_;
  }

  bool isLinked() const {
    return true;
  }

 protected:
  const char* str_;
};

template <size_t N>
struct StringAdapter<const char[N]> {
  using AdaptedString = StringLiteral<N>;

  static AdaptedString adapt(const char (&p)[N]) {
    return AdaptedString(p);
  }
};

ARDUINOJSON_END_PRIVATE_NAMESPACE
