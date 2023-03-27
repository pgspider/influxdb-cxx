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

#include "HTTP.h"
#include "InfluxDBException.h"
#include "Transport.h"
#include "mock/CurlMock.h"
#include <catch2/catch.hpp>
#include <catch2/trompeloeil.hpp>


namespace influxdb::test
{
    namespace
    {
        CurlHandleDummy dummy;
        CURL* handle = &dummy;
    }

    CurlMock curlMock;

    using influxdb::transports::HTTP;
    using trompeloeil::_;

    //Test for http_test connection api v1
    TEST_CASE("V1: Construction initializes curl", "[HttpTest]")
    {
        const std::string dbName{"test"};
        std::string returnValue = dbName;
        char* ptr = &returnValue[0];

        REQUIRE_CALL(curlMock, curl_global_init(CURL_GLOBAL_ALL)).RETURN(CURLE_OK);

        REQUIRE_CALL(curlMock, curl_easy_init()).RETURN(handle);
        REQUIRE_CALL(curlMock, curl_easy_init()).RETURN(handle);

        REQUIRE_CALL(curlMock, curl_easy_escape(_, ANY(char*), int{4})).RETURN(ptr);
        REQUIRE_CALL(curlMock, curl_free(_));
        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_TCP_KEEPIDLE, long{120})).RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_TCP_KEEPINTVL, long{60})).RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_WRITEFUNCTION, ANY(WriteCallbackFn))).RETURN(CURLE_OK);

        REQUIRE_CALL(curlMock, curl_easy_escape(_, ANY(char*), int{4})).RETURN(ptr);
        REQUIRE_CALL(curlMock, curl_free(_));
        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_TCP_KEEPIDLE, long{120})).RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_TCP_KEEPINTVL, long{60})).RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_WRITEFUNCTION, ANY(WriteCallbackFn))).RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_URL, "http://localhost:8086/write?db=test")).RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_POST, long{1})).RETURN(CURLE_OK);

        ALLOW_CALL(curlMock, curl_easy_cleanup(_));
        ALLOW_CALL(curlMock, curl_global_cleanup());

        auto conn = internal::ConnectionInfo::createConnectionInfoV1("http://localhost", 8086, "test");
        HTTP http{conn};
    }

    TEST_CASE("V1: Construction throws if curl init fails", "[HttpTest]")
    {
        ALLOW_CALL(curlMock, curl_global_init(CURL_GLOBAL_ALL)).RETURN(CURLE_FAILED_INIT);

        auto conn = internal::ConnectionInfo::createConnectionInfoV1("http://localhost", 8086, "test");
        CHECK_THROWS_AS(HTTP{conn}, InfluxDBException);
    }

    TEST_CASE("V1: Construction throws if curl write handle init fails", "[HttpTest]")
    {
        ALLOW_CALL(curlMock, curl_easy_escape(_, ANY(char*), ANY(int))).RETURN(&std::string(_2)[0]);
        ALLOW_CALL(curlMock, curl_free(_));
        ALLOW_CALL(curlMock, curl_global_init(CURL_GLOBAL_ALL)).RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_init()).RETURN(nullptr);
        ALLOW_CALL(curlMock, curl_easy_cleanup(_));
        ALLOW_CALL(curlMock, curl_global_cleanup());

        auto conn = internal::ConnectionInfo::createConnectionInfoV1("http://localhost", 8086, "test");
        CHECK_THROWS_AS(HTTP{conn}, InfluxDBException);
    }

    TEST_CASE("V1: Construction throws if curl read handle init fails", "[HttpTest]")
    {
        ALLOW_CALL(curlMock, curl_easy_escape(_, ANY(char*), ANY(int))).RETURN(&std::string(_2)[0]);
        ALLOW_CALL(curlMock, curl_free(_));
        ALLOW_CALL(curlMock, curl_global_init(CURL_GLOBAL_ALL)).RETURN(CURLE_OK);
        trompeloeil::sequence seq;
        REQUIRE_CALL(curlMock, curl_easy_init()).RETURN(handle).IN_SEQUENCE(seq);
        REQUIRE_CALL(curlMock, curl_easy_init()).RETURN(nullptr).IN_SEQUENCE(seq);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(std::string))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(long))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(WriteCallbackFn))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_cleanup(_));
        ALLOW_CALL(curlMock, curl_global_cleanup());

        auto conn = internal::ConnectionInfo::createConnectionInfoV1("http://localhost", 8086, "test");
        CHECK_THROWS_AS(HTTP{conn}, InfluxDBException);
    }

    TEST_CASE("Construction throws if no database parameter in url", "[HttpTest]")
    {
        ALLOW_CALL(curlMock, curl_global_init(CURL_GLOBAL_ALL)).RETURN(CURLE_OK);

        auto conn = internal::ConnectionInfo::createConnectionInfoV1("http://localhost", 8086, "");
        CHECK_THROWS_AS(HTTP{conn}, InfluxDBException);
    }

    TEST_CASE("V1: Destruction cleans up curl", "[HttpTest]")
    {
        ALLOW_CALL(curlMock, curl_global_init(CURL_GLOBAL_ALL)).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_init()).RETURN(handle);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(std::string))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(long))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(WriteCallbackFn))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_escape(_, ANY(char*), ANY(int))).RETURN(&std::string(_2)[0]);
        ALLOW_CALL(curlMock, curl_free(_));

        {
            REQUIRE_CALL(curlMock, curl_easy_cleanup(handle)).TIMES(2);
            REQUIRE_CALL(curlMock, curl_global_cleanup());

            auto conn = internal::ConnectionInfo::createConnectionInfoV1("http://localhost", 8086, "test");
            HTTP http{conn};
        }
    }

    TEST_CASE("V1: Send configures curl", "[HttpTest]")
    {
        ALLOW_CALL(curlMock, curl_global_init(_)).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_init()).RETURN(handle);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(std::string))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(long))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(WriteCallbackFn))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_cleanup(_));
        ALLOW_CALL(curlMock, curl_easy_escape(_, ANY(char*), ANY(int))).RETURN(&std::string(_2)[0]);
        ALLOW_CALL(curlMock, curl_free(_));
        ALLOW_CALL(curlMock, curl_global_cleanup());

        const std::string data{"content-to-send"};
        auto conn = internal::ConnectionInfo::createConnectionInfoV1("http://localhost", 8086, "test");
        HTTP http{conn};

        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_WRITEDATA, ANY(void*)))
            .LR_SIDE_EFFECT(*static_cast<std::string*>(_3) = "write-result")
            .RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_POSTFIELDS, data)).RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_POSTFIELDSIZE, static_cast<long>(data.size()))).RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_perform(handle)).RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_getinfo_(handle, CURLINFO_RESPONSE_CODE, _))
            .LR_SIDE_EFFECT(*static_cast<long*>(_3) = 200)
            .RETURN(CURLE_OK);

        http.send(std::string{data});
    }

    TEST_CASE("V1: Send fails on unsuccessful execution", "[HttpTest]")
    {
        ALLOW_CALL(curlMock, curl_global_init(_)).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_init()).RETURN(handle);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(std::string))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(long))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(WriteCallbackFn))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_cleanup(_));
        ALLOW_CALL(curlMock, curl_easy_escape(_, ANY(char*), ANY(int))).RETURN(&std::string(_2)[0]);
        ALLOW_CALL(curlMock, curl_free(_));
        ALLOW_CALL(curlMock, curl_global_cleanup());

        auto conn = internal::ConnectionInfo::createConnectionInfoV1("http://localhost", 8086, "test");
        HTTP http{conn};

        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_WRITEDATA, ANY(void*)))
            .LR_SIDE_EFFECT(*static_cast<std::string*>(_3) = "write-result")
            .RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_perform(handle)).RETURN(CURLE_FAILED_INIT);
        REQUIRE_CALL(curlMock, curl_easy_getinfo_(handle, CURLINFO_RESPONSE_CODE, _))
            .LR_SIDE_EFFECT(*static_cast<long*>(_3) = 99)
            .RETURN(CURLE_OK);

        REQUIRE_THROWS_AS(http.send("content"), ConnectionError);
    }

    TEST_CASE("V1: Send accepts successful response", "[HttpTest]")
    {
        ALLOW_CALL(curlMock, curl_global_init(_)).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_init()).RETURN(handle);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(std::string))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(long))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(WriteCallbackFn))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_cleanup(_));
        ALLOW_CALL(curlMock, curl_easy_escape(_, ANY(char*), ANY(int))).RETURN(&std::string(_2)[0]);
        ALLOW_CALL(curlMock, curl_free(_));
        ALLOW_CALL(curlMock, curl_global_cleanup());

        auto conn = internal::ConnectionInfo::createConnectionInfoV1("http://localhost", 8086, "test", "", "");
        HTTP http{conn};

        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_WRITEDATA, ANY(void*)))
            .LR_SIDE_EFFECT(*static_cast<std::string*>(_3) = "")
            .RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_perform(_)).RETURN(CURLE_OK);

        REQUIRE_CALL(curlMock, curl_easy_getinfo_(handle, CURLINFO_RESPONSE_CODE, _))
            .LR_SIDE_EFFECT(*static_cast<long*>(_3) = 200)
            .RETURN(CURLE_OK);
        http.send("content");

        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_WRITEDATA, ANY(void*)))
            .LR_SIDE_EFFECT(*static_cast<std::string*>(_3) = "")
            .RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_getinfo_(handle, CURLINFO_RESPONSE_CODE, _))
            .LR_SIDE_EFFECT(*static_cast<long*>(_3) = 204)
            .RETURN(CURLE_OK);
        http.send("content");
    }

    TEST_CASE("V1: Send throws on unsuccessful response", "[HttpTest]")
    {
        ALLOW_CALL(curlMock, curl_global_init(_)).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_init()).RETURN(handle);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(std::string))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(long))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(WriteCallbackFn))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_cleanup(_));
        ALLOW_CALL(curlMock, curl_easy_escape(_, ANY(char*), ANY(int))).RETURN(&std::string(_2)[0]);
        ALLOW_CALL(curlMock, curl_free(_));
        ALLOW_CALL(curlMock, curl_global_cleanup());

        auto conn = internal::ConnectionInfo::createConnectionInfoV1("http://localhost", 8086, "test");
        HTTP http{conn};

        ALLOW_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_WRITEDATA, ANY(void*)))
            .LR_SIDE_EFFECT(*static_cast<std::string*>(_3) = "{}")
            .RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_perform(_)).RETURN(CURLE_OK);

        REQUIRE_CALL(curlMock, curl_easy_getinfo_(handle, CURLINFO_RESPONSE_CODE, _))
            .LR_SIDE_EFFECT(*static_cast<long*>(_3) = 404)
            .RETURN(CURLE_OK);
        REQUIRE_THROWS_AS(http.send("content"), NonExistentDatabase);

        REQUIRE_CALL(curlMock, curl_easy_getinfo_(handle, CURLINFO_RESPONSE_CODE, _))
            .LR_SIDE_EFFECT(*static_cast<long*>(_3) = 400)
            .RETURN(CURLE_OK);
        REQUIRE_THROWS_AS(http.send("content"), BadRequest);

        REQUIRE_CALL(curlMock, curl_easy_getinfo_(handle, CURLINFO_RESPONSE_CODE, _))
            .LR_SIDE_EFFECT(*static_cast<long*>(_3) = 500)
            .RETURN(CURLE_OK);
        REQUIRE_THROWS_AS(http.send("content"), ServerError);

        REQUIRE_CALL(curlMock, curl_easy_getinfo_(handle, CURLINFO_RESPONSE_CODE, _))
            .LR_SIDE_EFFECT(*static_cast<long*>(_3) = 503)
            .RETURN(CURLE_OK);
        REQUIRE_THROWS_AS(http.send("content"), ServerError);
    }

    TEST_CASE("V1: Query configures curl", "[HttpTest]")
    {
        ALLOW_CALL(curlMock, curl_global_init(_)).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_init()).RETURN(handle);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(std::string))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(long))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(WriteCallbackFn))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_cleanup(_));
        ALLOW_CALL(curlMock, curl_easy_escape(_, ANY(char*), ANY(int))).RETURN(&std::string(_2)[0]);
        ALLOW_CALL(curlMock, curl_free(_));
        ALLOW_CALL(curlMock, curl_global_cleanup());

        auto conn = internal::ConnectionInfo::createConnectionInfoV1("http://localhost", 8086, "test");
        HTTP http{conn};

        const std::string query{"/12?ab=cd"};
        std::string returnValue = query;
        char* ptr = &returnValue[0];
        REQUIRE_CALL(curlMock, curl_easy_escape(_, query.c_str(), static_cast<int>(query.size()))).RETURN(ptr);
        ALLOW_CALL(curlMock, curl_free(ptr));
        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_URL, "http://localhost:8086/query?db=test&q=/12?ab=cd")).RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_WRITEDATA, ANY(void*)))
            .LR_SIDE_EFFECT(*static_cast<std::string*>(_3) = "query-result")
            .RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_perform(handle)).RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_getinfo_(handle, CURLINFO_RESPONSE_CODE, _))
            .LR_SIDE_EFFECT(*static_cast<long*>(_3) = 200)
            .RETURN(CURLE_OK);

        const auto result = http.query(query);
        CHECK(result == "query-result");
    }

    TEST_CASE("V1: Query fails on unsuccessful execution", "[HttpTest]")
    {
        ALLOW_CALL(curlMock, curl_global_init(_)).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_init()).RETURN(handle);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(std::string))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(long))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(WriteCallbackFn))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_cleanup(_));
        ALLOW_CALL(curlMock, curl_easy_escape(_, ANY(char*), ANY(int))).RETURN(&std::string(_2)[0]);
        ALLOW_CALL(curlMock, curl_free(_));
        ALLOW_CALL(curlMock, curl_global_cleanup());

        auto conn = internal::ConnectionInfo::createConnectionInfoV1("http://localhost", 8086, "test", "", "");
        HTTP http{conn};

        const std::string query{"/x?shouldfail=true"};
        std::string returnValue = query;
        char* ptr = &returnValue[0];
        ALLOW_CALL(curlMock, curl_easy_escape(_, ANY(char*), static_cast<int>(query.size()))).RETURN(ptr);
        ALLOW_CALL(curlMock, curl_free(_));
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_WRITEDATA, ANY(void*)))
            .LR_SIDE_EFFECT(*static_cast<std::string*>(_3) = "query-result")
            .RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_perform(handle)).RETURN(CURLE_FAILED_INIT);
        REQUIRE_CALL(curlMock, curl_easy_getinfo_(handle, CURLINFO_RESPONSE_CODE, _))
            .LR_SIDE_EFFECT(*static_cast<long*>(_3) = 99)
            .RETURN(CURLE_OK);

        REQUIRE_THROWS_AS(http.query(query), ConnectionError);
    }

    TEST_CASE("V1: Query accepts successful response", "[HttpTest]")
    {
        ALLOW_CALL(curlMock, curl_global_init(_)).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_init()).RETURN(handle);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(std::string))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(long))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(WriteCallbackFn))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_cleanup(_));
        ALLOW_CALL(curlMock, curl_easy_escape(_, ANY(char*), ANY(int))).RETURN(&std::string(_2)[0]);
        ALLOW_CALL(curlMock, curl_free(_));
        ALLOW_CALL(curlMock, curl_global_cleanup());

        auto conn = internal::ConnectionInfo::createConnectionInfoV1("http://localhost", 8086, "test", "", "");
        HTTP http{conn};

        const std::string query{"/x?shouldfail=true"};
        std::string returnValue = query;
        char* ptr = &returnValue[0];
        ALLOW_CALL(curlMock, curl_easy_escape(_, query.c_str(), static_cast<int>(query.size()))).RETURN(ptr);
        ALLOW_CALL(curlMock, curl_free(_));
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_WRITEDATA, ANY(void*)))
            .LR_SIDE_EFFECT(*static_cast<std::string*>(_3) = "query-result")
            .RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_perform(handle)).RETURN(CURLE_OK);

        REQUIRE_CALL(curlMock, curl_easy_getinfo_(handle, CURLINFO_RESPONSE_CODE, _))
            .LR_SIDE_EFFECT(*static_cast<long*>(_3) = 200)
            .RETURN(CURLE_OK);
        http.query(query);

        REQUIRE_CALL(curlMock, curl_easy_getinfo_(handle, CURLINFO_RESPONSE_CODE, _))
            .LR_SIDE_EFFECT(*static_cast<long*>(_3) = 204)
            .RETURN(CURLE_OK);

        const auto result = http.query(query);
        CHECK(result == "query-result");
    }

    TEST_CASE("V1: Query throws on unsuccessful response", "[HttpTest]")
    {
        ALLOW_CALL(curlMock, curl_global_init(_)).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_init()).RETURN(handle);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_URL, ANY(std::string))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(long))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(WriteCallbackFn))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_cleanup(_));
        ALLOW_CALL(curlMock, curl_easy_escape(_, ANY(char*), ANY(int))).RETURN(&std::string(_2)[0]);
        ALLOW_CALL(curlMock, curl_free(_));
        ALLOW_CALL(curlMock, curl_global_cleanup());

        auto conn = internal::ConnectionInfo::createConnectionInfoV1("http://localhost", 8086, "test");
        HTTP http{conn};

        const std::string query{"/x?shouldfail=true"};
        std::string returnValue = query;
        char* ptr = &returnValue[0];
        ALLOW_CALL(curlMock, curl_easy_escape(handle, query.c_str(), static_cast<int>(query.size()))).RETURN(ptr);
        ALLOW_CALL(curlMock, curl_free(_));
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_WRITEDATA, ANY(void*)))
            .LR_SIDE_EFFECT(*static_cast<std::string*>(_3) = "{}")
            .RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_perform(handle)).RETURN(CURLE_OK);

        REQUIRE_CALL(curlMock, curl_easy_getinfo_(handle, CURLINFO_RESPONSE_CODE, _))
            .LR_SIDE_EFFECT(*static_cast<long*>(_3) = 404)
            .RETURN(CURLE_OK);
        REQUIRE_THROWS_AS(http.query(query), NonExistentDatabase);

        REQUIRE_CALL(curlMock, curl_easy_getinfo_(handle, CURLINFO_RESPONSE_CODE, _))
            .LR_SIDE_EFFECT(*static_cast<long*>(_3) = 400)
            .RETURN(CURLE_OK);
        REQUIRE_THROWS_AS(http.query(query), BadRequest);

        REQUIRE_CALL(curlMock, curl_easy_getinfo_(handle, CURLINFO_RESPONSE_CODE, _))
            .LR_SIDE_EFFECT(*static_cast<long*>(_3) = 500)
            .RETURN(CURLE_OK);
        REQUIRE_THROWS_AS(http.query(query), ServerError);

        REQUIRE_CALL(curlMock, curl_easy_getinfo_(handle, CURLINFO_RESPONSE_CODE, _))
            .LR_SIDE_EFFECT(*static_cast<long*>(_3) = 503)
            .RETURN(CURLE_OK);
        REQUIRE_THROWS_AS(http.query(query), ServerError);
    }

    TEST_CASE("V1: Create database configures curl", "[HttpTest]")
    {
        ALLOW_CALL(curlMock, curl_global_init(_)).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_init()).RETURN(handle);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(std::string))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(long))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(WriteCallbackFn))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_escape(_, ANY(char*), ANY(int))).RETURN(&std::string(_2)[0]);
        ALLOW_CALL(curlMock, curl_free(_));
        ALLOW_CALL(curlMock, curl_easy_cleanup(_));
        ALLOW_CALL(curlMock, curl_global_cleanup());

        auto conn = internal::ConnectionInfo::createConnectionInfoV1("http://localhost", 8086, "example-to-create");
        HTTP http{conn};

        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_URL, "http://localhost:8086/query?db=example-to-create&q=CREATE DATABASE example-to-create")).RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_WRITEDATA, ANY(void*)))
            .LR_SIDE_EFFECT(*static_cast<std::string*>(_3) = "")
            .RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_perform(handle)).RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_getinfo_(handle, CURLINFO_RESPONSE_CODE, _))
            .LR_SIDE_EFFECT(*static_cast<long*>(_3) = 200)
            .RETURN(CURLE_OK);

        http.createDatabase();
    }

    TEST_CASE("V1: Create database fails on unsuccessful execution", "[HttpTest]")
    {
        ALLOW_CALL(curlMock, curl_global_init(_)).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_init()).RETURN(handle);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(std::string))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(long))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(WriteCallbackFn))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_cleanup(_));
        ALLOW_CALL(curlMock, curl_easy_escape(_, ANY(char*), ANY(int))).RETURN(&std::string(_2)[0]);
        ALLOW_CALL(curlMock, curl_free(_));
        ALLOW_CALL(curlMock, curl_global_cleanup());

        auto conn = internal::ConnectionInfo::createConnectionInfoV1("http://localhost", 8086, "example-to-create");
        HTTP http{conn};

        ALLOW_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_WRITEDATA, ANY(void*)))
            .LR_SIDE_EFFECT(*static_cast<std::string*>(_3) = "")
            .RETURN(CURLE_OK);
        const std::string data = "q=CREATE DATABASE example-to-create";
        REQUIRE_CALL(curlMock, curl_easy_perform(handle)).RETURN(CURLE_FAILED_INIT);
        REQUIRE_CALL(curlMock, curl_easy_getinfo_(handle, CURLINFO_RESPONSE_CODE, _))
            .LR_SIDE_EFFECT(*static_cast<long*>(_3) = 200)
            .RETURN(CURLE_OK);

        REQUIRE_THROWS_AS(http.createDatabase(), ConnectionError);
    }

    TEST_CASE("V1: Create database accepts successful response", "[HttpTest]")
    {
        ALLOW_CALL(curlMock, curl_global_init(_)).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_init()).RETURN(handle);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(std::string))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(long))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(WriteCallbackFn))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_cleanup(_));
        ALLOW_CALL(curlMock, curl_easy_escape(_, ANY(char*), ANY(int))).RETURN(&std::string(_2)[0]);
        ALLOW_CALL(curlMock, curl_free(_));
        ALLOW_CALL(curlMock, curl_global_cleanup());

        auto conn = internal::ConnectionInfo::createConnectionInfoV1("http://localhost", 8086, "example-to-create");
        HTTP http{conn};

        const std::string data = "q=CREATE DATABASE example-to-create";
        ALLOW_CALL(curlMock, curl_easy_perform(handle)).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_WRITEDATA, ANY(void*)))
            .LR_SIDE_EFFECT(*static_cast<std::string*>(_3) = "")
            .RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_getinfo_(handle, CURLINFO_RESPONSE_CODE, _))
            .LR_SIDE_EFFECT(*static_cast<long*>(_3) = 200)
            .RETURN(CURLE_OK);
        http.createDatabase();

        REQUIRE_CALL(curlMock, curl_easy_getinfo_(handle, CURLINFO_RESPONSE_CODE, _))
            .LR_SIDE_EFFECT(*static_cast<long*>(_3) = 204)
            .RETURN(CURLE_OK);
        http.createDatabase();
    }

    TEST_CASE("V1: Create database throws on unsuccessful response", "[HttpTest]")
    {
        ALLOW_CALL(curlMock, curl_global_init(_)).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_init()).RETURN(handle);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(std::string))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(long))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(WriteCallbackFn))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_cleanup(_));
        ALLOW_CALL(curlMock, curl_easy_escape(_, ANY(char*), ANY(int))).RETURN(&std::string(_2)[0]);
        ALLOW_CALL(curlMock, curl_free(_));
        ALLOW_CALL(curlMock, curl_global_cleanup());

        auto conn = internal::ConnectionInfo::createConnectionInfoV1("http://localhost", 8086, "example-to-create");
        HTTP http{conn};

        const std::string data = "q=CREATE DATABASE example-to-create";
        ALLOW_CALL(curlMock, curl_easy_perform(handle)).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_WRITEDATA, ANY(void*)))
            .LR_SIDE_EFFECT(*static_cast<std::string*>(_3) = "{}")
            .RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_getinfo_(handle, CURLINFO_RESPONSE_CODE, _))
            .LR_SIDE_EFFECT(*static_cast<long*>(_3) = 404)
            .RETURN(CURLE_OK);
        REQUIRE_THROWS_AS(http.createDatabase(), NonExistentDatabase);

        REQUIRE_CALL(curlMock, curl_easy_getinfo_(handle, CURLINFO_RESPONSE_CODE, _))
            .LR_SIDE_EFFECT(*static_cast<long*>(_3) = 400)
            .RETURN(CURLE_OK);
        REQUIRE_THROWS_AS(http.createDatabase(), BadRequest);

        REQUIRE_CALL(curlMock, curl_easy_getinfo_(handle, CURLINFO_RESPONSE_CODE, _))
            .LR_SIDE_EFFECT(*static_cast<long*>(_3) = 500)
            .RETURN(CURLE_OK);
        REQUIRE_THROWS_AS(http.createDatabase(), ServerError);

        REQUIRE_CALL(curlMock, curl_easy_getinfo_(handle, CURLINFO_RESPONSE_CODE, _))
            .LR_SIDE_EFFECT(*static_cast<long*>(_3) = 503)
            .RETURN(CURLE_OK);
        REQUIRE_THROWS_AS(http.createDatabase(), ServerError);
    }

    TEST_CASE("V1: Enabling basic auth sets curl options", "[HttpTest]")
    {
        ALLOW_CALL(curlMock, curl_global_init(_)).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_init()).RETURN(handle);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(std::string))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(long))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(WriteCallbackFn))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_cleanup(_));
        ALLOW_CALL(curlMock, curl_easy_escape(_, ANY(char*), ANY(int))).RETURN(&std::string(_2)[0]);
        ALLOW_CALL(curlMock, curl_free(_));
        ALLOW_CALL(curlMock, curl_global_cleanup());

        auto conn = internal::ConnectionInfo::createConnectionInfoV1("http://localhost", 8086, "example-database-0", "", "");
        HTTP http{conn};

        REQUIRE_CALL(curlMock, curl_easy_setopt_(handle, CURLOPT_HTTPAUTH, CURLAUTH_BASIC)).RETURN(CURLE_OK).TIMES(2);
        REQUIRE_CALL(curlMock, curl_easy_setopt_(handle, CURLOPT_USERPWD, "user0:pass0")).RETURN(CURLE_OK).TIMES(2);
        http.enableBasicAuth("user0:pass0");
    }

    TEST_CASE("V1: Database name is returned if valid", "[HttpTest]")
    {
        ALLOW_CALL(curlMock, curl_free(_));
        ALLOW_CALL(curlMock, curl_global_init(_)).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_escape(_, ANY(char*), ANY(int))).RETURN(&std::string(_2)[0]);
        ALLOW_CALL(curlMock, curl_easy_init()).RETURN(handle);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(std::string))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(long))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(WriteCallbackFn))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_cleanup(_));
        ALLOW_CALL(curlMock, curl_global_cleanup());

        auto conn = internal::ConnectionInfo::createConnectionInfoV1("http://localhost", 8086, "example-database-0", "", "");
        const HTTP http{conn};
        CHECK(http.databaseName() == "example-database-0");
    }

    TEST_CASE("V1: Database service url is returned if valid", "[HttpTest]")
    {
        ALLOW_CALL(curlMock, curl_easy_escape(_, ANY(char*), ANY(int))).RETURN(&std::string(_2)[0]);
        ALLOW_CALL(curlMock, curl_free(_));
        ALLOW_CALL(curlMock, curl_global_init(_)).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_init()).RETURN(handle);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(std::string))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(long))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(WriteCallbackFn))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_cleanup(_));
        ALLOW_CALL(curlMock, curl_global_cleanup());

        auto conn = internal::ConnectionInfo::createConnectionInfoV1("http://localhost", 8086, "example-database-1", "", "");
        const HTTP http{conn};
        CHECK(http.influxDbServiceUrl() == "http://localhost:8086");
    }

    TEST_CASE("V1: Query with valid param", "[HttpTest]")
    {
        ALLOW_CALL(curlMock, curl_global_init(_)).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_init()).RETURN(handle);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(std::string))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(long))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(WriteCallbackFn))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_cleanup(_));
        ALLOW_CALL(curlMock, curl_easy_escape(_, ANY(char*), ANY(int))).RETURN(&std::string(_2)[0]);
        ALLOW_CALL(curlMock, curl_free(_));
        ALLOW_CALL(curlMock, curl_global_cleanup());

        auto conn = internal::ConnectionInfo::createConnectionInfoV1("http://localhost", 8086, "test");
        HTTP http{conn};

        const std::string query{"SELECT * FROM tbl WHERE c1=$param1"};
        std::string returnValue = query;
        char* ptr = &returnValue[0];
        auto params = influxdb::InfluxDBParams{}.addParam("param1", 1);
        REQUIRE_CALL(curlMock, curl_easy_escape(_, query.c_str(), static_cast<int>(query.size()))).RETURN(ptr);
        ALLOW_CALL(curlMock, curl_free(ptr));
        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_URL, "http://localhost:8086/query?db=test&q=SELECT * FROM tbl WHERE c1=$param1&params={\"param1\":1}")).RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_WRITEDATA, ANY(void*)))
            .LR_SIDE_EFFECT(*static_cast<std::string*>(_3) = "query-result")
            .RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_perform(handle)).RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_getinfo_(handle, CURLINFO_RESPONSE_CODE, _))
            .LR_SIDE_EFFECT(*static_cast<long*>(_3) = 200)
            .RETURN(CURLE_OK);

        const auto result = http.query(query, params);
        CHECK(result == "query-result");
    }

    TEST_CASE("V1: Query with no param", "[HttpTest]")
    {
        ALLOW_CALL(curlMock, curl_global_init(_)).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_init()).RETURN(handle);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(std::string))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(long))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(WriteCallbackFn))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_cleanup(_));
        ALLOW_CALL(curlMock, curl_easy_escape(_, ANY(char*), ANY(int))).RETURN(&std::string(_2)[0]);
        ALLOW_CALL(curlMock, curl_free(_));
        ALLOW_CALL(curlMock, curl_global_cleanup());

        auto conn = internal::ConnectionInfo::createConnectionInfoV1("http://localhost", 8086, "test");
        HTTP http{conn};

        const std::string query{"SELECT * FROM tbl WHERE c1=$param1"};
        std::string returnValue = query;
        char* ptr = &returnValue[0];
        auto params = influxdb::InfluxDBParams{};
        REQUIRE_CALL(curlMock, curl_easy_escape(_, query.c_str(), static_cast<int>(query.size()))).RETURN(ptr);
        ALLOW_CALL(curlMock, curl_free(ptr));
        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_URL, "http://localhost:8086/query?db=test&q=SELECT * FROM tbl WHERE c1=$param1")).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_WRITEDATA, ANY(void*)))
            .LR_SIDE_EFFECT(*static_cast<std::string*>(_3) = "{}")
            .RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_perform(handle)).RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_getinfo_(handle, CURLINFO_RESPONSE_CODE, _))
            .LR_SIDE_EFFECT(*static_cast<long*>(_3) = 400)
            .RETURN(CURLE_OK);

        REQUIRE_THROWS_AS(http.query(query), BadRequest);
    }

    TEST_CASE("V1: Query is empty", "[HttpTest]")
    {
        ALLOW_CALL(curlMock, curl_global_init(_)).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_init()).RETURN(handle);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(std::string))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(long))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(WriteCallbackFn))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_cleanup(_));
        ALLOW_CALL(curlMock, curl_easy_escape(_, ANY(char*), ANY(int))).RETURN(&std::string(_2)[0]);
        ALLOW_CALL(curlMock, curl_free(_));
        ALLOW_CALL(curlMock, curl_global_cleanup());

        auto conn = internal::ConnectionInfo::createConnectionInfoV1("http://localhost", 8086, "test");
        HTTP http{conn};

        const std::string query{};
        std::string returnValue = query;
        char* ptr = &returnValue[0];
        REQUIRE_CALL(curlMock, curl_easy_escape(_, query.c_str(), static_cast<int>(query.size()))).RETURN(ptr);
        ALLOW_CALL(curlMock, curl_free(ptr));
        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_URL, "http://localhost:8086/query?db=test&q=")).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_WRITEDATA, ANY(void*)))
            .LR_SIDE_EFFECT(*static_cast<std::string*>(_3) = "{}")
            .RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_perform(handle)).RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_getinfo_(handle, CURLINFO_RESPONSE_CODE, _))
            .LR_SIDE_EFFECT(*static_cast<long*>(_3) = 400)
            .RETURN(CURLE_OK);

        REQUIRE_THROWS_AS(http.query(query), BadRequest);
    }

    TEST_CASE("V1: Query on database have special characters", "[HttpTest]")
    {
        ALLOW_CALL(curlMock, curl_global_init(_)).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_init()).RETURN(handle);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(std::string))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(long))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(WriteCallbackFn))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_cleanup(_));
        ALLOW_CALL(curlMock, curl_easy_escape(_, ANY(char*), ANY(int))).RETURN(&std::string(_2)[0]);
        ALLOW_CALL(curlMock, curl_free(_));
        ALLOW_CALL(curlMock, curl_global_cleanup());

        auto conn = internal::ConnectionInfo::createConnectionInfoV1("http://localhost", 8086, "test\\");
        HTTP http{conn};

        const std::string query{"SELECT * FROM test"};
        std::string returnValue = query;
        char* ptr = &returnValue[0];
        ALLOW_CALL(curlMock, curl_easy_escape(_, query.c_str(), static_cast<int>(query.size()))).RETURN(ptr);
        ALLOW_CALL(curlMock, curl_free(_));
        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_URL, R"(http://localhost:8086/query?db=test\&q=SELECT * FROM test)")).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_WRITEDATA, ANY(void*)))
            .LR_SIDE_EFFECT(*static_cast<std::string*>(_3) = "query-result")
            .RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_perform(handle)).RETURN(CURLE_OK);

        REQUIRE_CALL(curlMock, curl_easy_getinfo_(handle, CURLINFO_RESPONSE_CODE, _))
            .LR_SIDE_EFFECT(*static_cast<long*>(_3) = 200)
            .RETURN(CURLE_OK);

        const auto result = http.query(query);
        CHECK(result == "query-result");
    }

    //Test for http_test connection api v2
    TEST_CASE("V2: Construction initializes curl", "[HttpTest]")
    {
        REQUIRE_CALL(curlMock, curl_global_init(CURL_GLOBAL_ALL)).RETURN(CURLE_OK);

        REQUIRE_CALL(curlMock, curl_easy_init()).RETURN(handle);
        REQUIRE_CALL(curlMock, curl_easy_init()).RETURN(handle);

        REQUIRE_CALL(curlMock, curl_easy_escape(_, ANY(char *), int{4})).RETURN(&std::string(_2)[0]);
        REQUIRE_CALL(curlMock, curl_free(_));
        REQUIRE_CALL(curlMock, curl_easy_escape(_, ANY(char *), int{2})).RETURN(&std::string(_2)[0]);
        REQUIRE_CALL(curlMock, curl_free(_));
        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_TCP_KEEPIDLE, long{120})).RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_TCP_KEEPINTVL, long{60})).RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_WRITEFUNCTION, ANY(WriteCallbackFn))).RETURN(CURLE_OK);

        REQUIRE_CALL(curlMock, curl_easy_escape(_, ANY(char *), int{4})).RETURN(&std::string(_2)[0]);
        REQUIRE_CALL(curlMock, curl_free(_));
        REQUIRE_CALL(curlMock, curl_easy_escape(_, ANY(char *), int{2})).RETURN(&std::string(_2)[0]);
        REQUIRE_CALL(curlMock, curl_free(_));
        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_TCP_KEEPIDLE, long{120})).RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_TCP_KEEPINTVL, long{60})).RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_WRITEFUNCTION, ANY(WriteCallbackFn))).RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_URL, "http://localhost:8086/write?db=test&rp=rp")).RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_POST, long{1})).RETURN(CURLE_OK);

        ALLOW_CALL(curlMock, curl_easy_cleanup(_));
        ALLOW_CALL(curlMock, curl_global_cleanup());

        auto conn = internal::ConnectionInfo::createConnectionInfoV2("http://localhost", 8086, "test", "aheifAwakjfAPWwe", "rp");
        HTTP http{conn};
    }

    TEST_CASE("V2: Construction throws if curl init fails", "[HttpTest]")
    {
        ALLOW_CALL(curlMock, curl_global_init(CURL_GLOBAL_ALL)).RETURN(CURLE_FAILED_INIT);

        auto conn = internal::ConnectionInfo::createConnectionInfoV2("http://localhost", 8086, "test", "aheifAwakjfAPWwe", "rp");
        CHECK_THROWS_AS(HTTP{conn}, InfluxDBException);
    }

    TEST_CASE("V2: Construction throws if curl write handle init fails", "[HttpTest]")
    {
        ALLOW_CALL(curlMock, curl_global_init(CURL_GLOBAL_ALL)).RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_init()).RETURN(nullptr);
        ALLOW_CALL(curlMock, curl_easy_cleanup(_));
        ALLOW_CALL(curlMock, curl_easy_escape(_, ANY(char*), ANY(int))).RETURN(&std::string(_2)[0]);
        ALLOW_CALL(curlMock, curl_free(_));
        ALLOW_CALL(curlMock, curl_global_cleanup());

        auto conn = internal::ConnectionInfo::createConnectionInfoV2("http://localhost", 8086, "test", "aheifAwakjfAPWwe", "rp");
        CHECK_THROWS_AS(HTTP{conn}, InfluxDBException);
    }

    TEST_CASE("V2: Construction throws if curl read handle init fails", "[HttpTest]")
    {
        ALLOW_CALL(curlMock, curl_global_init(CURL_GLOBAL_ALL)).RETURN(CURLE_OK);
        trompeloeil::sequence seq;
        REQUIRE_CALL(curlMock, curl_easy_init()).RETURN(handle).IN_SEQUENCE(seq);
        REQUIRE_CALL(curlMock, curl_easy_init()).RETURN(nullptr).IN_SEQUENCE(seq);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(std::string))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(long))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(WriteCallbackFn))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_cleanup(_));
        ALLOW_CALL(curlMock, curl_easy_escape(_, ANY(char*), ANY(int))).RETURN(&std::string(_2)[0]);
        ALLOW_CALL(curlMock, curl_free(_));
        ALLOW_CALL(curlMock, curl_global_cleanup());

        auto conn = internal::ConnectionInfo::createConnectionInfoV2("http://localhost", 8086, "test", "aheifAwakjfAPWwe", "rp");
        CHECK_THROWS_AS(HTTP{conn}, InfluxDBException);
    }

    TEST_CASE("V2: Construction throws if no database parameter in url", "[HttpTest]")
    {
        ALLOW_CALL(curlMock, curl_global_init(CURL_GLOBAL_ALL)).RETURN(CURLE_OK);

        auto conn = internal::ConnectionInfo::createConnectionInfoV2("http://localhost", 8086, "", "aheifAwakjfAPWwe", "rp");
        CHECK_THROWS_AS(HTTP{conn}, InfluxDBException);
    }

    TEST_CASE("V2: Destruction cleans up curl", "[HttpTest]")
    {
        ALLOW_CALL(curlMock, curl_global_init(CURL_GLOBAL_ALL)).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_init()).RETURN(handle);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(std::string))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(long))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(WriteCallbackFn))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_escape(_, ANY(char*), ANY(int))).RETURN(&std::string(_2)[0]);
        ALLOW_CALL(curlMock, curl_free(_));

        {
            REQUIRE_CALL(curlMock, curl_easy_cleanup(handle)).TIMES(2);
            REQUIRE_CALL(curlMock, curl_global_cleanup());

            auto conn = internal::ConnectionInfo::createConnectionInfoV2("http://localhost", 8086, "test", "aheifAwakjfAPWwe", "rp");
            HTTP http{conn};
        }
    }

    TEST_CASE("V2: Send configures curl", "[HttpTest]")
    {
        ALLOW_CALL(curlMock, curl_global_init(_)).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_init()).RETURN(handle);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(std::string))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(long))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(WriteCallbackFn))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_cleanup(_));
        ALLOW_CALL(curlMock, curl_easy_escape(_, ANY(char*), ANY(int))).RETURN(&std::string(_2)[0]);
        ALLOW_CALL(curlMock, curl_free(_));
        ALLOW_CALL(curlMock, curl_global_cleanup());

        const std::string data{"content-to-send"};
        auto conn = internal::ConnectionInfo::createConnectionInfoV2("http://localhost", 8086, "test", "aheifAwakjfAPWwe", "rp");
        HTTP http{conn};

        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_WRITEDATA, ANY(void*)))
            .LR_SIDE_EFFECT(*static_cast<std::string*>(_3) = "")
            .RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_POSTFIELDS, data)).RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_POSTFIELDSIZE, static_cast<long>(data.size()))).RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_perform(handle)).RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_getinfo_(handle, CURLINFO_RESPONSE_CODE, _))
            .LR_SIDE_EFFECT(*static_cast<long*>(_3) = 200)
            .RETURN(CURLE_OK);

        http.send(std::string{data});
    }

    TEST_CASE("V2: Send fails on unsuccessful execution", "[HttpTest]")
    {
        ALLOW_CALL(curlMock, curl_global_init(_)).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_init()).RETURN(handle);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(std::string))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(long))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(WriteCallbackFn))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_cleanup(_));
        ALLOW_CALL(curlMock, curl_easy_escape(_, ANY(char*), ANY(int))).RETURN(&std::string(_2)[0]);
        ALLOW_CALL(curlMock, curl_free(_));
        ALLOW_CALL(curlMock, curl_global_cleanup());

        auto conn = internal::ConnectionInfo::createConnectionInfoV2("http://localhost", 8086, "test", "aheifAwakjfAPWwe", "rp");
        HTTP http{conn};

        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_WRITEDATA, ANY(void*)))
            .LR_SIDE_EFFECT(*static_cast<std::string*>(_3) = "")
            .RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_perform(handle)).RETURN(CURLE_FAILED_INIT);
        REQUIRE_CALL(curlMock, curl_easy_getinfo_(handle, CURLINFO_RESPONSE_CODE, _))
            .LR_SIDE_EFFECT(*static_cast<long*>(_3) = 99)
            .RETURN(CURLE_OK);

        REQUIRE_THROWS_AS(http.send("content"), ConnectionError);
    }

    TEST_CASE("V2: Send accepts successful response", "[HttpTest]")
    {
        ALLOW_CALL(curlMock, curl_global_init(_)).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_init()).RETURN(handle);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(std::string))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(long))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(WriteCallbackFn))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_cleanup(_));
        ALLOW_CALL(curlMock, curl_easy_escape(_, ANY(char*), ANY(int))).RETURN(&std::string(_2)[0]);
        ALLOW_CALL(curlMock, curl_free(_));
        ALLOW_CALL(curlMock, curl_global_cleanup());

        auto conn = internal::ConnectionInfo::createConnectionInfoV2("http://localhost", 8086, "test", "aheifAwakjfAPWwe", "rp");
        HTTP http{conn};

        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_WRITEDATA, ANY(void*)))
            .LR_SIDE_EFFECT(*static_cast<std::string*>(_3) = "write-result")
            .RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_perform(_)).RETURN(CURLE_OK);

        REQUIRE_CALL(curlMock, curl_easy_getinfo_(handle, CURLINFO_RESPONSE_CODE, _))
            .LR_SIDE_EFFECT(*static_cast<long*>(_3) = 200)
            .RETURN(CURLE_OK);
        http.send("content");

        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_WRITEDATA, ANY(void*)))
            .LR_SIDE_EFFECT(*static_cast<std::string*>(_3) = "write-result")
            .RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_getinfo_(handle, CURLINFO_RESPONSE_CODE, _))
            .LR_SIDE_EFFECT(*static_cast<long*>(_3) = 204)
            .RETURN(CURLE_OK);
        http.send("content");
    }

    TEST_CASE("V2: Send throws on unsuccessful response", "[HttpTest]")
    {
        ALLOW_CALL(curlMock, curl_global_init(_)).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_init()).RETURN(handle);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(std::string))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(long))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(WriteCallbackFn))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_cleanup(_));
        ALLOW_CALL(curlMock, curl_easy_escape(_, ANY(char*), ANY(int))).RETURN(&std::string(_2)[0]);
        ALLOW_CALL(curlMock, curl_free(_));
        ALLOW_CALL(curlMock, curl_global_cleanup());

        auto conn = internal::ConnectionInfo::createConnectionInfoV2("http://localhost", 8086, "test", "aheifAwakjfAPWwe", "rp");
        HTTP http{conn};

        ALLOW_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_WRITEDATA, ANY(void*)))
            .LR_SIDE_EFFECT(*static_cast<std::string*>(_3) = "{}")
            .RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_perform(_)).RETURN(CURLE_OK);

        REQUIRE_CALL(curlMock, curl_easy_getinfo_(handle, CURLINFO_RESPONSE_CODE, _))
            .LR_SIDE_EFFECT(*static_cast<long*>(_3) = 404)
            .RETURN(CURLE_OK);
        REQUIRE_THROWS_AS(http.send("content"), NonExistentDatabase);

        REQUIRE_CALL(curlMock, curl_easy_getinfo_(handle, CURLINFO_RESPONSE_CODE, _))
            .LR_SIDE_EFFECT(*static_cast<long*>(_3) = 400)
            .RETURN(CURLE_OK);
        REQUIRE_THROWS_AS(http.send("content"), BadRequest);

        REQUIRE_CALL(curlMock, curl_easy_getinfo_(handle, CURLINFO_RESPONSE_CODE, _))
            .LR_SIDE_EFFECT(*static_cast<long*>(_3) = 500)
            .RETURN(CURLE_OK);
        REQUIRE_THROWS_AS(http.send("content"), ServerError);

        REQUIRE_CALL(curlMock, curl_easy_getinfo_(handle, CURLINFO_RESPONSE_CODE, _))
            .LR_SIDE_EFFECT(*static_cast<long*>(_3) = 503)
            .RETURN(CURLE_OK);
        REQUIRE_THROWS_AS(http.send("content"), ServerError);
    }

    TEST_CASE("V2: Query configures curl", "[HttpTest]")
    {
        ALLOW_CALL(curlMock, curl_global_init(_)).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_init()).RETURN(handle);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(std::string))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(long))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(WriteCallbackFn))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_cleanup(_));
        ALLOW_CALL(curlMock, curl_easy_escape(_, ANY(char*), ANY(int))).RETURN(&std::string(_2)[0]);
        ALLOW_CALL(curlMock, curl_free(_));
        ALLOW_CALL(curlMock, curl_global_cleanup());

        auto conn = internal::ConnectionInfo::createConnectionInfoV2("http://localhost", 8086, "test", "aheifAwakjfAPWwe", "rp");
        HTTP http{conn};

        const std::string query{"/12?ab=cd"};
        std::string returnValue = query;
        char* ptr = &returnValue[0];
        REQUIRE_CALL(curlMock, curl_easy_escape(_, query.c_str(), static_cast<int>(query.size()))).RETURN(ptr);
        ALLOW_CALL(curlMock, curl_free(ptr));
        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_URL, "http://localhost:8086/query?db=test&rp=rp&q=/12?ab=cd")).RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_WRITEDATA, ANY(void*)))
            .LR_SIDE_EFFECT(*static_cast<std::string*>(_3) = "query-result")
            .RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_perform(handle)).RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_getinfo_(handle, CURLINFO_RESPONSE_CODE, _))
            .LR_SIDE_EFFECT(*static_cast<long*>(_3) = 200)
            .RETURN(CURLE_OK);

        const auto result = http.query(query);
        CHECK(result == "query-result");
    }

    TEST_CASE("V2: Query fails on unsuccessful execution", "[HttpTest]")
    {
        ALLOW_CALL(curlMock, curl_global_init(_)).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_init()).RETURN(handle);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(std::string))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(long))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(WriteCallbackFn))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_cleanup(_));
        ALLOW_CALL(curlMock, curl_easy_escape(_, ANY(char*), ANY(int))).RETURN(&std::string(_2)[0]);
        ALLOW_CALL(curlMock, curl_free(_));
        ALLOW_CALL(curlMock, curl_global_cleanup());

        auto conn = internal::ConnectionInfo::createConnectionInfoV2("http://localhost", 8086, "test", "aheifAwakjfAPWwe", "rp");
        HTTP http{conn};

        const std::string query{"/x?shouldfail=true"};
        std::string returnValue = query;
        char* ptr = &returnValue[0];
        ALLOW_CALL(curlMock, curl_easy_escape(_, query.c_str(), static_cast<int>(query.size()))).RETURN(ptr);
        ALLOW_CALL(curlMock, curl_free(_));
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_WRITEDATA, ANY(void*)))
            .LR_SIDE_EFFECT(*static_cast<std::string*>(_3) = "query-result")
            .RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_perform(handle)).RETURN(CURLE_FAILED_INIT);
        REQUIRE_CALL(curlMock, curl_easy_getinfo_(handle, CURLINFO_RESPONSE_CODE, _))
            .LR_SIDE_EFFECT(*static_cast<long*>(_3) = 99)
            .RETURN(CURLE_OK);

        REQUIRE_THROWS_AS(http.query(query), ConnectionError);
    }

    TEST_CASE("V2: Query accepts successful response", "[HttpTest]")
    {
        ALLOW_CALL(curlMock, curl_global_init(_)).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_init()).RETURN(handle);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(std::string))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(long))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(WriteCallbackFn))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_cleanup(_));
        ALLOW_CALL(curlMock, curl_easy_escape(_, ANY(char*), ANY(int))).RETURN(&std::string(_2)[0]);
        ALLOW_CALL(curlMock, curl_free(_));
        ALLOW_CALL(curlMock, curl_global_cleanup());

        auto conn = internal::ConnectionInfo::createConnectionInfoV2("http://localhost", 8086, "test", "aheifAwakjfAPWwe", "rp");
        HTTP http{conn};

        const std::string query{"/x?shouldfail=true"};
        std::string returnValue = query;
        char* ptr = &returnValue[0];
        ALLOW_CALL(curlMock, curl_easy_escape(_, query.c_str(), static_cast<int>(query.size()))).RETURN(ptr);
        ALLOW_CALL(curlMock, curl_free(_));
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_WRITEDATA, ANY(void*)))
            .LR_SIDE_EFFECT(*static_cast<std::string*>(_3) = "query-result")
            .RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_perform(handle)).RETURN(CURLE_OK);

        REQUIRE_CALL(curlMock, curl_easy_getinfo_(handle, CURLINFO_RESPONSE_CODE, _))
            .LR_SIDE_EFFECT(*static_cast<long*>(_3) = 200)
            .RETURN(CURLE_OK);
        http.query(query);

        REQUIRE_CALL(curlMock, curl_easy_getinfo_(handle, CURLINFO_RESPONSE_CODE, _))
            .LR_SIDE_EFFECT(*static_cast<long*>(_3) = 204)
            .RETURN(CURLE_OK);

        const auto result = http.query(query);
        CHECK(result == "query-result");
    }

    TEST_CASE("V2: Query throws on unsuccessful response", "[HttpTest]")
    {
        ALLOW_CALL(curlMock, curl_global_init(_)).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_init()).RETURN(handle);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_URL, ANY(std::string))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(long))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(WriteCallbackFn))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_cleanup(_));
        ALLOW_CALL(curlMock, curl_easy_escape(_, ANY(char*), ANY(int))).RETURN(&std::string(_2)[0]);
        ALLOW_CALL(curlMock, curl_free(_));
        ALLOW_CALL(curlMock, curl_global_cleanup());

        auto conn = internal::ConnectionInfo::createConnectionInfoV2("http://localhost", 8086, "test", "aheifAwakjfAPWwe", "rp");
        HTTP http{conn};

        const std::string query{"/x?shouldfail=true"};
        std::string returnValue = query;
        char* ptr = &returnValue[0];
        ALLOW_CALL(curlMock, curl_easy_escape(_, query.c_str(), static_cast<int>(query.size()))).RETURN(ptr);
        ALLOW_CALL(curlMock, curl_free(_));
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_WRITEDATA, ANY(void*)))
            .LR_SIDE_EFFECT(*static_cast<std::string*>(_3) = "{}")
            .RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_perform(handle)).RETURN(CURLE_OK);

        REQUIRE_CALL(curlMock, curl_easy_getinfo_(handle, CURLINFO_RESPONSE_CODE, _))
            .LR_SIDE_EFFECT(*static_cast<long*>(_3) = 404)
            .RETURN(CURLE_OK);
        REQUIRE_THROWS_AS(http.query(query), NonExistentDatabase);

        REQUIRE_CALL(curlMock, curl_easy_getinfo_(handle, CURLINFO_RESPONSE_CODE, _))
            .LR_SIDE_EFFECT(*static_cast<long*>(_3) = 400)
            .RETURN(CURLE_OK);
        REQUIRE_THROWS_AS(http.query(query), BadRequest);

        REQUIRE_CALL(curlMock, curl_easy_getinfo_(handle, CURLINFO_RESPONSE_CODE, _))
            .LR_SIDE_EFFECT(*static_cast<long*>(_3) = 500)
            .RETURN(CURLE_OK);
        REQUIRE_THROWS_AS(http.query(query), ServerError);

        REQUIRE_CALL(curlMock, curl_easy_getinfo_(handle, CURLINFO_RESPONSE_CODE, _))
            .LR_SIDE_EFFECT(*static_cast<long*>(_3) = 503)
            .RETURN(CURLE_OK);
        REQUIRE_THROWS_AS(http.query(query), ServerError);
    }

    TEST_CASE("V2: Create database configures curl", "[HttpTest]")
    {
        ALLOW_CALL(curlMock, curl_global_init(_)).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_init()).RETURN(handle);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_URL, ANY(std::string))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(long))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(WriteCallbackFn))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_cleanup(_));
        ALLOW_CALL(curlMock, curl_easy_escape(_, ANY(char*), ANY(int))).RETURN(&std::string(_2)[0]);
        ALLOW_CALL(curlMock, curl_free(_));
        ALLOW_CALL(curlMock, curl_global_cleanup());

        auto conn = internal::ConnectionInfo::createConnectionInfoV2("http://localhost", 8086, "example-to-create", "aheifAwakjfAPWwe", "rp");
        HTTP http{conn};
        CHECK_THROWS_AS(http.createDatabase(), InfluxDBException);
    }

    TEST_CASE("V2: Enabling token auth sets curl options", "[HttpTest]")
    {
        ALLOW_CALL(curlMock, curl_global_init(_)).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_init()).RETURN(handle);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(std::string))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(long))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(WriteCallbackFn))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_cleanup(_));
        ALLOW_CALL(curlMock, curl_easy_escape(_, ANY(char*), ANY(int))).RETURN(&std::string(_2)[0]);
        ALLOW_CALL(curlMock, curl_free(_));
        ALLOW_CALL(curlMock, curl_global_cleanup());

        auto conn = internal::ConnectionInfo::createConnectionInfoV2("http://localhost", 8086, "example-database-0", "aheifAwakjfAPWwe", "rp");
        HTTP http{conn};

        std::string auth = "Authorization: Token " + conn.token;

        curl_slist* slist_obj = new curl_slist();
        slist_obj->data = new char[auth.length() + 1];
        slist_obj->data = strcpy(slist_obj->data, auth.c_str());

        REQUIRE_CALL(curlMock, curl_slist_append(_, trompeloeil::ne(nullptr))).WITH(std::string(_2) == "Authorization: Token aheifAwakjfAPWwe").RETURN(slist_obj);
        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_HTTPHEADER, slist_obj)).RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_HTTPHEADER, slist_obj)).RETURN(CURLE_OK);
        http.enableTokenAuth(conn.token);
    }

    TEST_CASE("V2: Database name is returned if valid", "[HttpTest]")
    {
        ALLOW_CALL(curlMock, curl_global_init(_)).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_init()).RETURN(handle);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(std::string))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(long))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(WriteCallbackFn))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_cleanup(_));
        ALLOW_CALL(curlMock, curl_easy_escape(_, ANY(char*), ANY(int))).RETURN(&std::string(_2)[0]);
        ALLOW_CALL(curlMock, curl_free(_));
        ALLOW_CALL(curlMock, curl_global_cleanup());

        auto conn = internal::ConnectionInfo::createConnectionInfoV2("http://localhost", 8086, "example-database-0", "aheifAwakjfAPWwe", "rp");
        const HTTP http{conn};
        CHECK(http.databaseName() == "example-database-0");
    }

    TEST_CASE("V2: Database service url is returned if valid", "[HttpTest]")
    {
        ALLOW_CALL(curlMock, curl_global_init(_)).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_init()).RETURN(handle);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(std::string))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(long))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(WriteCallbackFn))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_cleanup(_));
        ALLOW_CALL(curlMock, curl_easy_escape(_, ANY(char*), ANY(int))).RETURN(&std::string(_2)[0]);
        ALLOW_CALL(curlMock, curl_free(_));
        ALLOW_CALL(curlMock, curl_global_cleanup());

        auto conn = internal::ConnectionInfo::createConnectionInfoV2("http://localhost", 8086, "example-database-1", "aheifAwakjfAPWwe", "rp");
        const HTTP http{conn};
        CHECK(http.influxDbServiceUrl() == "http://localhost:8086");
    }

    TEST_CASE("V2: Query with valid param", "[HttpTest]")
    {
        ALLOW_CALL(curlMock, curl_global_init(_)).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_init()).RETURN(handle);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(std::string))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(long))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(WriteCallbackFn))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_cleanup(_));
        ALLOW_CALL(curlMock, curl_easy_escape(_, ANY(char*), ANY(int))).RETURN(&std::string(_2)[0]);
        ALLOW_CALL(curlMock, curl_free(_));
        ALLOW_CALL(curlMock, curl_global_cleanup());

        auto conn = internal::ConnectionInfo::createConnectionInfoV2("http://localhost", 8086, "test", "aheifAwakjfAPWwe", "rp");
        HTTP http{conn};

        const std::string query{"SELECT * FROM tbl WHERE c1=$param1"};
        std::string returnValue = query;
        char* ptr = &returnValue[0];
        auto params = influxdb::InfluxDBParams{}.addParam("param1", 1);
        REQUIRE_CALL(curlMock, curl_easy_escape(_, query.c_str(), static_cast<int>(query.size()))).RETURN(ptr);
        ALLOW_CALL(curlMock, curl_free(ptr));
        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_URL, "http://localhost:8086/query?db=test&rp=rp&q=SELECT * FROM tbl WHERE c1=$param1&params={\"param1\":1}")).RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_WRITEDATA, ANY(void*)))
            .LR_SIDE_EFFECT(*static_cast<std::string*>(_3) = "query-result")
            .RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_perform(handle)).RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_getinfo_(handle, CURLINFO_RESPONSE_CODE, _))
            .LR_SIDE_EFFECT(*static_cast<long*>(_3) = 200)
            .RETURN(CURLE_OK);

        const auto result = http.query(query, params);
        CHECK(result == "query-result");
    }

    TEST_CASE("V2: Query with empty param", "[HttpTest]")
    {
        ALLOW_CALL(curlMock, curl_global_init(_)).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_init()).RETURN(handle);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(std::string))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(long))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(WriteCallbackFn))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_cleanup(_));
        ALLOW_CALL(curlMock, curl_easy_escape(_, ANY(char*), ANY(int))).RETURN(&std::string(_2)[0]);
        ALLOW_CALL(curlMock, curl_free(_));
        ALLOW_CALL(curlMock, curl_global_cleanup());

        auto conn = internal::ConnectionInfo::createConnectionInfoV2("http://localhost", 8086, "test", "aheifAwakjfAPWwe", "rp");
        HTTP http{conn};

        const std::string query{"SELECT * FROM tbl WHERE c1=$param1"};
        std::string returnValue = query;
        char* ptr = &returnValue[0];
        auto params = influxdb::InfluxDBParams{};
        REQUIRE_CALL(curlMock, curl_easy_escape(_, query.c_str(), static_cast<int>(query.size()))).RETURN(ptr);
        ALLOW_CALL(curlMock, curl_free(ptr));
        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_URL, "http://localhost:8086/query?db=test&rp=rp&q=SELECT * FROM tbl WHERE c1=$param1")).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_WRITEDATA, ANY(void*)))
            .LR_SIDE_EFFECT(*static_cast<std::string*>(_3) = "{}")
            .RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_perform(handle)).RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_getinfo_(handle, CURLINFO_RESPONSE_CODE, _))
            .LR_SIDE_EFFECT(*static_cast<long*>(_3) = 400)
            .RETURN(CURLE_OK);

        REQUIRE_THROWS_AS(http.query(query), BadRequest);
    }

    TEST_CASE("V2: Query is empty", "[HttpTest]")
    {
        ALLOW_CALL(curlMock, curl_global_init(_)).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_init()).RETURN(handle);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(std::string))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(long))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(WriteCallbackFn))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_cleanup(_));
        ALLOW_CALL(curlMock, curl_easy_escape(_, ANY(char*), ANY(int))).RETURN(&std::string(_2)[0]);
        ALLOW_CALL(curlMock, curl_free(_));
        ALLOW_CALL(curlMock, curl_global_cleanup());

        auto conn = internal::ConnectionInfo::createConnectionInfoV2("http://localhost", 8086, "test", "aheifAwakjfAPWwe", "rp");
        HTTP http{conn};

        const std::string query{};
        std::string returnValue = query;
        char* ptr = &returnValue[0];
        REQUIRE_CALL(curlMock, curl_easy_escape(_, query.c_str(), static_cast<int>(query.size()))).RETURN(ptr);
        ALLOW_CALL(curlMock, curl_free(ptr));
        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_URL, "http://localhost:8086/query?db=test&rp=rp&q=")).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_WRITEDATA, ANY(void*)))
            .LR_SIDE_EFFECT(*static_cast<std::string*>(_3) = "{}")
            .RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_perform(handle)).RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_getinfo_(handle, CURLINFO_RESPONSE_CODE, _))
            .LR_SIDE_EFFECT(*static_cast<long*>(_3) = 400)
            .RETURN(CURLE_OK);

        REQUIRE_THROWS_AS(http.query(query), BadRequest);
    }

    TEST_CASE("V2: Query on database have special characters", "[HttpTest]")
    {
        ALLOW_CALL(curlMock, curl_global_init(_)).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_init()).RETURN(handle);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(std::string))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(long))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(WriteCallbackFn))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_cleanup(_));
        ALLOW_CALL(curlMock, curl_easy_escape(_, ANY(char*), ANY(int))).RETURN(&std::string(_2)[0]);
        ALLOW_CALL(curlMock, curl_free(_));
        ALLOW_CALL(curlMock, curl_global_cleanup());

        auto conn = internal::ConnectionInfo::createConnectionInfoV2("http://localhost", 8086, "test\\", "aheifAwakjfAPWwe");
        HTTP http{conn};

        const std::string query{"SELECT * FROM test"};
        std::string returnValue = query;
        char* ptr = &returnValue[0];
        ALLOW_CALL(curlMock, curl_easy_escape(_, query.c_str(), static_cast<int>(query.size()))).RETURN(ptr);
        ALLOW_CALL(curlMock, curl_free(_));
        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_URL, R"(http://localhost:8086/query?db=test\&q=SELECT * FROM test)")).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_WRITEDATA, ANY(void*)))
            .LR_SIDE_EFFECT(*static_cast<std::string*>(_3) = "query-result")
            .RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_perform(handle)).RETURN(CURLE_OK);

        REQUIRE_CALL(curlMock, curl_easy_getinfo_(handle, CURLINFO_RESPONSE_CODE, _))
            .LR_SIDE_EFFECT(*static_cast<long*>(_3) = 200)
            .RETURN(CURLE_OK);

        const auto result = http.query(query);
        CHECK(result == "query-result");
    }
}
