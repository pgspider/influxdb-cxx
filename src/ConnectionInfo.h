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

#pragma once
#include <string>

namespace influxdb::internal
{
    /// \brief InfluxDB Connection information
    class ConnectionInfo
    {
    private:
        /// Private constructor disallows to create instance of
        /// ConnectionInfo with all empty attributes.
        ConnectionInfo() = default;

    public:
        /// ConnectionInfo
        /// Provides ConnectionInfo instance for InfluxDB 1.x connection
        /// \param host         InfluxDB 1.x server address
        /// \param port         InfluxDB 1.x server port
        /// \param dbName       target database name
        /// \param username     username for V1 basic authentication (optional)
        /// \param password     password for V1 basic authentication (optional)
        static ConnectionInfo createConnectionInfoV1(const std::string &host, const int port,
                                                     const std::string &dbName,
                                                     const std::string &username = std::string(),
                                                     const std::string &password = std::string());

        /// ConnectionInfo
        /// Provides ConnectionInfo instance for InfluxDB 2.x connection
        /// \param host         InfluxDB 2.x server address
        /// \param port         InfluxDB 2.x server port
        /// \param dbName       target database name mapped with a bucket
        /// \param token        token for V2 Token authentication
        /// \param rp           retention policy of target database (optional)
        static ConnectionInfo createConnectionInfoV2(const std::string &host, const int port,
                                                     const std::string &dbName,
                                                     const std::string &token,
                                                     const std::string &rp = std::string());

        /// Version of InfluxDB 1 or 2
        uint8_t     dbVersion;
        /// Server info
        std::string host;
        int         port;
        /// Write & query targets database
        std::string dbName;
        /// V2 Retention Policy for mapped database
        std::string rp;
        /// V2 authetication token
        std::string token;
        /// V1 Basic authetication
        std::string username;
        std::string password;
    }; /* !ConnectionInfo */
}