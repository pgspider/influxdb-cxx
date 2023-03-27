// MIT License
//
// Copyright (c) 2022 TOSHIBA CORPORATION
// Copyright (c) 2020-2022 offa
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "InfluxDBFactory.h"
#include "InfluxDBException.h"
#include <catch2/catch.hpp>

namespace influxdb::test
{
    TEST_CASE("V1: Accepts http urls", "[InfluxDBFactoryTest]")
    {
        CHECK(InfluxDBFactory::GetV1("http://localhost", 0, "test") != nullptr);
        CHECK(InfluxDBFactory::GetV1("https://localhost", 0, "test") != nullptr);

        CHECK(InfluxDBFactory::GetV1("http://localhost", 8086, "test") != nullptr);
        CHECK(InfluxDBFactory::GetV1("https://localhost", 8086, "test") != nullptr);

        CHECK(InfluxDBFactory::GetV1("https://localhost", 0, "test") != nullptr);
        CHECK(InfluxDBFactory::GetV1("https://localhost", 8086, "test") != nullptr);
    }

    TEST_CASE("V1: Throws on unrecognised backend", "[InfluxDBFactoryTest]")
    {
        CHECK_THROWS_AS(InfluxDBFactory::GetV1("httpX://localhost", 8086, "test"), InfluxDBException);
    }

    TEST_CASE("V1: Throws on malformed url", "[InfluxDBFactoryTest]")
    {
        CHECK_THROWS_AS(InfluxDBFactory::GetV1("localhost", 8086, "test"), InfluxDBException);
    }

    TEST_CASE("V1: Throws on missing database", "[InfluxDBFactoryTest]")
    {
        CHECK_THROWS_AS(InfluxDBFactory::GetV1("http://localhost", 8086, ""), InfluxDBException);
    }

    TEST_CASE("V1: Throws on invalid url", "[InfluxDBFactoryTest]")
    {
        CHECK_NOTHROW(InfluxDBFactory::GetV1("http://", 8086, "test"));
    }

    TEST_CASE("V1: Accept basic authentication", "[InfluxDBFactoryTest]")
    {
        CHECK(InfluxDBFactory::GetV1("http://localhost", 8086, "test", "user", "pass") != nullptr);

        CHECK(InfluxDBFactory::GetV1("http://localhost", 8086, "test", "user") != nullptr);
        CHECK(InfluxDBFactory::GetV1("http://localhost", 8086, "test", "user", "") != nullptr);

        CHECK(InfluxDBFactory::GetV1("http://localhost", 8086, "test", "", "pass") != nullptr);
        CHECK(InfluxDBFactory::GetV1("http://localhost", 8086, "test", "", "") != nullptr);
    }

    TEST_CASE("V2: Accepts http urls", "[InfluxDBFactoryTest]")
    {
        CHECK(InfluxDBFactory::GetV2("http://localhost", 0, "test", "znckeifSEW") != nullptr);
        CHECK(InfluxDBFactory::GetV2("https://localhost", 0, "test", "znckeifSEW") != nullptr);

        CHECK(InfluxDBFactory::GetV2("http://localhost", 8086, "test", "znckeifSEW") != nullptr);
        CHECK(InfluxDBFactory::GetV2("https://localhost", 8086, "test", "znckeifSEW") != nullptr);

        CHECK(InfluxDBFactory::GetV2("https://localhost", 0, "test", "znckeifSEW") != nullptr);
        CHECK(InfluxDBFactory::GetV2("https://localhost", 8086, "test", "znckeifSEW") != nullptr);
    }

    TEST_CASE("V2: Throws on unrecognised backend", "[InfluxDBFactoryTest]")
    {
        CHECK_THROWS_AS(InfluxDBFactory::GetV2("httpX://localhost", 8086, "test", "znckeifSEW"), InfluxDBException);
    }

    TEST_CASE("V2: Throws on malformed url", "[InfluxDBFactoryTest]")
    {
        CHECK_THROWS_AS(InfluxDBFactory::GetV2("localhost", 8086, "test", "znckeifSEW"), InfluxDBException);
    }

    TEST_CASE("V2: Throws on missing database", "[InfluxDBFactoryTest]")
    {
        CHECK_THROWS_AS(InfluxDBFactory::GetV2("http://localhost", 8086, "", "znckeifSEW"), InfluxDBException);
    }

    TEST_CASE("V2: Throws on invalid URL", "[InfluxDBFactoryTest]")
    {
        CHECK_NOTHROW(InfluxDBFactory::GetV2("http://", 8086, "test", "znckeifSEW"));
    }

    TEST_CASE("V2: Accept specify retention policy", "[InfluxDBFactoryTest]")
    {
        CHECK(InfluxDBFactory::GetV2("http://localhost", 8086, "test", "znckeifSEW", "myrp") != nullptr);
    }

    TEST_CASE("V2: Empty authen token", "[InfluxDBFactoryTest]")
    {
        CHECK(InfluxDBFactory::GetV2("http://localhost", 8086, "test", "") != nullptr);
    }
}
