// MIT License
//
// Copyright (c) 2022 TOSHIBA CORPORATION
// Copyright (c) 2020-2022 offa
// Copyright (c) 2019 Adam Wegrzynek
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

///
/// \author Adam Wegrzynek <adam.wegrzynek@cern.ch>
///

#include "InfluxDBFactory.h"
#include <functional>
#include <string>
#include <memory>
#include <map>
#include "UriParser.h"
#include "HTTP.h"
#include "InfluxDBException.h"
#include "BoostSupport.h"
#include "ConnectionInfo.h"

namespace influxdb
{
    namespace internal
    {
        std::unique_ptr<Transport> withHttpTransport(ConnectionInfo conn)
        {
            /// Build HTTP transport
            auto transport = std::make_unique<transports::HTTP>(conn);
            if (conn.dbVersion == 1 && !conn.username.empty())
            {
                transport->enableBasicAuth(conn.username + ":" + conn.password);
            }
            else if (conn.dbVersion == 2 && !conn.token.empty())
            {
                /// enable token authorization for InfluxDB 2.x
                transport->enableTokenAuth(conn.token);
            }
            return transport;
        }

        std::unique_ptr<Transport> GetTransport(const ConnectionInfo &conn)
        {
            auto urlCopy = conn.host;
            http::url parsedUrl = http::ParseHttpUrl(urlCopy);

            if (parsedUrl.protocol.empty())
            {
                throw InfluxDBException(__func__, "Ill-formed URI");
            }

            if (parsedUrl.protocol == "http" || parsedUrl.protocol == "https")
            {
                /// Support both InfluxDB v1 and InfluxDB v2
                return withHttpTransport(conn);
            }
            else if (conn.dbVersion == 1 && parsedUrl.protocol == "udp")
            {
                /// Support InfluxDB v1 only
                return withUdpTransport(parsedUrl.path, conn.port);
            }
            else if (conn.dbVersion == 1 && parsedUrl.protocol == "unix")
            {
                /// Support InfluxDB v1 only
                return withUnixSocketTransport(parsedUrl.path);
            }
            else
            {
                throw InfluxDBException(__func__, "Unrecognized backend " + parsedUrl.protocol);
            }
        }
    }

    std::unique_ptr<InfluxDB> InfluxDBFactory::GetV1(const std::string &host, const int port,
                                                const std::string &dbName,
                                                const std::string &username,
                                                const std::string &password)
    {
        auto conn = internal::ConnectionInfo::createConnectionInfoV1(host, port, dbName, username, password);
        return std::make_unique<InfluxDB>(GetTransport(conn));
    }

    std::unique_ptr<InfluxDB> InfluxDBFactory::GetV2(const std::string &host, const int port,
                                                const std::string &dbName,
                                                const std::string &token,
                                                const std::string &rp)
    {
        auto conn = internal::ConnectionInfo::createConnectionInfoV2(host, port, dbName, token, rp);
        return std::make_unique<InfluxDB>(GetTransport(conn));
    }

} // namespace influxdb
