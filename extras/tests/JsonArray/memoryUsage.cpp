// ArduinoJson - https://arduinojson.org
// Copyright © 2014-2023, Benoit BLANCHON
// MIT License

#include <ArduinoJson.h>
#include <catch.hpp>

using ArduinoJson::detail::sizeofArray;
using ArduinoJson::detail::sizeofObject;
using ArduinoJson::detail::sizeofString;

TEST_CASE("JsonArray::memoryUsage()") {
  JsonDocument doc;
  JsonArray arr = doc.to<JsonArray>();

  SECTION("return 0 if uninitialized") {
    JsonArray unitialized;
    REQUIRE(unitialized.memoryUsage() == 0);
  }

  SECTION("sizeofArray(0) if empty") {
    REQUIRE(arr.memoryUsage() == sizeofArray(0));
  }

  SECTION("sizeofArray(1) after add") {
    arr.add("hello");
    REQUIRE(arr.memoryUsage() == sizeofArray(1));
  }

  SECTION("includes the size of the string") {
    arr.add(std::string("hello"));
    REQUIRE(arr.memoryUsage() == sizeofArray(1) + sizeofString(5));
  }

  SECTION("includes the size of the nested array") {
    JsonArray nested = arr.createNestedArray();
    nested.add(42);
    REQUIRE(arr.memoryUsage() == 2 * sizeofArray(1));
  }

  SECTION("includes the size of the nested arrect") {
    JsonObject nested = arr.createNestedObject();
    nested["hello"] = "world";
    REQUIRE(arr.memoryUsage() == sizeofObject(1) + sizeofArray(1));
  }
}
