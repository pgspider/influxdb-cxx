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
#include "HTTP.h"
#include <cstdlib>
#include <catch2/catch.hpp>

namespace influxdb::test
{
    namespace
    {
        std::size_t rowCount(influxdb::InfluxDBTable tbl)
        {
            size_t size = 0;
            for (auto series: tbl.series)
                size += series.rows.size();
            return size;
        }

        std::size_t querySize(influxdb::InfluxDB& db, const std::string& tag)
        {
            auto result = db.query("select * from x where type='" + tag + "'");
            return rowCount(result[0]);
        }
    }

    namespace influxdb_v1
    {
        std::string dbServerUrl()
        {
            if (const auto host = std::getenv("INFLUXDB_V1_SYSTEMTEST_HOST"); host != nullptr)
            {
                return host;
            }
            return "localhost";
        }

        int dbServerPort()
        {
            if (const auto port = std::getenv("INFLUXDB_V1_SYSTEMTEST_PORT"); port != nullptr)
            {
                return std::stoi(port);
            }
            return 8086;
        }

        std::string dbServerUrlAuth()
        {
            if (const auto host = std::getenv("INFLUXDB_V1_SYSTEMTEST_HOST_AUTH"); host != nullptr)
            {
                return host;
            }
            return "localhost";
        }

        int dbServerPortAuth()
        {
            if (const auto port = std::getenv("INFLUXDB_V1_SYSTEMTEST_PORT_AUTH"); port != nullptr)
            {
                return std::stoi(port);
            }
            return 8086;
        }

        std::string dbServerDatabase()
        {
            if (const auto dbName = std::getenv("INFLUXDB_V1_SYSTEMTEST_DATABASE"); dbName != nullptr)
            {
                return dbName;
            }
            return "st_db";
        }

        std::string dbServerUsername()
        {
            if (const auto user = std::getenv("INFLUXDB_V1_SYSTEMTEST_USER"); user != nullptr)
            {
                return user;
            }
            return "user";
        }

        std::string dbServerPassword()
        {
            if (const auto pass = std::getenv("INFLUXDB_V1_SYSTEMTEST_PASS"); pass != nullptr)
            {
                return pass;
            }
            return "pass";
        }
    }

    namespace influxdb_v2
    {
        std::string dbServerUrl()
        {
            if (const auto host = std::getenv("INFLUXDB_V2_SYSTEMTEST_HOST"); host != nullptr)
            {
                return host;
            }
            return "localhost";
        }

        int dbServerPort()
        {
            if (const auto port = std::getenv("INFLUXDB_V2_SYSTEMTEST_PORT"); port != nullptr)
            {
                return std::stoi(port);
            }
            return 8086;
        }

        std::string dbServerToken()
        {
            if (const auto token = std::getenv("INFLUXDB_V2_SYSTEMTEST_TOKEN"); token != nullptr)
            {
                return token;
            }
            return "mytoken";
        }

        std::string dbServerDatabase()
        {
            if (const auto dbName = std::getenv("INFLUXDB_V2_SYSTEMTEST_DATABASE"); dbName != nullptr)
            {
                return dbName;
            }
            return "st_db";
        }

        std::string dbServerRP()
        {
            if (const auto pass = std::getenv("INFLUXDB_V2_SYSTEMTEST_RP"); pass != nullptr)
            {
                return pass;
            }
            return std::string();
        }
    }

    void executeSystemTest(const internal::ConnectionInfo& conn)
    {
        using namespace influxdb;
        using namespace influxdb::transports;
        using namespace Catch::Matchers;

        std::unique_ptr<influxdb::InfluxDB> db;
        HTTP http{conn};
        if (conn.dbVersion == 1)
        {
            db = InfluxDBFactory::GetV1(conn.host, conn.port, conn.dbName, conn.username, conn.password);
            // enable Basic athentication if needed
            if (conn.username.length() > 0 && conn.password.length() > 0)
            {
                http.enableBasicAuth(conn.username + ":" + conn.password);
            }
        }
        else
        {
            db = InfluxDBFactory::GetV2(conn.host, conn.port, conn.dbName, conn.token, conn.rp);
            http.enableTokenAuth(conn.token);
        }


        SECTION("Database does not exist")
        {
            const auto response = http.query("show databases");
            if (conn.dbVersion == 1)
            {
                CHECK_THAT(response, !Contains(R"(["st_db"])"));
            }
            else
            {
                CHECK_THAT(response, !Contains(R"(["non_existen_database"])"));
            }
        }

        SECTION("Query on non existing database returns empty")
        {
            InfluxDBTable res;
            if (conn.dbVersion == 1)
            {
                res = db->query("select * from st_db")[0];
            }
            else
            {
                /// connect to non_existen_database
                auto invalidDb = InfluxDBFactory::GetV2(conn.host, conn.port, "non_existen_database", conn.token, conn.rp);
                res = invalidDb->query("select * from non_existen_database")[0];
            }

            CHECK(res.series.empty());
        }

        SECTION("Query on non existing database returns error on ressult set")
        {
            InfluxDBTable res;
            if (conn.dbVersion == 1)
            {
                res = db->query("select * from st_db")[0];
            }
            else
            {
                /// connect to non_existen_database
                auto invalidDb = InfluxDBFactory::GetV2(conn.host, conn.port, "non_existen_database", conn.token, conn.rp);
                res = invalidDb->query("select * from non_existen_database")[0];
            }

            CHECK(!res.error.empty());
        }

        SECTION("Create database if not existing")
        {
            if (conn.dbVersion == 1)
            {
                db->createDatabaseIfNotExists();
                CHECK_THAT(http.query("show databases"), Contains(R"(["st_db"])"));
            }
            else
            {
                /// unexpected
                CHECK_THROWS_AS(db->createDatabaseIfNotExists(), InfluxDBException);
            }
        }

        SECTION("Create database if not existing does nothing on existing database")
        {
            if (conn.dbVersion == 1)
            {
                db->createDatabaseIfNotExists();
            }
            else
            {
                // unexpected
                CHECK_THROWS_AS(db->createDatabaseIfNotExists(), InfluxDBException);
            }
        }

        SECTION("Created database is empty")
        {
            CHECK((db->query("select * from st_db"))[0].series.empty());
            CHECK((db->query("select * from st_db"))[0].error.empty());
        }

        SECTION("Write point")
        {
            CHECK(querySize(*db, "sp") == 0);
            db->write(Point{"x"}.addField("value", 20).addTag("type", "sp"));
            CHECK(querySize(*db, "sp") == 1);
        }

        SECTION("Query point")
        {
            const auto response = db->query("select * from x");
            CHECK(rowCount(response[0]) == 1);
            CHECK(response[0].series[0].name == "x");
            CHECK(response[0].series[0].rows[0].tuple[1] == "sp");
            CHECK(response[0].series[0].rows[0].tuple[2] == "20");
        }

        SECTION("Query point with no matches")
        {
            const auto response = db->query("select * from nothing_to_find");
            CHECK(response[0].series.empty());
        }

        SECTION("Query point grouped by tag")
        {
            const auto response = db->query("select * from x group by type");
            CHECK(rowCount(response[0]) == 1);
            CHECK(response[0].series[0].tagKeys[0] == "type");
            CHECK(response[0].series[0].tagValues[0] == "sp");
        }

        SECTION("Query point throws on invalid query")
        {
            CHECK_THROWS_AS(db->query("select* from_INVALID"), BadRequest);
        }

        SECTION("Write multiple points")
        {
            CHECK(querySize(*db, "mp") == 0);
            db->write(Point{"x"}.addField("n", 0).addTag("type", "mp"));
            CHECK(querySize(*db, "mp") == 1);
            db->write(Point{"x"}.addField("n", 1).addTag("type", "mp"));
            CHECK(querySize(*db, "mp") == 2);
            db->write(Point{"x"}.addField("n", 2).addTag("type", "mp"));
            CHECK(querySize(*db, "mp") == 3);
            db->write(Point{"x"}.addField("n", 2).addTag("type", "mp"));
            CHECK(querySize(*db, "mp") == 4);
        }

        SECTION("Write multiple points by container")
        {
            CHECK(querySize(*db, "mpc") == 0);

            db->write({Point{"x"}.addField("n", 0).addTag("type", "mpc"),
                       Point{"x"}.addField("n", 1).addTag("type", "mpc"),
                       Point{"x"}.addField("n", 2).addTag("type", "mpc")});

            CHECK(querySize(*db, "mpc") == 3);
        }

        SECTION("Query points")
        {
            const auto response = db->query(R"(select * from x where type='mpc')");
            CHECK(rowCount(response[0]) == 3);
            CHECK(response[0].series[0].name == "x");
            CHECK(response[0].series[0].rows[0].tuple[1] == "0");
            CHECK(response[0].series[0].rows[1].tuple[1] == "1");
            CHECK(response[0].series[0].rows[2].tuple[1] == "2");
        }

        SECTION("Write as batch doesn't send if batch size not reached")
        {
            db->batchOf(3);

            CHECK(querySize(*db, "bpns") == 0);
            db->write({Point{"x"}.addField("n", 0).addTag("type", "bpns"),
                       Point{"x"}.addField("n", 1).addTag("type", "bpns")});
            CHECK(querySize(*db, "bpns") == 0);
        }

        SECTION("Write as batch sends if batch size reached")
        {
            db->batchOf(2);

            CHECK(querySize(*db, "bp") == 0);
            db->write({Point{"x"}.addField("n", 1).addTag("type", "bp"),
                       Point{"x"}.addField("n", 2).addTag("type", "bp"),
                       Point{"x"}.addField("n", -1).addTag("type", "bp")});
            CHECK(querySize(*db, "bp") == 2);
        }

        SECTION("Write as batch sends if on flush")
        {
            db->batchOf(200);

            CHECK(querySize(*db, "bpf") == 0);
            db->write({Point{"x"}.addField("n", 1).addTag("type", "bpf"),
                       Point{"x"}.addField("n", 2).addTag("type", "bpf"),
                       Point{"x"}.addField("n", -1).addTag("type", "bpf")});
            CHECK(querySize(*db, "bpf") == 0);
            db->flushBatch();
            CHECK(querySize(*db, "bpf") == 3);
        }

        SECTION("Write of invalid line protocol throws")
        {
            CHECK_THROWS_AS(db->write(Point{"test,this=is ,,====,, invalid"}), BadRequest);
        }

        SECTION("Write to unreachable host throws")
        {
            std::unique_ptr<influxdb::InfluxDB> invalidDb;
            if (conn.dbVersion == 1)
            {
                invalidDb = influxdb::InfluxDBFactory::GetV1("http://non_existing_host", 123456, "not_existing_db");
            }
            else
            {
                invalidDb = influxdb::InfluxDBFactory::GetV2("http://non_existing_host", 123456, "not_existing_db", "a_token");
            }

            CHECK_THROWS_AS(invalidDb->write(Point{"test"}.addField("value", 10)), ConnectionError);
        }

        SECTION("Write to nonexistent database throws")
        {
            std::unique_ptr<influxdb::InfluxDB> invalidDb;
            if (conn.dbVersion == 1)
            {
                invalidDb = influxdb::InfluxDBFactory::GetV1(conn.host, conn.port, "_nonexistent", conn.username, conn.password);
            }
            else
            {
                invalidDb = influxdb::InfluxDBFactory::GetV2(conn.host, conn.port, "_nonexistent", conn.token);
            }
            CHECK_THROWS_AS(invalidDb->write(Point{"test"}.addField("value", 10)), NonExistentDatabase);
        }

        SECTION("Write special value for point")
        {
            std::string dbname = R"(z=a"b",c)";
            std::string dbname_escaped = R"("z=a\"b\",c")";
            db->batchOf(1);
            db->write(Point{dbname}.addField("a", 0).addTag("type", "escape"));
            db->write(Point{dbname}.addField("b", R"(b=\",)").addTag(R"(t=\",)", R"(v=\",)").addTag("type", "escape"));
            db->write(Point{dbname}.addField(R"(b=\",)", R"(b=\",)").addTag("type", "escape"));

            const auto response = db->query("select * from " + dbname_escaped);
            CHECK(rowCount(response[0]) == 3);
            CHECK(response[0].series[0].name == dbname);

            CHECK(response[0].series[0].columnNames[1] == "a");
            CHECK(response[0].series[0].rows[0].tuple[1] == "0");
            CHECK(response[0].series[0].rows[1].tuple[1].empty());
            CHECK(response[0].series[0].rows[2].tuple[1].empty());

            CHECK(response[0].series[0].columnNames[2] == "b");
            CHECK(response[0].series[0].rows[0].tuple[2].empty());
            CHECK(response[0].series[0].rows[1].tuple[2] == R"(b=\",)");
            CHECK(response[0].series[0].rows[2].tuple[2].empty());

            CHECK(response[0].series[0].columnNames[3] == R"(b=",)");
            CHECK(response[0].series[0].rows[0].tuple[3].empty());
            CHECK(response[0].series[0].rows[1].tuple[3].empty());
            CHECK(response[0].series[0].rows[2].tuple[3] == R"(b=\",)");

            CHECK(response[0].series[0].columnNames[4] == R"(t=\",)");
            CHECK(response[0].series[0].rows[0].tuple[4].empty());
            CHECK(response[0].series[0].rows[1].tuple[4] == R"(v=\",)");
            CHECK(response[0].series[0].rows[2].tuple[4].empty());

            CHECK(response[0].series[0].columnNames[5] == "type");
            CHECK(response[0].series[0].rows[0].tuple[5] == "escape");
            CHECK(response[0].series[0].rows[1].tuple[5] == "escape");
            CHECK(response[0].series[0].rows[2].tuple[5] == "escape");
        }

        SECTION("Authentication failed")
        {
            std::unique_ptr<influxdb::InfluxDB> invalidDb;
            if (!conn.username.empty() || !conn.token.empty())
            {
                if (conn.dbVersion == 1)
                {
                    invalidDb = influxdb::InfluxDBFactory::GetV1(conn.host, conn.port, conn.dbName, "wrong_user", conn.password);
                    CHECK_THROWS_AS(invalidDb->write(Point{"test"}.addField("value", 10)), BadRequest);
                    invalidDb = influxdb::InfluxDBFactory::GetV1(conn.host, conn.port, conn.dbName, conn.username, "wrong_password");
                    CHECK_THROWS_AS(invalidDb->write(Point{"test"}.addField("value", 10)), BadRequest);
                }
                else
                {
                    invalidDb = influxdb::InfluxDBFactory::GetV2(conn.host, conn.port, conn.dbName, "wrong_token");
                    CHECK_THROWS_AS(invalidDb->write(Point{"test"}.addField("value", 10)), BadRequest);
                }
            }
        }

        SECTION("Port not correct")
        {
            std::unique_ptr<influxdb::InfluxDB> invalidDb;
            if (conn.dbVersion == 1)
            {
                invalidDb = influxdb::InfluxDBFactory::GetV1(conn.host, 9999, conn.dbName, conn.username, conn.password);
            }
            else
            {
                invalidDb = influxdb::InfluxDBFactory::GetV2(conn.host, 9999, conn.dbName, conn.token);
            }

            CHECK_THROWS_AS(invalidDb->write(Point{"test"}.addField("value", 10)), InfluxDBException);
        }

        SECTION("Query with valid params")
        {
            db->write({Point{"params_test"}.addField("c1", 0).addField("c2", 100).addField("c3", "special case"),
                       Point{"params_test"}.addField("c1", 1).addField("c2", 200).addField("c3", R"(b=\",)"),
                       Point{"params_test"}.addField("c1", 2).addField("c2", 300).addField("c3", R"(c=\",)")});

            std::string query = "SELECT c2 - $param1 FROM params_test WHERE c1 > $param2";
            auto params1 = InfluxDBParams{}.addParam("param1", 50).addParam("param2", 1);
            auto params2 = InfluxDBParams{}.addParam("param1", 100).addParam("param2", 0);

            auto response = db->query(query, params1);
            CHECK(rowCount(response[0]) == 1);
            CHECK(response[0].series[0].rows[0].tuple[1] == "250");

            response = db->query(query, params2);
            CHECK(rowCount(response[0]) == 2);
            CHECK(response[0].series[0].rows[0].tuple[1] == "100");
            CHECK(response[0].series[0].rows[1].tuple[1] == "200");

            query = R"(SELECT c2 - $"param\"1\"" FROM params_test WHERE c3 = $param1)";
            auto params3 = InfluxDBParams{}.addParam("param1", R"(b=\",)").addParam(R"(param"1")", 100);

            CHECK(params3.size() == 2);

            response = db->query(query, params3);
            CHECK(rowCount(response[0]) == 1);
            CHECK(response[0].series[0].rows[0].tuple[1] == "100");
        }

        SECTION("Query with empty params")
        {
            std::string query = "SELECT c2 - $param1 FROM params_test WHERE c1 > $param2";
            auto params = InfluxDBParams{};
            CHECK_THROWS_AS(db->query(query, params), BadRequest);
            CHECK_THROWS_AS(db->query(query), BadRequest);
        }

        SECTION("Query is empty")
        {
            CHECK_THROWS_AS(db->query(""), BadRequest);
        }

        SECTION("Cleanup")
        {
            http.query("drop database st_db");
        }
    }

    TEST_CASE("V1: InfluxDB system test", "[InfluxDBST]")
    {
        using namespace influxdb_v1;

        const std::string url{"http://" + dbServerUrl()};
        auto conn = internal::ConnectionInfo::createConnectionInfoV1(url, dbServerPort(), dbServerDatabase());
        executeSystemTest(conn);
    }

    TEST_CASE("V1: InfluxDB system test with authentication", "[InfluxDBST]")
    {
        using namespace influxdb_v1;

        const std::string url{"http://" + dbServerUrlAuth()};
        auto conn = internal::ConnectionInfo::createConnectionInfoV1(url, dbServerPortAuth(), dbServerDatabase(), dbServerUsername(), dbServerPassword());
        executeSystemTest(conn);
    }

    TEST_CASE("V2: InfluxDB system test", "[InfluxDBST]")
    {
        using namespace influxdb_v2;

        const std::string url{"http://" + dbServerUrl()};
        auto conn = internal::ConnectionInfo::createConnectionInfoV2(url, dbServerPort(), dbServerDatabase(), dbServerToken(), dbServerRP());
        executeSystemTest(conn);
    }
}
