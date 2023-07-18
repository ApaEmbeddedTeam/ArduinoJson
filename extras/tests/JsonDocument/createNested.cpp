// ArduinoJson - https://arduinojson.org
// Copyright © 2014-2023, Benoit BLANCHON
// MIT License

#include <ArduinoJson.h>
#include <catch.hpp>

TEST_CASE("JsonDocument::createNestedArray()") {
  JsonDocument doc;

  SECTION("promotes to array") {
    doc.createNestedArray();

    REQUIRE(doc.is<JsonArray>());
  }
}

TEST_CASE("JsonDocument::createNestedArray(key)") {
  JsonDocument doc;

  SECTION("key is const char*") {
    SECTION("promotes to object") {
      doc.createNestedArray("hello");

      REQUIRE(doc.is<JsonObject>());
    }
  }

  SECTION("key is std::string") {
    SECTION("promotes to object") {
      doc.createNestedArray(std::string("hello"));

      REQUIRE(doc.is<JsonObject>());
    }
  }
}

TEST_CASE("JsonDocument::createNestedObject()") {
  JsonDocument doc;

  SECTION("promotes to array") {
    doc.createNestedObject();

    REQUIRE(doc.is<JsonArray>());
  }
}

TEST_CASE("JsonDocument::createNestedObject(key)") {
  JsonDocument doc;

  SECTION("key is const char*") {
    SECTION("promotes to object") {
      doc.createNestedObject("hello");

      REQUIRE(doc.is<JsonObject>());
    }
  }

  SECTION("key is std::string") {
    SECTION("promotes to object") {
      doc.createNestedObject(std::string("hello"));

      REQUIRE(doc.is<JsonObject>());
    }
  }
}
