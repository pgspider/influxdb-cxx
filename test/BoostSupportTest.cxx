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

#include "BoostSupport.h"
#include "InfluxDBException.h"
#include "ConnectionInfo.h"
#include "mock/TransportMock.h"
#include <catch2/catch.hpp>
#include <catch2/trompeloeil.hpp>
#include <sstream>

namespace influxdb::test
{
    //New tests for check connection, use ConnectionInfo
    TEST_CASE("With UDP returns transport", "[BoostSupportTest]")
    {
        auto conn = internal::ConnectionInfo::createConnectionInfoV1("", 0, "", "", "");
        CHECK(internal::withUdpTransport(conn.host, conn.port) != nullptr);
    }

    TEST_CASE("With Unix socket returns transport", "[BoostSupportTest]")
    {
        auto conn = internal::ConnectionInfo::createConnectionInfoV1("", 0, "", "", "");
        CHECK(internal::withUnixSocketTransport(conn.host) != nullptr);
    }

    TEST_CASE("UDP transport throws on create database", "[BoostSupportTest]")
    {
        auto conn = internal::ConnectionInfo::createConnectionInfoV1("", 0, "", "", "");
        auto udp = internal::withUdpTransport(conn.host, conn.port);
        CHECK_THROWS_AS(udp->createDatabase(), std::runtime_error);
    }

    TEST_CASE("Unix socket transport throws on create database", "[BoostSupportTest]")
    {
        auto conn = internal::ConnectionInfo::createConnectionInfoV1("", 0, "", "", "");
        auto udp = internal::withUnixSocketTransport(conn.host);
        CHECK_THROWS_AS(udp->createDatabase(), std::runtime_error);
    }

}
