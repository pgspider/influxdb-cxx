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

#include "Query.h"
#include "InfluxDBException.h"
#include "mock/TransportMock.h"
#include <catch2/catch.hpp>
#include <catch2/trompeloeil.hpp>
#include <sstream>

namespace influxdb::test
{
    TEST_CASE("Query is passed to transport", "[QueryTest]")
    {
        using trompeloeil::_;
        TransportMock transport;
        InfluxDBParams params{};
        REQUIRE_CALL(transport, query("SELECT * from test WHERE host = 'localhost'", _))
            .WITH(params.size() == 0)
            .RETURN(R"({"results":[{"statement_id":0}]})");

        internal::queryImpl(&transport, "SELECT * from test WHERE host = 'localhost'");
    }

    TEST_CASE("Query throws if transport throws", "[QueryTest]")
    {
        using trompeloeil::_;

        TransportMock transport;
        InfluxDBParams params{};
        ALLOW_CALL(transport, query(_, _)).WITH(params.size() == 0).THROW(InfluxDBException{"unit test", "Intentional"});

        CHECK_THROWS_AS(internal::queryImpl(&transport, "select should throw", params), InfluxDBException);
    }

    TEST_CASE("Query returns empty if empty result", "[QueryTest]")
    {
        using trompeloeil::_;

        TransportMock transport;
        InfluxDBParams params{};
        ALLOW_CALL(transport, query(_, _)).WITH(params.size() == 0).RETURN(R"({"results":[]})");

        CHECK(internal::queryImpl(&transport, "SELECT * from test", params).empty());
    }

    TEST_CASE("Query returns point of single result", "[QueryTest]")
    {
        using trompeloeil::_;

        TransportMock transport;
        InfluxDBParams params{};
        ALLOW_CALL(transport, query(_, _)).WITH(params.size() == 0)
            .RETURN(R"({"results":[{"statement_id":0,)"
                    R"("series":[{"name":"unittest","columns":["time","host","value"],)"
                    R"("values":[["2021-01-01T00:11:22.123456789Z","localhost",112233]]}]}]})");

        const auto result = internal::queryImpl(&transport, "SELECT * from test");
        CHECK(result.size() == 1);
        const auto point = result[0];
        CHECK(point.series[0].name == "unittest");
        CHECK(point.series[0].rows[0].tuple[0] == "2021-01-01T00:11:22.123456789Z");
        CHECK(point.series[0].columnNames[0] == "time");
        CHECK(point.series[0].rows[0].tuple[2] == "112233");
    }

    TEST_CASE("Query returns points of multiple results", "[QueryTest]")
    {
        using trompeloeil::_;

        TransportMock transport;
        InfluxDBParams params{};
        ALLOW_CALL(transport, query(_, _)).WITH(params.size() == 0)
            .RETURN(R"({"results":[{"statement_id":0,)"
                    R"("series":[{"name":"unittest","columns":["time","host","value"],)"
                    R"("values":[["2021-01-01:11:22.000000000Z","host-0",100],)"
                    R"(["2021-01-01T00:11:23.560000000Z","host-1",30],)"
                    R"(["2021-01-01T00:11:24.780000000Z","host-2",54]]}]}]})");

        const auto result = internal::queryImpl(&transport, "SELECT * from test");
        CHECK(result[0].series[0].rows.size() == 3);
        CHECK(result[0].series[0].name == "unittest");
        CHECK(result[0].series[0].rows[0].tuple[2] == "100");
        CHECK(result[0].series[0].rows[1].tuple[2] == "30");
        CHECK(result[0].series[0].rows[2].tuple[2] == "54");
    }

    TEST_CASE("Query throws on invalid result", "[QueryTest]")
    {
        using trompeloeil::_;

        TransportMock transport;
        InfluxDBParams params{};
        ALLOW_CALL(transport, query(_, _)).WITH(params.size() == 0)
            .RETURN(R"({"invalid-results":[]})");

        const auto result = internal::queryImpl(&transport, "SELECT * from test", params);
        CHECK(result.size() == 0);
    }

    TEST_CASE("Query is safe to empty name", "[QueryTest]")
    {
        using trompeloeil::_;

        TransportMock transport;
        InfluxDBParams params{};
        ALLOW_CALL(transport, query(_, _)).WITH(params.size() == 0)
            .RETURN(R"({"results":[{"statement_id":0,"series":[{"columns":["time","host","value"],)"
                    R"("values":[["2021-01-01:11:22.000000000Z","x",8]]}]}]})");

        const auto result = internal::queryImpl(&transport, "SELECT * from test");
        CHECK(result.size() == 1);
        CHECK(result[0].series[0].name == "");
    }

    TEST_CASE("Query reads optional tags element", "[QueryTest]")
    {
        using trompeloeil::_;

        TransportMock transport;
        InfluxDBParams params{};
        ALLOW_CALL(transport, query(_, _)).WITH(params.size() == 0)
            .RETURN(R"({"results":[{"statement_id":0,"series":[{"name":"x","tags":{"type":"sp"},"columns":["time","value"],)"
                    R"("values":[["2022-01-01:01:02.000000000ZZ",99]]}]}]})");

        const auto result = internal::queryImpl(&transport, "SELECT * from test");
        CHECK(result.size() == 1);
        CHECK(result[0].series[0].tagValues[0] == "sp");
    }
}
