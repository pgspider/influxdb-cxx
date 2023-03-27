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

#include "ConnectionInfo.h"

namespace influxdb::internal
{
    ConnectionInfo ConnectionInfo::createConnectionInfoV1(const std::string &host, const int port,
                                                          const std::string &dbName,
                                                          const std::string &username,
                                                          const std::string &password)
    {
        ConnectionInfo conn;
        conn.dbVersion = 1;
        conn.host = host;
        conn.port = port;
        conn.dbName = dbName;
        conn.username = username;
        conn.password = password;
        return conn;
    }

    ConnectionInfo ConnectionInfo::createConnectionInfoV2(const std::string &host, const int port,
                                                          const std::string &dbName,
                                                          const std::string &token,
                                                          const std::string &rp)
    {
        ConnectionInfo conn;
        conn.dbVersion = 2;
        conn.host = host;
        conn.port = port;
        conn.dbName = dbName;
        conn.token = token;
        conn.rp = rp;
        return conn;
    }
}
