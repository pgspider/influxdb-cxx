// MIT License
//
// Copyright (c) 2022 TOSHIBA CORPORATION
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

#include "InfluxDBParams.h"
#include <catch2/catch.hpp>

namespace influxdb::test
{
    using namespace Catch::Matchers;

    TEST_CASE("Param with empty name is not added", "[InfluxDBParamsTest]")
    {
        const auto params = InfluxDBParams{}.addParam("", "not added");
        CHECK_THAT(params.toJSON(), Equals("{}"));
        CHECK(params.size() == 0);
    }

    TEST_CASE("Param with empty value is added", "[InfluxDBParamsTest]")
    {
        const auto params = InfluxDBParams{}.addParam("added", "");
        CHECK_THAT(params.toJSON(), Equals(R"({"added":""})"));
        CHECK(params.size() == 1);
    }

    TEST_CASE("Add one param", "[InfluxDBParamsTest]")
    {
        const auto params = InfluxDBParams{}.addParam("param_name", "param_value");
        CHECK_THAT(params.toJSON(), Equals(R"({"param_name":"param_value"})"));
        CHECK(params.size() == 1);
    }

    TEST_CASE("Add different param types: bool, int, long, string, double", "[InfluxDBParamsTest]")
    {
        const auto params = InfluxDBParams{}.addParam("bool_param", true)
                                            .addParam("int_param", 3)
                                            .addParam("longlong_param", 1234LL)
                                            .addParam("string_param", "string value")
                                            .addParam("double_param", 3.859);
        CHECK_THAT(params.toJSON(), Equals(R"({"bool_param":true,"int_param":3,"longlong_param":1234,"string_param":"string value","double_param":3.859})"));
        CHECK(params.size() == 5);
    }

    TEST_CASE("Param name with special character", "[InfluxDBParamsTest]")
    {
        const auto params = InfluxDBParams{}.addParam("test\"", "escape");
        CHECK_THAT(params.toJSON(), Equals(R"({"test\"":"escape"})"));
        CHECK(params.size() == 1);
    }
}
