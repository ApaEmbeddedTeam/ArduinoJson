// ArduinoJson - https://arduinojson.org
// Copyright © 2014-2024, Benoit BLANCHON
// MIT License

#include <ArduinoJson.h>
#include <catch.hpp>

#include "Allocators.hpp"
#include "Literals.hpp"

TEST_CASE("JsonDocument::operator[]") {
  JsonDocument doc;
  const JsonDocument& cdoc = doc;

  SECTION("object") {
    deserializeJson(doc, "{\"hello\":\"world\"}");

    SECTION("string literal") {
      REQUIRE(doc["hello"] == "world");
      REQUIRE(cdoc["hello"] == "world");
    }

    SECTION("std::string") {
      REQUIRE(doc["hello"_s] == "world");
      REQUIRE(cdoc["hello"_s] == "world");
    }

    SECTION("JsonVariant") {
      doc["key"] = "hello";
      REQUIRE(doc[doc["key"]] == "world");
      REQUIRE(cdoc[cdoc["key"]] == "world");
    }

    SECTION("supports operator|") {
      REQUIRE((doc["hello"] | "nope") == "world"_s);
      REQUIRE((doc["world"] | "nope") == "nope"_s);
    }

#if defined(HAS_VARIABLE_LENGTH_ARRAY) && \
    !defined(SUBSCRIPT_CONFLICTS_WITH_BUILTIN_OPERATOR)
    SECTION("supports VLAs") {
      size_t i = 16;
      char vla[i];
      strcpy(vla, "hello");

      doc[vla] = "world";

      REQUIRE(doc[vla] == "world");
      REQUIRE(cdoc[vla] == "world");
      REQUIRE(doc.as<std::string>() == "{\"hello\":\"world\"}");
    }
#endif
  }

  SECTION("array") {
    deserializeJson(doc, "[\"hello\",\"world\"]");

    SECTION("int") {
      REQUIRE(doc[1] == "world");
      REQUIRE(cdoc[1] == "world");
    }

    SECTION("JsonVariant") {
      doc[2] = 1;
      REQUIRE(doc[doc[2]] == "world");
      REQUIRE(cdoc[doc[2]] == "world");
    }
  }
}

TEST_CASE("JsonDocument automatically promotes to object") {
  JsonDocument doc;

  doc["one"]["two"]["three"] = 4;

  REQUIRE(doc["one"]["two"]["three"] == 4);
}

TEST_CASE("JsonDocument automatically promotes to array") {
  JsonDocument doc;

  doc[2] = 2;

  REQUIRE(doc.as<std::string>() == "[null,null,2]");
}

TEST_CASE("JsonDocument::operator[] key storage") {
  SpyingAllocator spy;
  JsonDocument doc(&spy);

  SECTION("string literal") {
    doc["hello"] = 0;

    REQUIRE(doc.as<std::string>() == "{\"hello\":0}");
    REQUIRE(spy.log() == AllocatorLog{
                             Allocate(sizeofPool()),
                         });
  }

  SECTION("const char*") {
    const char* key = "hello";
    doc[key] = 0;

    REQUIRE(doc.as<std::string>() == "{\"hello\":0}");
    REQUIRE(spy.log() == AllocatorLog{
                             Allocate(sizeofPool()),
                             Allocate(sizeofString("hello")),
                         });
  }

  SECTION("char[]") {
    char key[] = "hello";
    doc[key] = 0;

    REQUIRE(doc.as<std::string>() == "{\"hello\":0}");
    REQUIRE(spy.log() == AllocatorLog{
                             Allocate(sizeofPool()),
                             Allocate(sizeofString("hello")),
                         });
  }

  SECTION("std::string") {
    doc["hello"_s] = 0;

    REQUIRE(doc.as<std::string>() == "{\"hello\":0}");
    REQUIRE(spy.log() == AllocatorLog{
                             Allocate(sizeofPool()),
                             Allocate(sizeofString("hello")),
                         });
  }
#if defined(HAS_VARIABLE_LENGTH_ARRAY) && \
    !defined(SUBSCRIPT_CONFLICTS_WITH_BUILTIN_OPERATOR)
  SECTION("VLA") {
    size_t i = 16;
    char vla[i];
    strcpy(vla, "hello");

    doc[vla] = 0;

    REQUIRE(doc.as<std::string>() == "{\"hello\":0}");
    REQUIRE(spy.log() == AllocatorLog{
                             Allocate(sizeofPool()),
                             Allocate(sizeofString("hello")),
                         });
  }
#endif
}
