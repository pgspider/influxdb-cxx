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
/// \author Adam Wegrzynek
///

#ifndef INFLUXDATA_INFLUXDB_FACTORY_H
#define INFLUXDATA_INFLUXDB_FACTORY_H

#include "InfluxDB.h"
#include "Transport.h"
#include "influxdb_export.h"

namespace influxdb
{

/// \brief InfluxDB factory
class INFLUXDB_EXPORT InfluxDBFactory
{
 public:
   /// Disables copy constructor
   InfluxDBFactory & operator=(const InfluxDBFactory&) = delete;

   /// Disables copy constructor
   InfluxDBFactory(const InfluxDBFactory&) = delete;

    /// InfluxDB factory
    /// Provides InfluxDB instance which can connect to InfluxDB 1.x
    /// \param host     InfluxDB 1.x server address
    /// \param port     InfluxDB 1.x server port
    /// \param dbName   target database name
    /// \param username username for V1 basic authentication (optional)
    /// \param password password for V1 basic authentication (optional)
    /// \throw InfluxDBException    if host is unrecognised backend or missing protocol
    static std::unique_ptr<InfluxDB> GetV1(const std::string &host, const int port = 0,
                                          const std::string &dbName = std::string(),
                                          const std::string &username = std::string(),
                                          const std::string &password = std::string()) noexcept(false);

    /// InfluxDB factory
    /// Provides InfluxDB instance which can connect to InfluxDB 2.x
    /// \param host     InfluxDB 2.x server address
    /// \param port     InfluxDB 2.x server port
    /// \param dbName   target database name mapped with a bucket
    /// \param token    token for V2 Token authentication
    /// \param rp       retention policy of target database (optional)
    /// \throw InfluxDBException    if host is unrecognised backend or missing protocol
    static std::unique_ptr<InfluxDB> GetV2(const std::string &host, const int port,
                                          const std::string &db,
                                          const std::string &token,
                                          const std::string &rp = std::string()) noexcept(false);

 private:
   /// Private constructor disallows to create instance of Factory
   InfluxDBFactory() = default;
};

} // namespace influxdb

#endif // INFLUXDATA_INFLUXDB_FACTORY_H
